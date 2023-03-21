#include "flash_control.h"
#include "config.h"
#include "hardware/flash.h"
#include "pico/critical_section.h"


static critical_section_t g_flash_cri_sec;

void init_flash_cri_section(void)
{
    critical_section_init(&g_flash_cri_sec);    
}

void load_flash_provisioned_cert_info(AWS_FP_DEVICE_CERT_INFO_t *pFpCertInfo)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_PROVIONED_CERT_INFO_START_ADDR;
    MEMCPY(pFpCertInfo, (uint8_t *)pFlashStartAddr, sizeof(AWS_FP_DEVICE_CERT_INFO_t));
}

void load_flash_certificate_ownership_token(uint8_t *pData, uint16_t size)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_PROVIONED_OWNERSHIP_START_ADDR;
    MEMCPY(pData, (uint8_t *)pFlashStartAddr, size);
}

void load_flash_certificate_cert_id(uint8_t *pData, uint16_t size)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_PROVIONED_CERT_ID_START_ADDR;
    MEMCPY(pData, (uint8_t *)pFlashStartAddr, size);
}

void erase_flash_provisioned_cert_info(void)
{
    uint32_t offset, dataSize;
    uint16_t eraseSectorCount=0;

    offset = FLASH_PROVIONED_CERT_INFO_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(AWS_FP_DEVICE_CERT_INFO_t);

    eraseSectorCount = dataSize / USER_FLASH_SECTOR_SIZE;
    if ( (dataSize % USER_FLASH_SECTOR_SIZE) > 0 )
    {
        eraseSectorCount++;
    }

    TRACE_DEBUG("Erasing. Offset : %lX, Sector Size : %X, Erase Count : %d", offset, USER_FLASH_SECTOR_SIZE, eraseSectorCount);

    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_erase(offset, ( eraseSectorCount * USER_FLASH_SECTOR_SIZE) );
    critical_section_exit(&g_flash_cri_sec);    
}

void save_flash_provisioned_cert_info(AWS_FP_DEVICE_CERT_INFO_t *pFpCertInfo)
{
    uint32_t offset, dataSize;
    uint16_t writePageCount=0 ;

    erase_flash_provisioned_cert_info();

    offset = FLASH_PROVIONED_CERT_INFO_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(AWS_FP_DEVICE_CERT_INFO_t);

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("Writing. Offset : %lX, Page Size : %X, Write Count : %d", offset, USER_FLASH_PAGE_SIZE, writePageCount);
    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_program(offset, (uint8_t *)pFpCertInfo, (writePageCount * USER_FLASH_PAGE_SIZE));
    critical_section_exit(&g_flash_cri_sec);    
}

void print_flash_provioned_cert_info(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_PROVIONED_CERT_INFO_START_ADDR;
    print_dump_data(pFlashStartAddr, sizeof(AWS_FP_DEVICE_CERT_INFO_t));
}

void erase_flash_common_config_info(void)
{
    uint32_t offset, dataSize;
    uint16_t eraseSectorCount=0;

    offset = FLASH_COMMON_CONFIG_START_ADDR - FLASH_START_ADDR;

    dataSize = sizeof(APP_COMMON_t);
    eraseSectorCount = dataSize / USER_FLASH_SECTOR_SIZE;
    if ( (dataSize % USER_FLASH_SECTOR_SIZE) > 0 )
    {
        eraseSectorCount++;
    }

    TRACE_DEBUG("Erasing. Offset : %lX, Sector Size : %X, Erase Count : %d", offset, USER_FLASH_SECTOR_SIZE, eraseSectorCount);

    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_erase(offset, (eraseSectorCount * USER_FLASH_SECTOR_SIZE) );
    critical_section_exit(&g_flash_cri_sec);    
}

void load_flash_common_config_info(APP_COMMON_t *pAppCommonInfo)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_COMMON_CONFIG_START_ADDR;
    MEMCPY(pAppCommonInfo, (uint8_t *)pFlashStartAddr, sizeof(APP_COMMON_t));
}

void save_flash_common_config_info(APP_COMMON_t *pAppCommonInfo)
{
    uint32_t offset, dataSize;
    uint16_t writePageCount=0 ;

    erase_flash_common_config_info();

    offset = FLASH_COMMON_CONFIG_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(APP_COMMON_t);

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("Writing. Offset : %lX, Page Size : %X, Write Count : %d", offset, USER_FLASH_PAGE_SIZE, writePageCount);
    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_program(offset, (uint8_t *)pAppCommonInfo, (writePageCount * USER_FLASH_PAGE_SIZE));
    critical_section_exit(&g_flash_cri_sec);    
}

void print_flash_common_config_info(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_COMMON_CONFIG_START_ADDR;
    print_dump_data(pFlashStartAddr, sizeof(APP_COMMON_t));
}

bool check_flash_common_config_info_empty(void)
{
    APP_COMMON_t emptyCommonInfo;
    APP_COMMON_t *pCommonInfo = (APP_COMMON_t *)FLASH_COMMON_CONFIG_START_ADDR;

    MEMSET(&emptyCommonInfo, 0xFF, sizeof(APP_COMMON_t));

    if(MEMCMP(pCommonInfo, &emptyCommonInfo, sizeof(APP_COMMON_t)) != 0)
    {
        return false;
    }

    // DATA IS EMPTY
    return true;
}

void erase_flash_ota_binary(uint32_t binarySize)
{
    uint32_t offset;
    uint16_t eraseSectorCount=0;

    offset = FLASH_TEMP_FW_START_ADDR - FLASH_START_ADDR;

    eraseSectorCount = binarySize / USER_FLASH_SECTOR_SIZE;
    if ( (binarySize % USER_FLASH_SECTOR_SIZE) > 0 )
    {
        eraseSectorCount++;
    }

    TRACE_DEBUG("Erasing. Offset : %lX, Sector Size : %X, Erase Count : %d", offset, USER_FLASH_SECTOR_SIZE, eraseSectorCount);

    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_erase(offset, (eraseSectorCount * USER_FLASH_SECTOR_SIZE));
    critical_section_exit(&g_flash_cri_sec);    
}

void write_flash_ota_binary(uint32_t flashOffset, uint8_t *pBinary, uint16_t dataSize)
{
    uint32_t offset;
    uint16_t writePageCount=0 ;

    offset = (FLASH_TEMP_FW_START_ADDR - FLASH_START_ADDR) + flashOffset;

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("Writing. Offset : %lX, Page Size : %X, Write Count : %d", offset, USER_FLASH_PAGE_SIZE, writePageCount);

    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_program(offset, (uint8_t *)pBinary, (writePageCount * USER_FLASH_PAGE_SIZE));
    critical_section_exit(&g_flash_cri_sec);    
}

void print_flash_ota_binary(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_TEMP_FW_START_ADDR;
    print_dump_data(pFlashStartAddr, 0x3000);
}

uint8_t* get_flash_ota_binary(void)
{
    return (uint8_t *)FLASH_TEMP_FW_START_ADDR;

}


