#include "tls_socket_interface.h"
#include "os_common.h"
#include "device_common.h"

int tls_random_callback(void *p_rng, unsigned char *output, size_t output_len)
{
    int i;

    if (output_len <= 0)
    {
        return 1;
    }

    for (i = 0; i < output_len; i++)
    {
        *output++ = rand() % 0xff;
    }

    srand(rand());

    return 0;
}

int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    uint32_t start_ms = get_time_ms();

    do
    {
        if (getSn_RX_RSR((uint8_t)ctx) > 0)
        {
            return recv((uint8_t)ctx, (uint8_t *)buf, (uint16_t)len);
        }
    } while ((get_time_ms() - start_ms) < timeout);

    return 0;
}

#if defined(MBEDTLS_DEBUG_C)
void tls_debug_callback(void *ctx, int level, const char *file, int line, const char *str)
{
    TRACE_DEBUG("%s", str);
}
#endif

int tls_socket_init(tlsContext_t *tlsContext, int *socket_fd, const char *host)
{
    int ret = 1;

#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_init(&tlsContext->entropy);
#endif

    mbedtls_ctr_drbg_init(&tlsContext->ctr_drbg);
    mbedtls_ssl_init(&tlsContext->ssl);
    mbedtls_ssl_config_init(&tlsContext->conf);
    mbedtls_x509_crt_init(&tlsContext->cacert);
    mbedtls_x509_crt_init(&tlsContext->clicert);
    mbedtls_pk_init(&tlsContext->pkey);

#if defined(MBEDTLS_ENTROPY_C)
    if ((ret = mbedtls_ctr_drbg_seed(&tlsContext->ctr_drbg, mbedtls_entropy_func, &tlsContext->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0)
    {
        TRACE_ERROR("Failed. mbedtls_ctr_drbg_seed returned -0x%x", -ret);
        return -1;
    }
#endif

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_ssl_conf_dbg(&tlsContext->conf, tls_debug_callback, stdout);
#endif

    if (tlsContext->rootca_option != MBEDTLS_SSL_VERIFY_NONE)
    {
        ret = mbedtls_x509_crt_parse(&tlsContext->cacert, (const unsigned char *)tlsContext->root_ca, strlen(tlsContext->root_ca) + 1);

        if (ret < 0)
        {
            TRACE_ERROR("Failed. mbedtls_x509_crt_parse returned -0x%x while parsing root cert", -ret);
            return -1;
        }
        TRACE_DEBUG("OK. mbedtls_x509_crt_parse returned -0x%x while parsing root cert", -ret);

        if ((ret = mbedtls_ssl_set_hostname(&tlsContext->ssl, host)) != 0)
        {
            TRACE_ERROR("Failed. mbedtls_ssl_set_hostname returned %d", ret);
            return -1;
        }
         TRACE_DEBUG("OK. mbedtls_ssl_set_hostname returned %d", ret);
    }

    if (tlsContext->clica_option == true)
    {
        ret = mbedtls_x509_crt_parse((&tlsContext->clicert), (const unsigned char *)tlsContext->client_cert, strlen(tlsContext->client_cert) + 1);
        if (ret != 0)
        {
            TRACE_ERROR("Failed. mbedtls_x509_crt_parse returned -0x%x while parsing device cert", -ret);
            return -1;
        }
         TRACE_DEBUG("OK. mbedtls_x509_crt_parse returned -0x%x while parsing device cert", -ret);

        ret = mbedtls_pk_parse_key(&tlsContext->pkey, (const unsigned char *)tlsContext->private_key, strlen(tlsContext->private_key) + 1, NULL, 0);
        if (ret != 0)
        {
            TRACE_ERROR("Failed. mbedtls_pk_parse_key returned -0x%x while parsing private key", -ret);
            return -1;
        }
        TRACE_DEBUG("OK. mbedtls_pk_parse_key returned -0x%x while parsing private key", -ret);
    }

    if ((ret = mbedtls_ssl_config_defaults(&tlsContext->conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        TRACE_ERROR("Failed. mbedtls_ssl_config_defaults returned %d", ret);
        return -1;
    }

    TRACE_DEBUG("Root CA verify option %d\r\n", tlsContext->rootca_option);

    mbedtls_ssl_conf_authmode(&tlsContext->conf, tlsContext->rootca_option);
    mbedtls_ssl_conf_ca_chain(&tlsContext->conf, &tlsContext->cacert, NULL);
    mbedtls_ssl_conf_rng(&tlsContext->conf, tls_random_callback, &tlsContext->ctr_drbg);

    if (tlsContext->clica_option == true)
    {
        if ((ret = mbedtls_ssl_conf_own_cert(&tlsContext->conf, &tlsContext->clicert, &tlsContext->pkey)) != 0)
        {
            TRACE_ERROR("Failed. mbedtls_ssl_conf_own_cert returned %d", ret);
            return -1;
        }
        TRACE_DEBUG("OK. mbedtls_ssl_conf_own_cert returned %d", ret);
    }

    mbedtls_ssl_conf_endpoint(&tlsContext->conf, MBEDTLS_SSL_IS_CLIENT);
    mbedtls_ssl_conf_read_timeout(&tlsContext->conf, SSL_RECV_TIMEOUT);

    if ((ret = mbedtls_ssl_setup(&tlsContext->ssl, &tlsContext->conf)) != 0)
    {
        TRACE_ERROR("Failed. mbedtls_ssl_setup returned -0x%x", -ret);

        return -1;
    }
    mbedtls_ssl_set_bio(&tlsContext->ssl, socket_fd, send, recv, recv_timeout);

    return 0;
}

void tls_socket_deinit(tlsContext_t *tlsContext)
{
    /*  free SSL context memory  */
    mbedtls_ssl_free(&tlsContext->ssl);
    mbedtls_ssl_config_free(&tlsContext->conf);
    mbedtls_ctr_drbg_free(&tlsContext->ctr_drbg);
#if defined(MBEDTLS_ENTROPY_C)
    mbedtls_entropy_free(&tlsContext->entropy);
#endif
    mbedtls_x509_crt_free(&tlsContext->cacert);
    mbedtls_x509_crt_free(&tlsContext->clicert);
    mbedtls_pk_free(&tlsContext->pkey);
}

int tls_socket_connect_timeout(tlsContext_t *tlsContext, char *addr, unsigned int port, unsigned int local_port, uint32_t timeout)
{
    int ret;
    uint32_t start_ms = get_time_ms();

    uint8_t sock = (uint8_t)(tlsContext->ssl.p_bio);

    /*socket open*/
    ret = socket(sock, Sn_MR_TCP, local_port, 0x00);
    if (ret != sock)
        return ret;

    /*Connect to the target*/
    ret = connect(sock, addr, port);
    if (ret != SOCK_OK)
        return ret;

    TRACE_DEBUG("Performing the SSL/TLS handshake...");

    while ((ret = mbedtls_ssl_handshake(&tlsContext->ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            TRACE_ERROR("Failed. mbedtls_ssl_handshake returned -0x%x", -ret);
            return (-1);
        }

        if ((get_time_ms() - start_ms) > timeout) // timeout
        {
            TRACE_ERROR("Timeout. mbedtls_ssl_handshake returned -0x%x", ret);
            return (-1);
        }

        OS_DELAY_MS(10);
    }

    TRACE_DEBUG("[ Ciphersuite is %s ]", mbedtls_ssl_get_ciphersuite(&tlsContext->ssl));

    return (0);
}

int tls_socket_read(tlsContext_t *tlsContext, unsigned char *readbuf, unsigned int len)
{
    return mbedtls_ssl_read(&tlsContext->ssl, readbuf, len);
}

int tls_socket_write(tlsContext_t *tlsContext, unsigned char *writebuf, unsigned int len)
{
    return mbedtls_ssl_write(&tlsContext->ssl, writebuf, len);
}

int tls_transport_disconnect(tlsContext_t *tlsContext, uint32_t timeout)
{
    int ret = 0;
    uint8_t sock = (uint8_t)(tlsContext->ssl.p_bio);
    uint32_t tickStart = get_time_ms();

    do
    {
        ret = disconnect(sock);
        if ((ret == SOCK_OK) || (ret == SOCKERR_TIMEOUT))
            break;
    } while ((get_time_ms() - tickStart) < timeout);

    if (ret == SOCK_OK)
        ret = sock; // socket number

    return ret;
}