#ifndef _HTTP_INTERFACE_H_
#define _HTTP_INTERFACE_H_

#include <stdbool.h>
#include "tls_socket_interface.h"
#include "transport_interface.h"
#include "core_http_config.h"
#include "core_http_client.h"
//#include "core_http_client_private.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
typedef enum
{
    HTTP_IDLE = 0,
    HTTP_RUNNING,
} http_state_t;

typedef enum
{
    GET = 0,
    POST,
} http_method_t;

typedef struct __http_config
{
    http_state_t http_state;
    http_method_t http_method;
    
    char http_domain[HTTP_DOMAIN_MAX_SIZE];
    uint32_t http_domain_len;
    
    char http_path[HTTP_PATH_MAX_SIZE];
    uint32_t http_path_len;
    
    char http_ip[4];
    uint32_t http_port;
    
    uint8_t ssl_flag;
    
    char *bodyDataPtr;
    uint32_t bodyLen;
} http_config_t;


typedef int32_t (*HTTP_REQUEST_CALLBACK)(TransportInterface_t *pTransportInterface, char *pMethod, http_config_t *http_config);



// /**
//  * ----------------------------------------------------------------------------------------------------
//  * Functions
//  * ----------------------------------------------------------------------------------------------------
//  */
// /* Common */
void    set_http_request_callback     (HTTP_REQUEST_CALLBACK http_request_callback);
void    unset_http_request_callback   (HTTP_REQUEST_CALLBACK http_request_callback);



int32_t default_http_send_request_callback(TransportInterface_t *pTransportInterface, char *pMethod, http_config_t *http_config);

// /* HTTP */
int8_t http_connect (uint8_t sock, http_config_t *http_config);
int8_t http_close   (uint8_t sock, http_config_t *http_config);
int32_t http_write  (NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t http_read   (NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);
int32_t http_get    (uint8_t sock, char *http_url, tlsContext_t *tls_context, bool force_http);
// int32_t http_post(uint8_t sock, uint8_t *buffer, char *http_url, tlsContext_t *tls_context);

// /* HTTPS */
int8_t https_connect(uint8_t sock, http_config_t *http_config, tlsContext_t *tls_context);
int32_t https_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t https_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);

// /* Util */
// HTTPStatus_t getUrlPath(const char *pUrl, size_t urlLen, const char **pPath, size_t *pPathLen);
// HTTPStatus_t getUrlAddress(const char *pUrl, size_t urlLen, const char **pAddress, size_t *pAddressLen, uint32_t *port);
// HTTPStatus_t getUrlInfo(const char *pUrl, size_t urlLen, const char **pAddress, size_t *pAddressLen, const char **pPath, size_t *pPathLen, uint32_t *port);
// int is_https(const char *pUrl);

bool parse_url_info(    uint8_t *pUrl, uint16_t urlLen,
                        uint8_t *pAddress, uint16_t maxAddressLen,
                        uint8_t *pPath, uint16_t maxPathLen,
                        uint32_t *port );


#endif