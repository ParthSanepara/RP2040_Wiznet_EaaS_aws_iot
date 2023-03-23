#ifndef AWS_IOT_IOT_JOBS_HANDLER_H_
#define AWS_IOT_IOT_JOBS_HANDLER_H_

#include "aws_iot_config.h"
#include "config.h"




bool iot_jobs_connect_procedure     (DEVICE_INFO_t *pDeviceInfo);
void iot_jobs_close_procedure       (DEVICE_INFO_t *pDeviceInfo);

bool iot_jobs_get_procedure                 (DEVICE_INFO_t *pDeviceInfo);
bool iot_jobs_start_job_document_procedure  (APP_COMMON_t *pAppCommon);
bool start_job_document                     (APP_COMMON_t *pAppCommon, AWS_JOBS_EXECUTION_PARAMS_t *pJobExecutionParam);

bool iot_jobs_start_next_procedure          (DEVICE_INFO_t *pDeviceInfo);
bool iot_jobs_update_job_status_procedure   (DEVICE_INFO_t *pDeviceInfo, uint8_t *pJobId, uint8_t *pJobStatus);

bool iot_jobs_mqtt_connect  (DEVICE_INFO_t *pDeviceInfo);
bool iot_jobs_mqtt_close    (void);

bool iot_jobs_mqtt_subscribe_jobs_get_topic     (uint8_t *pThingName, uint8_t qos);
bool iot_jobs_mqtt_unsubscribe_jobs_get_topic   (uint8_t *pThingName);

bool iot_jobs_mqtt_subscribe_jobs_start_next_topic  (uint8_t *pThingName, uint8_t qos);
bool iot_jobs_mqtt_unsubscribe_jobs_start_next_topic(uint8_t *pThingName);

bool iot_jobs_mqtt_subscribe_jobs_update_topic  (uint8_t *pThingName, uint8_t *pJobId, uint8_t qos);
bool iot_jobs_mqtt_unsubscribe_jobs_update_topic(uint8_t *pThingName, uint8_t *pJobId);

bool iot_jobs_mqtt_wait_recv_get_job                (uint32_t timeoutMs);
bool iot_jobs_mqtt_wait_recv_start_next_job         (uint32_t timeoutMs);
bool iot_jobs_mqtt_wait_recv_job_update_accepted    (uint32_t timeoutMs);

bool parsing_get_job_accepted           (   uint8_t *pPayload, uint16_t payloadLength, uint8_t *pJobId, uint16_t maxJobIdLength);

bool parsing_start_next_job_accepted    (   uint8_t *pPayload, uint16_t payloadLength, 
                                            uint8_t *pJobId, uint16_t maxJobIdLength, 
                                            uint8_t *pJobDocument, uint16_t maxJobDocumentLength,
                                            uint8_t *pJobType, uint16_t maxJobTypeLength );

bool parsing_ota_job_params             (   uint8_t *pPayload, uint16_t payloadLength,
                                            uint8_t *pFwUrl, uint16_t maxFwUrlLength,
                                            uint8_t *pFwVersion, uint16_t maxFwVersionLength,
                                            uint32_t *pFwSize, uint16_t *pFwCrc
                                        );


#endif