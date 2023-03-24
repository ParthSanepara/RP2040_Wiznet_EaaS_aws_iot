#include "ethernet.h"
#include "dhcp.h"
#include "dns.h"
#include "w5x00_spi.h"
#include "timer.h"
#include "logger.h"
#include "device_common.h"
#include "os_common.h"

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)


/* Parameters for DHCP */
#define DHCP_RETRY_COUNT 5
#define DHCP_RETRY_TIMEOUT_MS  10000

#define DNS_RETRY_COUNT 5

static wiz_NetInfo g_temp_net_info_for_dhcp;


static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {0,};

/* DHCP */
static volatile uint32_t g_msec_cnt = 0;

static void repeating_timer_callback(void);


void init_ethernet(wiz_NetInfo *pNetInfo)
{
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);
    MEMCPY(&g_temp_net_info_for_dhcp, pNetInfo, sizeof(g_temp_net_info_for_dhcp));
}


/* DHCP */
static void wizchip_dhcp_assign(void)
{
    TRACE_DEBUG("DHCP assigned");
    
    getIPfromDHCP(g_temp_net_info_for_dhcp.ip);
    getGWfromDHCP(g_temp_net_info_for_dhcp.gw);
    getSNfromDHCP(g_temp_net_info_for_dhcp.sn);
    getDNSfromDHCP(g_temp_net_info_for_dhcp.dns);

    g_temp_net_info_for_dhcp.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_temp_net_info_for_dhcp); // apply from DHCP

    TRACE_DEBUG("DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void)
{
    TRACE_ERROR("Conflict IP from DHCP\n");
}

static void wizchip_dhcp_init(void)
{
    TRACE_DEBUG("DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}


bool dhcp_process(wiz_NetInfo *pNetInfo)
{
    uint8_t retval;
    int8_t  link;

    uint32_t dhcpRetryCnt = 0;
    uint32_t startTimeStamp = 0;
    uint32_t currentTimeStamp = 0;

    if(pNetInfo->dhcp == NETINFO_DHCP)
    {
        wizchip_dhcp_init();
    }
    else
    {
        network_initialize(*pNetInfo);

        /* Get network information */
        display_network_information(pNetInfo);

        goto DHCP_PROCESS_DONE;
    }

    startTimeStamp = get_time_ms();
    while(1)
    {
        OS_DELAY_MS(100);

        link = wizphy_getphylink();
        if( link == PHY_LINK_OFF )
        {
            TRACE_ERROR("Phy Link status is down");
            return false;
        }

        currentTimeStamp = get_time_ms();
        if( (currentTimeStamp - startTimeStamp) > DHCP_RETRY_TIMEOUT_MS )
        {
            TRACE_WARN("DHCP Retry Timeout. %dms", (currentTimeStamp - startTimeStamp));
            TRACE_WARN("PHY Link Status : %d",link);
            return false;
        }

        // if(link == PHY_LINK_OFF)
        // {
        //     DHCP_stop();
        //     continue;
        // }

        // DHCP 완료 될 때 까지 DHCP_run()을 계속 호출 해야 함.
        retval = DHCP_run();
        if(dhcpRetryCnt > DHCP_RETRY_COUNT)
        {
            TRACE_WARN("Try to dhcp over max retry count : %d",dhcpRetryCnt);
            return false;
        }

        if(retval == DHCP_FAILED)
        {
            dhcpRetryCnt++;
            TRACE_WARN("Fail to DHCP. Retry Count : %d",dhcpRetryCnt);
            DHCP_stop();
            TRACE_DEBUG("Wait 500MS");
            OS_DELAY_MS(500);
            continue;
        }
        else if(retval == DHCP_IP_LEASED)
        {
            TRACE_DEBUG("Success to DHCP");
            break;
        }
    }

DHCP_PROCESS_DONE:
    //MEMCPY(&g_temp_net_info_for_dhcp, pNetInfo, sizeof(g_temp_net_info_for_dhcp));
    MEMCPY(&pNetInfo->ip, &g_temp_net_info_for_dhcp.ip, sizeof(g_temp_net_info_for_dhcp.ip));
    MEMCPY(&pNetInfo->gw, &g_temp_net_info_for_dhcp.gw, sizeof(g_temp_net_info_for_dhcp.gw));
    MEMCPY(&pNetInfo->sn, &g_temp_net_info_for_dhcp.sn, sizeof(g_temp_net_info_for_dhcp.sn));
    MEMCPY(&pNetInfo->dns, &g_temp_net_info_for_dhcp.dns, sizeof(g_temp_net_info_for_dhcp.dns));
    display_network_information(pNetInfo);
    
    return true;
}

// return 되는 단위 확인 필요
// iolibrary에서 리턴되는 lease time 단위가 이상함.
bool get_dhcp_check_leasetime(void)
{
    uint32_t leaseTime;
    leaseTime = getDHCPLeasetime();

    return leaseTime;
}


int8_t get_ipaddr_from_dns(uint8_t *domain, uint8_t *ip_from_dns, uint8_t *buf, uint32_t timeoutMs)
{
    int8_t ret;
    volatile uint32_t startTimeStamp = 0, currentTimeStamp = 0;

    DNS_init(SOCKET_DNS, buf);

    startTimeStamp = get_time_ms();
    while(1)
    {
        OS_DELAY_MS(100);
        
        currentTimeStamp = get_time_ms();

        ret = DNS_run(g_temp_net_info_for_dhcp.dns, domain, ip_from_dns);
        if( ret == DNS_RET_SUCCESS )
        {
            TRACE_DEBUG("DNS: [%s] Get Server IP - %d.%d.%d.%d\r\n", domain, ip_from_dns[0], ip_from_dns[1], ip_from_dns[2], ip_from_dns[3]);
            break;
        }
        else if( ret == DNS_RET_FAILED )
        {
            TRACE_DEBUG("Fail to DNS");
            return DNS_RET_FAILED;
        }

        if( (currentTimeStamp-startTimeStamp) > timeoutMs )
        {
            TRACE_DEBUG("Timeout DNS. %dms", (currentTimeStamp-startTimeStamp));
            return DNS_RET_TIMEOUT;
        }
    }

    return DNS_RET_SUCCESS;
}

void display_network_information(wiz_NetInfo *pNetInfo)
{
    uint8_t tmp_str[8] = {
        0,
    };

    ctlnetwork(CN_GET_NETINFO, (void *)pNetInfo);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);

    if (pNetInfo->dhcp == NETINFO_DHCP)
    {
        TRACE_DEBUG("====================================================================================================");
        TRACE_DEBUG(" %s network configuration : DHCP", (char *)tmp_str);
    }
    else
    {
        TRACE_DEBUG("====================================================================================================");
        TRACE_DEBUG(" %s network configuration : static", (char *)tmp_str);
    }

    TRACE_DEBUG(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X", pNetInfo->mac[0], pNetInfo->mac[1], pNetInfo->mac[2], pNetInfo->mac[3], pNetInfo->mac[4], pNetInfo->mac[5]);
    TRACE_DEBUG(" IP          : %d.%d.%d.%d", pNetInfo->ip[0], pNetInfo->ip[1], pNetInfo->ip[2], pNetInfo->ip[3]);
    TRACE_DEBUG(" Subnet Mask : %d.%d.%d.%d", pNetInfo->sn[0], pNetInfo->sn[1], pNetInfo->sn[2], pNetInfo->sn[3]);
    TRACE_DEBUG(" Gateway     : %d.%d.%d.%d", pNetInfo->gw[0], pNetInfo->gw[1], pNetInfo->gw[2], pNetInfo->gw[3]);
    TRACE_DEBUG(" DNS         : %d.%d.%d.%d", pNetInfo->dns[0], pNetInfo->dns[1], pNetInfo->dns[2], pNetInfo->dns[3]);
    TRACE_DEBUG("====================================================================================================");
}

/* Timer */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;

    if (g_msec_cnt >= 1000 - 1)
    {
        g_msec_cnt = 0;

        //vPortEnterCritical();
        DHCP_time_handler();
        //vPortExitCritical();
    }
}



