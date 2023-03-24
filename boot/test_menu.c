#include <stdio.h>
#include "device_common.h"
#include "test_menu.h"
#include "flash_control.h"
#include "port_common.h"

void display_menu(void)
{
    TRACE_MSG("==========================================");
    TRACE_MSG("1  : Erase Flash Menu");
    TRACE_MSG("2  : Print Flash Menu");
    TRACE_MSG("3  : Factory Fw Menu");
    TRACE_MSG("q  : Quit");
    TRACE_MSG("==========================================");
    TRACE_MSG("Input Key : ");

}

void display_erase_flash_menu(void)
{
    TRACE_MSG("==========================================");
    TRACE_MSG("1  : Erase Config Area");
    TRACE_MSG("2  : Erase App Area");
    TRACE_MSG("3  : Erase Factory Area");
    TRACE_MSG("4  : Erase OTA Area");
    TRACE_MSG("5  : Erase Provisioned Cert Info");
    TRACE_MSG("q  : Quit");
    TRACE_MSG("==========================================");
    TRACE_MSG("Input Key : ");
}

void display_print_flash_menu(void)
{
    TRACE_MSG("==========================================");
    TRACE_MSG("1  : Print Config Area");
    TRACE_MSG("2  : Print Provisoned Cert Area");
    TRACE_MSG("3  : Print App Area");
    TRACE_MSG("4  : Print Factory Area");
    TRACE_MSG("5  : Print OTA Area");
    TRACE_MSG("q  : Quit");
    TRACE_MSG("==========================================");
    TRACE_MSG("Input Key : ");
}

void display_factory_fw_menu(void)
{
    TRACE_MSG("==========================================");
    TRACE_MSG("1  : Copy App Area to Factory Area");
    TRACE_MSG("2  : Copy Factory Area to App Area");
    TRACE_MSG("q  : Quit");
    TRACE_MSG("==========================================");
    TRACE_MSG("Input Key : ");
}

void procedure_erase_flash(void)
{
    int ch;

    display_erase_flash_menu();
    while(1)
    {
        ch = getchar_timeout_us(1000);
        if(ch < 0)
        {
            continue;
        }

        if(ch == KEY_VALUE_QUIT)
        {
            break;
        }
        else if(ch == KEY_VALUE_ERASE_CONFIG_FLASH)
        {
            erase_flash_total_config_area();
        }
        else if(ch == KEY_VALUE_ERASE_APP_FLASH)
        {
            erase_flash_total_config_area();
        }
        else if(ch == KEY_VALUE_ERASE_FACTORY_FLASH)
        {
            erase_flash_total_factory_area();
        }
        else if(ch == KEY_VALUE_ERASE_OTA_FLASH)
        {
            erase_flash_total_ota_area();
        }
        else if(ch == KEY_VALUE_ERASE_PROVISONED_CERT)
        {
            erase_flash_provisioned_cert_info();
        }
    }
}

void procedure_print_flash(void)
{
    int ch;

    display_print_flash_menu();
    while(1)
    {
        ch = getchar_timeout_us(1000);
        if(ch < 0)
        {
            continue;
        }

        if(ch == KEY_VALUE_QUIT)
        {
            break;
        }
        else if(ch == KEY_VALUE_PRINT_CONFIG_FLASH)
        {
            print_dump_data((uint8_t*)FLASH_COMMON_CONFIG_START_ADDR, 0x100);
        }
        else if(ch == KEY_VALUE_PRINT_PROVISONED_CERT_FLASH)
        {
            print_dump_data((uint8_t*)FLASH_PROVIONED_CERT_ID_START_ADDR, 0x100);
        }
        else if(ch == KEY_VALUE_PRINT_APP_FLASH)
        {
            print_dump_data((uint8_t*)FLASH_APP_START_ADDR, 4000);
        }
        else if(ch == KEY_VALUE_PRINT_FACTORY_FLASH)
        {
            print_dump_data((uint8_t*)FLASH_FACTORY_FW_START_ADDR, 0x100);
        }
        else if(ch == KEY_VALUE_PRINT_OTA_FLASH)
        {
            print_dump_data((uint8_t*)FLASH_TEMP_FW_START_ADDR, 0x100);
        }

        display_print_flash_menu();
    }
}

void procedure_factory_fw_(void)
{
    int ch;

    display_factory_fw_menu();
    while(1)
    {
        ch = getchar_timeout_us(1000);
        if(ch < 0)
        {
            continue;
        }

        if(ch == KEY_VALUE_QUIT)
        {
            break;
        }
        else if(ch == KEY_VALUE_COPY_APP_AREA_TO_FACTORY_AREA)
        {
            copy_app_area_data_to_factory_area(APPLICATION_SIZE);
        }
        else if(ch == KEY_VALUE_COPY_FACTORY_AREA_TO_APP_AREA)
        {
            copy_factory_area_data_to_app_area(APPLICATION_SIZE);
        }

        display_factory_fw_menu();
    }
}