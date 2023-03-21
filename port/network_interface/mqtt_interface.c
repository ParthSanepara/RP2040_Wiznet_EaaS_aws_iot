#include "mqtt_interface.h"
#include "device_common.h"
#include "transport_interface.h"

#define OUTGOING_PUBLISH_RECORD_LEN              ( 10U )
#define INCOMING_PUBLISH_RECORD_LEN              ( 10U )

static MQTTPubAckInfo_t pOutgoingPublishRecords[ OUTGOING_PUBLISH_RECORD_LEN ];
static MQTTPubAckInfo_t pIncomingPublishRecords[ INCOMING_PUBLISH_RECORD_LEN ];

static MQTT_EVENT_CALLBACK g_mqtt_event_callback = NULL;
static 

NetworkContext_t g_network_context;
TransportInterface_t g_transport_interface;
mqtt_config_t g_mqtt_config;

/* SSL context pointer */
tlsContext_t *g_mqtt_tls_context_ptr = NULL;

static uint8_t g_mqtt_buf[MQTT_BUF_MAX_SIZE] = { 0, };


void dummy_mqtt_event_callback(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo)
{
    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if ((pPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        /* Handle incoming publish. */
        if (pDeserializedInfo->pPublishInfo->payloadLength)
        {
            TRACE_DEBUG("%.*s,%d,%.*s\n", pDeserializedInfo->pPublishInfo->topicNameLength, pDeserializedInfo->pPublishInfo->pTopicName,
                   pDeserializedInfo->pPublishInfo->payloadLength,
                   pDeserializedInfo->pPublishInfo->payloadLength, pDeserializedInfo->pPublishInfo->pPayload);
        }
    }
    else
    {
        /* Handle other packets. */
        switch (pPacketInfo->type)
        {
            case MQTT_PACKET_TYPE_SUBACK:
            {
                TRACE_DEBUG("Received SUBACK: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PINGRESP:
            {
                /* Nothing to be done from application as library handles
                                        * PINGRESP. */
                TRACE_DEBUG("Received PINGRESP\n");

                break;
            }

            case MQTT_PACKET_TYPE_UNSUBACK:
            {
                TRACE_DEBUG("Received UNSUBACK: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBACK:
            {
                TRACE_DEBUG("Received PUBACK: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBREC:
            {
                TRACE_DEBUG("Received PUBREC: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBREL:
            {
                /* Nothing to be done from application as library handles
                                        * PUBREL. */
                TRACE_DEBUG("Received PUBREL: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            case MQTT_PACKET_TYPE_PUBCOMP:
            {
                /* Nothing to be done from application as library handles
                                            * PUBCOMP. */
                TRACE_DEBUG("Received PUBCOMP: PacketID=%u", pDeserializedInfo->packetIdentifier);

                break;
            }

            /* Any other packet type is invalid. */
            default:
            {
                TRACE_DEBUG("Unknown packet type received:(%02x).", pPacketInfo->type);
            }
        }
    }
}

void set_mqtt_state(mqtt_state_t state)
{
    g_mqtt_config.mqtt_state = state;
}

mqtt_state_t get_mqtt_state(void)
{
    return g_mqtt_config.mqtt_state;
}

void set_mqtt_event_callback(MQTT_EVENT_CALLBACK event_callback)
{
    g_mqtt_event_callback = event_callback;
}

void unset_mqtt_event_callback(MQTT_EVENT_CALLBACK event_callback)
{
    g_mqtt_event_callback = NULL;
}

int32_t mqtt_transport_yield(void)
{
    int ret;

    ret = MQTT_ProcessLoop(&g_mqtt_config.mqtt_context);
    if (ret != 0)
    {
        TRACE_WARN("MQTT process loop error : %d", ret);
    }

    return ret;
}

int32_t mqtt_transport_init(uint8_t cleanSession, uint8_t *ClientId, uint8_t *userName, uint8_t *password, uint32_t keepAlive)
{
    if (g_mqtt_config.mqtt_state != MQTT_IDLE)
    {
        TRACE_ERROR("Invalid mqtt_status. status : %d", g_mqtt_config.mqtt_state);
        return -1;
    }
        
    if (ClientId == NULL)
    {
        TRACE_ERROR("Invalid Client Id");
        return -1;
    }
        
    memset((void *)&g_mqtt_config, 0x00, sizeof(mqtt_config_t));

    /* Set MQTT connection information */
    g_mqtt_config.mqtt_connect_info.cleanSession = cleanSession;
    // Client ID must be unique to broker. This field is required.

    g_mqtt_config.mqtt_connect_info.pClientIdentifier = ClientId;
    g_mqtt_config.mqtt_connect_info.clientIdentifierLength = strlen(g_mqtt_config.mqtt_connect_info.pClientIdentifier);

    // The following fields are optional.
    // Value for keep alive.
    g_mqtt_config.mqtt_connect_info.keepAliveSeconds = keepAlive;
    // Optional username and password.

    g_mqtt_config.mqtt_connect_info.pUserName = userName;
    if (userName == NULL)
        g_mqtt_config.mqtt_connect_info.userNameLength = 0;
    else
        g_mqtt_config.mqtt_connect_info.userNameLength = strlen(userName);

    g_mqtt_config.mqtt_connect_info.pPassword = password;
    if (password == NULL)
        g_mqtt_config.mqtt_connect_info.passwordLength = 0;
    else
        g_mqtt_config.mqtt_connect_info.passwordLength = strlen(password);

    return 0;
}

int32_t mqtt_transport_subscribe(uint8_t qos, char *subscribe_topic)
{
    int packet_id = 0;
    packet_id = MQTT_GetPacketId(&g_mqtt_config.mqtt_context);
    uint32_t ret;

    if (g_mqtt_config.subscribe_count > MQTT_SUBSCRIPTION_MAX_NUM)
    {
        TRACE_DEBUG("MQTT subscription count error : %d", g_mqtt_config.subscribe_count);

        return -1;
    }

    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].qos = qos;
    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].pTopicFilter = subscribe_topic;
    g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count].topicFilterLength = strlen(subscribe_topic);

    /* Receive message */
    ret = MQTT_Subscribe(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_subscribe_info[g_mqtt_config.subscribe_count], 1, packet_id);

    if (ret != 0)
    {
        TRACE_DEBUG("MQTT subscription is error : %d. Topic : %s", ret, subscribe_topic);

        return -1;
    }
    else
    {
        TRACE_DEBUG("MQTT subscription is success. Topic : %s", subscribe_topic);
    }
    g_mqtt_config.subscribe_count++;

    return 0;
}

int32_t mqtt_transport_unsubscribe(char *subscribe_topic)
{
    int packet_id = 0, i;
    packet_id = MQTT_GetPacketId(&g_mqtt_config.mqtt_context);
    uint32_t ret;
    uint8_t *pTopic;

    if (g_mqtt_config.subscribe_count == 0)
    {
        TRACE_DEBUG("There is no subscribe topic");
        return 0;
    }

    for(i=0; i<g_mqtt_config.subscribe_count; i++)
    {
        pTopic = (uint8_t*)g_mqtt_config.mqtt_subscribe_info[i].pTopicFilter;
        if( strncmp(subscribe_topic, pTopic, strlen(pTopic)) == 0)
        {
            ret = MQTT_Unsubscribe(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_subscribe_info[i], 1, packet_id);
            if(ret !=0)
            {
                TRACE_DEBUG("MQTT unsubscription is error : %d. Topic : %s", ret, subscribe_topic);
                return -1;
            }
            g_mqtt_config.subscribe_count--;
        }
    }

    return 0;
}

int32_t mqtt_transport_connect(uint8_t sock, uint8_t ssl_flag, uint8_t *domain, uint32_t port, tlsContext_t *tls_context)
{
    bool session_present;
    int ret = -1;
    int packet_id = 0;
    MQTT_EVENT_CALLBACK event_callback = dummy_mqtt_event_callback;

    if (g_mqtt_config.mqtt_state != MQTT_IDLE)
        return -1;

    if (!is_ipaddr(domain, g_mqtt_config.mqtt_ip)) // IP
    {
        uint8_t dns_buf[512];
        ret = get_ipaddr_from_dns(domain, g_mqtt_config.mqtt_ip, dns_buf, DNS_TIMEOUT);
        if (ret != 1)
        {
            TRACE_ERROR("DNS Query Error : %d", ret);
            mqtt_transport_close(sock, ssl_flag);

            return -1;
        }
    }

    if (ssl_flag == 0)
    {
        ret = socket(sock, Sn_MR_TCP, 0, 0x00); // port 0 : any port
        if (ret != sock)
        {
            TRACE_ERROR("Socket Open Error : %d", ret);
            mqtt_transport_close(sock, ssl_flag);
            return -1;
        }
        ret = connect(sock, g_mqtt_config.mqtt_ip, port);
        if (ret != SOCK_OK)
        {
            TRACE_ERROR("Socket Connect Error : %d", ret);
            mqtt_transport_close(sock, ssl_flag);

            return -1;
        }
        g_transport_interface.send = mqtt_write;
        g_transport_interface.recv = mqtt_read;
    }
    else
    {
        /* Initialize SSL context */
        TRACE_DEBUG("SSL initialization");        
        ret = tls_socket_init(tls_context, (int *)(intptr_t)sock, domain);
        if (ret != 0)
        {
            TRACE_ERROR("SSL initialization error : %d", ret);
            mqtt_transport_close(sock, ssl_flag);

            return ret;
        }
        else
        {
            TRACE_DEBUG("SSL initialization is success");
        }

        TRACE_DEBUG("SSL connection");
        ret = tls_socket_connect_timeout(tls_context, g_mqtt_config.mqtt_ip, port, 0, MQTT_CONNECT_TIMEOUT_MS);

        if (ret != 0)
        {
            TRACE_ERROR("SSL connection is error : %d", ret);
            mqtt_transport_close(sock, ssl_flag);

            return ret;
        }
        else
        {
            TRACE_DEBUG("SSL connection is success");
        }
        g_mqtt_tls_context_ptr = tls_context;

        g_transport_interface.send = mqtts_write;
        g_transport_interface.recv = mqtts_read;
    }
    g_network_context.socketDescriptor = sock;
    g_transport_interface.pNetworkContext = &g_network_context;
    //g_mqtt_config.mqtt_fixed_buf.pBuffer = recv_buf;
    //g_mqtt_config.mqtt_fixed_buf.size = recv_buf_len;
    g_mqtt_config.mqtt_fixed_buf.pBuffer = g_mqtt_buf;
    g_mqtt_config.mqtt_fixed_buf.size = MQTT_BUF_MAX_SIZE;



    if(g_mqtt_event_callback != NULL)
    {
        event_callback = g_mqtt_event_callback;
    }

    /* Initialize MQTT context */
    TRACE_DEBUG("MQTT initialization");
    ret = MQTT_Init(&g_mqtt_config.mqtt_context,
                    &g_transport_interface,
                    (MQTTGetCurrentTimeFunc_t)get_time_ms,
                    event_callback,
                    &g_mqtt_config.mqtt_fixed_buf);

    if (ret != 0)
    {
        mqtt_transport_close(sock, ssl_flag);
        TRACE_ERROR("MQTT initialization is error : %d", ret);

        return -1;
    }
    else
    {
        TRACE_DEBUG("MQTT initialization is success");
    }
          
    ret = MQTT_InitStatefulQoS(&g_mqtt_config.mqtt_context,
                                pOutgoingPublishRecords, OUTGOING_PUBLISH_RECORD_LEN,
                                pIncomingPublishRecords, INCOMING_PUBLISH_RECORD_LEN );
    if (ret != 0)
    {
        mqtt_transport_close(sock, ssl_flag);
        TRACE_ERROR("MQTT init stateful qos is failed : %d", ret);

        return -1;
    }

    /* Connect to the MQTT broker */
    TRACE_DEBUG("MQTT connection", ret);
    ret = MQTT_Connect(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_connect_info, NULL, MQTT_CONNECT_TIMEOUT_MS, &session_present);
    if (ret != 0)
    {
        mqtt_transport_close(sock, ssl_flag);
        TRACE_ERROR("MQTT connection is error : %d", ret);

        return -1;
    }
    else
    {
        TRACE_DEBUG("MQTT connection is success");
    }

    g_mqtt_config.mqtt_state = MQTT_RUNNING;

    return 0;
}

int32_t mqtt_transport_close(uint8_t sock, uint8_t ssl_flag)
{
    int ret;

    if (ssl_flag == true && g_mqtt_tls_context_ptr != NULL)
    {
        mbedtls_ssl_close_notify(&g_mqtt_tls_context_ptr->ssl);
        mbedtls_ssl_free(&g_mqtt_tls_context_ptr->ssl);
        mbedtls_ssl_config_free(&g_mqtt_tls_context_ptr->conf);
        mbedtls_ctr_drbg_free(&g_mqtt_tls_context_ptr->ctr_drbg);
#if defined(MBEDTLS_ENTROPY_C)
        mbedtls_entropy_free(&g_tlsContext.entropy);
#endif
        mbedtls_x509_crt_free(&g_mqtt_tls_context_ptr->cacert);
        mbedtls_x509_crt_free(&g_mqtt_tls_context_ptr->clicert);
        mbedtls_pk_free(&g_mqtt_tls_context_ptr->pkey);

        g_mqtt_tls_context_ptr = NULL;
    }
    
    g_mqtt_config.subscribe_count = 0;
    ret = disconnect(sock);

    g_mqtt_config.mqtt_state = MQTT_IDLE;
    
    if (ret != SOCK_OK)
        return -1;

    return 0;
}

int32_t mqtt_transport_publish(uint8_t *pub_topic, uint8_t *pub_data, uint32_t pub_data_len, uint8_t qos)
{
    int packet_id;
    int ret;

    g_mqtt_config.mqtt_publish_info.qos = qos;

    g_mqtt_config.mqtt_publish_info.pTopicName = pub_topic;
    g_mqtt_config.mqtt_publish_info.topicNameLength = strlen(pub_topic);

    g_mqtt_config.mqtt_publish_info.pPayload = pub_data;
    g_mqtt_config.mqtt_publish_info.payloadLength = pub_data_len;

    packet_id = MQTT_GetPacketId(&g_mqtt_config.mqtt_context);
    
    
    TRACE_DEBUG("MQTT publishing. Topic : %s", pub_topic);
    /* Send message */
    ret = MQTT_Publish(&g_mqtt_config.mqtt_context, &g_mqtt_config.mqtt_publish_info, packet_id);

    if (ret != 0)
    {
        TRACE_DEBUG("MQTT publishing is error : %d", ret);
        TRACE_DEBUG("PUBLISH FAILED");

        return -1;
    }
    else
    {
        TRACE_DEBUG("MQTT publishing is success");
        TRACE_DEBUG("PUBLISH OK");

        return 0;
    }
}

int32_t mqtt_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
    {
        size = send(pNetworkContext->socketDescriptor, (uint8_t *)pBuffer, bytesToSend);
    }

    return size;
}

int32_t mqtt_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
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
        //sleep_ms(10);
        DELAY_MS(10);
    } while ((get_time_ms() - tickStart) <= MQTT_READ_TIMEOUT_MS);

    return size;
}

int32_t mqtts_write(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
        size = tls_socket_write(g_mqtt_tls_context_ptr, (uint8_t *)pBuffer, bytesToSend);

    return size;
}

int32_t mqtts_read(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv)
{
    int32_t size = 0;

    if (getSn_SR(pNetworkContext->socketDescriptor) == SOCK_ESTABLISHED)
        size = tls_socket_read(g_mqtt_tls_context_ptr, pBuffer, bytesToRecv);

    return size;
}
