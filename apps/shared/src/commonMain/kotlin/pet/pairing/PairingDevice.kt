package pet.pairing

import pet.control.MonotonicClock
import kotlin.uuid.ExperimentalUuidApi
import kotlin.uuid.Uuid

/**
 * Simulated device for contracts/EMPAREJAMIENTO.md: USB provisioning frames, offers, `POST /v1/pair`,
 * client credentials, revocation, the physical confirmation slot and the authenticated routes of the v1
 * contract (delegated to [pet.control.DeviceAuthority]). Shared by tests and the simulator; it models the
 * firmware rules, it is not the firmware.
 *
 * [provisioning] is true when the board booted while the sensor was held; no remote order can set it.
 */
@OptIn(ExperimentalUuidApi::class)
class PairingDevice(
    private val clock: MonotonicClock,
    private val identity: DeviceIdentity,
    private val offerSecrets: TokenSource,
    private val credentials: TokenSource,
    private val wifi: WifiJoiner,
    private val provisioning: Boolean,
    private val log: PairingLog,
    private val bootId: String = Uuid.random().toString(),
) {
    /** Handles one USB line (`@pp1 {json}`) and returns the response line. */
    suspend fun usb(frame: String): String = TODO("T02 paso 3")

    /** Handles one HTTPS request after the TLS layer: pairing, offers, clients, revocation, state and control. */
    suspend fun handle(request: WireRequest): WireResponse = TODO("T02 paso 3")

    /** Reports a touch on the sensor. Only a press that started after the reservation counts, at its release. */
    suspend fun touch(pressedAtMs: Long, releasedAtMs: Long): Unit = TODO("T02 paso 3")

    suspend fun snapshot(): PairingSnapshot = TODO("T02 paso 3")
}
