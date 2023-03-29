#include "iot_jobs_handler.h"
#include "system_common.h"
#include "os_common.h"
#include "tls_socket_interface.h"
#include "mqtt_interface.h"
#include "ethernet.h"
#include "aws_iot_cert.h"
#include "flash_control.h"
#include "core_json.h"
#include "iot_jobs_job_list.h"
#include "iot_jobs_job_ota_handler.h"


#define AWS_SUB_TOPIC_JOBS_GET_ACCEPTED         "$aws/things/%s/jobs/get/accepted"          // (THING NAME)
//#define AWS_SUB_TOPIC_JOBS_GET_REJECTED         "$aws/things/%s/jobs/get/rejected"        // (THING NAME)
#define AWS_SUB_TOPIC_JOBS_START_NEXT_ACCEPTED  "$aws/things/%s/jobs/start-next/accepted"   // (THING NAME)
//#define AWS_SUB_TOPIC_JOBS_START_NEXT_REJECTED  "$aws/things/%s/jobs/start-next/rejected" // (THING NAME)
#define AWS_SUB_TOPIC_JOBS_UPDATE_ACCEPTED      "$aws/things/%s/jobs/%s/update/accepted"    // (THING NAME, JOB ID)

#define AWS_PUB_TOPIC_JOBS_GET                  "$aws/things/%s/jobs/get"                   // (THING NAME)
#define AWS_PUB_TOPIC_JOBS_START_NEXT           "$aws/things/%s/jobs/start-next"            // (THING NAME)
#define AWS_PUB_TOPIC_JOBS_UPDATE               "$aws/things/%s/jobs/%s/update"             // (THING NAME, JOB ID)

#define AWS_JOBS_UPDATE_PARAMETERS              "{\r\n"                        \
                                                "   \"status\" : \"%s\" \r\n"  \
                                                "}"



tlsContext_t                g_iot_jobs_mqtt_tls_context;
AWS_JOBS_EXECUTION_PARAMS_t g_iot_jobs_execution_param;

volatile uint8_t *p_aws_thing_name_for_event_callback;
volatile uint8_t *p_aws_job_id_for_event_callback;

bool g_recved_mqtt_get_job_accept_msg_for_event_callback = false;
bool g_recved_mqtt_start_next_job_accept_msg_for_event_callback = false;
bool g_recved_mqtt_job_update_accept_msg_for_event_callback = false;


uint8_t g_aws_mqtt_iot_jobs_pub_msg_buf[1024] = { 0, };


