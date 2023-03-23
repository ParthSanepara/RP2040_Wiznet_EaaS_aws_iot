#include "iot_jobs_job_ota_handler.h"
#include "os_common.h"
#include "http_interface.h"
#include "ethernet.h"
#include "crc16.h"
#include "flash_control.h"
#include "core_http_client.h"
#include "core_http_client_private.h"

#define AWS_S3_DOWNLOAD_BINARY_REQ  "GET %s HTTP/1.1\r\n"                           \
                                    "Host: %s\r\n"                                  \
                                    "Accept-Encoding: gzip, deflate, br\r\n"        \
                                    "Connection: keep-alive\r\n\r\n"



//static uint8_t      g_http_ota_buf[HTTP_BUF_MAX_SIZE] = {0,};
static tlsContext_t g_http_ota_tls_context;


int32_t ota_http_send_request_callback(TransportInterface_t *pTransportInterface, char *pMethod, http_config_t *http_config)
{
    /* Return value of this method. */
    int32_t returnStatus = 0;

    uint32_t currentReceived = 0;
    uint32_t received2nd = 0;
    uint32_t currentTotalLen = 0;

    bool        isFirstRecv = true;
    uint32_t    ota_flash_index = 0;
    
    uint8_t tempHttpBuffer[HTTP_BUF_MAX_SIZE]={0,};
    
    
    uint8_t tempOtaBinaryBuffer[HTTP_BUF_MAX_SIZE]={0,};

    /* Configurations of the initial request headers that are passed to
     * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t requestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t response;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t requestHeaders;

    /* Return value of all methods from the HTTP Client library API. */
    HTTPStatus_t httpStatus = HTTPSuccess;

    /* Initialize all HTTP Client library API structs to 0. */
    (void)memset(&requestInfo, 0, sizeof(requestInfo));
    (void)memset(&response, 0, sizeof(response));
    (void)memset(&requestHeaders, 0, sizeof(requestHeaders));

    /* Initialize the request object. */
    requestInfo.pHost = http_config->http_domain;
    requestInfo.hostLen = http_config->http_domain_len;
    requestInfo.pMethod = pMethod;
    requestInfo.methodLen = strlen(pMethod);
    requestInfo.pPath = http_config->http_path;
    requestInfo.pathLen = http_config->http_path_len;

    /* Set the buffer used for storing request headers. */
    requestHeaders.pBuffer = tempHttpBuffer;
    requestHeaders.bufferLen = HTTP_BUF_MAX_SIZE;

    /* Initialize the response object. The same buffer used for storing
     * request headers is reused here. */

    response.pBuffer = tempHttpBuffer;
    response.bufferLen = HTTP_BUF_MAX_SIZE;
    response.getTime = (HTTPClient_GetCurrentTimeFunc_t)get_time_ms;

    httpStatus = HTTPClient_InitializeRequestHeaders(&requestHeaders, &requestInfo);
    if (httpStatus != HTTPSuccess)
        TRACE_ERROR("Failed to initialize HTTP request headers: Error=%s.", HTTPClient_strerror(httpStatus));

    httpStatus = HTTPClient_AddHeader(&requestHeaders,
                                      HTTP_CONTENT_TYPE_FIELD,
                                      HTTP_CONTENT_TYPE_FIELD_LEN,
                                      HTTP_CONTENT_TYPE_VALUE,
                                      HTTP_CONTENT_TYPE_VALUE_LEN);
    if (httpStatus != HTTPSuccess)
        TRACE_ERROR("Failed to initialize HTTP request headers: Error=%s.", HTTPClient_strerror(httpStatus));

    if ((HTTP_REQUEST_KEEP_ALIVE_FLAG & requestInfo.reqFlags) == 0U)
    {
        httpStatus = HTTPClient_AddHeader(&requestHeaders,
                                          HTTP_CONNECTION_FIELD,
                                          HTTP_CONNECTION_FIELD_LEN,
                                          HTTP_CONNECTION_CLOSE_VALUE,
                                          HTTP_CONNECTION_CLOSE_VALUE_LEN);
        if (httpStatus != HTTPSuccess)
            TRACE_ERROR("Failed to initialize HTTP request headers: Error=%s.", HTTPClient_strerror(httpStatus));
    }

    if (httpStatus == HTTPSuccess)
    {
        TRACE_DEBUG("Sending HTTP %.*s request to %.*s%.*s",
                   (int32_t)requestInfo.methodLen, requestInfo.pMethod,
                   (int32_t)requestInfo.hostLen, requestInfo.pHost,
                   (int32_t)requestInfo.pathLen, requestInfo.pPath);

        // TRACE_DEBUG("Request Headers:\n%.*s\n"
        //             "Request Body:\n%.*s\n",
        //             (int32_t)requestHeaders.headersLen, (char *)requestHeaders.pBuffer,
        //             (int32_t)http_config->bodyLen, http_config->bodyDataPtr);

        /* Send the request and receive the response. */
        httpStatus = HTTPClient_Send(pTransportInterface,
                                     &requestHeaders,
                                     (uint8_t *)http_config->bodyDataPtr,
                                     http_config->bodyLen,
                                     &response,
                                     0);
    }
    else
    {
        TRACE_ERROR("Failed to initialize HTTP request headers: Error=%s.", HTTPClient_strerror(httpStatus));
    }

    TRACE_DEBUG("Response Headers Length: %d", response.headersLen);
    if (httpStatus == HTTPInsufficientMemory)
    {
        ota_flash_index = 0;
        currentTotalLen = 0;

        TRACE_DEBUG("Response buffer has insufficient");
        // TRACE_DEBUG("Response Content Length: %d", response.contentLength);
        // TRACE_DEBUG("Response Body Length: %d", response.bodyLen);
        //TRACE_DEBUG("Response Body:\n%.*s", response.bodyLen, response.pBody);

        TRACE_DEBUG("Temp Flash Erase for OTA");
        erase_flash_total_ota_area();
        OS_DELAY_MS(1000);

        while (1)
        {
            OS_DELAY_MS(100);
            memset(tempOtaBinaryBuffer, 0, HTTP_BUF_MAX_SIZE);
            
            if( isFirstRecv == true )
            {
                memcpy(tempOtaBinaryBuffer, response.pBody, response.bodyLen);
                
                currentReceived = pTransportInterface->recv(pTransportInterface->pNetworkContext, 
                                                            &tempOtaBinaryBuffer[response.bodyLen], 
                                                            (HTTP_BUF_MAX_SIZE - response.bodyLen));
                isFirstRecv = false;
                currentReceived += response.bodyLen;
            }
            else
            {
                
                currentReceived = pTransportInterface->recv(pTransportInterface->pNetworkContext, 
                                                            tempOtaBinaryBuffer, 
                                                            HTTP_BUF_MAX_SIZE);
                //TRACE_DEBUG("1st Recv. %d",currentReceived);                                                            

                if(currentReceived < HTTP_BUF_MAX_SIZE)
                {
                    received2nd = pTransportInterface->recv(pTransportInterface->pNetworkContext, 
                                                            &tempOtaBinaryBuffer[currentReceived], 
                                                            (HTTP_BUF_MAX_SIZE - currentReceived));
                    currentReceived += received2nd;
                    
                    //TRACE_DEBUG("2nd Recv. %d/%d",received2nd, currentReceived);
                }

            }
            
            TRACE_DEBUG("Current Received: %d", currentReceived);
            // TRACE_DEBUG("Current Received Total Length: %d/%d", currentTotalLen + currentReceived, response.contentLength);

            if (currentReceived > 0)
            {
                //TRACE_DEBUG("\n%.*s", response.bodyLen, response.pBody);
                TRACE_DEBUG("Write Data to Ota area. Addr : %lX, data size : %d", ota_flash_index, currentReceived);
                write_flash_ota_binary(ota_flash_index, tempOtaBinaryBuffer, currentReceived);

                currentTotalLen += currentReceived;
                ota_flash_index = currentTotalLen;

                if (currentTotalLen >= response.contentLength)
                {
                    httpStatus = HTTPSuccess;
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    else if( httpStatus == HTTPSuccess )
    {
        TRACE_DEBUG("Response Header:\r\n%.*s", response.headersLen, response.pHeaders);
        TRACE_DEBUG("Response Body:\r\n%.*s", response.bodyLen, response.pBody);
    }

    if (httpStatus != HTTPSuccess)
    {
        returnStatus = -1;
    }

    // 수신할 바이너리가 없는 경우
    if (currentReceived == 0)
    {
        returnStatus = -2;
    }

    return returnStatus;
}

bool ota_download_firmware(uint8_t *pUrl, uint32_t fwSize, uint16_t fwCrc)
{
    uint8_t *pOtaBinFlashStartAddress;
    uint16_t calcedCrc=0;
    int retHttpResult = 0;
    TRACE_DEBUG("URL : %s", pUrl);

    memset(&g_http_ota_tls_context, 0x00, sizeof(g_http_ota_tls_context));


    set_http_request_callback(ota_http_send_request_callback);

    g_http_ota_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used client certificate
    g_http_ota_tls_context.clica_option = 0;                        // not used Root CA verify

    retHttpResult = http_get(SOCKET_HTTP, pUrl, &g_http_ota_tls_context, true);
    unset_http_request_callback(ota_http_send_request_callback);

    if( retHttpResult == -1 )
    {
        TRACE_ERROR("Fail to http get.");
        return false;
    }
    else if( retHttpResult == -2 )
    {
        TRACE_WARN("There is no binary data");
        return false;
    }

    TRACE_INFO("Success to download firmware.")

    TRACE_DEBUG("Verify firmware");
    TRACE_DEBUG("FW Size : %d", fwSize);
    
    pOtaBinFlashStartAddress = get_flash_ota_binary_start_address();
    calcedCrc = crc16_ccitt(pOtaBinFlashStartAddress, fwSize);

    TRACE_DEBUG("Calced CRC : %02X, Wanted CRC : %02X", calcedCrc, fwCrc);
    if(calcedCrc != fwCrc)
    {
        TRACE_DEBUG("Invalid CRC");
        return false;
    }
    
    return true;;
}







bool ota_procedure(uint8_t *pDownloadUrl, uint32_t fwSize, uint16_t fwCrc)
{
    return true;
}