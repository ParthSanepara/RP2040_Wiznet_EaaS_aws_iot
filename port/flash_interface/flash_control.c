#include "flash_control.h"
#include "config.h"
#include "hardware/flash.h"
#include "pico/critical_section.h"


static critical_section_t g_flash_cri_sec;

void init_flash_cri_section(void)
{
    critical_section_init(&g_flash_cri_sec);    
}

void erase_flash(uint32_t offset, uint32_t dataSize)
{
    uint16_t eraseSectorCount=0;

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

void write_flash(uint32_t offset, uint8_t *pData, uint32_t dataSize)
{
    uint16_t writePageCount=0;

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("Writing. Offset : %lX, Page Size : %X, Write Count : %d", offset, USER_FLASH_PAGE_SIZE, writePageCount);
    critical_section_enter_blocking(&g_flash_cri_sec);
    flash_range_program(offset, (uint8_t *)pData, (writePageCount * USER_FLASH_PAGE_SIZE));
    critical_section_exit(&g_flash_cri_sec);    
}

void erase_flash_provisioned_cert_info(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_PROVIONED_CERT_INFO_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(AWS_FP_DEVICE_CERT_INFO_t);

    erase_flash(offset, dataSize);
}

void erase_flash_common_config_info(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_COMMON_CONFIG_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(APP_COMMON_t);

    erase_flash(offset, dataSize);
}

// void erase_flash_ota_binary(uint32_t binarySize)
// {
//     uint32_t offset;

//     offset = FLASH_TEMP_FW_START_ADDR - FLASH_START_ADDR;

//     erase_flash(offset, binarySize);
// }

void erase_flash_total_config_area(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_COMMON_CONFIG_START_ADDR - FLASH_START_ADDR;
    dataSize = CONFIG_AREA_SIZE;

    erase_flash(offset, dataSize);    
}

void erase_flash_total_app_area(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_APP_START_ADDR - FLASH_START_ADDR;
    dataSize = APPLICATION_SIZE;

    erase_flash(offset, dataSize);

    DELAY_MS(1000);
}

void erase_flash_total_factory_area(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_FACTORY_FW_START_ADDR - FLASH_START_ADDR;
    dataSize = APPLICATION_SIZE;

    erase_flash(offset, dataSize);
}

void erase_flash_total_ota_area(void)
{
    uint32_t offset, dataSize;

    offset = FLASH_TEMP_FW_START_ADDR - FLASH_START_ADDR;
    dataSize = APPLICATION_SIZE;

    erase_flash(offset, dataSize);
}

void save_flash_provisioned_cert_info(AWS_FP_DEVICE_CERT_INFO_t *pFpCertInfo)
{
    uint32_t offset, dataSize;

    erase_flash_provisioned_cert_info();

    offset = FLASH_PROVIONED_CERT_INFO_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(AWS_FP_DEVICE_CERT_INFO_t);

    write_flash(offset, (uint8_t*)pFpCertInfo, dataSize);
}

void save_flash_common_config_info(APP_COMMON_t *pAppCommonInfo)
{
    uint32_t offset, dataSize;

    erase_flash_common_config_info();

    offset = FLASH_COMMON_CONFIG_START_ADDR - FLASH_START_ADDR;
    dataSize = sizeof(APP_COMMON_t);

    write_flash(offset, (uint8_t*)pAppCommonInfo, dataSize);
}

void write_flash_ota_binary(uint32_t flashOffset, uint8_t *pBinary, uint32_t dataSize)
{
    uint32_t offset;
 
    offset = (FLASH_TEMP_FW_START_ADDR - FLASH_START_ADDR) + flashOffset;

    write_flash(offset, (uint8_t *)pBinary, dataSize);
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

void load_flash_common_config_info(APP_COMMON_t *pAppCommonInfo)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_COMMON_CONFIG_START_ADDR;
    MEMCPY(pAppCommonInfo, (uint8_t *)pFlashStartAddr, sizeof(APP_COMMON_t));
}

void print_flash_provioned_cert_info(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_PROVIONED_CERT_INFO_START_ADDR;
    print_dump_data(pFlashStartAddr, sizeof(AWS_FP_DEVICE_CERT_INFO_t));
}

void print_flash_common_config_info(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_COMMON_CONFIG_START_ADDR;
    print_dump_data(pFlashStartAddr, sizeof(APP_COMMON_t));
}

void print_flash_ota_binary(void)
{
    uint8_t *pFlashStartAddr = (uint8_t *)FLASH_TEMP_FW_START_ADDR;
    print_dump_data(pFlashStartAddr, 0x3000);
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

void copy_ota_area_data_to_app_area(uint32_t dataSize)
{
    uint32_t appAreaOffset, otaAreaOffset, i=0;
    uint16_t writePageCount = 0;
    uint8_t  tempBuffer[USER_FLASH_PAGE_SIZE];
    uint8_t *pOtaAreaData = (uint8_t *)FLASH_TEMP_FW_START_ADDR;

    appAreaOffset = (FLASH_APP_START_ADDR - FLASH_START_ADDR);
    otaAreaOffset = 0;
    
    TRACE_DEBUG("Erase App Area Flash");
    erase_flash_total_app_area();

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("APP Offset : %08lX, Write Page Count : %d", appAreaOffset, writePageCount);
    for(i=0; i<writePageCount; i++)
    {
        otaAreaOffset = (i * USER_FLASH_PAGE_SIZE);
        
        memset(tempBuffer, 0x00, sizeof(tempBuffer));
        memcpy(tempBuffer, (pOtaAreaData + otaAreaOffset), sizeof(tempBuffer));
    
        critical_section_enter_blocking(&g_flash_cri_sec);
        flash_range_program(appAreaOffset, (uint8_t *)tempBuffer, USER_FLASH_PAGE_SIZE);
        critical_section_exit(&g_flash_cri_sec);    

        appAreaOffset += USER_FLASH_PAGE_SIZE;
    }
}

void copy_factory_area_data_to_app_area(uint32_t dataSize)
{
    uint32_t appAreaOffset, factoryAreaOffset, i=0;
    uint16_t writePageCount = 0;
    uint8_t  tempBuffer[USER_FLASH_PAGE_SIZE];
    uint8_t *pFactoryAreaData = (uint8_t *)FLASH_FACTORY_FW_START_ADDR;

    appAreaOffset = (FLASH_APP_START_ADDR - FLASH_START_ADDR);
    factoryAreaOffset = 0;
    
    TRACE_DEBUG("Erase App Area Flash");
    erase_flash_total_app_area();

    writePageCount = dataSize / USER_FLASH_PAGE_SIZE;
    if( (dataSize % USER_FLASH_PAGE_SIZE) > 0 )
    {
        writePageCount++;
    }

    TRACE_DEBUG("APP Offset : %08lX, Write Page Count : %d", appAreaOffset, writePageCount);
    for(i=0; i<writePageCount; i++)
    {
        factoryAreaOffset = (i * USER_FLASH_PAGE_SIZE);
        
        memset(tempBuffer, 0x00, sizeof(tempBuffer));
        memcpy(tempBuffer, (pFactoryAreaData + factoryAreaOffset), sizeof(tempBuffer));
    
        critical_section_enter_blocking(&g_flash_cri_sec);
        flash_range_program(appAreaOffset, (uint8_t *)tempBuffer, USER_FLASH_PAGE_SIZE);
        critical_section_exit(&g_flash_cri_sec);    

        appAreaOffset += USER_FLASH_PAGE_SIZE;
    }    
}




uint8_t* get_flash_ota_binary_start_address(void)
{
    return (uint8_t *)FLASH_TEMP_FW_START_ADDR;
}



