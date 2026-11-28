package pet.pairing

import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.test.TestScope
import kotlinx.coroutines.test.runTest
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonObjectBuilder
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.contentOrNull
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.longOrNull
import kotlinx.serialization.json.put
import pet.control.MonotonicClock
import pet.protocol.ClientPlatform
import pet.protocol.ErrorCode
import pet.security.SecretStore
import kotlin.test.Test
import kotlin.test.assertContentEquals
import kotlin.test.assertEquals
import kotlin.test.assertFalse
import kotlin.test.assertNotEquals
import kotlin.test.assertNotNull
import kotlin.test.assertNull
import kotlin.test.assertTrue
import kotlin.test.fail

/**
 * Shared-environment cases of docs/pruebas/T02-casos.md against contracts/EMPAREJAMIENTO.md.
 * Each test name carries its case ID (T02-xx). Every value is synthetic (`TEST-ONLY-*`).
 * Time is the virtual time of [runTest], used as the device and client monotonic clock (origin 0 ms).
 */
class PairingTest {
    // T02-01: received at 59 999 ms reserves; received at 60 000 ms is OFFER_EXPIRED.
    @Test
    fun expiredOfferIsRejected_T02_01_monotonicBoundary() = runTest {
        Bench(this).run {
            firstOffer()
            delay(59_999)
            val body = pairBody(SECRET_1)
            assertPending(post("/v1/pair", body))
            tap()
            val applied = post("/v1/pair", body)
            assertEquals(200, applied.status)
            assertEquals("applied", applied.text("status"))
            assertEquals(1, device.snapshot().clientIds.size)
        }
        Bench(this).run {
            val offerJson = firstOffer()
            delay(1_000)
            val a = client()
            val offer = a.client.decodeOffer(offerJson)
            delay(59_000)
            val error = failure { a.client.pair(offer) }
            assertEquals(PairingFailure.REJECTED, error.failure)
            assertEquals(ErrorCode.OFFER_EXPIRED, error.code)
            assertEquals(listOf(409), a.transport.statuses, "the device, not the client, decided the expiry")
            val snapshot = device.snapshot()
            assertTrue(snapshot.clientIds.isEmpty())
            assertFalse(snapshot.confirmationPending, "an expired offer must not wait for a touch")
            assertTrue(a.store.puts.isEmpty())
            assertNoSecretsLogged()
        }
    }

    // T02-02: only the issuer monotonic clock decides; the client civil clock never does.
    @Test
    fun expiredOfferIsRejected_T02_02_civilClockIgnored() = runTest {
        Bench(this).run {
            var civilOffsetMs = 0L
            val a = client(civil = CivilClock { CIVIL_EPOCH_MS + now + civilOffsetMs })
            val offer = a.client.decodeOffer(firstOffer())
            delay(30_000)
            civilOffsetMs = HOUR_MS
            val paired = pairWithTouch(a, offer)
            assertEquals(D1, paired.deviceId)
            assertEquals(listOf("pairing/$D1"), a.store.puts)
        }
        Bench(this).run {
            var civilOffsetMs = 0L
            val a = client(civil = CivilClock { CIVIL_EPOCH_MS + now + civilOffsetMs })
            val offerJson = firstOffer()
            delay(59_000)
            val offer = a.client.decodeOffer(offerJson)
            delay(1_000)
            civilOffsetMs = -HOUR_MS
            val error = failure { a.client.pair(offer) }
            assertEquals(ErrorCode.OFFER_EXPIRED, error.code)
            assertTrue(device.snapshot().clientIds.isEmpty())
            assertTrue(a.store.puts.isEmpty())
            assertNoSecretsLogged()
        }
    }

