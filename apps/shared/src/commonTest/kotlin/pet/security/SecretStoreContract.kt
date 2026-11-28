package pet.security

import kotlin.test.assertContentEquals
import kotlin.test.assertNull

/**
 * Behaviour every [SecretStore] must show (T02-22): `get` returns the same bytes after `put`, and `null` after
 * `delete`. Shared by the in-memory double and the platform stores of T02 step 4.
 */
fun assertSecretStoreContract(store: SecretStore, alias: String, value: ByteArray) {
    store.put(alias, value)
    assertContentEquals(value, store.get(alias))
    store.delete(alias)
    assertNull(store.get(alias))
}
