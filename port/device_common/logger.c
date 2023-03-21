#include "logger.h"

uint8_t LOGGER_LEVEL = LOG_LEVEL_DEBUG;

#ifdef INC_FREERTOS_H
static xSemaphoreHandle LOGGER_MUTEX = NULL;
#endif

void logger_mutex_init(void)
{
#ifdef INC_FREERTOS_H
    if(LOGGER_MUTEX != NULL)
    {
        return;
    }

    LOGGER_MUTEX = xSemaphoreCreateMutex();
#endif
}

void logger_mutex_lock(void)
{
#ifdef INC_FREERTOS_H
    if(LOGGER_MUTEX != NULL)
    {
        xSemaphoreTake(LOGGER_MUTEX, portMAX_DELAY);
    }
#endif
}

void logger_mutex_unlock(void)
{
#ifdef INC_FREERTOS_H
    if(LOGGER_MUTEX != NULL)
    {
        xSemaphoreGive(LOGGER_MUTEX);
    }
#endif
}

void set_logger_level(uint8_t level)
{
    if(level > LOG_LEVEL_DEBUG)
    {
        LOGGER_LEVEL = LOG_LEVEL_DEBUG;
    }

    LOGGER_LEVEL = level;
    return;
}

uint8_t get_logger_level(void)
{
    return LOGGER_LEVEL;
}