void iot_jobs_event_callback(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo)
{
    uint8_t temp_topic[MAX_MQTT_TOPIC_SIZE]={0,};
    uint8_t temp_job_id[MAX_AWS_JOB_ID_SIZE]={0,};
    uint8_t temp_job_type[MAX_AWS_JOB_TYPE_SIZE]={0,};
    uint8_t temp_job_documet[MAX_AWS_JOB_DOCUMENT_SIZE]={0,};
    uint16_t temp_str_length=0;
    bool retStatus = false;

    if ((pPacketInfo->type & 0xF0U) == MQTT_PACKET_TYPE_PUBLISH)
    {
        TRACE_DEBUG("");
        TRACE_DEBUG("======== Iot Jobs Mqtt Event Callback ========");
        TRACE_DEBUG("Thing Name : %s", p_aws_thing_name_for_event_callback);
        TRACE_DEBUG("Recveved Topic : %.*s", pDeserializedInfo->pPublishInfo->topicNameLength, pDeserializedInfo->pPublishInfo->pTopicName);
        TRACE_DEBUG("%.*s\r\n", pDeserializedInfo->pPublishInfo->payloadLength, pDeserializedInfo->pPublishInfo->pPayload);

        sprintf(temp_topic, AWS_SUB_TOPIC_JOBS_GET_ACCEPTED, p_aws_thing_name_for_event_callback);
        //TRACE_DEBUG("Temp Topic for compare : %s", temp_topic);
        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, temp_topic, pDeserializedInfo->pPublishInfo->topicNameLength) == 0 )
        {
            g_recved_mqtt_get_job_accept_msg_for_event_callback = true;

            // queuedJobs에 데이터 있으면
            // g_aws_job_id 저장
            retStatus = parsing_get_job_accepted(   (uint8_t*)pDeserializedInfo->pPublishInfo->pPayload, 
                                                    pDeserializedInfo->pPublishInfo->payloadLength, 
                                                    temp_job_id,
                                                    sizeof(temp_job_id)
            );
            if(retStatus == true)
            {
                // 이미 등록되어 있는 JOB이면 무시
                if(is_exist_job_id(temp_job_id, strlen(temp_job_id)) == true)
                {
                    return;
                }

                if( add_job_id(temp_job_id, strlen(temp_job_id)) == false )
                {
                    TRACE_ERROR("Fail to add new job");
                    return;
                }

                TRACE_DEBUG("Success to add new job. Job ID : %s", temp_job_id);
            }
        }

        sprintf(temp_topic, AWS_SUB_TOPIC_JOBS_START_NEXT_ACCEPTED, p_aws_thing_name_for_event_callback);
        //TRACE_DEBUG("Temp Topic for compare : %s", temp_topic);
        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, temp_topic, pDeserializedInfo->pPublishInfo->topicNameLength) == 0 )
        {
            g_recved_mqtt_start_next_job_accept_msg_for_event_callback = true;
            retStatus = parsing_start_next_job_accepted((uint8_t*)pDeserializedInfo->pPublishInfo->pPayload, 
                                                        pDeserializedInfo->pPublishInfo->payloadLength, 
                                                        temp_job_id,
                                                        sizeof(temp_job_id),
                                                        temp_job_documet,
                                                        sizeof(temp_job_documet),
                                                        temp_job_type,
                                                        sizeof(temp_job_type));
            if(retStatus == true)
            {
                // 이미 동작 중인 JOB이 있다면, start_next_job으로 할당된 JOB은 보류 함.
                if( g_iot_jobs_execution_param.IS_JOB_STARTED == false )
                {
                    temp_str_length = strlen(temp_job_id);
                    strncpy( g_iot_jobs_execution_param.JOB_ID, temp_job_id, temp_str_length );
                    g_iot_jobs_execution_param.JOB_ID[temp_str_length] = '\0';

                    temp_str_length = strlen(temp_job_documet);
                    strncpy( g_iot_jobs_execution_param.JOB_DOCUMENT, temp_job_documet, temp_str_length );
                    g_iot_jobs_execution_param.JOB_DOCUMENT[temp_str_length] = '\0';

                    // Wiznet Custom Value
                    temp_str_length = strlen(temp_job_type);
                    strncpy( g_iot_jobs_execution_param.JOB_TYPE, temp_job_type, temp_str_length );
                    g_iot_jobs_execution_param.JOB_TYPE[temp_str_length] = '\0';
                }
            }
        }

        sprintf(temp_topic, AWS_SUB_TOPIC_JOBS_UPDATE_ACCEPTED, p_aws_thing_name_for_event_callback, p_aws_job_id_for_event_callback);
        // TRACE_DEBUG("Temp Topic for compare : %s", temp_topic);
        if( strncmp(pDeserializedInfo->pPublishInfo->pTopicName, temp_topic, pDeserializedInfo->pPublishInfo->topicNameLength) == 0 )
        {
            g_recved_mqtt_job_update_accept_msg_for_event_callback = true;
        }


    }
}

