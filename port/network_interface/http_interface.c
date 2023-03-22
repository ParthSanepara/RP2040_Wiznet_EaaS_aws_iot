#include <string.h>

#include "os_common.h"

#include "http_interface.h"
#include "ethernet.h"
#include "device_common.h"

NetworkContext_t g_http_network_context;
TransportInterface_t g_http_transport_interface;
http_config_t g_http_config;

/* SSL context */
tlsContext_t *g_http_tls_context_ptr;

static HTTP_REQUEST_CALLBACK g_http_request_callback = NULL;


void set_http_request_callback (HTTP_REQUEST_CALLBACK http_request_callback)
{
    g_http_request_callback = http_request_callback;
}

void unset_http_request_callback (HTTP_REQUEST_CALLBACK http_request_callback)
{
    g_http_request_callback = NULL;
}

int32_t default_http_send_request_callback(TransportInterface_t *pTransportInterface, char *pMethod, http_config_t *http_config)
{

}

/* HTTP */
int8_t http_connect(uint8_t sock, http_config_t *http_config)
{
    int8_t ret = SOCK_ERROR;

    /* Open TCP socket */
    ret = socket(g_http_network_context.socketDescriptor, Sn_MR_TCP, 0, 0);

    if (ret != sock)
    {
        TRACE_DEBUG(" failed\r\n  ! socket returned %d", ret);

        return -1;
    }

    /* Connect to HTTP server */
    ret = connect(g_http_network_context.socketDescriptor, http_config->http_ip, http_config->http_port);

    if (ret != SOCK_OK)
    {
        TRACE_DEBUG(" failed\r\n  ! connect returned %d", ret);

        return -1;
    }

    return 0;
}

int8_t http_close(uint8_t sock, http_config_t *http_config)
{
    int8_t ret = SOCK_ERROR;

    if (http_config->ssl_flag)
    {
        mbedtls_ssl_close_notify(&g_http_tls_context_ptr->ssl);
#ifdef MBEDTLS_ENTROPY_C
        mbedtls_entropy_free(&g_http_tls_context_ptr->entropy);
#endif // MBEDTLS_ENTROPY_C
        mbedtls_x509_crt_free(&g_http_tls_context_ptr->cacert);
        mbedtls_x509_crt_free(&g_http_tls_context_ptr->clicert);
        mbedtls_pk_free(&g_http_tls_context_ptr->pkey);
        mbedtls_ssl_free(&g_http_tls_context_ptr->ssl);
        mbedtls_ssl_config_free(&g_http_tls_context_ptr->conf);
        mbedtls_ctr_drbg_free(&g_http_tls_context_ptr->ctr_drbg);
        g_http_tls_context_ptr = NULL;
    }
    ret = disconnect(sock);
    http_config->http_state = HTTP_IDLE;
    if (ret != SOCK_OK)
    {
        TRACE_DEBUG(" failed\r\n  ! disconnect returned %d", ret);

        return ret;
    }

    return ret;
}

int32_t http_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = send(pNetworkContext->socketDescriptor, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t http_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;
    uint32_t tickStart = get_time_ms();

    do
    {
        if (getSn_RX_RSR(pNetworkContext->socketDescriptor) > 0)
            size = recv(pNetworkContext->socketDescriptor, pBuffer, bytesToRecv);
        if (size != 0)
        {
            break;
        }
        OS_DELAY_MS(10);
    } while ((get_time_ms() - tickStart) <= HTTP_TIMEOUT);

    return size;
}

