#ifndef AWS_IOT_IOT_JOBS_JOB_OTA_HANDLER_H_
#define AWS_IOT_IOT_JOBS_JOB_OTA_HANDLER_H_

#include "aws_iot_config.h"
#include "config.h"

int32_t ota_http_send_request_callback  (TransportInterface_t *pTransportInterface, char *pMethod, http_config_t *http_config);

bool ota_procedure          (uint8_t *pDownloadUrl, uint32_t fwSize, uint16_t fwCrc);
bool ota_download_firmware  (uint8_t *pUrl, uint32_t fwSize, uint16_t fwCrc);

#endif