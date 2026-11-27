package pet.control

import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.async
import kotlinx.coroutines.test.runTest
import pet.protocol.ClientPlatform
import pet.protocol.Command
import pet.protocol.ControlAction
import pet.protocol.ControlRequest
import pet.protocol.ErrorCode
import pet.protocol.Expression
import pet.protocol.Principal
import pet.protocol.RequestHeader
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertNotEquals
import kotlin.test.assertNotNull
import kotlin.test.assertNull

class DeviceAuthorityTest {
    private var now = 0L
    private val a = Principal("aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa", ClientPlatform.ANDROID)
    private val b = Principal("bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb", ClientPlatform.WINDOWS)
    private val boot = "33333333-3333-4333-8333-333333333333"
    private val authority = DeviceAuthority(MonotonicClock { now }, boot)

    private fun header(sequence: Int, client: Principal = a, request: Int = sequence): RequestHeader {
        val requestId = "11111111-1111-4111-8111-" + request.toString().padStart(12, '0')
        return RequestHeader(requestId, client.clientId, boot, sequence)
    }

    private fun acquire(sequence: Int, client: Principal = a, request: Int = sequence) =
        ControlRequest(header(sequence, client, request), ControlAction.ACQUIRE)

    private fun command(sequence: Int = 2, epoch: Int = 1) =
        Command(header(sequence), epoch, Expression.HAPPY, 1000)

    @Test
    fun concurrentAcquireHasOneOwner() = runTest {
        val start = CompletableDeferred<Unit>()
        val attempts = listOf(a, b).map { client ->
            async(Dispatchers.Default) {
                start.await()
                authority.handle(acquire(1, client), client)
            }
        }
        start.complete(Unit)
        val replies = attempts.map { it.await() }
        val winner = replies.single { it.code == null }
        assertEquals(ErrorCode.BUSY, replies.single { it != winner }.code)
        assertEquals(assertNotNull(winner.state).ownerClientId, authority.snapshot().ownerClientId)
        assertEquals(1, authority.snapshot().leaseEpoch)
    }

    @Test
    fun acquireAgainDoesNotExtendLease() = runTest {
        authority.handle(acquire(1), a)
        now = 2000
        val state = assertNotNull(authority.handle(acquire(2), a).state)
        assertEquals(a.clientId, state.ownerClientId)
        assertEquals(1, state.leaseEpoch)
        assertEquals(4000L, state.leaseRemainingMs)
    }

    @Test
    fun leaseExpiresAtExactBoundary() = runTest {
        authority.handle(acquire(1), a)
        now = 5999
        assertEquals(1L, authority.snapshot().leaseRemainingMs)
        now = 6000
        assertEquals(ErrorCode.LEASE_EXPIRED, authority.handle(command(), a).code)
        assertNull(authority.snapshot().ownerClientId)
        assertEquals(Expression.IDLE, authority.snapshot().expression)
    }

    @Test
    fun renewRetryDoesNotExtendLeaseTwice() = runTest {
        authority.handle(acquire(1), a)
        now = 2000
        val renewal = ControlRequest(header(2), ControlAction.RENEW, 1)
        val applied = authority.handle(renewal, a)
        assertEquals(6000L, applied.state?.leaseRemainingMs)
        now = 3000
        assertEquals(applied, authority.handle(renewal, a))
        assertEquals(5000L, authority.snapshot().leaseRemainingMs)
        now = 8000
        assertEquals(ErrorCode.LEASE_EXPIRED, authority.handle(command(3), a).code)
    }

    @Test
    fun wallClockDoesNotChangeLeaseOrSession() = runTest {
        var time = 1_700_000_000_000L
        val elapsed = DeviceAuthority(MonotonicClock { time }, boot)
        elapsed.handle(acquire(1), a)
        time += 2000
        assertEquals(2000L, elapsed.snapshot().uptimeMs)
        assertEquals(4000L, elapsed.snapshot().leaseRemainingMs)
        time += 4000
        assertEquals(ErrorCode.LEASE_EXPIRED, elapsed.handle(command(), a).code)
    }

    @Test
    fun expressionRetryDoesNotApplyTwice() = runTest {
        authority.handle(acquire(1), a)
        val request = command()
        val applied = authority.handle(request, a)
        assertEquals(Expression.HAPPY, applied.state?.expression)
        now = 500
        assertEquals(applied, authority.handle(request, a))
        now = 999
        assertEquals(Expression.HAPPY, authority.snapshot().expression)
        now = 1000
        assertEquals(Expression.IDLE, authority.snapshot().expression)
        now = 6000
        assertEquals(ErrorCode.LEASE_EXPIRED, authority.handle(request, a).code)
        assertEquals(2, authority.handle(acquire(1, b), b).state?.leaseEpoch)
        assertEquals(ErrorCode.STALE_EPOCH, authority.handle(request, a).code)
        assertEquals(Expression.IDLE, authority.snapshot().expression)
    }

    @Test
    fun requestWithChangedContentIsRejected() = runTest {
        authority.handle(acquire(1), a)
        val request = command()
        authority.handle(request, a)
        now = 500
        val changed = request.copy(expression = Expression.ALERT, durationMs = 5000)
        val conflict = authority.handle(changed, a)
        assertEquals(ErrorCode.ID_CONFLICT, conflict.code)
        assertNull(conflict.state)
        now = 999
        assertEquals(Expression.HAPPY, authority.snapshot().expression)
        now = 1000
        assertEquals(Expression.IDLE, authority.snapshot().expression)
    }