int32_t http_get(uint8_t sock, char *http_url, tlsContext_t *tls_context, bool force_http)
{
    bool retStatus = false;
    int32_t ret = -1;
    uint8_t *pPath, *pDomain;
    uint32_t pPathLen, pDomainLen;

    if (g_http_config.http_state != HTTP_IDLE)
        return -1;

    memset(&g_http_config, 0, sizeof(http_config_t));

    g_http_config.http_state = HTTP_RUNNING;

    retStatus = parse_url_info( http_url, strlen(http_url),
                                g_http_config.http_domain, sizeof(g_http_config.http_domain),
                                g_http_config.http_path, sizeof(g_http_config.http_path),
                                &g_http_config.http_port
    );

    if(g_http_config.http_port == 443)
    {
        g_http_config.ssl_flag = 1;
    }
    else if(g_http_config.http_port == 80)
    {
        g_http_config.ssl_flag = 0;
    }
    else
    {
        return -1;
    }

    // force_http 옵션이 1인 경우, HTTPS 주소여도, HTTP로 접속하도록 하는 옵션
    if(force_http == true)
    {
        g_http_config.http_port = 80;
        g_http_config.ssl_flag = 0;
    }

    g_http_config.http_domain_len = strlen(g_http_config.http_domain);
    g_http_config.http_path_len = strlen(g_http_config.http_path);

    if (!is_ipaddr(g_http_config.http_domain, g_http_config.http_ip)) // IP
    {
        uint8_t dns_buf[512];
        ret = get_ipaddr_from_dns(g_http_config.http_domain, g_http_config.http_ip, dns_buf, DNS_TIMEOUT);
        if (ret != 1)
        {
            http_close(sock, &g_http_config);

            return -1;
        }
    }
    g_http_config.http_method = GET;

    g_http_network_context.socketDescriptor = sock;
    g_http_transport_interface.pNetworkContext = &g_http_network_context;

    if (g_http_config.ssl_flag == 0)
    {
        ret = http_connect(g_http_network_context.socketDescriptor, &g_http_config);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = http_write;
        g_http_transport_interface.recv = http_read;
    }
    else if (g_http_config.ssl_flag == 1)
    {
        if (tls_context == NULL)
        {
            http_close(sock, &g_http_config);

            return -1;
        }
        ret = https_connect(g_http_network_context.socketDescriptor, &g_http_config, tls_context);
        if (ret != 0)
        {
            http_close(sock, &g_http_config);

            return HTTPNetworkError;
        }
        g_http_transport_interface.send = https_write;
        g_http_transport_interface.recv = https_read;
    }

    if(g_http_request_callback == NULL)
    {
        ret = default_http_send_request_callback( &g_http_transport_interface,
                                                  HTTP_METHOD_GET,
                                                  &g_http_config);
    }
    else
    {
        ret = g_http_request_callback( &g_http_transport_interface,
                                        HTTP_METHOD_GET,
                                        &g_http_config);
    }

    http_close(sock, &g_http_config);
    if (ret != HTTPSuccess)
    {
        printf(" failed\n  ! http_send_request returned %d\n\n", ret);
    }

    return ret;
}

// int32_t http_post(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context)
// {
//     int32_t ret = -1;
//     uint8_t *pPath, *pDomain;
//     uint32_t pPathLen, pDomainLen;
//     uint32_t port = 0;

//     if (g_http_config.http_state != HTTP_IDLE)
//         return -1;
//     memset(&g_http_config, 0, sizeof(http_config_t));
//     g_http_config.http_state = HTTP_RUNNING;

//     ret = is_https(http_url);
//     if (ret < 0)
//     {
//         http_close(sock, &g_http_config);

//         return -1;
//     }
//     if (ret == 1)
//         g_http_config.http_port = 443;
//     else
//         g_http_config.http_port = 80;
//     g_http_config.ssl_flag = ret;

//     ret = getUrlInfo(http_url, strlen(http_url), &pDomain, (size_t *)&pDomainLen, &pPath, (size_t *)&pPathLen, &port);
//     if (ret)
//     {
//         http_close(sock, &g_http_config);

//         return -1;
//     }

//     memset(g_http_config.http_path, 0, HTTP_DOMAIN_MAX_SIZE);
//     memset(g_http_config.http_domain, 0, HTTP_DOMAIN_MAX_SIZE);

//     memcpy(g_http_config.http_path, pPath, pPathLen);
//     g_http_config.http_path_len = pPathLen;

//     memcpy(g_http_config.http_domain, pDomain, pDomainLen);
//     g_http_config.http_domain_len = pDomainLen;
//     if (port)
//         g_http_config.http_port = port;

//     if (!is_ipaddr(g_http_config.http_domain, g_http_config.http_ip)) // IP
//     {
//         uint8_t dns_buf[512];
//         ret = get_ipaddr_from_dns(g_http_config.http_domain, g_http_config.http_ip, dns_buf, HTTP_TIMEOUT);
//         if (ret != 1)
//         {
//             http_close(sock, &g_http_config);

//             return -1;
//         }
//     }
//     g_http_config.http_method = POST;

//     g_http_network_context.socketDescriptor = sock;
//     g_http_transport_interface.pNetworkContext = &g_http_network_context;

//     if (g_http_config.ssl_flag == 0)
//     {
//         ret = http_connect(g_http_network_context.socketDescriptor, &g_http_config);
//         if (ret != 0)
//         {
//             http_close(sock, &g_http_config);

