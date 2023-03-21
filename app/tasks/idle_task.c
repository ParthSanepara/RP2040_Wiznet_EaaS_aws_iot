#include "idle_task.h"
#include "config.h"
#include "ethernet.h"
#include "mqtt_interface.h"
#include "w5x00_spi.h"
#include "device_common.h"
#include "dhcp.h"


void idle_task(void *pParam)
{
    bool dhcpStatus;
    uint32_t refreshDhcpTimeStamp, currentTimeStamp;
    uint32_t dhcpLeaseTimeS;
    
    APP_COMMON_t *pAppCommon = (APP_COMMON_t *)pParam;

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
                refreshDhcpTimeStamp = get_time_ms();
                TRACE_DEBUG("DHCP Lease time is over. Lease Time : %ds", dhcpLeaseTimeS);
                TRACE_DEBUG("Refresh DHCP. Refresh Timeout : %dms", (currentTimeStamp - refreshDhcpTimeStamp));
                dhcp_process(&pAppCommon->DEVICE_INFO.ETH_NET_INFO);
            }
        }
        /////////////////////////////////////////////////////////////////////////////////////


        // 디바이스 등록 확인






        DELAY_MS(100);
    }
}
