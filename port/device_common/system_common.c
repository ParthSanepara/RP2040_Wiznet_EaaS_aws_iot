#include "system_common.h"
#include "hardware/resets.h"
#include "hardware/watchdog.h"

void device_reboot(void)
{
    watchdog_enable(1, 1);
    while(1);
}

