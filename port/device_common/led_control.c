#include "led_control.h"

#define RUN_STATUS_LED_PIN 25

static int run_status_gpio_pin_value = 0;

void init_run_status_led(void)
{
    gpio_init(RUN_STATUS_LED_PIN);
    gpio_set_dir(RUN_STATUS_LED_PIN, GPIO_OUT);
}

void toggle_run_status_led(void)
{
    if(run_status_gpio_pin_value == 0)
    {
        gpio_put(RUN_STATUS_LED_PIN,1);
        run_status_gpio_pin_value = 1;
    }
    else
    {
        gpio_put(RUN_STATUS_LED_PIN,0);
        run_status_gpio_pin_value = 0;
    }
}