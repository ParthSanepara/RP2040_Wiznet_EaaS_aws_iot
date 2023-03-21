#ifndef _MQTT_INTERFACE_H_
#define _MQTT_INTERFACE_H_

#include "core_mqtt.h"
#include "tls_socket_interface.h"
#include "ethernet.h"


#define MQTT_BUF_MAX_SIZE (1024 * 25)
#define MQTT_DOMAIN_MAX_SIZE 128

/* Timeout */
#define MQTT_CONNECT_TIMEOUT_MS (30 * 1000)  // 10 seconds
#define MQTT_READ_TIMEOUT_MS    ( 1 * 1000)  // 1 seconds
#define MQTT_KEEP_ALIVE_S       (30)         // 30 seconds


/* Subscription number */
#define MQTT_SUBSCRIPTION_MAX_NUM 5

typedef void (*MQTT_EVENT_CALLBACK)(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo);

typedef enum
{
    MQTT_IDLE = 0,
    MQTT_RUNNING,
} mqtt_state_t;

typedef struct __mqtt_config
{
    MQTTContext_t mqtt_context;
    MQTTConnectInfo_t mqtt_connect_info;
    MQTTFixedBuffer_t mqtt_fixed_buf;
    MQTTPublishInfo_t mqtt_publish_info;
    mqtt_state_t mqtt_state;
    MQTTSubscribeInfo_t mqtt_subscribe_info[MQTT_SUBSCRIPTION_MAX_NUM];
    uint8_t subscribe_count;
    uint8_t mqtt_ip[4];
} mqtt_config_t;

void            set_mqtt_state  (mqtt_state_t state);
mqtt_state_t    get_mqtt_state  (void);


void    set_mqtt_event_callback     (MQTT_EVENT_CALLBACK event_callback);
void    unset_mqtt_event_callback   (MQTT_EVENT_CALLBACK event_callback);

int32_t  mqtt_transport_yield        (void);
int32_t  mqtt_transport_init         (uint8_t cleanSession, uint8_t *ClientId, uint8_t *userName, uint8_t *password, uint32_t keepAlive);
int32_t  mqtt_transport_subscribe    (uint8_t qos, char *subscribe_topic);
int32_t  mqtt_transport_unsubscribe  (char *subscribe_topic);
//int32_t  mqtt_transport_connect      (uint8_t sock, uint8_t ssl_flag, uint8_t *recv_buf, uint32_t recv_buf_len, uint8_t *domain, uint32_t port, tlsContext_t *tls_context);
int32_t  mqtt_transport_connect      (uint8_t sock, uint8_t ssl_flag, uint8_t *domain, uint32_t port, tlsContext_t *tls_context);
int32_t  mqtt_transport_close        (uint8_t sock, uint8_t ssl_flag);
int32_t  mqtt_transport_publish      (uint8_t *pub_topic, uint8_t *pub_data, uint32_t pub_data_len, uint8_t qos);

int32_t mqtt_write  (NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t mqtt_read   (NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);
int32_t mqtts_write (NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend);
int32_t mqtts_read  (NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv);


#endif