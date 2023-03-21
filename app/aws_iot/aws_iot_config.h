#ifndef _AWS_IOT_AWS_IOT_CONFIG_H_
#define _AWS_IOT_AWS_IOT_CONFIG_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "http_interface.h"

//#define AWS_MQTT_DOMAIN                     "a3uz5t2azg1xdz-ats.iot.ap-northeast-2.amazonaws.com"

//#define AWS_TEMPLATE_NAME                   "wiznet-service-template"
#define AWS_MQTT_USERNAME                   NULL
#define AWS_MQTT_PASSWORD                   NULL
#define AWS_MQTT_CLIENT_ID                  "test_1234"

#define AWS_MQTT_PORT                       8883

#define MAX_PEM_SIZE                        2048
#define MAX_FP_CERT_ID_SIZE                 256
#define MAX_FP_CERT_OWNERSHIP_TOKEN_SIZE    512

#define MAX_MQTT_CLIENT_ID_SIZE             50
#define MAX_MQTT_TOPIC_SIZE                 256
#define MAX_AWS_OTA_URL_SIZE                HTTP_URL_MAX_SIZE
#define MAX_AWS_JOB_LIST_SIZE               5
#define MAX_AWS_JOB_ID_SIZE                 64
#define MAX_AWS_JOB_DOCUMENT_SIZE           4096

#define MAX_AWS_JOB_TYPE_SIZE               64      // Wiznet Custom
#define MAX_AWS_JOB_FIRMWARE_VERSION_SIZE   64      // Wiznet Custom

#define AWS_JOB_TYPE_OTA    "OTA"
#define AWS_JOB_TYPE_TEST   "TEST"                  // For Dummy


#define DEFAULT_AWS_MQTT_END_POINT          "a3uz5t2azg1xdz-ats.iot.ap-northeast-2.amazonaws.com"
#define DEFAULT_AWS_TEMPLATE_NAME           "wiznet-service-template"

#define DEFAULT_DEVICE_LOCATION             "Seoul"
#define DEFAULT_MANUFACTURER                "WIZnet"

#define AWS_THING_NAME_PREFIX              "eaas-device-%s"         // (MAC ADDRESS)

// #define MQTT_CLIENT_ID_PREFIX               "wiznet_eaas_%s"         // (THING_NAME)


// #define FP_API_CSR_KEY                "certificateSigningRequest"
#define FP_API_OWNERSHIP_TOKEN_KEY    "certificateOwnershipToken"
#define FP_API_CERTIFICATE_ID_KEY     "certificateId"
#define FP_API_CERTIFICATE_PEM_KEY    "certificatePem"
#define FP_API_PRIVATE_KEY_KEY        "privateKey"
// #define FP_API_PARAMETERS_KEY         "parameters"
// #define FP_API_DEVICE_CONFIG_KEY      "deviceConfiguration"
// #define FP_API_THING_NAME_KEY         "thingName"
// #define FP_API_STATUS_CODE_KEY        "statusCode"
// #define FP_API_ERROR_CODE_KEY         "errorCode"
// #define FP_API_ERROR_MESSAGE_KEY      "errorMessage"

#define JOBS_API_STATUS_IN_PROGRESS    "IN_PROGRESS" /*!< The job document has be received on the device and update is in progress. */
#define JOBS_API_STATUS_FAILED         "FAILED"      /*!< OTA update failed due to an error. */
#define JOBS_API_STATUS_SUCCEEDED      "SUCCEEDED"   /*!< OTA update succeeded. */
#define JOBS_API_STATUS_REJECTED       "REJECTED"    /*!< The job was rejected due to invalid parameters. */


typedef struct 
{
    uint8_t DEVICE_ROOT_CA          [MAX_PEM_SIZE];
    uint8_t DEVICE_CLAIM_CERT       [MAX_PEM_SIZE];
    uint8_t DEVICE_CLAIM_PRIVATE_KEY[MAX_PEM_SIZE];
} AWS_CLAIM_CERT_INFO_t;

typedef struct
{
    uint8_t DEVICE_PROVISIONED_CERT         [MAX_PEM_SIZE];                 // 0x0000
    uint8_t DEVICE_PROVISIONED_PRIVATE_KEY  [MAX_PEM_SIZE];                 // 0x0800
} AWS_PROVISONED_CERT_INFO_t;

typedef struct
{
    AWS_PROVISONED_CERT_INFO_t PROVISONED_CERT_INFO;                         // 0x0000

    uint8_t DEVICE_CERT_OWNERSHIP_TOKEN [MAX_FP_CERT_OWNERSHIP_TOKEN_SIZE];  // 0x1000
    uint8_t DEVICE_CERT_ID              [MAX_FP_CERT_ID_SIZE];               // 0x1200
} AWS_FP_DEVICE_CERT_INFO_t;

typedef struct
{
    uint8_t     FIRMWARE_URL[MAX_AWS_OTA_URL_SIZE];
    uint16_t    FIRMWARE_CRC;
    uint8_t     FIRMWARE_VERSION[MAX_AWS_JOB_FIRMWARE_VERSION_SIZE];
    uint32_t    FIRMWARE_SIZE;
} AWS_OTA_PARAMS_t;

typedef struct
{
    bool    IS_JOB_STARTED;
    uint8_t JOB_TYPE[MAX_AWS_JOB_TYPE_SIZE];

    uint8_t JOB_ID[MAX_AWS_JOB_ID_SIZE];
    uint8_t JOB_DOCUMENT[MAX_AWS_JOB_DOCUMENT_SIZE];

    AWS_OTA_PARAMS_t    JOB_OTA;
} AWS_JOBS_EXECUTION_PARAMS_t;


#endif