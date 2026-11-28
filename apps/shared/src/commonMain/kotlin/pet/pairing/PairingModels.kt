package pet.pairing

import pet.protocol.ClientPlatform
import pet.protocol.ErrorCode

/**
 * Offer read from the QR code or the USB channel, already validated by [PairingClient.decodeOffer].
 *
 * [expiresAt] is a local deadline on the reader's monotonic clock (`scan_monotonic_ms + expires_in_ms`).
 * It only drives the countdown shown to the user; the device alone decides expiry.
 * Not a data class: [oneTimeSecret] must never appear in `toString`.
 */
class PairingOffer(
    val deviceId: String,
    val certificateFingerprint: String,
    val oneTimeSecret: String,
    val expiresAt: Long,
    val host: String,
    val port: Int,
    val clientKind: ClientPlatform,
)

/** Result of a completed pairing. The credential itself lives only in the [pet.security.SecretStore]. */
class PairedDevice(
    val deviceId: String,
    val certificateFingerprint: String,
    val clientId: String,
    val clientKind: ClientPlatform,
    val host: String,
    val port: Int,
)

enum class PairingFailure {
    /** The offer failed local validation; no connection was opened. */
    MALFORMED_OFFER,

    /** The peer key fingerprint did not match during the TLS handshake; no application byte was sent. */
    IDENTITY_MISMATCH,

    /** The device answered with a stable rejection code. */
    REJECTED,

    /** The device answered with a status or body the client does not recognise. */
    UNEXPECTED_RESPONSE,
}

/** Pairing error. Its message carries only the failure and the device code, never secrets. */
class PairingException(val failure: PairingFailure, val code: ErrorCode? = null) :
    Exception(code?.let { "${failure.name}: ${it.name}" } ?: failure.name)

enum class OfferState { ACTIVE, RESERVED, CONSUMED }

/** Observable device-side pairing state, used by tests to check the absence of effects. */
data class PairingSnapshot(
    val clientIds: List<String>,
    val offerState: OfferState?,
    val confirmationPending: Boolean,
)

/** Identity generated on the device. [fingerprint] is the SHA-256 of the server SPKI, 64 lowercase hex chars. */
data class DeviceIdentity(val deviceId: String, val fingerprint: String, val port: Int)

/** Source of 43-character base64url tokens (32 random bytes). Tests inject synthetic `TEST-ONLY-*` values. */
fun interface TokenSource {
    fun next(): String
}

/** Joins the network given over USB. Returns the obtained host, or null when joining failed (`WIFI_FAILED`). */
fun interface WifiJoiner {
    fun join(ssid: String, password: String): String?
}

/** Civil clock of the client, in epoch milliseconds. Only for display; never decides acceptance. */
fun interface CivilClock {
    fun nowEpochMs(): Long
}

/** Log sink. Callers must never pass secrets, credentials, `Authorization` headers or pairing bodies. */
fun interface PairingLog {
    fun log(message: String)
}