    @Test
    fun releaseRetryOnlyAppliesUntilNextAcquire() = runTest {
        authority.handle(acquire(1), a)
        val release = ControlRequest(header(2), ControlAction.RELEASE, 1)
        val applied = authority.handle(release, a)
        assertNull(assertNotNull(applied.state).ownerClientId)
        assertEquals(applied, authority.handle(release, a))
        authority.handle(acquire(1, b), b)
        assertEquals(ErrorCode.STALE_EPOCH, authority.handle(release, a).code)
        assertEquals(b.clientId, authority.snapshot().ownerClientId)
    }

    @Test
    fun evictedRetryIsStale() = runTest {
        authority.handle(acquire(1), a)
        val order = command()
        authority.handle(order, a)
        for (sequence in 3..66) assertNotNull(authority.handle(acquire(sequence), a).state)
        now = 1000
        assertEquals(Expression.IDLE, authority.snapshot().expression)
        assertEquals(ErrorCode.STALE_SEQUENCE, authority.handle(order, a).code)
        assertEquals(Expression.IDLE, authority.snapshot().expression)
    }

    @Test
    fun busyRequestConsumesSequence() = runTest {
        authority.handle(acquire(1), a)
        assertEquals(ErrorCode.BUSY, authority.handle(acquire(1, b), b).code)
        authority.handle(ControlRequest(header(2), ControlAction.RELEASE, 1), a)
        assertEquals(ErrorCode.STALE_SEQUENCE, authority.handle(acquire(1, b, request = 3), b).code)
        assertEquals(b.clientId, authority.handle(acquire(2, b), b).state?.ownerClientId)
    }

    @Test
    fun rejectsMismatchedIdentityAndStaleContext() = runTest {
        authority.handle(acquire(1), a)
        val order = command()
        val mismatch = authority.handle(order, b)
        assertEquals(ErrorCode.CLIENT_MISMATCH, mismatch.code)
        assertNull(mismatch.state)
        val rebooted = DeviceAuthority(MonotonicClock { now })
        assertNotEquals(boot, rebooted.snapshot().bootId)
        val staleBoot = rebooted.handle(order, a)
        assertEquals(ErrorCode.STALE_BOOT, staleBoot.code)
        assertNull(staleBoot.state)
        assertEquals(Expression.IDLE, rebooted.snapshot().expression)
        now = 6000
        authority.handle(acquire(3), a)
        val stale = authority.handle(command(4), a)
        assertEquals(ErrorCode.STALE_EPOCH, stale.code)
        assertNull(stale.state)
        assertEquals(Expression.IDLE, authority.snapshot().expression)
    }

    @Test
    fun authorizationAndValidityPrecedeDeduplication() = runTest {
        authority.handle(acquire(1), a)
        val order = command()
        assertEquals(ErrorCode.INVALID_MESSAGE, authority.handle(order.copy(durationMs = 99), a).code)
        assertNotNull(authority.handle(order, a).state)
        val unauthorized = authority.handle(order, null)
        assertEquals(ErrorCode.UNAUTHENTICATED, unauthorized.code)
        assertNull(unauthorized.state)
        assertEquals(ErrorCode.CLIENT_MISMATCH, authority.handle(order, b).code)
    }

    @Test
    fun authenticationPrecedesMessageValidity() = runTest {
        authority.handle(acquire(1), a)
        val reply = authority.handle(command().copy(durationMs = 99), null)
        assertEquals(ErrorCode.UNAUTHENTICATED, reply.code)
        assertNull(reply.state)
    }

    @Test
    fun otherClientCannotUseCurrentEpoch() = runTest {
        authority.handle(acquire(1), a)
        now = 1000
        val attempts = listOf(
            ControlRequest(header(1, b, request = 101), ControlAction.RENEW, 1),
            Command(header(2, b, request = 102), 1, Expression.ALERT, 1000),
            ControlRequest(header(3, b, request = 103), ControlAction.RELEASE, 1),
        )
        for (attempt in attempts) {
            val reply = authority.handle(attempt, b)
            assertEquals(ErrorCode.NOT_OWNER, reply.code)
            assertNull(reply.state)
        }
        val state = authority.snapshot()
        assertEquals(a.clientId, state.ownerClientId)
        assertEquals(1, state.leaseEpoch)
        assertEquals(5000L, state.leaseRemainingMs)
        assertEquals(Expression.IDLE, state.expression)
    }

    @Test
    fun ownerCannotRenewOrReleaseExpiredLease() = runTest {
        authority.handle(acquire(1), a)
        now = 6000
        for ((sequence, action) in listOf(2 to ControlAction.RENEW, 3 to ControlAction.RELEASE)) {
            val reply = authority.handle(ControlRequest(header(sequence), action, 1), a)
            assertEquals(ErrorCode.LEASE_EXPIRED, reply.code)
            assertNull(reply.state)
        }
        val state = authority.snapshot()
        assertNull(state.ownerClientId)
        assertEquals(1, state.leaseEpoch)
        assertEquals(0L, state.leaseRemainingMs)
        assertEquals(2, authority.handle(acquire(1, b, request = 101), b).state?.leaseEpoch)
    }
}
