#ifndef _LED_CONTROL_H_
#define _LED_CONTROL_H_

#include "pico/stdlib.h"
#include "pico/binary_info.h"

void init_run_status_led    (void);
void toggle_run_status_led  (void);

#endif