    @Test
    fun offerCannotBeReused_T02_03() = runTest {
        Bench(this).run {
            firstOffer()
            val original = pairBody(SECRET_1, name = "Android A")
            assertPending(post("/v1/pair", original))
            tap()
            val applied = post("/v1/pair", original)
            assertEquals(200, applied.status)
            val credentialA = assertNotNull(applied.pairing()?.get("credential")?.jsonPrimitive?.content)
            val finalAt = now

            assertRejected(post("/v1/pair", pairBody(SECRET_1, name = "Android A")), 409, ErrorCode.OFFER_CONSUMED)
            assertRejected(post("/v1/pair", pairBody(SECRET_1, name = "Windows B")), 409, ErrorCode.OFFER_CONSUMED)

            delay(finalAt + 29_999 - now)
            val retained = post("/v1/pair", original)
            assertEquals(200, retained.status)
            assertEquals(applied.json, retained.json, "an exact repeat returns the original result")
            delay(1)
            assertRejected(post("/v1/pair", original), 409, ErrorCode.OFFER_CONSUMED)

            assertEquals(200, getState(credentialA).status)
            val snapshot = device.snapshot()
            assertEquals(1, snapshot.clientIds.size)
            assertFalse(snapshot.confirmationPending, "no new touch is requested")
            assertNoSecretsLogged()
        }
    }

