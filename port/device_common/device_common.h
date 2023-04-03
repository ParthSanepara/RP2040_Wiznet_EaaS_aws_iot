#ifndef _DEVICE_COMMON_H_
#define _DEVICE_COMMON_H_

#include "config.h"
#include "aws_iot_config.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>


//#include "port_common.h"

//#include "wizchip_conf.h"
//#include "w5x00_spi.h"

#include "pico/time.h"

#include "logger.h"

#define BOOTLOADER_INFO                                "Wiznet EaaS Boot"
#define BOOTLOADER_VERSION_MAJOR                       1
#define BOOTLOADER_VERSION_MINOR                       0
#define BOOTLOADER_VERSION_PATCH                       0

#define APPLICATION_INFO                                "Wiznet EaaS"
#define APPLICATION_VERSION_MAJOR                       1
#define APPLICATION_VERSION_MINOR                       0
#define APPLICATION_VERSION_PATCH                       0

#define DELAY_S(x)          sleep_ms(( x * 1000))
#define DELAY_MS(x)         sleep_ms(x)

#define MEMSET(x, y, z)     memset(x, y, z)
#define MEMCMP(x, y, z)     memcmp(x, y, z)
#define MEMCPY(x, y, z)     memcpy(x, y, z)

#define NULL ((void *)0)

#define in_range(c, lo, up)  ((uint8_t)c >= lo && (uint8_t)c <= up)
#define IS_PRINT(c)          in_range(c, 0x20, 0x7f)

void        print_boot_info (void);
void        print_app_info  (void);

void        print_hex_line  (uint8_t *pInStr, int32_t len);
void        print_dump_data (uint8_t *payload, int32_t len);

void        print_hex       (uint8_t *pData, uint16_t dataLength);
uint32_t    get_time_ms     (void);

uint32_t    string_to_int       (uint8_t *pString);
uint32_t    hex_string_to_int   (uint8_t *pHexString);

uint8_t     is_ipaddr           (uint8_t *ipaddr, uint8_t *ret_ip);

uint16_t    get_char_count      (uint8_t *pString, uint8_t wantedChar);
bool        is_upper_version_fw (uint8_t *pFwVersion);



#endif