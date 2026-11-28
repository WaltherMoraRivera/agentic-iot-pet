package pet.pairing

/** HTTP request as the device receives it. Not a data class: [authorization] and [body] may hold secrets. */
class WireRequest(
    val method: String,
    val path: String,
    val contentType: String?,
    val authorization: String?,
    val body: String,
)

/** HTTP response. Not a data class: [body] may hold a credential. */
class WireResponse(val status: Int, val body: String)

/** Thrown by a [TlsTransport] when the peer verification callback rejects the server key. */
class TlsHandshakeException : Exception("TLS peer verification failed")

/**
 * TLS client transport. [verifyPeer] receives the SHA-256 of the server SubjectPublicKeyInfo (64 lowercase hex)
 * during the handshake; if it returns false the transport closes the connection before any application byte
 * and throws [TlsHandshakeException]. There is no variant that skips [verifyPeer].
 */
interface TlsTransport {
    suspend fun open(host: String, port: Int, verifyPeer: (fingerprint: String) -> Boolean): TlsConnection
}

interface TlsConnection : AutoCloseable {
    suspend fun send(request: WireRequest): WireResponse
}
