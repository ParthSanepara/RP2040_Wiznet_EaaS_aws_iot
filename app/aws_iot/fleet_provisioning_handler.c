#include "fleet_provisioning_handler.h"
#include "os_common.h"
#include "tls_socket_interface.h"
#include "mqtt_interface.h"
#include "ethernet.h"
#include "flash_control.h"
#include "aws_iot_cert.h"
#include "cbor.h"
#include "core_json.h"

#define AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED  "$aws/certificates/create/cbor/accepted"
#define AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED  "$aws/certificates/create/cbor/rejected"
#define AWS_SUB_TOPIC_REGISTER_THING_ACCEPTED       "$aws/provisioning-templates/%s/provision/json/accepted"        // Need TEMPLETE_NAME
#define AWS_SUB_TOPIC_REGISTER_THING_REJECTED       "$aws/provisioning-templates/%s/provision/json/rejected"        // Need TEMPLETE_NAME

#define AWS_PUB_TOPIC_CREATE_KEY_AND_CERT           "$aws/certificates/create/cbor"
#define AWS_PUB_TOPIC_REGISTER_THING                "$aws/provisioning-templates/%s/provision/json"

#define AWS_REGISTER_THING_PARAMETERS               "{\r\n"                                                         \
                                                    "   \"certificateOwnershipToken\" : \"%s\",\r\n"                \
                                                    "   \"parameters\" : {\r\n"                                     \
                                                    "       \"SerialNumber\" : \"%s\",\r\n"                         \
                                                    "       \"DeviceLocation\" : \"%s\",\r\n"                       \
                                                    "       \"Manufacturer\" : \"%s\"\r\n"                          \
                                                    "   }\r\n"                                                      \
                                                    "}"



tlsContext_t g_fleet_prov_mqtt_tls_context;

uint8_t g_aws_mqtt_prov_pub_msg_buf[1024] = { 0, };
uint8_t *g_aws_template_name;
uint8_t g_temp_ownership_token[MAX_FP_CERT_OWNERSHIP_TOKEN_SIZE] = {0,};

bool g_recved_mqtt_create_key_and_cert_accept_msg = false;
bool g_recved_mqtt_register_thing_accept_msg = false;

void fleet_provisioning_event_callback(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo)
{
    uint8_t temp_topic[MAX_MQTT_TOPIC_SIZE];
    static AWS_FP_DEVICE_CERT_INFO_t awsFpDeviceCertInfo = {0,};

    if ((pPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        TRACE_DEBUG("Recveved Topic : %.*s", pDeserializedInfo->pPublishInfo->topicNameLength, pDeserializedInfo->pPublishInfo->pTopicName);

        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED, strlen(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED)) == 0 )
        {
            parsing_accept_certificate_create_msg((uint8_t*)pDeserializedInfo->pPublishInfo->pPayload, pDeserializedInfo->pPublishInfo->payloadLength, &awsFpDeviceCertInfo);
            g_recved_mqtt_create_key_and_cert_accept_msg = true;
            return;
        }
        else if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED, strlen(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED)) == 0 )
        {
            TRACE_DEBUG("%.*s\r\n", pDeserializedInfo->pPublishInfo->payloadLength, pDeserializedInfo->pPublishInfo->pPayload);
            return;
        }

        sprintf(temp_topic, AWS_SUB_TOPIC_REGISTER_THING_ACCEPTED, g_aws_template_name);
        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, temp_topic, strlen(temp_topic)) == 0)
        {
            g_recved_mqtt_register_thing_accept_msg = true;
            // Register Thing이 완료 되면 Flash에 저장 한다.
            // Thing 등록이 실패 했는데, 바뀐 인증서가 저장 되는 문제를 막기 위해
            save_flash_provisioned_cert_info(&awsFpDeviceCertInfo);

            return;
        }
        
        sprintf(temp_topic, AWS_SUB_TOPIC_REGISTER_THING_REJECTED, g_aws_template_name);
        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, temp_topic, strlen(temp_topic)) == 0)
        {
            TRACE_DEBUG("%.*s\r\n", pDeserializedInfo->pPublishInfo->payloadLength, pDeserializedInfo->pPublishInfo->pPayload);
        }
    }
}