bool iot_jobs_connect_procedure(DEVICE_INFO_t *pDeviceInfo)
{
    bool retStatus = false;
    int qos = 1;

    retStatus = iot_jobs_mqtt_connect(pDeviceInfo);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to iot_jobs_mqtt_connect");
        return false;
    }

    retStatus = iot_jobs_mqtt_subscribe_jobs_get_topic(pDeviceInfo->THING_NAME, qos);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to iot_jobs_mqtt_subscribe_jobs_get_topic");
        return false;
    }

    retStatus = iot_jobs_mqtt_subscribe_jobs_start_next_topic(pDeviceInfo->THING_NAME, qos);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to iot_jobs_mqtt_subscribe_jobs_start_next_topic");
        return false;
    }

    return true;
}

void iot_jobs_close_procedure(DEVICE_INFO_t *pDeviceInfo)
{
    iot_jobs_mqtt_unsubscribe_jobs_get_topic(pDeviceInfo->THING_NAME);
    iot_jobs_mqtt_unsubscribe_jobs_start_next_topic(pDeviceInfo->THING_NAME);

    iot_jobs_mqtt_close();
}

bool iot_jobs_get_procedure (DEVICE_INFO_t *pDeviceInfo)
{
    bool retStatus = false;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];
    int qos = 1;

    p_aws_thing_name_for_event_callback = pDeviceInfo->THING_NAME;

    memset(g_aws_mqtt_iot_jobs_pub_msg_buf, 0x00, sizeof(g_aws_mqtt_iot_jobs_pub_msg_buf));

    sprintf(topic, AWS_PUB_TOPIC_JOBS_GET, pDeviceInfo->THING_NAME);
    if( mqtt_transport_publish(topic, g_aws_mqtt_iot_jobs_pub_msg_buf, strlen(g_aws_mqtt_iot_jobs_pub_msg_buf), qos) == -1) 
    {
        TRACE_ERROR("Fail to mqtt publish. Topic : %s", topic);
        return false;
    }

    retStatus = iot_jobs_mqtt_wait_recv_get_job(1000);

    if(retStatus == false)
    {
        TRACE_ERROR("Fail to get jobs")
        return false;
    }

    return true;
}

bool iot_jobs_start_job_document_procedure (APP_COMMON_t *pAppCommon)
{
    bool retStatus = false;
    uint8_t tempJobId[MAX_AWS_JOB_ID_SIZE]={0,};    
    uint16_t tempJobIdLength;

    pAppCommon->enableReboot = false;

    if(get_first_job_id(tempJobId, &tempJobIdLength) == false)
    {
        // 할당 된 JOB ID가 없음.
        return true;
    }

    // 이미 동작 중인 JOB이 있으면, start_next_procedure를 수행 하지 않음
    if( g_iot_jobs_execution_param.IS_JOB_STARTED == true )
    {
        TRACE_DEBUG("There is already operating job");
        TRACE_DEBUG("Already Operating JOB ID : %s", g_iot_jobs_execution_param.JOB_ID);
        return true;
    }

    TRACE_INFO("Exist Job ID : %s", tempJobId);
    TRACE_INFO("Do Start Next Procedure");
    
    memset(&g_iot_jobs_execution_param, 0x00, sizeof(g_iot_jobs_execution_param));
    retStatus = iot_jobs_start_next_procedure(&pAppCommon->DEVICE_INFO);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to start next procedure");
        return false;
    }

    g_iot_jobs_execution_param.IS_JOB_STARTED = true;
    
    retStatus = start_job_document(pAppCommon, &g_iot_jobs_execution_param);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to start job documnet");

        g_iot_jobs_execution_param.IS_JOB_STARTED = false;

        retStatus = iot_jobs_update_job_status_procedure(&pAppCommon->DEVICE_INFO, g_iot_jobs_execution_param.JOB_ID, JOBS_API_STATUS_FAILED);
        if(retStatus == true)
        {
            del_first_job_id();
        }
        return false;
    }

    g_iot_jobs_execution_param.IS_JOB_STARTED = false;

    retStatus = iot_jobs_update_job_status_procedure(&pAppCommon->DEVICE_INFO, g_iot_jobs_execution_param.JOB_ID, JOBS_API_STATUS_SUCCEEDED);
    if(retStatus == true)
    {
        del_first_job_id();
    }

    // TRACE_DEBUG("JOB ID : %s", g_iot_jobs_execution_param.JOB_ID);
    // TRACE_DEBUG("JOB Document : %s", g_iot_jobs_execution_param.JOB_DOCUMENT);
    if(pAppCommon->enableReboot == true)
    {
        TRACE_DEBUG("Device Reboot");
        device_reboot();
    }

    return true;
}

