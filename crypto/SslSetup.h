/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */

#ifndef __ROO_CRYPTO_SSL_SETUP_H__
#define __ROO_CRYPTO_SSL_SETUP_H__

#include <openssl/ssl.h>

namespace roo {

bool Ssl_thread_setup();
void Ssl_thread_clean();


extern SSL_CTX* global_ssl_ctx;


} // roo

#endif //__ROO_CRYPTO_SSL_SETUP_H__
