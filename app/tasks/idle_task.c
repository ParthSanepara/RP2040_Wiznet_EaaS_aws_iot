#include "idle_task.h"
#include "os_common.h"
#include "config.h"
#include "ethernet.h"
#include "mqtt_interface.h"
#include "w5x00_spi.h"
#include "device_common.h"
#include "dhcp.h"
#include "led_control.h"

#define LED_BLINK_TIME_MS   1000

void idle_task(void *pParam)
{
    bool dhcpStatus;
    uint32_t refreshDhcpTimeStamp, runStatusLedToggleTimeStamp, currentTimeStamp;
    uint32_t dhcpLeaseTimeS;

    APP_COMMON_t *pAppCommon = (APP_COMMON_t *)pParam;

    init_run_status_led();

    runStatusLedToggleTimeStamp = get_time_ms();
    while(1)
    {
        currentTimeStamp = get_time_ms();

        // Check ethernet link status
        if(get_phy_link_status() == false)
        {
            pAppCommon->isWizChipLinkUp = false;
            pAppCommon->isDhcpDone = false;
        }
        else
        {
            pAppCommon->isWizChipLinkUp = true;
        }

        // RUN LED TOGGLE
        if( (currentTimeStamp - runStatusLedToggleTimeStamp) > LED_BLINK_TIME_MS )
        {
            toggle_run_status_led();
            runStatusLedToggleTimeStamp = get_time_ms();
        }

        /////////////////////////////////////////////////////////////////////////////////////
        // DHCP
        if(pAppCommon->isDhcpDone == false && pAppCommon->isWizChipLinkUp == true)
        {
            dhcpStatus = dhcp_process(&pAppCommon->DEVICE_INFO.ETH_NET_INFO);
            if(dhcpStatus == true)
            {
                pAppCommon->isDhcpDone = true;
                refreshDhcpTimeStamp = get_time_ms();
            }
        }
        // DHCP 갱신
        else if(pAppCommon->isDhcpDone == true && pAppCommon->isWizChipLinkUp == true)
        {
            dhcpLeaseTimeS = getDHCPLeasetime();

            if( (currentTimeStamp - refreshDhcpTimeStamp) > dhcpLeaseTimeS * 1000 )
            {
                TRACE_DEBUG("DHCP Lease time is over. Lease Time : %ds", dhcpLeaseTimeS);
                TRACE_DEBUG("Refresh DHCP. Refresh Timeout : %dms", (currentTimeStamp - refreshDhcpTimeStamp));
                TRACE_DEBUG("DHCP Stop");
                DHCP_stop();
                pAppCommon->isDhcpDone = false;

                dhcpStatus = dhcp_process(&pAppCommon->DEVICE_INFO.ETH_NET_INFO);
                if(dhcpStatus == true)
                {
                    pAppCommon->isDhcpDone = true;
                    refreshDhcpTimeStamp = get_time_ms();
                }
            }
        }
        /////////////////////////////////////////////////////////////////////////////////////

        OS_DELAY_MS(100);
    }
}