bool start_job_document(APP_COMMON_t *pAppCommon, AWS_JOBS_EXECUTION_PARAMS_t *pJobExecutionParam)
{
    bool retStatus;

    size_t jobDocumentLength = 0;
    APP_COMMON_t  tempAppCommonInfo;

    jobDocumentLength = strlen(pJobExecutionParam->JOB_DOCUMENT);

    // JOB TYPE이 OTA면 아래 수행
    if( strncmp( pJobExecutionParam->JOB_TYPE, AWS_JOB_TYPE_OTA, sizeof(AWS_JOB_TYPE_OTA)) == 0 )
    {
        pAppCommon->DEVICE_OTA_INFO.START_FIRMWARE_OTA_STATUS = START_FW_OTA_DISABLE;

        memset(&pJobExecutionParam->JOB_OTA, 0x00, sizeof(pJobExecutionParam->JOB_OTA));
        retStatus = parsing_ota_job_params( pJobExecutionParam->JOB_DOCUMENT, jobDocumentLength,
                                            pJobExecutionParam->JOB_OTA.FIRMWARE_URL, sizeof(pJobExecutionParam->JOB_OTA.FIRMWARE_URL),
                                            pJobExecutionParam->JOB_OTA.FIRMWARE_VERSION, sizeof(pJobExecutionParam->JOB_OTA.FIRMWARE_VERSION),
                                            &pJobExecutionParam->JOB_OTA.FIRMWARE_SIZE, &pJobExecutionParam->JOB_OTA.FIRMWARE_CRC 
        );
        if(retStatus == false)
        {
            TRACE_ERROR("Fail to parsing ota job params");
            return false;
        }

        retStatus = ota_download_firmware(pJobExecutionParam->JOB_OTA.FIRMWARE_URL, pJobExecutionParam->JOB_OTA.FIRMWARE_SIZE, pJobExecutionParam->JOB_OTA.FIRMWARE_CRC);
        if(retStatus == false)
        {
            TRACE_ERROR("Fail to download firmware");
            return false;
        }

        // ota_download_firmware 함수 내에서 CRC, FW size 유효성 검사 수행
        pAppCommon->DEVICE_OTA_INFO.START_FIRMWARE_OTA_STATUS = START_FW_OTA_ENABLE;
        pAppCommon->DEVICE_OTA_INFO.FIRMWARE_SIZE = pJobExecutionParam->JOB_OTA.FIRMWARE_SIZE;
        pAppCommon->DEVICE_OTA_INFO.FIRMWARE_CRC = pJobExecutionParam->JOB_OTA.FIRMWARE_CRC;

        // Bootloader에서 FW Update를 시작 할 수 있도록 Flag를 Enable
        //load_flash_common_config_info(&tempAppCommonInfo);
        memcpy(&tempAppCommonInfo, pAppCommon, sizeof(tempAppCommonInfo));
        save_flash_common_config_info(&tempAppCommonInfo);

        TRACE_INFO("Set Enable Reboot Triiger");
        pAppCommon->enableReboot = true;
    }
    // JOB TYPE이 TEST면 아래 수행
    // 해당 JOB은 Example용 DUMMY JOB
    else if( strncmp( pJobExecutionParam->JOB_TYPE, AWS_JOB_TYPE_TEST, sizeof(AWS_JOB_TYPE_TEST)) == 0 )
    {
        
    }


    return true;
}

