
#include <stdio.h>
#include "device_common.h"
#include "port_common.h"
#include "hardware/clocks.h"
#include "flash_control.h"
#include "system_common.h"
#include "ethernet.h"

#include <FreeRTOS.h>
#include <task.h>

#include "main_app_task.h"
#include "aws_iot_jobs_task.h"
#include "idle_task.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Task Priority */
#define IDLE_TASK_PRIORITY      3
#define MAIN_TASK_PRIORITY      3
#define IOT_JOBS_TASK_PRIORITY  3


/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

APP_COMMON_t            appCommon;

/* Network */
static wiz_NetInfo default_net_info =
{
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
    .ip = {192, 168, 11, 2},                     // IP address
    .sn = {255, 255, 255, 0},                    // Subnet Mask
    .gw = {192, 168, 11, 1},                     // Gateway
    .dns = {8, 8, 8, 8},                         // DNS server
    .dhcp = NETINFO_DHCP                         // DHCP enable/disable
};

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Task */
void idle_task(void *pParam);
void main_task(void *pParam);

/* Clock */
static void set_clock_khz(void);

void set_default_net_info(wiz_NetInfo *pNetInfo)
{
    memcpy(pNetInfo, &default_net_info, sizeof(default_net_info));
}

void set_default_device_info(DEVICE_INFO_t *pDevieInfo)
{
    uint8_t thingName[100]={0,};
    uint8_t tempMac[13]={0,};

    set_default_net_info(&pDevieInfo->ETH_NET_INFO);

    sprintf(tempMac, "%02X%02X%02X%02X%02X%02X", pDevieInfo->ETH_NET_INFO.mac[0], pDevieInfo->ETH_NET_INFO.mac[1],
                                                 pDevieInfo->ETH_NET_INFO.mac[2], pDevieInfo->ETH_NET_INFO.mac[3],
                                                 pDevieInfo->ETH_NET_INFO.mac[4], pDevieInfo->ETH_NET_INFO.mac[5]
    );
    sprintf(thingName, AWS_THING_NAME_PREFIX, tempMac);

    strncpy(pDevieInfo->THING_NAME, thingName, strlen(thingName));
    strncpy(pDevieInfo->LOCATION, DEFAULT_DEVICE_LOCATION, sizeof(DEFAULT_DEVICE_LOCATION));
    strncpy(pDevieInfo->MANUFACTURER, DEFAULT_MANUFACTURER, sizeof(DEFAULT_MANUFACTURER));

    strncpy(pDevieInfo->AWS_END_POINT, DEFAULT_AWS_MQTT_END_POINT, strlen(DEFAULT_AWS_MQTT_END_POINT));
    strncpy(pDevieInfo->AWS_TEMPLATE_NAME, DEFAULT_AWS_TEMPLATE_NAME, strlen(DEFAULT_AWS_TEMPLATE_NAME));
}


/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    set_clock_khz();

    stdio_init_all();

    logger_mutex_init();
    init_flash_cri_section();
    
    print_app_info();

    memset(&appCommon, 0x00, sizeof(appCommon));

    // ToDo : 나중에는 플래시에서 읽어 오도록 수정
    if( check_flash_common_config_info_empty() == true )
    {
        set_default_device_info(&appCommon.DEVICE_INFO);
        save_flash_common_config_info(&appCommon);
    }
    else
    {
        load_flash_common_config_info(&appCommon);
    }

    xTaskCreate(main_app_task,      "MAIN_TASK",        (50 * 256), &appCommon, MAIN_TASK_PRIORITY, NULL);
    xTaskCreate(aws_iot_jobs_task,  "IoT_JOBS_TASK",    (50 * 256), &appCommon, IOT_JOBS_TASK_PRIORITY, NULL);
    xTaskCreate(idle_task,          "IDLE_TASK",        (2 * 256), &appCommon, IDLE_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    return 0;
}


/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

// void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
// {
//     printf("STACK OVERFLOW. %s",*pcTaskName);
// }