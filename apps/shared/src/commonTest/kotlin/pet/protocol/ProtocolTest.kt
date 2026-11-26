package pet.protocol

import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertFailsWith

class ProtocolTest {
    private val common = """"version":1,"request_id":"11111111-1111-4111-8111-111111111111","client_id":"22222222-2222-4222-8222-222222222222","boot_id":"33333333-3333-4333-8333-333333333333","sequence":2"""
    private fun command(duration: String = "1000") =
        """{$common,"lease_epoch":1,"action":"expression.set","params":{"expression":"happy","duration_ms":$duration}}"""

    private fun assertError(expected: ErrorCode, json: String) {
        assertEquals(expected, assertFailsWith<ProtocolException> { ProtocolCodec.decodeCommand(json) }.code)
    }

    @Test
    fun durationIsAnIntegerInRange() {
        for (duration in listOf(100, 5000)) {
            val decoded = ProtocolCodec.decodeCommand(command(duration.toString()))
            assertEquals(duration, decoded.durationMs)
            assertEquals(Expression.HAPPY, decoded.expression)
        }
        for (duration in listOf("99", "5001", "\"100\"", "true", "100.0", "1e2", "0100", "+100")) {
            assertError(ErrorCode.INVALID_MESSAGE, command(duration))
        }
    }

    @Test
    fun rejectsAmbiguousOrUnsupportedMessages() {
        val json = command()
        val invalid = listOf(
            json.replace("\"version\":1", "\"version\":1,\"version\":1"),
            json.replace("\"version\":1", "\"version\":1,\"\\u0076ersion\":1"),
            json.replace("\"version\":1", "\"version\":2"),
            json.replace("\"version\":1", "\"version\":1,\"extra\":0"),
            json.replace("\"happy\"", "\"happy\",\"expression\":\"happy\""),
            json.replace("\"happy\"", "happy"),
            json.replace("\"expression.set\"", "\"unknown\""),
            json.replace("22222222-2222-4222-8222-222222222222", "AAAAAAAA-AAAA-4AAA-8AAA-AAAAAAAAAAAA"),
            json.replace("\"sequence\":2", "\"sequence\":2147483648"),
        )
        invalid.forEach { assertError(ErrorCode.INVALID_MESSAGE, it) }
        assertEquals(2, ProtocolCodec.decodeCommand(json).header.sequence)
    }

    @Test
    fun requestSizeLimitUsesUtf8Bytes() {
        val json = command()
        assertEquals(Expression.HAPPY, ProtocolCodec.decodeCommand(json.padEnd(1024)).expression)
        assertError(ErrorCode.MESSAGE_TOO_LARGE, json.padEnd(1025))
        assertError(ErrorCode.MESSAGE_TOO_LARGE, json.replace("happy", "é".repeat(520)))
    }

    @Test
    fun controlFieldsDependOnTheRoute() {
        val acquire = ProtocolCodec.decodeControl(ControlAction.ACQUIRE, "{$common}")
        assertEquals(null, acquire.leaseEpoch)
        for (action in listOf(ControlAction.RENEW, ControlAction.RELEASE)) {
            assertEquals(3, ProtocolCodec.decodeControl(action, """{$common,"lease_epoch":3}""").leaseEpoch)
            assertFailsWith<ProtocolException> { ProtocolCodec.decodeControl(action, "{$common}") }
        }
        assertFailsWith<ProtocolException> {
            ProtocolCodec.decodeControl(ControlAction.ACQUIRE, """{$common,"lease_epoch":1}""")
        }
    }

    @Test
    fun wireReplyConfirmsAppliedStateAndKeepsRejectionPrivate() {
        val header = ProtocolCodec.decodeCommand(command()).header
        val state = DeviceState(header.bootId, Expression.HAPPY, header.clientId, 1, 6000, 0)
        val applied = Json.parseToJsonElement(ProtocolCodec.encodeReply(Reply(header.requestId, state, null))).jsonObject
        assertEquals("applied", applied.getValue("status").jsonPrimitive.content)
        assertEquals("happy", applied.getValue("state").jsonObject.getValue("expression").jsonPrimitive.content)
        assertEquals(JsonNull, applied["code"])
        val rejected = Json.parseToJsonElement(ProtocolCodec.encodeReply(Reply(null, null, ErrorCode.UNAUTHENTICATED))).jsonObject
        assertEquals("rejected", rejected.getValue("status").jsonPrimitive.content)
        assertEquals("UNAUTHENTICATED", rejected.getValue("code").jsonPrimitive.content)
        assertEquals(JsonNull, rejected["state"])
        assertEquals(JsonNull, rejected["request_id"])
    }
}