bool iot_jobs_start_next_procedure(DEVICE_INFO_t *pDeviceInfo)
{
    bool retStatus = false;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];
    int qos = 1;

    p_aws_thing_name_for_event_callback = pDeviceInfo->THING_NAME;
    
    memset(g_aws_mqtt_iot_jobs_pub_msg_buf, 0x00, sizeof(g_aws_mqtt_iot_jobs_pub_msg_buf));

    sprintf(topic, AWS_PUB_TOPIC_JOBS_START_NEXT, pDeviceInfo->THING_NAME);
    if( mqtt_transport_publish(topic, g_aws_mqtt_iot_jobs_pub_msg_buf, strlen(g_aws_mqtt_iot_jobs_pub_msg_buf), qos) == -1)
    {
        TRACE_ERROR("Fail to mqtt publish. Topic : %s", topic);
        return false;
    }

    retStatus = iot_jobs_mqtt_wait_recv_start_next_job(1000);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to start next jobs");
        return false;
    }

    return true;
}

bool iot_jobs_update_job_status_procedure(DEVICE_INFO_t *pDeviceInfo, uint8_t *pJobId, uint8_t *pJobStatus)
{
    bool retStatus = false;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];
    int qos = 1;

    p_aws_thing_name_for_event_callback = pDeviceInfo->THING_NAME;
    p_aws_job_id_for_event_callback = pJobId;

    memset(g_aws_mqtt_iot_jobs_pub_msg_buf, 0x00, sizeof(g_aws_mqtt_iot_jobs_pub_msg_buf));
    
    sprintf(topic, AWS_PUB_TOPIC_JOBS_UPDATE, pDeviceInfo->THING_NAME, pJobId);
    sprintf(g_aws_mqtt_iot_jobs_pub_msg_buf, AWS_JOBS_UPDATE_PARAMETERS, pJobStatus);

    if( mqtt_transport_publish(topic, g_aws_mqtt_iot_jobs_pub_msg_buf, strlen(g_aws_mqtt_iot_jobs_pub_msg_buf), qos) == -1)
    {
        TRACE_ERROR("Fail to mqtt publish. Topic : %s", topic);
        return false;
    }


    retStatus = iot_jobs_mqtt_wait_recv_job_update_accepted(1000);
    if(retStatus == false)
    {
        TRACE_ERROR("Fail to update job status");
        return false;
    }

    return true;
}

bool iot_jobs_mqtt_connect(DEVICE_INFO_t *pDeviceInfo)
{
    int retval = 0, qos=1;
    uint8_t mqttClientId[MAX_MQTT_CLIENT_ID_SIZE];
    uint8_t sslEnable = 1;

    AWS_FP_DEVICE_CERT_INFO_t provisionedCertInfo;

    load_flash_provisioned_cert_info(&provisionedCertInfo);

    g_iot_jobs_mqtt_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_REQUIRED; // use Root CA verify
    g_iot_jobs_mqtt_tls_context.clica_option = 1;
    g_iot_jobs_mqtt_tls_context.root_ca     = (uint8_t*)AWS_IOT_ROOT_CA;
    g_iot_jobs_mqtt_tls_context.client_cert = (uint8_t*)provisionedCertInfo.PROVISONED_CERT_INFO.DEVICE_PROVISIONED_CERT;
    g_iot_jobs_mqtt_tls_context.private_key = (uint8_t*)provisionedCertInfo.PROVISONED_CERT_INFO.DEVICE_PROVISIONED_PRIVATE_KEY;

    set_mqtt_event_callback(iot_jobs_event_callback);

    // make client id    
    sprintf(mqttClientId,"%s",pDeviceInfo->THING_NAME);
    retval = mqtt_transport_init(true, mqttClientId, NULL, NULL, MQTT_KEEP_ALIVE_S);
    if(retval != 0)
    {
        TRACE_ERROR("Failed, mqtt_transport_init for iot jobs. returned %d", retval);
        return false;
    }

    retval = mqtt_transport_connect(SOCKET_MQTT_IOT_JOBS, sslEnable, pDeviceInfo->AWS_END_POINT, AWS_MQTT_PORT, &g_iot_jobs_mqtt_tls_context);
    if(retval != 0)
    {
        TRACE_ERROR("Failed, mqtt_transport_connect for iot jobs. returned %d", retval);
        mqtt_transport_close(SOCKET_MQTT_IOT_JOBS, sslEnable);
        return false;
    }

    return true;
}

