#include <stdio.h>
#include "device_common.h"
#include "port_common.h"
#include "hardware/clocks.h"
#include "flash_control.h"
#include "system_common.h"

#define JUMP_APP_TIMEOUT_MS             1000

#define KEY_VALUE_BOOT_DOWN_ENTER       '!'


/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

APP_COMMON_t            appCommon;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    uint32_t startTimeoutMs, currentTimeoutMs;
    int ch;

    /* Initialize */
    set_clock_khz();

    stdio_init_all();

    logger_mutex_init();
    init_flash_cri_section();

    print_boot_info();

    memset(&appCommon, 0x00, sizeof(appCommon));
    load_flash_common_config_info(&appCommon);

    startTimeoutMs = get_time_ms();
    TRACE_MSG("Jump to App after %ldms", (uint32_t)JUMP_APP_TIMEOUT_MS);
    while(1)
    {
        currentTimeoutMs = get_time_ms();
        if( (currentTimeoutMs - startTimeoutMs) > JUMP_APP_TIMEOUT_MS )
        {
            break;
        }
        else if( (currentTimeoutMs - startTimeoutMs) > 100 )
        {
            TRACE_MSG_WITHOUT_NL("\b\b\b\b\b\b\b\b%ldms", (currentTimeoutMs - startTimeoutMs));
        }

        if(appCommon.DEVICE_INFO.START_FW_OTA_STATUS == START_FW_OTA_ENABLE)
        {
            // Start OTA
            printf("Start OTA\r\n");
        }


        // Wait 1MS
        ch = getchar_timeout_us(1000);
        if(ch < 0)
        {
            continue;
        }

        if(ch == KEY_VALUE_BOOT_DOWN_ENTER)
        {
            printf("Get C\r\n");
        }
        else if(ch == '1')
        {
            uint8_t *pFlashStartAddr = (uint8_t *)FLASH_APP_START_ADDR;
            print_dump_data(pFlashStartAddr, 100);

        }
        
        
        
        
    }

    printf("\r\nJump Addr = 0x%08X\r\n\r\n", FLASH_APP_START_ADDR);
    DELAY_MS_NON_OS(100);

    disable_interrupts();
    jump_to_app(FLASH_APP_START_ADDR);

    while(1)
    {

    }

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
