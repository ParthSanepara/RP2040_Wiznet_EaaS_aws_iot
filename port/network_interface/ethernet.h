#ifndef _COMMON_UTILS_ETHERNET_H_
#define _COMMON_UTILS_ETHERNET_H_

#include <stdbool.h>
#include "wizchip_conf.h"



/* Socket */
#define SOCKET_MQTT_FLEET_PROVISIONING  0
#define SOCKET_MQTT_IOT_JOBS            1
#define SOCKET_DNS                      2
// W5100S Socket 갯수가 4개인 관계로 HTTP와 DHCP 소켓 번호를 같이 사용
#define SOCKET_HTTP                     3
#define SOCKET_DHCP                     3

#define DNS_TIMEOUT 1000

enum
{
    DNS_RET_TIMEOUT = -2,
    DNS_RET_FAILED = -1,
    DNS_RET_RUNNING = 0,
    DNS_RET_SUCCESS = 1,
    DNS_RET_STOPPED = 2
};

void init_ethernet                  (wiz_NetInfo *pNetInfo);
bool dhcp_process                   (wiz_NetInfo *pNetInfo);

int8_t  get_ipaddr_from_dns          (uint8_t *domain, uint8_t *ip_from_dns, uint8_t *buf, uint32_t timeoutMs);
void    display_network_information  (wiz_NetInfo *pNetInfo);

#endif