package pet.protocol

import kotlinx.coroutines.test.runTest
import kotlinx.serialization.json.Json
import pet.control.DeviceAuthority
import pet.control.MonotonicClock
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertNotNull

class ContractExamplesTest {
    @Test
    fun repositoryExamplesMatchTheSimulatedFlow() = runTest {
        val command = ProtocolCodec.decodeCommand(example("command-expression.json"))
        val principal = Principal(command.header.clientId, ClientPlatform.ANDROID)
        var now = 0L
        val authority = DeviceAuthority(MonotonicClock { now }, command.header.bootId)

        now = 1000
        val acquisition = command.header.copy(
            requestId = "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa",
            sequence = 1,
        )
        assertNotNull(authority.handle(ControlRequest(acquisition, ControlAction.ACQUIRE), principal).state)
        assertEquals(
            Json.parseToJsonElement(example("state.json")),
            Json.parseToJsonElement(ProtocolCodec.encodeState(authority.snapshot())),
        )

        now = 1100
        assertEquals(
            Json.parseToJsonElement(example("reply-expression.json")),
            Json.parseToJsonElement(ProtocolCodec.encodeReply(authority.handle(command, principal))),
        )
    }

    private fun example(file: String): String =
        checkNotNull(javaClass.getResource("/$file")).readText()
}
