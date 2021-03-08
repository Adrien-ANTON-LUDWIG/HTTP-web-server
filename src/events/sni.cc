#include "sni.hh"

#include "config/config.hh"
#include "vhost/dispatcher.hh"

int sni_callback(SSL *s, int *, void *)
{
    int type = SSL_get_servername_type(s);
    if (type == -1)
        return SSL_TLSEXT_ERR_NOACK;
    auto name = SSL_get_servername(s, type);
    if (!name)
        return SSL_TLSEXT_ERR_NOACK;
    for (auto &v : http::dispatcher)
    {
        auto &conf = v->conf_get();
        if (conf.server_name == name || conf.ip == name)
        {
            if (conf.ssl_cert.size() == 0 || conf.ssl_key.size() == 0)
                return SSL_TLSEXT_ERR_NOACK;
            SSL_set_SSL_CTX(s, v->ctx_get().get());
            return SSL_TLSEXT_ERR_OK;
        }
    }
    return SSL_TLSEXT_ERR_NOACK;
}