bool iot_jobs_mqtt_close(void)
{
    uint8_t sslEnable = 1;

    mqtt_transport_close(SOCKET_MQTT_IOT_JOBS, sslEnable);

    return true;
}

bool iot_jobs_mqtt_subscribe_jobs_get_topic(uint8_t *pThingName, uint8_t qos)
{
    int retval = 0;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_GET_ACCEPTED, pThingName);
    retval = mqtt_transport_subscribe(qos, topic);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",topic);
        return false;
    }

    return true;
}

bool iot_jobs_mqtt_unsubscribe_jobs_get_topic(uint8_t *pThingName)
{
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_GET_ACCEPTED, pThingName);
    mqtt_transport_unsubscribe(topic);

    return true;
}

bool iot_jobs_mqtt_subscribe_jobs_start_next_topic(uint8_t *pThingName, uint8_t qos)
{
    int retval = 0;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_START_NEXT_ACCEPTED, pThingName);
    retval = mqtt_transport_subscribe(qos, topic);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",topic);
        return false;
    }

    return true;
}

bool iot_jobs_mqtt_unsubscribe_jobs_start_next_topic(uint8_t *pThingName)
{
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_START_NEXT_ACCEPTED, pThingName);
    mqtt_transport_unsubscribe(topic);

    return true;
}

bool iot_jobs_mqtt_subscribe_jobs_update_topic(uint8_t *pThingName, uint8_t *pJobId, uint8_t qos)
{
    int retval = 0;
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_UPDATE_ACCEPTED, pThingName, pJobId);
    retval = mqtt_transport_subscribe(qos, topic);
    if(retval != 0)
    {
        TRACE_DEBUG("Failed, subscribe topic : %s\r\n",topic);
        return false;
    }

    return true;
}

bool iot_jobs_mqtt_unsubscribe_jobs_update_topic(uint8_t *pThingName, uint8_t *pJobId)
{
    uint8_t topic[MAX_MQTT_TOPIC_SIZE];

    sprintf(topic, AWS_SUB_TOPIC_JOBS_START_NEXT_ACCEPTED, pThingName, pJobId);
    mqtt_transport_unsubscribe(topic);

    return true;
}

bool iot_jobs_mqtt_wait_recv_get_job(uint32_t timeoutMs)
{
    uint32_t startTimeoutMs, currentTimeoutMs;

    TRACE_DEBUG("Wait Recv Get Job Accepted");
    startTimeoutMs = get_time_ms();
    while(1)
    {
        currentTimeoutMs = get_time_ms();

        if( (currentTimeoutMs - startTimeoutMs) > timeoutMs )
        {
            TRACE_ERROR("Recv get job timeout occured. Timeout %ldms", (currentTimeoutMs-startTimeoutMs));
            return false;
        }

        mqtt_transport_yield();

        if( g_recved_mqtt_get_job_accept_msg_for_event_callback == true )
        {
            g_recved_mqtt_get_job_accept_msg_for_event_callback = false;
            break;
        }
        
        OS_DELAY_MS(100);
    }

    TRACE_DEBUG("Success. Recv Get Job Accepted");
    return true;
}

