#ifndef _COMMON_UTILS_TLS_SOCKET_INTERFACE_H_
#define _COMMON_UTILS_TLS_SOCKET_INTERFACE_H_

#include <stdio.h>
#include <stdlib.h>

#include "socket.h"

#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/entropy.h"

#define SSL_RECV_TIMEOUT (1000 * 5)     // 5seconds

// typedef struct{
//     int32_t fdSocket;
//     mbedtls_entropy_context entropy;
//     mbedtls_ctr_drbg_context ctr_drbg;
// 	mbedtls_ssl_context ssl;
// 	mbedtls_ssl_config conf;
// 	mbedtls_x509_crt claimCert;
// }AWS_TLS_CONTEXT;

typedef struct
{
#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_context entropy;
#endif
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
    uint8_t *root_ca;
    uint8_t *client_cert;
    uint8_t *private_key;
    uint8_t rootca_option;
    uint8_t clica_option;
} tlsContext_t;


int tls_random_callback (void *p_rng, unsigned char *output, size_t output_len);
int recv_timeout        (void *ctx, unsigned char *buf, size_t len, uint32_t timeout);

int     tls_socket_init     (tlsContext_t *tlsContext, int *socket_fd, const char *host);
void    tls_socket_deinit   (tlsContext_t *tlsContext);
int     tls_socket_connect_timeout(tlsContext_t *tlsContext, char *addr, unsigned int port, unsigned int local_port, uint32_t timeout);
int     tls_socket_read     (tlsContext_t *tlsContext, unsigned char *readbuf, unsigned int len);
int     tls_socket_write    (tlsContext_t *tlsContext, unsigned char *writebuf, unsigned int len);
int     tls_transport_disconnect(tlsContext_t *tlsContext, uint32_t timeout);


#endif