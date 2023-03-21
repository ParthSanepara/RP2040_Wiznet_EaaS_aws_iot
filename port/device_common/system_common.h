#ifndef _SYSTEM_COMMON_H_
#define _SYSTEM_COMMON_H_

#include <stdint.h>

void device_reboot      (void);
void disable_interrupts (void);
void reset_peripherals  (void);
void jump_to_app        (uint32_t app_addr);

#endif