bool parsing_accept_certificate_create_msg(uint8_t *pPayload, uint16_t payloadLength, AWS_FP_DEVICE_CERT_INFO_t *pAwsFpDeviceCertInfo)
{
    uint8_t temp_str[MAX_PEM_SIZE];
    size_t requiredLen = 0, bufferLen = 0;

    CborError cborRet;
    CborParser parser;
    CborValue map;
    CborValue value;

    cborRet = cbor_parser_init( pPayload, payloadLength, 0, &parser, &map );

    if( cborRet != CborNoError )
    {
        TRACE_ERROR( "Error initializing parser for CreateCertificateFromCsr response: %s.", cbor_error_string( cborRet ) );
        return false;
    }
    else if( !cbor_value_is_map( &map ) )
    {
        TRACE_ERROR( "Response is not a valid map container type." );
        return false;
    }

    // PARSE CERTIFICATE ID
    cborRet = cbor_value_map_find_value( &map, FP_API_CERTIFICATE_ID_KEY, &value );
    if( cborRet == CborNoError )
    {
        bufferLen = sizeof(pAwsFpDeviceCertInfo->DEVICE_CERT_ID);
        cborRet = cbor_value_copy_text_string(&value, pAwsFpDeviceCertInfo->DEVICE_CERT_ID, &bufferLen, NULL);
        if(cborRet == CborErrorOutOfMemory)
        {
            ( void ) cbor_value_calculate_string_length( &value, &requiredLen );
            TRACE_ERROR ("Certificate ID Length : %lu", (unsigned long)requiredLen );
            return false;
        }

        // 배포시에는 주석 처리 할 예정
        TRACE_DEBUG("CERTIFICATE ID : %s",pAwsFpDeviceCertInfo->DEVICE_CERT_ID);
    }
    else
    {
        TRACE_ERROR ( "Error response: %s. value type = %x", cbor_error_string(cborRet), value.type );
        return false;
    }

    // PARSE CERTIFICATE PEM
    cborRet = cbor_value_map_find_value( &map, FP_API_CERTIFICATE_PEM_KEY, &value );
    if( cborRet == CborNoError )
    {
        bufferLen = sizeof(pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_CERT);
        cborRet = cbor_value_copy_text_string(&value, pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_CERT, &bufferLen, NULL);
        if(cborRet == CborErrorOutOfMemory)
        {
            ( void ) cbor_value_calculate_string_length( &value, &requiredLen );
            TRACE_ERROR ("Certificate PEM Length : %lu", (unsigned long)requiredLen );
            return false;
        }

        // 배포시에는 주석 처리 할 예정
        TRACE_DEBUG("CERTIFICATE PEM : %s",pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_CERT);
    }
    else
    {
        TRACE_ERROR ( "Error response: %s. value type = %x", cbor_error_string(cborRet), value.type );
        return false;
    }

    // PARSE PRIVATE KEY
    cborRet = cbor_value_map_find_value( &map, FP_API_PRIVATE_KEY_KEY, &value );
    if( cborRet == CborNoError )
    {
        bufferLen = sizeof(pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_PRIVATE_KEY);
        cborRet = cbor_value_copy_text_string(&value, pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_PRIVATE_KEY, &bufferLen, NULL);
        if(cborRet == CborErrorOutOfMemory)
        {
            ( void ) cbor_value_calculate_string_length( &value, &requiredLen );
            TRACE_ERROR ("Certificate Private Key Length : %lu", (unsigned long)requiredLen );
            return false;
        }

        // 배포시에는 주석 처리 할 예정
        TRACE_DEBUG("PRIVATE KEY : %s",pAwsFpDeviceCertInfo->PROVISONED_CERT_INFO.DEVICE_PROVISIONED_PRIVATE_KEY);
    }
    else
    {
        TRACE_ERROR ( "Error response: %s. value type = %x", cbor_error_string(cborRet), value.type );
        return false;
    }

    // PARSE Ownership token
    cborRet = cbor_value_map_find_value( &map, FP_API_OWNERSHIP_TOKEN_KEY, &value );
    if( cborRet == CborNoError )
    {
        bufferLen = sizeof(pAwsFpDeviceCertInfo->DEVICE_CERT_OWNERSHIP_TOKEN);
        cborRet = cbor_value_copy_text_string(&value, pAwsFpDeviceCertInfo->DEVICE_CERT_OWNERSHIP_TOKEN, &bufferLen, NULL);
        if(cborRet == CborErrorOutOfMemory)
        {
            ( void ) cbor_value_calculate_string_length( &value, &requiredLen );
            TRACE_ERROR ("Ownership Token Length : %lu", (unsigned long)requiredLen );
            return false;
        }

        strncpy(g_temp_ownership_token, pAwsFpDeviceCertInfo->DEVICE_CERT_OWNERSHIP_TOKEN, strlen(pAwsFpDeviceCertInfo->DEVICE_CERT_OWNERSHIP_TOKEN));

        // 배포시에는 주석 처리 할 예정
        TRACE_DEBUG("Ownership Token : %s",pAwsFpDeviceCertInfo->DEVICE_CERT_OWNERSHIP_TOKEN);
    }
    else
    {
        TRACE_ERROR ( "Error response: %s. value type = %x", cbor_error_string(cborRet), value.type );
        return false;
    }

    return true;
}