bool iot_jobs_mqtt_wait_recv_start_next_job(uint32_t timeoutMs)
{
    uint32_t startTimeoutMs, currentTimeoutMs;

    TRACE_DEBUG("Wait Recv Start Next Job Accepted");
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

        if( g_recved_mqtt_start_next_job_accept_msg_for_event_callback == true )
        {
            g_recved_mqtt_start_next_job_accept_msg_for_event_callback = false;
            break;
        }
        
        OS_DELAY_MS(100);
    }

    TRACE_DEBUG("Success. Recv Start Next Job Accepted");
    return true;
}

bool iot_jobs_mqtt_wait_recv_job_update_accepted(uint32_t timeoutMs)
{
    uint32_t startTimeoutMs, currentTimeoutMs;

    TRACE_DEBUG("Wait Recv Job Update Accepted");
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

        if( g_recved_mqtt_job_update_accept_msg_for_event_callback == true )
        {
            g_recved_mqtt_job_update_accept_msg_for_event_callback = false;
            break;
        }
        
        OS_DELAY_MS(100);
    }
    
    TRACE_DEBUG("Success. Recv Job Update Accepted");
    return true;
}


bool parsing_get_job_accepted(uint8_t *pPayload, uint16_t payloadLength, uint8_t *pJobId, uint16_t maxJobIdLength)
{
    JSONStatus_t result;
    char *jobId = NULL;
    size_t jobIdLength = 0;

    TRACE_DEBUG("Start Parsing get Job accepted");

    result = JSON_Validate(pPayload, payloadLength);
    if( result != JSONSuccess )
    {
        TRACE_ERROR("Invalid Payload Format. It's not json format");
        return false;
    }

    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "queuedJobs[0].jobId",
                            ( sizeof( "queuedJobs[0].jobId" ) -1 ),
                            &jobId,
                            &jobIdLength );
    
    if( result != JSONSuccess)
    {
        TRACE_DEBUG("queuedJobs is empty");
        //TRACE_ERROR("Invalid Payload. There is no jobId key in queuedJobs array");
        return false;
    }

    if(jobIdLength > maxJobIdLength)
    {
        TRACE_ERROR("Invalid Job Id. Length : %d/%d", jobIdLength, maxJobIdLength);
        return false;
    }

    strncpy(pJobId, jobId, jobIdLength);

    return true;
}

bool parsing_start_next_job_accepted(   uint8_t *pPayload, uint16_t payloadLength, 
                                        uint8_t *pJobId, uint16_t maxJobIdLength,
                                        uint8_t *pJobDocument, uint16_t maxJobDocumentLength,
                                        uint8_t *pJobType, uint16_t maxJobTypeLength
                                        )
{
    JSONStatus_t result;
    char *tempJobId = NULL, *tempJobDocument = NULL, *tempJobType = NULL;
    size_t tempJobIdLength = 0, tempJobDocumentLength = 0, tempJobTypeLength = 0;


    TRACE_DEBUG("Start start next job accepted");

    result = JSON_Validate(pPayload, payloadLength);
    if( result != JSONSuccess )
    {
        TRACE_ERROR("Invalid Payload Format. It's not json format");
        return false;
    }
    
    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "execution.jobId",
                            ( sizeof( "execution.jobId" ) -1 ),
                            &tempJobId,
                            &tempJobIdLength );
    if(result == JSONSuccess)
    {
        strncpy(pJobId, tempJobId, tempJobIdLength);
        pJobId[tempJobIdLength] = '\0';
    }
    else
    {
        TRACE_ERROR("Fail to find execution.jobId");
    }

    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "execution.jobDocument",
                            ( sizeof( "execution.jobDocument" ) -1 ),
                            &tempJobDocument,
                            &tempJobDocumentLength );
    if(result == JSONSuccess)
    {
        strncpy(pJobDocument, tempJobDocument, tempJobDocumentLength);
        pJobDocument[tempJobDocumentLength] = '\0';
    }
    else
    {
        TRACE_ERROR("Fail to find execution.jobDocument");
    }


    // Wiznet Custom Value
    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "execution.jobDocument.jobType",
                            ( sizeof( "execution.jobDocument.jobType" ) -1 ),
                            &tempJobType,
                            &tempJobTypeLength );
    if(result == JSONSuccess)
    {
        strncpy(pJobType, tempJobType, tempJobTypeLength);
        pJobType[tempJobTypeLength] = '\0';
    }
    else
    {
        TRACE_ERROR("Fail to find execution.jobDocument.jobType");
    }

    //TRACE_DEBUG("Job ID : %s", pJobId);
    //TRACE_DEBUG("Job Document : %s", pJobDocument);

    return true;
}