    @Test
    fun concurrentPairWithSameOfferHasOneWinner_T02_04() = runTest {
        Bench(this).run {
            val offerJson = firstOffer()
            val a = client(name = "Android A")
            val b = client(name = "Android B")
            val offers = listOf(a, b).map { it.client.decodeOffer(offerJson) }
            launch {
                delay(100)
                val pressed = now
                delay(400)
                device.touch(pressed, now)
            }
            val outcomes = listOf(a, b).zip(offers).map { (side, offer) ->
                async { outcome { side.client.pair(offer) } }
            }.awaitAll()

            val winners = outcomes.filterIsInstance<PairedDevice>()
            assertEquals(1, winners.size)
            val loser = outcomes.filterIsInstance<PairingException>().single()
            assertEquals(ErrorCode.OFFER_CONSUMED, loser.code)
            assertEquals(1, a.store.puts.size + b.store.puts.size, "a single put in total")
            assertEquals(listOf(winners.single().clientId), device.snapshot().clientIds)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun offerForAnotherDeviceIsRejected_T02_05() = runTest {
        // The address answers as D2: the client aborts before sending the secret.
        Bench(this).run {
            val other = Bench(this@runTest, D2, FP2)
            val a = client(presented = FP2, target = other.device)
            val error = failure { a.client.pair(a.client.decodeOffer(firstOffer())) }
            assertEquals(PairingFailure.IDENTITY_MISMATCH, error.failure)
            assertEquals(0, a.transport.applicationBytes)
            assertTrue(a.transport.handshakes.isNotEmpty() && a.transport.handshakes.none { it })
            assertTrue(a.store.puts.isEmpty())
            assertEquals(OfferState.ACTIVE, device.snapshot().offerState)
            assertTrue(other.device.snapshot().clientIds.isEmpty())
            assertNoSecretsLogged()
        }
        // D1 receives a request naming D2: DEVICE_MISMATCH, offer untouched.
        Bench(this).run {
            firstOffer()
            assertRejected(post("/v1/pair", pairBody(SECRET_1, deviceId = D2)), 409, ErrorCode.DEVICE_MISMATCH)
            val snapshot = device.snapshot()
            assertTrue(snapshot.clientIds.isEmpty())
            assertEquals(OfferState.ACTIVE, snapshot.offerState)
            assertPending(post("/v1/pair", pairBody(SECRET_1)), "the offer was not consumed")
        }
    }

    @Test
    fun malformedOfferIsRejectedLocally_T02_06() = runTest {
        Bench(this).run {
            val a = client()
            val valid = a.client.decodeOffer(offerJson())
            assertEquals(D1, valid.deviceId)
            assertEquals(FP1, valid.certificateFingerprint)
            assertEquals(now + 60_000, valid.expiresAt)

            val malformed = mapOf(
                "truncated fingerprint" to offerJson(fingerprint = "AA:BB"),
                "non-canonical device_id" to offerJson(deviceId = D1.uppercase()),
                "empty secret" to offerJson(secret = ""),
                "unknown field" to offerJson { put("extra", 1) },
            )
            for ((variant, payload) in malformed) {
                val error = failure { a.client.decodeOffer(payload) }
                assertEquals(PairingFailure.MALFORMED_OFFER, error.failure, variant)
                assertFalse(SECRET_PREFIX in error.toString(), "$variant: the error shows no secret")
            }
            assertTrue(a.transport.handshakes.isEmpty(), "no connection is opened")
            assertTrue(a.store.puts.isEmpty())
            assertNoSecretsLogged()
        }
    }

    @Test
    fun wrongFingerprintNeverSendsCredentials_T02_07() = runTest {
        Bench(this).run {
            val a = client(presented = FP99)
            val error = failure { a.client.pair(a.client.decodeOffer(firstOffer())) }
            assertEquals(PairingFailure.IDENTITY_MISMATCH, error.failure)
            assertEquals(0, a.transport.applicationBytes, "no request, Authorization or secret")
            assertTrue(a.transport.handshakes.isNotEmpty() && a.transport.handshakes.none { it })
            assertTrue(a.store.puts.isEmpty())
            assertEquals(OfferState.ACTIVE, device.snapshot().offerState)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun noTouchDoesNotPair_T02_10() = runTest {
        // No touch at all: 202, then CONFIRMATION_EXPIRED once the 15 000 ms window ends.
        Bench(this).run {
            val a = client()
            val error = failure { a.client.pair(a.client.decodeOffer(firstOffer())) }
            assertEquals(ErrorCode.CONFIRMATION_EXPIRED, error.code)
            assertEquals(202, a.transport.statuses.first())
            assertEquals(409, a.transport.statuses.last())
            assertTrue(now >= 15_000)
            val snapshot = device.snapshot()
            assertTrue(snapshot.clientIds.isEmpty())
            assertEquals(OfferState.CONSUMED, snapshot.offerState)
            assertTrue(a.store.puts.isEmpty())
            assertRejected(post("/v1/pair", pairBody(SECRET_1)), 409, ErrorCode.OFFER_CONSUMED)
            assertNoSecretsLogged()
        }
        // A press released at reserved_at_ms + 15 000 does not complete it.
        Bench(this).run {
            firstOffer()
            val body = pairBody(SECRET_1)
            assertPending(post("/v1/pair", body))
            delay(15_000)
            device.touch(14_000, 15_000)
            assertRejected(post("/v1/pair", body), 409, ErrorCode.CONFIRMATION_EXPIRED)
            assertTrue(device.snapshot().clientIds.isEmpty())
        }
    }

    @Test
    fun firstClientPairsAfterTouch_T02_12() = runTest {
        Bench(this).run {
            val hello = usb("hello")
            assertEquals("applied", hello.text("status"))
            assertEquals(D1, hello.text("device_id"))
            assertEquals(FP1, hello.text("fingerprint"))
            assertEquals(JsonNull, hello["host"])
            assertEquals(HOST, usb("wifi.set") { put("ssid", WIFI_SSID); put("password", WIFI_PASSWORD) }.text("host"))
            val created = usb("offer.create") { put("client_kind", "android") }
            assertEquals("applied", created.text("status"))
            val offerJson = assertNotNull(created["offer"] as? JsonObject).toString()

            val a = client()
            val offer = a.client.decodeOffer(offerJson)
            assertEquals(HOST, offer.host)
            val paired = pairWithTouch(a, offer)
            assertEquals(D1, paired.deviceId)
            assertEquals(FP1, paired.certificateFingerprint)
            assertEquals(ClientPlatform.ANDROID, paired.clientKind)
            assertTrue(UUID.matches(paired.clientId))
            assertEquals(202, a.transport.statuses.first())
            assertEquals(200, a.transport.statuses.last())
            assertEquals(listOf("pairing/$D1"), a.store.puts)

            val state = getState(a.store.credential())
            assertEquals(200, state.status)
            assertEquals(STATE_FIELDS, state.json.keys)
            val snapshot = device.snapshot()
            assertEquals(listOf(paired.clientId), snapshot.clientIds)
            assertEquals(OfferState.CONSUMED, snapshot.offerState)
            assertRejected(post("/v1/pair", pairBody(SECRET_1)), 409, ErrorCode.OFFER_CONSUMED)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun secondClientNeedsAuthorizedOfferAndTouch_T02_13() = runTest {
        Bench(this).run {
            val a = pairFirst()
            val credentialBefore = a.client.store.values.getValue("pairing/$D1").copyOf()
            val offerJson = requestOffer(a, "windows")
            val b = client(ClientPlatform.WINDOWS, "Windows B")
            val pairedB = pairWithTouch(b, b.client.decodeOffer(offerJson))
            assertNotEquals(a.device.clientId, pairedB.clientId)
            assertEquals(ClientPlatform.WINDOWS, pairedB.clientKind)
            assertEquals(listOf("pairing/$D1"), b.store.puts)
            assertEquals(200, getState(a.credential).status)
            assertContentEquals(credentialBefore, a.client.store.values.getValue("pairing/$D1"))
            assertEquals(listOf(a.device.clientId, pairedB.clientId).sorted(), device.snapshot().clientIds.sorted())
            assertNoSecretsLogged()
        }
    }

    @Test
    fun pairingWithoutAuthorizedOfferIsRejected_T02_14() = runTest {
        Bench(this).run {
            val a = pairFirst()
            val forged = pairBody(SECRET_FORGED, name = "Windows B")
            assertRejected(post("/v1/pair", forged), 401, ErrorCode.UNAUTHENTICATED)
            tap()
            assertRejected(post("/v1/pair", forged), 401, ErrorCode.UNAUTHENTICATED)
            val snapshot = device.snapshot()
            assertEquals(listOf(a.device.clientId), snapshot.clientIds)
            assertFalse(snapshot.confirmationPending)
            assertEquals(200, getState(a.credential).status)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun revokedClientCannotReadState_T02_15() = runTest {
        Bench(this).run {
            val a = pairFirst()
            val b = pairSecond(a)
            val revoke = revokeBody(b, a.device.clientId)
            val pending = post("/v1/clients/revoke", revoke, b.credential)
            assertPending(pending)
            tap()
            val applied = post("/v1/clients/revoke", revoke, b.credential)
            assertEquals(200, applied.status)
            assertEquals("applied", applied.text("status"))

            val denied = getState(a.credential)
            assertEquals(401, denied.status)
            assertNoState(denied)
            assertEquals(200, getState(b.credential).status)
            assertEquals(listOf(b.device.clientId), device.snapshot().clientIds)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun revokedClientCannotReuseCachedReplies_T02_16() = runTest {
        Bench(this).run {
            val a = pairFirst()
            val b = pairSecond(a)
            val acquired = post("/v1/control/acquire", controlBody(a), a.credential)
            assertEquals(200, acquired.status)
            val command = commandBody(a, leaseEpoch = 1)
            assertEquals(200, post("/v1/commands", command, a.credential).status)
            delay(500)
            assertEquals(200, post("/v1/clients/revoke", revokeBody(a, a.device.clientId), a.credential).status)

            val repeated = post("/v1/commands", command, a.credential)
            assertRejected(repeated, 401, ErrorCode.UNAUTHENTICATED)
            assertNoState(repeated)
            val reacquire = post("/v1/control/acquire", controlBody(a), a.credential)
            assertRejected(reacquire, 401, ErrorCode.UNAUTHENTICATED)
            assertNoState(reacquire)

            delay(500)
            val state = getState(b.credential).json
            assertEquals(JsonNull, state["owner_client_id"], "the lease ends like a release")
            assertEquals(0L, state["lease_remaining_ms"]?.jsonPrimitive?.longOrNull)
            assertEquals(1L, state["lease_epoch"]?.jsonPrimitive?.longOrNull, "the epoch is kept")
            assertEquals("idle", state["expression"]?.jsonPrimitive?.content, "no second effect")
            assertNoSecretsLogged()
        }
    }

    @Test
    fun foreignClientIsRejected_T02_17() = runTest {
        Bench(this).run {
            val a = pairFirst()
            val state = getState(CREDENTIAL_X)
            assertEquals(401, state.status)
            assertNoState(state)
            val acquire = post("/v1/control/acquire", controlBody(CLIENT_X, sequence = 1), CREDENTIAL_X)
            assertRejected(acquire, 401, ErrorCode.UNAUTHENTICATED)
            assertNoState(acquire)
            val seen = getState(a.credential).json
            assertEquals(JsonNull, seen["owner_client_id"])
            assertEquals(0L, seen["lease_epoch"]?.jsonPrimitive?.longOrNull)
            assertEquals(listOf(a.device.clientId), device.snapshot().clientIds)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun pairingSecretOnlyAuthenticatesPairRoute_T02_18() = runTest {
        Bench(this).run {
            firstOffer()
            val attempts = listOf(
                getState(SECRET_1),
                post("/v1/control/acquire", controlBody(CLIENT_X, sequence = 1), SECRET_1),
                post("/v1/commands", commandBody(CLIENT_X, sequence = 2, leaseEpoch = 1), SECRET_1),
            )
            for (attempt in attempts) {
                assertEquals(401, attempt.status)
                assertNoState(attempt)
            }
            val before = device.snapshot()
            assertTrue(before.clientIds.isEmpty())
            assertEquals(OfferState.ACTIVE, before.offerState)
            assertPending(post("/v1/pair", pairBody(SECRET_1)), "the offer still reserves")
            assertEquals(OfferState.RESERVED, device.snapshot().offerState)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun secretsAbsentFromLogs_T02_20() = runTest {
        Bench(this).run {
            val a = pairFirst()

            val wrongOffer = requestOffer(a, "android")
            val wrong = client(presented = FP99)
            val decodedWrong = wrong.client.decodeOffer(wrongOffer)
            val identityError = failure { wrong.client.pair(decodedWrong) }

            val lateOffer = requestOffer(a, "android")
            val issuedAt = now
            delay(1_000)
            val late = client(name = "Android C")
            val decodedLate = late.client.decodeOffer(lateOffer)
            delay(issuedAt + 60_000 - now)
            val expiredError = failure { late.client.pair(decodedLate) }
            assertEquals(ErrorCode.OFFER_EXPIRED, expiredError.code)

            assertEquals(200, post("/v1/clients/revoke", revokeBody(a, a.device.clientId), a.credential).status)

            val texts = log.lines + listOf(
                decodedWrong.toString(), decodedLate.toString(), a.device.toString(),
                identityError.toString(), expiredError.toString(),
            )
            for (text in texts) assertNoSecret(text)
            assertTrue(log.lines.any { ErrorCode.OFFER_EXPIRED.name in it }, "the expiry is identifiable")
        }
    }

    @Test
    fun secretStoreRoundTripAndDelete_T02_22() = runTest {
        val store = RecordingSecretStore()
        val value = SECRET_1.encodeToByteArray()
        store.put("pairing/$D1", value)
        assertContentEquals(value, store.get("pairing/$D1"))
        store.delete("pairing/$D1")
        assertNull(store.get("pairing/$D1"))
        assertTrue(store.values.isEmpty(), "no value remains after delete")

        // A failed pairing (as in T02-07 and T02-10) never calls put.
        Bench(this).run {
            val offerJson = firstOffer()
            val wrong = client(presented = FP99)
            failure { wrong.client.pair(wrong.client.decodeOffer(offerJson)) }
            assertTrue(wrong.store.puts.isEmpty())
            val untouched = client()
            failure { untouched.client.pair(untouched.client.decodeOffer(offerJson)) }
            assertTrue(untouched.store.puts.isEmpty())
        }
    }

    @Test
    fun revokingAnotherClientRequiresTouch_T02_26() = runTest {
        // B revokes A and nobody touches.
        Bench(this).run {
            val a = pairFirst()
            val b = pairSecond(a)
            val revoke = revokeBody(b, a.device.clientId)
            assertPending(post("/v1/clients/revoke", revoke, b.credential))
            delay(15_000)
            assertRejected(post("/v1/clients/revoke", revoke, b.credential), 409, ErrorCode.CONFIRMATION_EXPIRED)
            assertTrue(a.device.clientId in device.snapshot().clientIds)
            assertEquals(200, getState(a.credential).status)
        }
        // A revokes itself: applied at once, without touch.
        Bench(this).run {
            val a = pairFirst()
            val b = pairSecond(a)
            val applied = post("/v1/clients/revoke", revokeBody(a, a.device.clientId), a.credential)
            assertEquals(200, applied.status)
            assertEquals("applied", applied.text("status"))
            assertEquals(401, getState(a.credential).status)
            val snapshot = device.snapshot()
            assertEquals(listOf(b.device.clientId), snapshot.clientIds)
            assertFalse(snapshot.confirmationPending)
            assertNoSecretsLogged()
        }
    }

    @Test
    fun usbOfferRejectedWhenAlreadyPaired_T02_27() = runTest {
        Bench(this).run {
            val a = pairFirst()
            // Still inside the same provisioning mode (< 300 000 ms since boot).
            assertEquals("applied", usb("hello").text("status"))
            assertEquals(
                "applied",
                usb("wifi.set") { put("ssid", WIFI_SSID); put("password", WIFI_PASSWORD) }.text("status"),
            )
            val rejected = usb("offer.create") { put("client_kind", "windows") }
            assertEquals("rejected", rejected.text("status"))
            assertEquals("ALREADY_PAIRED", rejected.text("code"))
            assertTrue(rejected["offer"] == null || rejected["offer"] == JsonNull, "no offer over USB")
            assertFalse(SECRET_PREFIX in rejected.toString())
            val snapshot = device.snapshot()
            assertEquals(listOf(a.device.clientId), snapshot.clientIds)
            assertEquals(OfferState.CONSUMED, snapshot.offerState, "no new offer was issued")
            assertNoSecretsLogged()
        }
    }
}

// --- Synthetic data (docs/pruebas/T02-casos.md, "Datos sintéticos comunes") ---

private const val D1 = "00000000-0000-4000-8000-0000000000d1"
private const val D2 = "00000000-0000-4000-8000-0000000000d2"
private const val CLIENT_X = "00000000-0000-4000-8000-00000000000f"
private const val BOOT = "33333333-3333-4333-8333-333333333333"
private const val HOST = "192.168.1.50"
private const val WIFI_SSID = "TEST-ONLY-NETWORK"
private const val WIFI_PASSWORD = "TEST-ONLY-WIFI-PASSWORD"
private const val SECRET_PREFIX = "TEST-ONLY-SECRET"
private const val CREDENTIAL_PREFIX = "TEST-ONLY-CREDENTIAL"
private const val CIVIL_EPOCH_MS = 1_795_000_000_000L
private const val HOUR_MS = 3_600_000L

/** `AA:BB:CC:...:nn` of the case list, in wire format (64 lowercase hex). */
private fun fingerprint(suffix: String) = "0".repeat(62) + suffix

/** Pads a synthetic name to the 43 base64url characters the contract requires. */
private fun synthetic(name: String) = name.padEnd(43, 'x')

private val FP1 = fingerprint("01")
private val FP2 = fingerprint("02")
private val FP99 = fingerprint("99")
private val SECRET_1 = synthetic("$SECRET_PREFIX-0001")
private val SECRET_FORGED = synthetic("$SECRET_PREFIX-9999")
private val CREDENTIAL_X = synthetic("$CREDENTIAL_PREFIX-X")
private val UUID = Regex("[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}")
private val STATE_FIELDS =
    setOf("version", "boot_id", "expression", "owner_client_id", "lease_epoch", "lease_remaining_ms", "uptime_ms")
private val FORBIDDEN_IN_TEXT =
    listOf(SECRET_PREFIX, CREDENTIAL_PREFIX, "TEST-ONLY-WIFI", "Authorization", "Bearer")

private fun assertNoSecret(text: String) {
    for (forbidden in FORBIDDEN_IN_TEXT) assertFalse(forbidden in text, "text must not contain $forbidden")
}

private fun offerJson(
    deviceId: String = D1,
    fingerprint: String = FP1,
    secret: String = SECRET_1,
    extra: JsonObjectBuilder.() -> Unit = {},
): String = buildJsonObject {
    put("version", 1)
    put("device_id", deviceId)
    put("fingerprint", fingerprint)
    put("host", HOST)
    put("port", 443)
    put("client_kind", "android")
    put("secret", secret)
    put("expires_in_ms", 60_000)
    extra()
}.toString()

/** Catches only [PairingException]; anything else (e.g. `NotImplementedError`) fails the test as is. */
private suspend fun failure(block: suspend () -> Any?): PairingException {
    try {
        block()
    } catch (e: PairingException) {
        return e
    }
    fail("expected a PairingException")
}

private suspend fun outcome(block: suspend () -> PairedDevice): Any =
    try {
        block()
    } catch (e: PairingException) {
        e
    }

// --- Test doubles ---

private class RecordingSecretStore : SecretStore {
    val values = mutableMapOf<String, ByteArray>()
    val puts = mutableListOf<String>()

    override fun put(alias: String, value: ByteArray) {
        puts += alias
        values[alias] = value.copyOf()
    }

    override fun get(alias: String): ByteArray? = values[alias]?.copyOf()

    override fun delete(alias: String) {
        values.remove(alias)
    }

    fun credential(): String = values.getValue("pairing/$D1").decodeToString()
}

private class CapturedLog : PairingLog {
    val lines = mutableListOf<String>()

    override fun log(message: String) {
        lines += message
    }
}

private class SequenceTokens(vararg values: String) : TokenSource {
    private val remaining = values.iterator()

    override fun next(): String = remaining.next()
}

/** Simulated TLS: reports the presented key fingerprint and records what crosses after the handshake. */
private class SimulatedTlsTransport(
    private val device: PairingDevice,
    private val presentedFingerprint: String,
) : TlsTransport {
    val handshakes = mutableListOf<Boolean>()
    val statuses = mutableListOf<Int>()
    var applicationBytes = 0

    override suspend fun open(host: String, port: Int, verifyPeer: (fingerprint: String) -> Boolean): TlsConnection {
        val accepted = verifyPeer(presentedFingerprint)
        handshakes += accepted
        if (!accepted) throw TlsHandshakeException()
        return object : TlsConnection {
            override suspend fun send(request: WireRequest): WireResponse {
                applicationBytes += listOfNotNull(request.method, request.path, request.contentType, request.authorization)
                    .sumOf { it.encodeToByteArray().size } + request.body.encodeToByteArray().size
                return device.handle(request).also { statuses += it.status }
            }

            override fun close() = Unit
        }
    }
}

private class ClientSide(val client: PairingClient, val store: RecordingSecretStore, val transport: SimulatedTlsTransport)

private class Paired(val device: PairedDevice, val credential: String, val client: ClientSide) {
    var sequence = 0
}

private class Answer(val status: Int, val json: JsonObject) {
    fun text(key: String): String? = (json[key] as? JsonPrimitive)?.contentOrNull

    fun pairing(): JsonObject? = json["pairing"] as? JsonObject
}

private fun JsonObject.text(key: String): String? = (this[key] as? JsonPrimitive)?.contentOrNull

private fun assertPending(answer: Answer, message: String? = null) {
    assertEquals(202, answer.status, message)
    assertEquals("pending_confirmation", answer.text("status"), message)
    assertNotNull(answer.json["confirmation"] as? JsonObject, message)
}

private fun assertRejected(answer: Answer, status: Int, code: ErrorCode) {
    assertEquals(status, answer.status)
    assertEquals(code.name, answer.text("code"))
    assertNull(answer.pairing())
}

private fun assertNoState(answer: Answer) {
    assertTrue(answer.json.keys.intersect(STATE_FIELDS - "version").isEmpty(), "no state fields")
    assertTrue(answer.json["state"] == null || answer.json["state"] == JsonNull, "no state object")
}

/** One device under test plus the helpers that drive it at wire level. */
@OptIn(ExperimentalCoroutinesApi::class)
private class Bench(
    private val scope: TestScope,
    deviceId: String = D1,
    fingerprint: String = FP1,
) {
    val log = CapturedLog()
    val clock = MonotonicClock { scope.testScheduler.currentTime }
    val now: Long get() = scope.testScheduler.currentTime
    val device = PairingDevice(
        clock,
        DeviceIdentity(deviceId, fingerprint, 443),
        SequenceTokens(*(1..5).map { synthetic("$SECRET_PREFIX-000$it") }.toTypedArray()),
        SequenceTokens(*listOf("A", "B", "C", "D").map { synthetic("$CREDENTIAL_PREFIX-$it") }.toTypedArray()),
        WifiJoiner { _, _ -> HOST },
        provisioning = true,
        log,
        BOOT,
    )
    private var requests = 0

    fun nextRequestId(): String = "11111111-1111-4111-8111-" + (++requests).toString().padStart(12, '0')

    fun client(
        kind: ClientPlatform = ClientPlatform.ANDROID,
        name: String = "Android A",
        presented: String = FP1,
        target: PairingDevice = device,
        civil: CivilClock = CivilClock { CIVIL_EPOCH_MS + now },
    ): ClientSide {
        val store = RecordingSecretStore()
        val transport = SimulatedTlsTransport(target, presented)
        return ClientSide(PairingClient(transport, store, clock, civil, kind, name, log), store, transport)
    }

    suspend fun usb(op: String, fields: JsonObjectBuilder.() -> Unit = {}): JsonObject {
        val frame = buildJsonObject {
            put("version", 1)
            put("op", op)
            put("request_id", nextRequestId())
            fields()
        }
        val line = device.usb("${PairingLimits.USB_FRAME_PREFIX}$frame\n")
        assertTrue(line.startsWith(PairingLimits.USB_FRAME_PREFIX))
        return Json.parseToJsonElement(line.removePrefix(PairingLimits.USB_FRAME_PREFIX).trim()).jsonObject
    }

    /** First offer over USB in provisioning mode (hello, wifi.set, offer.create), as JSON. */
    suspend fun firstOffer(kind: String = "android"): String {
        usb("hello")
        usb("wifi.set") {
            put("ssid", WIFI_SSID)
            put("password", WIFI_PASSWORD)
        }
        val created = usb("offer.create") { put("client_kind", kind) }
        return assertNotNull(created["offer"] as? JsonObject, "offer.create applied").toString()
    }

    suspend fun post(path: String, body: String, bearer: String? = null): Answer =
        answer(device.handle(WireRequest("POST", path, "application/json", bearer?.let { "Bearer $it" }, body)))

    suspend fun getState(bearer: String): Answer =
        answer(device.handle(WireRequest("GET", "/v1/state", null, "Bearer $bearer", "")))

    private fun answer(response: WireResponse): Answer =
        Answer(response.status, response.body.takeIf { it.isNotBlank() }?.let { Json.parseToJsonElement(it).jsonObject } ?: JsonObject(emptyMap()))

    fun pairBody(secret: String, deviceId: String = D1, name: String = "Android A"): String = buildJsonObject {
        put("version", 1)
        put("request_id", nextRequestId())
        put("device_id", deviceId)
        put("secret", secret)
        put("client_name", name)
    }.toString()

    private fun header(clientId: String, sequence: Int, extra: JsonObjectBuilder.() -> Unit) = buildJsonObject {
        put("version", 1)
        put("request_id", nextRequestId())
        put("client_id", clientId)
        put("boot_id", BOOT)
        put("sequence", sequence)
        extra()
    }.toString()

    fun controlBody(clientId: String, sequence: Int): String = header(clientId, sequence) {}

    fun controlBody(by: Paired): String = controlBody(by.device.clientId, ++by.sequence)

    fun commandBody(clientId: String, sequence: Int, leaseEpoch: Int): String = header(clientId, sequence) {
        put("lease_epoch", leaseEpoch)
        put("action", "expression.set")
        put("params", buildJsonObject {
            put("expression", "happy")
            put("duration_ms", 1000)
        })
    }

    fun commandBody(by: Paired, leaseEpoch: Int): String = commandBody(by.device.clientId, ++by.sequence, leaseEpoch)

    fun revokeBody(by: Paired, target: String): String =
        header(by.device.clientId, ++by.sequence) { put("target_client_id", target) }

    /** Presses and releases the sensor now (200 ms hold). */
    suspend fun tap() {
        val pressed = now
        delay(200)
        device.touch(pressed, now)
    }

    /** Pairs [side] while a touch arrives 100–500 ms after the first request. */
    suspend fun pairWithTouch(side: ClientSide, offer: PairingOffer): PairedDevice {
        scope.launch {
            delay(100)
            val pressed = now
            delay(400)
            device.touch(pressed, now)
        }
        return side.client.pair(offer)
    }

    suspend fun pairFirst(): Paired {
        val a = client(name = "Android A")
        val paired = pairWithTouch(a, a.client.decodeOffer(firstOffer()))
        return Paired(paired, a.store.credential(), a)
    }

    /** `POST /v1/pair/offers` by an already paired client. Returns the offer JSON. */
    suspend fun requestOffer(by: Paired, kind: String): String {
        val reply = post(
            "/v1/pair/offers",
            header(by.device.clientId, ++by.sequence) { put("client_kind", kind) },
            by.credential,
        )
        assertEquals(200, reply.status)
        assertEquals("applied", reply.text("status"))
        return assertNotNull(reply.json["offer"] as? JsonObject).toString()
    }

    suspend fun pairSecond(a: Paired): Paired {
        val b = client(ClientPlatform.WINDOWS, "Windows B")
        val paired = pairWithTouch(b, b.client.decodeOffer(requestOffer(a, "windows")))
        return Paired(paired, b.store.credential(), b)
    }

    fun assertNoSecretsLogged() {
        for (line in log.lines) assertNoSecret(line)
    }
}
