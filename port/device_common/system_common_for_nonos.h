#ifndef _SYSTEM_COMMON_FOR_NON_OS_H_
#define _SYSTEM_COMMON_FOR_NON_OS_H_

#include <stdint.h>

void disable_interrupts (void);
void reset_peripherals  (void);
void jump_to_app        (uint32_t app_addr);

#endif