package pet.protocol

internal const val PROTOCOL_VERSION = 1
internal val canonicalUuid = Regex("[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}")

enum class ClientPlatform { ANDROID, WINDOWS }

data class Principal(val clientId: String, val platform: ClientPlatform)

data class RequestHeader(
    val requestId: String,
    val clientId: String,
    val bootId: String,
    val sequence: Int,
    val version: Int = PROTOCOL_VERSION,
)

sealed interface Request {
    val header: RequestHeader
    val leaseEpoch: Int?
}

enum class ControlAction { ACQUIRE, RENEW, RELEASE }

data class ControlRequest(
    override val header: RequestHeader,
    val action: ControlAction,
    override val leaseEpoch: Int? = null,
) : Request

enum class Expression(val wireName: String) {
    IDLE("idle"),
    HAPPY("happy"),
    THINKING("thinking"),
    ALERT("alert"),
}

data class Command(
    override val header: RequestHeader,
    override val leaseEpoch: Int,
    val expression: Expression,
    val durationMs: Int,
) : Request

data class DeviceState(
    val bootId: String,
    val expression: Expression,
    val ownerClientId: String?,
    val leaseEpoch: Int,
    val leaseRemainingMs: Long,
    val uptimeMs: Long,
)

enum class ErrorCode(val httpStatus: Int) {
    INVALID_MESSAGE(400),
    UNAUTHENTICATED(401),
    CLIENT_MISMATCH(403),
    NOT_OWNER(403),
    STALE_BOOT(409),
    STALE_EPOCH(409),
    STALE_SEQUENCE(409),
    BUSY(409),
    ID_CONFLICT(409),
    LEASE_EXPIRED(409),
    MESSAGE_TOO_LARGE(413),
    UNSUPPORTED_MEDIA_TYPE(415),
}

data class Reply(val requestId: String?, val state: DeviceState?, val code: ErrorCode?) {
    init {
        require((state == null) != (code == null))
    }
}

class ProtocolException(val code: ErrorCode) : IllegalArgumentException(code.name)

internal fun Request.isValid(): Boolean {
    if (header.version != PROTOCOL_VERSION || header.sequence <= 0 ||
        !canonicalUuid.matches(header.requestId) || !canonicalUuid.matches(header.clientId) ||
        !canonicalUuid.matches(header.bootId)
    ) return false

    return when (this) {
        is Command -> leaseEpoch > 0 && durationMs in 100..5000
        is ControlRequest -> when (action) {
            ControlAction.ACQUIRE -> leaseEpoch == null
            ControlAction.RENEW, ControlAction.RELEASE -> leaseEpoch != null && leaseEpoch > 0
        }
    }
}
