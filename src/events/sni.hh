#pragma once
#include "config/config.hh"
#include "misc/openssl/ssl.hh"

int sni_callback(SSL *s, int *, void *arg);