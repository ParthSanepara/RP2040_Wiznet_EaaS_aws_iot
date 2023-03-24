#include "system_common.h"
#include "RP2040.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"

void device_reboot(void)
{
    watchdog_enable(1, 1);
    while(1);
}

void disable_interrupts(void)
{
    SysTick->CTRL &= ~1;

    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;
}

void reset_peripherals(void)
{
    reset_block(~(
            RESETS_RESET_IO_QSPI_BITS |
            RESETS_RESET_PADS_QSPI_BITS |
            RESETS_RESET_SYSCFG_BITS |
            RESETS_RESET_PLL_SYS_BITS
    ));
}

void jump_to_app(uint32_t app_addr)
{
    // Derived from the Leaf Labs Cortex-M3 bootloader.
    // Copyright (c) 2010 LeafLabs LLC.
    // Modified 2021 Brian Starkey <stark3y@gmail.com>
    // Originally under The MIT License
    uint32_t reset_vector = *(volatile uint32_t *)(app_addr + 0x04);
    SCB->VTOR = app_addr;

    asm volatile("msr msp, %0"::"g"
        (*(volatile uint32_t *)app_addr));
    asm volatile("bx %0"::"r" (reset_vector));
}

