#ifndef AWS_IOT_FLEET_PROVISIONING_HANDLER_H_
#define AWS_IOT_FLEET_PROVISIONING_HANDLER_H_

#include "aws_iot_config.h"
#include "config.h"

bool parsing_accept_certificate_create_msg(uint8_t *pPaylod, uint16_t paylodLength, AWS_FP_DEVICE_CERT_INFO_t *pAwsFpDeviceCertInfo);

bool processCreateCertAndKey    (DEVICE_INFO_t *pDeviceInfo, uint8_t qos);
bool processRegisterThing       (DEVICE_INFO_t *pDeviceInfo, uint8_t qos);


bool fleet_provisioning_handle          (DEVICE_INFO_t *pDeviceInfo);
bool waitRecvCreateCertAndKeyAcceptMsg  (uint32_t timeoutMs);

#endif