package pet.security

/**
 * Platform storage for client credentials (contracts/EMPAREJAMIENTO.md, "Credenciales de cliente").
 * Android protects it with Keystore and Windows with the system protected storage (T02 step 4).
 * Values never reach logs, error messages or `toString`.
 */
interface SecretStore {
    fun put(alias: String, value: ByteArray)

    fun get(alias: String): ByteArray?

    fun delete(alias: String)
}