bool waitRecvCreateCertAndKeyAcceptMsg(uint32_t timeoutMs)
{
    uint32_t startTimeoutMs, currentTimeoutMs;

    startTimeoutMs = get_time_ms();
    while(1)
    {
        currentTimeoutMs = get_time_ms();

        if( (currentTimeoutMs - startTimeoutMs) > timeoutMs )
        {
            TRACE_ERROR("Timeout occured. Timeout %ldms", (currentTimeoutMs-startTimeoutMs));
            return false;
        }

        mqtt_transport_yield();

        if( g_recved_mqtt_create_key_and_cert_accept_msg == true )
        {
            g_recved_mqtt_create_key_and_cert_accept_msg = false;
            break;
        }
        
        OS_DELAY_MS(100);
    }

    return true;
}

bool waitRegisterThingAccepttMsg(uint32_t timeoutMs)
{
    uint32_t startTimeoutMs, currentTimeoutMs;

    startTimeoutMs = get_time_ms();
    while(1)
    {
        currentTimeoutMs = get_time_ms();

        if( (currentTimeoutMs - startTimeoutMs) > timeoutMs )
        {
            TRACE_ERROR("Timeout occured. Timeout %ldms", (currentTimeoutMs-startTimeoutMs));
            return false;
        }

        mqtt_transport_yield();

        if( g_recved_mqtt_register_thing_accept_msg == true )
        {
            g_recved_mqtt_register_thing_accept_msg = false;
            break;
        }
        
        OS_DELAY_MS(100);
    }

    return true;
}

bool processCreateCertAndKey(DEVICE_INFO_t *pDeviceInfo, uint8_t qos)
{
    int retval = 0;
    bool status = false;

    retval = mqtt_transport_subscribe(qos, AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED);
        return false;
    }

    retval = mqtt_transport_subscribe(qos, AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED);
        mqtt_transport_unsubscribe(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED);
        return false;
    }

    mqtt_transport_publish(AWS_PUB_TOPIC_CREATE_KEY_AND_CERT, g_aws_mqtt_prov_pub_msg_buf, strlen(g_aws_mqtt_prov_pub_msg_buf), qos);
    status = waitRecvCreateCertAndKeyAcceptMsg(5000);
    if(status == false)
    {
        TRACE_DEBUG("Failed, Create Key and Cert");
        mqtt_transport_unsubscribe(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED);
        mqtt_transport_unsubscribe(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED);
        return false;
    }

    TRACE_DEBUG("Success to create key and cert");
    mqtt_transport_unsubscribe(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_ACCEPTED);
    mqtt_transport_unsubscribe(AWS_SUB_TOPIC_CREATE_KEY_AND_CERT_REJECTED);
    
    // TRACE_DEBUG("Load Certificate Ownership Token from Flash");
    // load_flash_certificate_ownership_token(pDeviceInfo->CERT_OWNERSHIP_TOKEN, sizeof(pDeviceInfo->CERT_OWNERSHIP_TOKEN));

    return true;
}

