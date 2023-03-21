#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "aws_iot_config.h"
#include "wizchip_conf.h"

#define DISABLE_START_FW_OTA    0
#define ENABLE_START_FW_OTA     1

#pragma pack(push, 4)

typedef enum
{
    START_FW_OTA_DISABLE = 0,
    START_FW_OTA_ENABLE  = 1
} OTA_STATUS_t;

typedef struct
{
    wiz_NetInfo ETH_NET_INFO;

    uint8_t THING_NAME[64];
    uint8_t LOCATION[64];
    uint8_t MANUFACTURER[64];

    uint8_t AWS_END_POINT[128];
    uint8_t AWS_TEMPLATE_NAME[64];

    OTA_STATUS_t START_FW_OTA_STATUS;
    
    uint8_t CERT_OWNERSHIP_TOKEN[MAX_FP_CERT_OWNERSHIP_TOKEN_SIZE];
} DEVICE_INFO_t;

typedef enum
{
    AWS_IOT_JOBS_DISCONNECTED = 0,
    AWS_IOT_JOBS_CONNECTED = 1,
} AWS_IOT_JOBS_STATUS_t;

typedef struct 
{
    DEVICE_INFO_t           DEVICE_INFO;
    AWS_IOT_JOBS_STATUS_t   AWS_IOT_JOBS_STATUS; 

    bool isWizChipLinkUp;
    bool isDhcpDone;

    bool isValidClaimCertInfo;
    bool isValidProvisionedCertInfo;

    bool isRunAwsIotJobs;

} APP_COMMON_t;

#pragma pack(pop)

#endif
