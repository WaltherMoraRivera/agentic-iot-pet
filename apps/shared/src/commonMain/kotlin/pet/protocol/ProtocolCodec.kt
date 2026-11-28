package pet.protocol

import kotlinx.serialization.SerializationException
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.put

object ProtocolCodec {
    private const val MAX_REQUEST_BYTES = 1024
    private val headerFields = setOf("version", "request_id", "client_id", "boot_id", "sequence")
    private val commandFields = headerFields + setOf("lease_epoch", "action", "params")
    private val leaseFields = headerFields + "lease_epoch"
    private val expressionFields = setOf("expression", "duration_ms")
    private val integerToken = Regex("0|[1-9][0-9]*")
    private val keyTokens = Regex(""""(?:[^"\\]|\\.)*"|[{}:]""")

    fun decodeCommand(json: String): Command {
        val body = readObject(json, commandFields)
        if (body.string("action") != "expression.set") invalid()

        val params = (body["params"] as? JsonObject)?.takeIf { it.keys == expressionFields } ?: invalid()
        val expression = params.string("expression")
        val command = Command(
            body.header(),
            body.integer("lease_epoch"),
            Expression.entries.firstOrNull { it.wireName == expression } ?: invalid(),
            params.integer("duration_ms"),
        )
        if (!command.isValid()) invalid()
        return command
    }

    fun decodeControl(action: ControlAction, json: String): ControlRequest {
        val acquire = action == ControlAction.ACQUIRE
        val body = readObject(json, if (acquire) headerFields else leaseFields)
        val request = ControlRequest(body.header(), action, if (acquire) null else body.integer("lease_epoch"))
        if (!request.isValid()) invalid()
        return request
    }

    fun encodeState(state: DeviceState): String = stateJson(state).toString()

    fun encodeReply(reply: Reply): String = buildJsonObject {
        put("version", PROTOCOL_VERSION)
        put("request_id", JsonPrimitive(reply.requestId))
        put("status", if (reply.code == null) "applied" else "rejected")
        put("state", reply.state?.let(::stateJson) ?: JsonNull)
        put("code", JsonPrimitive(reply.code?.name))
    }.toString()

    private fun stateJson(state: DeviceState): JsonObject = buildJsonObject {
        put("version", PROTOCOL_VERSION)
        put("boot_id", state.bootId)
        put("expression", state.expression.wireName)
        put("owner_client_id", JsonPrimitive(state.ownerClientId))
        put("lease_epoch", state.leaseEpoch)
        put("lease_remaining_ms", state.leaseRemainingMs)
        put("uptime_ms", state.uptimeMs)
    }

    private fun readObject(json: String, fields: Set<String>): JsonObject {
        if (json.length > MAX_REQUEST_BYTES || json.encodeToByteArray().size > MAX_REQUEST_BYTES) {
            throw ProtocolException(ErrorCode.MESSAGE_TOO_LARGE)
        }
        try {
            val body = (Json.parseToJsonElement(json) as? JsonObject)?.takeIf { it.keys == fields } ?: invalid()
            rejectDuplicateKeys(json)
            return body
        } catch (_: SerializationException) {
            invalid()
        }
    }

    private fun rejectDuplicateKeys(json: String) {
        // JsonObject retains only the last value for a repeated key.
        val scopes = ArrayDeque<MutableSet<String>>()
        var previous = ""
        for (token in keyTokens.findAll(json).map { it.value }) {
            when (token) {
                "{" -> scopes.addLast(mutableSetOf())
                "}" -> scopes.removeLastOrNull() ?: invalid()
                ":" -> {
                    if (!previous.startsWith('"')) invalid()
                    val key = Json.decodeFromString<String>(previous)
                    if (scopes.lastOrNull()?.add(key) != true) invalid()
                }
            }
            previous = token
        }
    }

    private fun JsonObject.header() = RequestHeader(
        requestId = string("request_id"),
        clientId = string("client_id"),
        bootId = string("boot_id"),
        sequence = integer("sequence"),
        version = integer("version"),
    )

    private fun JsonObject.string(key: String): String =
        (this[key] as? JsonPrimitive)?.takeIf { it.isString }?.content ?: invalid()

    private fun JsonObject.integer(key: String): Int =
        (this[key] as? JsonPrimitive)?.takeIf { !it.isString && integerToken.matches(it.content) }
            ?.content?.toIntOrNull() ?: invalid()

    private fun invalid(): Nothing = throw ProtocolException(ErrorCode.INVALID_MESSAGE)
}
