package pet.control

import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import pet.protocol.Command
import pet.protocol.ControlAction
import pet.protocol.ControlRequest
import pet.protocol.DeviceState
import pet.protocol.ErrorCode
import pet.protocol.Expression
import pet.protocol.Principal
import pet.protocol.Reply
import pet.protocol.Request
import pet.protocol.canonicalUuid
import pet.protocol.isValid
import kotlin.uuid.ExperimentalUuidApi
import kotlin.uuid.Uuid

fun interface MonotonicClock {
    fun nowMs(): Long
}

@OptIn(ExperimentalUuidApi::class)
class DeviceAuthority(private val clock: MonotonicClock, private val bootId: String = Uuid.random().toString()) {
    private data class Lease(val ownerClientId: String, val expiresAtMs: Long)

    private data class StoredReply(val request: Request, val epoch: Int, val reply: Reply)

    private class ClientRequests {
        var highestSequence = 0
        val replies = linkedMapOf<String, StoredReply>()
    }

    private val mutex = Mutex()
    private val startedAt = clock.nowMs()
    private val clients = mutableMapOf<String, ClientRequests>()
    private var lease: Lease? = null
    private var leaseEpoch = 0
    private var expression = Expression.IDLE
    private var expressionExpiresAtMs = 0L

    init {
        require(canonicalUuid.matches(bootId))
    }

    suspend fun handle(request: Request, principal: Principal?): Reply = mutex.withLock {
        val header = request.header
        if (principal == null) return@withLock reject(request, ErrorCode.UNAUTHENTICATED)
        if (!request.isValid()) return@withLock reject(request, ErrorCode.INVALID_MESSAGE)
        if (principal.clientId != header.clientId) return@withLock reject(request, ErrorCode.CLIENT_MISMATCH)
        if (header.bootId != bootId) return@withLock reject(request, ErrorCode.STALE_BOOT)

        val now = clock.nowMs() - startedAt
        val history = clients.getOrPut(header.clientId) { ClientRequests() }
        val stored = history.replies[header.requestId]
        if (stored != null) return@withLock retry(request, stored, now)
        if (header.sequence <= history.highestSequence) return@withLock reject(request, ErrorCode.STALE_SEQUENCE)

        val reply = apply(request, now)
        history.highestSequence = header.sequence
        history.replies[header.requestId] = StoredReply(request, leaseEpoch, reply)
        if (history.replies.size > MAX_REPLIES) history.replies.remove(history.replies.keys.first())
        reply
    }

    suspend fun snapshot(): DeviceState = mutex.withLock {
        stateAt(clock.nowMs() - startedAt)
    }

    private fun retry(request: Request, stored: StoredReply, now: Long): Reply {
        if (request != stored.request) return reject(request, ErrorCode.ID_CONFLICT)
        if (stored.epoch != leaseEpoch) return reject(request, ErrorCode.STALE_EPOCH)
        if (request is ControlRequest && request.action == ControlAction.RELEASE && stored.reply.code == null) {
            return stored.reply
        }
        val current = lease ?: return reject(request, ErrorCode.NOT_OWNER)
        if (now >= current.expiresAtMs) return reject(request, ErrorCode.LEASE_EXPIRED)
        return stored.reply
    }

    private fun apply(request: Request, now: Long): Reply {
        val clientId = request.header.clientId
        val current = lease
        if (request is ControlRequest && request.action == ControlAction.ACQUIRE) {
            if (current == null || now >= current.expiresAtMs) {
                leaseEpoch++
                lease = Lease(clientId, now + LEASE_DURATION_MS)
            } else if (current.ownerClientId != clientId) {
                return reject(request, ErrorCode.BUSY)
            }
        } else {
            if (request.leaseEpoch != leaseEpoch) return reject(request, ErrorCode.STALE_EPOCH)
            if (current?.ownerClientId != clientId) return reject(request, ErrorCode.NOT_OWNER)
            if (now >= current.expiresAtMs) return reject(request, ErrorCode.LEASE_EXPIRED)

            when (request) {
                is Command -> {
                    expression = request.expression
                    expressionExpiresAtMs = now + request.durationMs
                }
                is ControlRequest -> {
                    lease = if (request.action == ControlAction.RENEW) {
                        current.copy(expiresAtMs = now + LEASE_DURATION_MS)
                    } else {
                        null
                    }
                }
            }
        }
        return Reply(request.header.requestId, stateAt(now), null)
    }

    private fun stateAt(now: Long): DeviceState {
        val current = lease?.takeIf { now < it.expiresAtMs }
        return DeviceState(
            bootId,
            if (now < expressionExpiresAtMs) expression else Expression.IDLE,
            current?.ownerClientId,
            leaseEpoch,
            current?.let { it.expiresAtMs - now } ?: 0L,
            now,
        )
    }

    private fun reject(request: Request, code: ErrorCode): Reply =
        Reply(request.header.requestId.takeIf(canonicalUuid::matches), null, code)

    private companion object {
        const val LEASE_DURATION_MS = 6000L
        const val MAX_REPLIES = 64
    }
}
