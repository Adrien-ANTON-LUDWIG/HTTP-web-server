#include "sni.hh"

int sni_callback(SSL *s, int *, void *arg)
{
    // Advanced technique copyrighted by Pierre Pelletier Inc - 2021.
    void **args = static_cast<void **>(arg);
    http::ServerConfig *conf = static_cast<http::ServerConfig *>(args[0]);
    SSL_CTX *ctx = static_cast<SSL_CTX *>(args[1]);
    // End of the advanded technique

    int type = SSL_get_servername_type(s);
    if (type == -1)
        return SSL_TLSEXT_ERR_NOACK;
    auto name = SSL_get_servername(s, type);
    if (!name)
        return SSL_TLSEXT_ERR_NOACK;
    for (auto v : conf->vhosts)
    {
        if (v.server_name == name || v.ip == name)
        {
            if (v.ssl_cert.empty() || v.ssl_key.empty())
                return SSL_TLSEXT_ERR_NOACK;
            SSL_set_SSL_CTX(s, ctx);
            return SSL_TLSEXT_ERR_OK;
        }
    }
    return SSL_TLSEXT_ERR_NOACK;
}