bool parsing_ota_job_params (uint8_t *pPayload, uint16_t payloadLength,
                             uint8_t *pFwUrl, uint16_t maxFwUrlLength,
                             uint8_t *pFwVersion, uint16_t maxFwVersionLength,
                             uint32_t *pFwSize, uint16_t *pFwCrc)
{
    JSONStatus_t result;
    char *tempFwUrl = NULL, *tempFwVersion = NULL;
    char *tempFwSize = NULL, *tempFwCrc = NULL;
    size_t tempFwUrlLength = 0, tempFwVersionLength = 0;
    size_t tempFwSizeLength = 0, tempFwCrcLength = 0;

    result = JSON_Validate(pPayload, payloadLength);
    if( result != JSONSuccess )
    {
        TRACE_ERROR("Invalid Payload Format. It's not json format");
        return false;
    }

    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "fwUrl",
                            ( sizeof( "fwUrl" ) -1 ),
                            &tempFwUrl,
                            &tempFwUrlLength );
    if(result != JSONSuccess)
    {
        TRACE_ERROR("Fail to find fwUrl");
        return false;
    }
    else if(tempFwUrlLength > maxFwUrlLength)
    {
        TRACE_ERROR("Invalid maxFwUrlLength. %d/%d",tempFwUrlLength, maxFwUrlLength);
        return false;
    }


    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "fwVersion",
                            ( sizeof( "fwVersion" ) -1 ),
                            &tempFwVersion,
                            &tempFwVersionLength );
    if(result != JSONSuccess)
    {
        TRACE_ERROR("Fail to find fwVersion");
        return false;
    }
    else if(tempFwVersionLength > maxFwVersionLength)
    {
        TRACE_ERROR("Invalid maxFwVersionLength. %d/%d",tempFwVersionLength, maxFwVersionLength);
        return false;
    }


    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "fwSize",
                            ( sizeof( "fwSize" ) -1 ),
                            &tempFwSize,
                            &tempFwSizeLength );
    if(result != JSONSuccess)
    {
        TRACE_ERROR("Fail to find fwSize");
        return false;
    }

    result = JSON_Search(   pPayload, 
                            payloadLength,
                            "fwCrc",
                            ( sizeof( "fwCrc" ) -1 ),
                            &tempFwCrc,
                            &tempFwCrcLength );
    if(result != JSONSuccess)
    {
        TRACE_ERROR("Fail to find fwCrc");
        return false;
    }

    strncpy(pFwUrl, tempFwUrl, tempFwUrlLength);
    pFwUrl[tempFwUrlLength] = '\0';

    strncpy(pFwVersion, tempFwVersion, tempFwVersionLength);
    pFwVersion[tempFwVersionLength] = '\0';

    *pFwSize = string_to_int(tempFwSize);

    *pFwCrc = hex_string_to_int(tempFwCrc);

    // TRACE_DEBUG("FwUrl : %s",pFwUrl);
    // TRACE_DEBUG("FwVersion : %s",pFwVersion);
    // TRACE_DEBUG("FwSize : %d",*pFwSize);
    // TRACE_DEBUG("FwCrc : %0xX",*pFwCrc);

    return true;
}