bool processRegisterThing(DEVICE_INFO_t *pDeviceInfo, uint8_t qos)
{
    uint8_t topic_accepted[256];
    uint8_t topic_rejected[256];
    uint8_t topic_publish[256];
    int retval = 0;
    bool status = false;

    // Make
    sprintf(topic_accepted, AWS_SUB_TOPIC_REGISTER_THING_ACCEPTED, pDeviceInfo->AWS_TEMPLATE_NAME);
    retval = mqtt_transport_subscribe(qos, topic_accepted);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",topic_accepted);
        return false;
    }

    sprintf(topic_rejected, AWS_SUB_TOPIC_REGISTER_THING_REJECTED, pDeviceInfo->AWS_TEMPLATE_NAME);
    retval = mqtt_transport_subscribe(qos, topic_rejected);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",topic_rejected);
        mqtt_transport_unsubscribe(topic_accepted);
        return false;
    }

    sprintf(topic_publish, AWS_PUB_TOPIC_REGISTER_THING, pDeviceInfo->AWS_TEMPLATE_NAME);
    //sprintf(g_aws_mqtt_prov_pub_msg_buf, AWS_REGISTER_THING_PARAMETERS,  pDeviceInfo->CERT_OWNERSHIP_TOKEN, pDeviceInfo->THING_NAME, 
    //                                                            pDeviceInfo->LOCATION, pDeviceInfo->MANUFACTURER);

    sprintf(g_aws_mqtt_prov_pub_msg_buf, AWS_REGISTER_THING_PARAMETERS,  g_temp_ownership_token, pDeviceInfo->THING_NAME, 
                                                               pDeviceInfo->LOCATION, pDeviceInfo->MANUFACTURER);


    TRACE_DEBUG("Publish Topic : %s", topic_publish);
    //TRACE_DEBUG("Publish Payload : %s", g_aws_mqtt_prov_pub_msg_buf);

    mqtt_transport_publish(topic_publish, g_aws_mqtt_prov_pub_msg_buf, strlen(g_aws_mqtt_prov_pub_msg_buf), qos);
    status = waitRegisterThingAccepttMsg(5000);
    if(status == false)
    {
        TRACE_DEBUG("Failed, Register Thing");
        mqtt_transport_unsubscribe(topic_accepted);
        mqtt_transport_unsubscribe(topic_rejected);
        return false;
    }

    TRACE_DEBUG("Success to register thing");
    mqtt_transport_unsubscribe(topic_accepted);
    mqtt_transport_unsubscribe(topic_rejected);
    return true;
}

bool fleet_provisioning_handle(DEVICE_INFO_t *pDeviceInfo)
{
    int retval = 0, qos=1;
    bool status = false;
    uint8_t sslEnable = 1;
    uint8_t mqttClientId[MAX_MQTT_CLIENT_ID_SIZE];

    g_fleet_prov_mqtt_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_REQUIRED; // use Root CA verify
    g_fleet_prov_mqtt_tls_context.clica_option = 1;                            // use client certificate
    g_fleet_prov_mqtt_tls_context.root_ca = (uint8_t*)AWS_IOT_ROOT_CA;
    g_fleet_prov_mqtt_tls_context.client_cert = (uint8_t*)AWS_IOT_CLIENT_CERT;
    g_fleet_prov_mqtt_tls_context.private_key = (uint8_t*)AWS_IOT_PRIVATE_KEY;

    g_aws_template_name = pDeviceInfo->AWS_TEMPLATE_NAME;
    
    TRACE_INFO("Start Fleet Provisioning");
    TRACE_INFO("Thing Name : %s, Template Name : %s", pDeviceInfo->THING_NAME, g_aws_template_name);

    set_mqtt_event_callback(fleet_provisioning_event_callback);

    g_recved_mqtt_create_key_and_cert_accept_msg = false;
    g_recved_mqtt_register_thing_accept_msg = false;

    sprintf(mqttClientId,"%s",pDeviceInfo->THING_NAME);
    retval = mqtt_transport_init(true, mqttClientId, NULL, NULL, MQTT_KEEP_ALIVE_S);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, mqtt_transport_init returned %d", retval);
        return false;
    }

    retval = mqtt_transport_connect(SOCKET_MQTT_FLEET_PROVISIONING, sslEnable, pDeviceInfo->AWS_END_POINT, AWS_MQTT_PORT, &g_fleet_prov_mqtt_tls_context);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, mqtt_transport_connect returned %d", retval);
        mqtt_transport_close(SOCKET_MQTT_FLEET_PROVISIONING, sslEnable);
        return false;
    }
 
    status = processCreateCertAndKey(pDeviceInfo, qos);
    if(status == false)
    {
        mqtt_transport_close(SOCKET_MQTT_FLEET_PROVISIONING, sslEnable);
        return false;
    }

    status = processRegisterThing(pDeviceInfo, qos);
    if(status == false)
    {
        mqtt_transport_close(SOCKET_MQTT_FLEET_PROVISIONING, sslEnable);
        return false;
    }

    mqtt_transport_close(SOCKET_MQTT_FLEET_PROVISIONING, sslEnable);
    return true;
}