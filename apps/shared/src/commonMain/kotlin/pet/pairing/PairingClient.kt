package pet.pairing

import pet.control.MonotonicClock
import pet.protocol.ClientPlatform
import pet.security.SecretStore

/**
 * App side of contracts/EMPAREJAMIENTO.md: validates the offer locally, pins the key fingerprint during the
 * handshake, sends `POST /v1/pair`, repeats the exact request every 1 000 ms while it receives 202 and stores
 * the credential with `SecretStore.put("pairing/<device_id>", ...)` only after a 200.
 */
class PairingClient(
    private val transport: TlsTransport,
    private val store: SecretStore,
    private val clock: MonotonicClock,
    private val civilClock: CivilClock,
    private val clientKind: ClientPlatform,
    private val clientName: String,
    private val log: PairingLog,
) {
    /** Strictly parses the QR/USB offer before any connection. Throws [PairingException] with `MALFORMED_OFFER`. */
    fun decodeOffer(payload: String): PairingOffer = TODO("T02 paso 3")

    /** Pairs with the device named by [offer]. Throws [PairingException] on any outcome other than 200 `applied`. */
    suspend fun pair(offer: PairingOffer): PairedDevice = TODO("T02 paso 3")
}