//             return HTTPNetworkError;
//         }
//         g_http_transport_interface.send = http_write;
//         g_http_transport_interface.recv = http_read;
//     }
//     else if (g_http_config.ssl_flag == 1)
//     {
//         ret = https_connect(g_http_network_context.socketDescriptor, &g_http_config, tls_context);
//         if (ret != 0)
//         {
//             http_close(sock, &g_http_config);

//             return HTTPNetworkError;
//         }
//         g_http_transport_interface.send = https_write;
//         g_http_transport_interface.recv = https_read;
//     }

//     ret = http_send_request(&g_http_transport_interface,
//                             buffer,
//                             HTTP_METHOD_POST,
//                             &g_http_config);

//     http_close(sock, &g_http_config);
//     if (ret != HTTPSuccess)
//     {
//         printf(" failed\n  ! http_send_request returned %d\n\n", ret);

//         return ret;
//     }

//     return ret;
// }

/* HTTPS */
int8_t https_connect(uint8_t sock, http_config_t *http_config, tlsContext_t *tls_context)
{
    int8_t ret = -1;

    /* Initialize SSL context */
    ret = tls_socket_init(tls_context, (int *)(intptr_t)sock, http_config->http_domain);
    if (ret != 0)
    {
        printf(" failed\n  ! wiz_tls_init returned %d\n\n", ret);

        return ret;
    }

    /* Connect to HTTPS server */
    ret = tls_socket_connect_timeout(tls_context, http_config->http_ip, http_config->http_port, 0, HTTP_TIMEOUT);
    if (ret != 0)
    {
        printf(" failed\n  ! connect returned %d\n\n", ret);

        return ret;
    }
    g_http_tls_context_ptr = tls_context;

    return ret;
}

int32_t https_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = tls_socket_write(g_http_tls_context_ptr, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t https_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;
    uint32_t tickStart = get_time_ms();

    do
    {
        if (getSn_RX_RSR(pNetworkContext->socketDescriptor) > 0)
            size = tls_socket_read(g_http_tls_context_ptr, pBuffer, bytesToRecv);
        if (size != 0)
            break;
    } while ((get_time_ms() - tickStart) <= HTTP_TIMEOUT);

    return size;
}

bool parse_url_info(    uint8_t *pUrl, uint16_t urlLen,
                        uint8_t *pAddress, uint16_t maxAddressLen,
                        uint8_t *pPath, uint16_t maxPathLen,
                        uint32_t *port )
{
    uint8_t *pStr;
    uint8_t tempUrl[HTTP_URL_MAX_SIZE]={0,};

    uint8_t *pTempAddr, *pTempPath;
    uint16_t tempAddrLen = 0, tempPathLen = 0; 

    if(urlLen > sizeof(tempUrl))
    {
        TRACE_ERROR("Invalid Url. Url length is too long");
        return false;
    }
    // strtok 시, 원본 데이터에 NULL 문자를 삽입함. 원본 데이터를 보존하기 위해 tempUrl에 복사해서 사용
    strncpy(tempUrl, pUrl, urlLen);

    pStr = strtok(tempUrl,"://");
    if( strncmp(pStr, "https", strlen("https")) == 0 )
    {
        *port = 443;
    }
    else if( strncmp(pStr, "http", strlen("http")) == 0 )
    {
        *port = 80;
    }
    else
    {
        goto ERROR;
    }


    pTempAddr = strtok(NULL,"/");
    tempAddrLen = strlen(pTempAddr);
    if(tempAddrLen > maxAddressLen)
    {
        goto ERROR;
    }
    strncpy(pAddress, pTempAddr, tempAddrLen);
    pAddress[tempAddrLen] = '\0';
    //TRACE_DEBUG("%s\r\n",pTempAddr);

    pTempPath = strtok(NULL,"");
    tempPathLen = strlen(pTempPath);
    if(tempPathLen > maxPathLen)
    {
        goto ERROR;
    }

    sprintf(pPath, "/%s", pTempPath, tempPathLen);
    tempPathLen += 1;                               // '/' 추가되었기 때문에 Path Length를 1 증가
    pPath[tempPathLen] = '\0';                    
    //TRACE_DEBUG("%s\r\n",pTempPath);

    return true;

ERROR:
    TRACE_ERROR("Invalid Url");
    TRACE_ERROR("%s", pUrl);
    return false;
}