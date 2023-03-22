#ifndef _OS_COMMON_H_
#define _OS_COMMON_H_

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


#define OS_DELAY_MS(x)      vTaskDelay(x)
#define OS_DELAY_S(x)       vTaskDelay(( x * 1000))


#endif
