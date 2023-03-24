#ifndef _FLASH_CONTROL_H_
#define _FLASH_CONTROL_H_

#include "device_common.h"
#include "aws_iot_config.h"
#include "config.h"

#define USER_FLASH_PAGE_SIZE          ( FLASH_PAGE_SIZE )       // 256B
#define USER_FLASH_SECTOR_SIZE        (  FLASH_SECTOR_SIZE )    // 4KB

#define CONFIG_AREA_SIZE            ( 0x40000 )     // 256KB
#define BOOT_LOADER_SIZE            ( 0x40000 )     // 256KB
#define APPLICATION_SIZE            ( 0x80000 )     // 512KB

#define FLASH_START_ADDR                        ( 0x10000000U)
#define FLASH_BOOT_START_ADDR                   ( FLASH_START_ADDR )                                    // 0x1000 0000
#define FLASH_APP_START_ADDR                    ( FLASH_BOOT_START_ADDR + BOOT_LOADER_SIZE )            // 0x1004 0000
#define FLASH_FACTORY_FW_START_ADDR             ( FLASH_APP_START_ADDR + APPLICATION_SIZE )             // 0x100C 0000
#define FLASH_TEMP_FW_START_ADDR                ( FLASH_FACTORY_FW_START_ADDR + APPLICATION_SIZE )      // 0x1014 0000

#define FLASH_COMMON_CONFIG_START_ADDR          ( FLASH_TEMP_FW_START_ADDR + APPLICATION_SIZE )         // 0x101C 0000
#define FLASH_CLAIM_CERT_INFO_START_ADDR        ( FLASH_COMMON_CONFIG_START_ADDR + 0x2000U)             // 0x101C 2000
#define FLASH_PROVIONED_CERT_INFO_START_ADDR    ( FLASH_CLAIM_CERT_INFO_START_ADDR + 0x1000U)           // 0x101C 3000

#define FLASH_PROVIONED_OWNERSHIP_START_ADDR    ( FLASH_PROVIONED_CERT_INFO_START_ADDR + 0x1000U)       // 0x101C 4000
#define FLASH_PROVIONED_CERT_ID_START_ADDR      ( FLASH_PROVIONED_OWNERSHIP_START_ADDR + 0x0200U)       // 0x101C 4200

void init_flash_cri_section   (void);

void erase_flash    (uint32_t offset, uint32_t size);
void write_flash    (uint32_t offset, uint8_t *pData, uint32_t dataSize);

void erase_flash_provisioned_cert_info  (void);
void erase_flash_common_config_info     (void);
//void erase_flash_ota_binary             (uint32_t binarySize);
void erase_flash_total_config_area      (void);
void erase_flash_total_factory_area     (void);
void erase_flash_total_ota_area         (void);

void save_flash_provisioned_cert_info   (AWS_FP_DEVICE_CERT_INFO_t *pFpCertInfo);
void save_flash_common_config_info      (APP_COMMON_t *pAppCommonInfo);

void write_flash_ota_binary             (uint32_t flashOffset, uint8_t *pBinary, uint32_t dataSize);

void load_flash_provisioned_cert_info       (AWS_FP_DEVICE_CERT_INFO_t *pFpCertInfo);
void load_flash_certificate_ownership_token (uint8_t *pData, uint16_t size);
void load_flash_certificate_cert_id         (uint8_t *pData, uint16_t size);
void load_flash_common_config_info          (APP_COMMON_t *pAppCommonInfo);

void print_flash_provioned_cert_info        (void);
void print_flash_common_config_info         (void);
void print_flash_ota_binary                 (void);

void copy_ota_area_data_to_app_area         (uint32_t dataSize);
void copy_factory_area_data_to_app_area     (uint32_t dataSize);
void copy_app_area_data_to_factory_area     (uint32_t dataSize);

bool    check_flash_common_config_info_empty  (void);
uint8_t* get_flash_ota_binary_start_address   (void);

#endif