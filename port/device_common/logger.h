#ifndef _COMMON_UTILS_LOGGER_H_
#define _COMMON_UTILS_LOGGER_H_

#include <stdlib.h>
#include <stdint.h>

#define LOG_LEVEL_NONE          0
#define LOG_LEVEL_ERROR         1
#define LOG_LEVEL_WARN          2
#define LOG_LEVEL_INFO          3
#define LOG_LEVEL_DEBUG         4

#define ANSI_COLOR_RED               			        (0x031)
#define ANSI_COLOR_GREEN             			        (0x032)
#define ANSI_COLOR_YELLOW            			        (0x033)
#define ANSI_COLOR_BLUE              			        (0x034)
#define ANSI_COLOR_MAGENTA           			        (0x035)
#define ANSI_COLOT_CYAN              			        (0x036)

#define TRACE_MSG(fmt, ...) 							printf(fmt"\r\n", ##__VA_ARGS__)
#define TRACE_COLOR(COLOR_CODE, fmt, ...) 				printf("\033[%xm"fmt"\033[m\r\n", COLOR_CODE, ##__VA_ARGS__)

#define TRACE_COLOR_RED(fmt, ...) 						printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOR_RED, ##__VA_ARGS__)
#define TRACE_COLOR_GREEN(fmt, ...) 					printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOR_GREEN, ##__VA_ARGS__)
#define TRACE_COLOR_YELLOW(fmt, ...) 					printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOR_YELLOW, ##__VA_ARGS__)
#define TRACE_COLOR_BLUE(fmt, ...) 						printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOR_BLUE, ##__VA_ARGS__)
#define TRACE_COLOR_MAGENTA(fmt, ...) 					printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOR_MAGENTA, ##__VA_ARGS__)
#define TRACE_COLOR_CYAN(fmt, ...) 						printf("\033[%xm"fmt"\033[m\r\n", ANSI_COLOT_CYAN, ##__VA_ARGS__)

#define TRACE_DEBUG(fmt, ...)   						\
{                                                       \
    if( get_logger_level() >= LOG_LEVEL_DEBUG )         \
    {                                                   \
        logger_mutex_lock();                            \
        TRACE_MSG(fmt, ##__VA_ARGS__);                  \
        logger_mutex_unlock();                          \
    }                                                   \
}                                                       \

#define TRACE_INFO(fmt, ...)                            \
{                                                       \
    if( get_logger_level() >= LOG_LEVEL_INFO )          \
    {                                                   \
        logger_mutex_lock();                            \
        TRACE_COLOR_GREEN(fmt, ##__VA_ARGS__);          \
        logger_mutex_unlock();                          \
    }                                                   \
}                                                       \
    						
#define TRACE_WARN(fmt, ...)    						\
{                                                       \
    if( get_logger_level() >= LOG_LEVEL_WARN )          \
    {                                                   \
        logger_mutex_lock();                            \
        TRACE_COLOR_CYAN(fmt, ##__VA_ARGS__);           \
        logger_mutex_unlock();                          \
    }                                                   \
}                                                       \

#define TRACE_ERROR(fmt, ...)   						\
{                                                       \
    if( get_logger_level() >= LOG_LEVEL_ERROR )         \
    {                                                   \
        logger_mutex_lock();                            \
        TRACE_COLOR_RED(fmt, ##__VA_ARGS__);            \
        logger_mutex_unlock();                          \
    }                                                   \
}                                                       \

#define TRACE_MSG_WITHOUT_NL(fmt, ...)          		printf(fmt, ##__VA_ARGS__)
#define TRACE_COLOR_WITHOUT_NL(COLOR_CODE, fmt, ...)	printf("\033[%xm"fmt"\033[m", COLOR_CODE, ##__VA_ARGS__)
#define TRACE_COLOR_RED_WITHOUT_NL(fmt, ...) 			printf("\033[%xm"fmt"\033[m", ANSI_COLOR_RED, ##__VA_ARGS__)
#define TRACE_COLOR_GREEN_WITHOUT_NL(fmt, ...) 			printf("\033[%xm"fmt"\033[m", ANSI_COLOR_GREEN, ##__VA_ARGS__)
#define TRACE_COLOR_YELLOW_WITHOUT_NL(fmt, ...) 		printf("\033[%xm"fmt"\033[m", ANSI_COLOR_YELLOW, ##__VA_ARGS__)
#define TRACE_COLOR_BLUE_WITHOUT_NL(fmt, ...) 			printf("\033[%xm"fmt"\033[m", ANSI_COLOR_BLUE, ##__VA_ARGS__)
#define TRACE_COLOR_MAGENTA_WITHOUT_NL(fmt, ...) 		printf("\033[%xm"fmt"\033[m", ANSI_COLOR_MAGENTA, ##__VA_ARGS__)
#define TRACE_COLOR_CYAN_WITHOUT_NL(fmt, ...) 			printf("\033[%xm"fmt"\033[m", ANSI_COLOT_CYAN, ##__VA_ARGS__)

#define TRACE_DEBUG_WITHOUT_NL(fmt, ...)   				        \
{                                                               \
    if( get_logger_level() >= LOG_LEVEL_DEBUG )                 \
    {                                                           \
        logger_mutex_lock();                                    \
        TRACE_MSG_WITHOUT_NL(fmt, ##__VA_ARGS__);               \
        logger_mutex_unlock();                                  \
    }                                                           \
}                                                               \

#define TRACE_INFO_WITHOUT_NL(fmt, ...)    				        \
{                                                               \
    if( get_logger_level() >= LOG_LEVEL_INFO )                  \
    {                                                           \
        logger_mutex_lock();                                    \
        TRACE_COLOR_GREEN_WITHOUT_NL(fmt, ##__VA_ARGS__);       \
        logger_mutex_unlock();                                  \
    }                                                           \
}                                                               \

#define TRACE_WARN_WITHOUT_NL(fmt, ...)    				        \
{                                                               \
    if( get_logger_level() >= LOG_LEVEL_WARN )                  \
    {                                                           \
        logger_mutex_lock();                                    \
        TRACE_COLOR_CYAN_WITHOUT_NL(fmt, ##__VA_ARGS__);        \
        logger_mutex_unlock();                                  \
    }                                                           \
}                                                               \

#define TRACE_ERROR_WITHOUT_NL(fmt, ...)   				        \
{                                                               \
    if( get_logger_level() >= LOG_LEVEL_WARN )                  \
    {                                                           \
        logger_mutex_lock();                                    \
        TRACE_COLOR_RED_WITHOUT_NL(fmt, ##__VA_ARGS__);         \
        logger_mutex_unlock();                                  \
    }                                                           \
}                                                               \

void    logger_mutex_init  (void);
void    logger_mutex_lock  (void);
void    logger_mutex_unlock(void);
void    set_logger_level   (uint8_t level);
uint8_t get_logger_level   (void);


#endif