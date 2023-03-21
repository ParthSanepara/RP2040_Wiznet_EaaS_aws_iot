#include "device_common.h"
#include "hardware/timer.h"

void print_boot_info(void)
{
    TRACE_DEBUG("===========================================================");    

    TRACE_DEBUG("##      ## #### ######## ##    ## ######## ########"); 
    TRACE_DEBUG("##  ##  ##  ##       ##  ###   ## ##          ##   ");
    TRACE_DEBUG("##  ##  ##  ##      ##   ####  ## ##          ##   "); 
    TRACE_DEBUG("##  ##  ##  ##     ##    ## ## ## ######      ##   "); 
    TRACE_DEBUG("##  ##  ##  ##    ##     ##  #### ##          ##   "); 
    TRACE_DEBUG("##  ##  ##  ##   ##      ##   ### ##          ##   "); 
    TRACE_DEBUG(" ###  ###  #### ######## ##    ## ########    ##   "); 
    TRACE_DEBUG("===========================================================");
    TRACE_DEBUG("Bootloader : %s",BOOTLOADER_INFO);
    TRACE_DEBUG("Bootloader Ver : %d . %d . %d",BOOTLOADER_VERSION_MAJOR, BOOTLOADER_VERSION_MINOR, BOOTLOADER_VERSION_PATCH);
    TRACE_DEBUG("Build Date  : %s %s", __DATE__, __TIME__);
    TRACE_DEBUG("===========================================================");
}

void print_app_info(void)
{
    TRACE_DEBUG("===========================================================");    

    TRACE_DEBUG("##      ## #### ######## ##    ## ######## ########"); 
    TRACE_DEBUG("##  ##  ##  ##       ##  ###   ## ##          ##   ");
    TRACE_DEBUG("##  ##  ##  ##      ##   ####  ## ##          ##   "); 
    TRACE_DEBUG("##  ##  ##  ##     ##    ## ## ## ######      ##   "); 
    TRACE_DEBUG("##  ##  ##  ##    ##     ##  #### ##          ##   "); 
    TRACE_DEBUG("##  ##  ##  ##   ##      ##   ### ##          ##   "); 
    TRACE_DEBUG(" ###  ###  #### ######## ##    ## ########    ##   "); 
    TRACE_DEBUG("===========================================================");
    TRACE_DEBUG("Application : %s",APPLICATION_INFO);
    TRACE_DEBUG("Application Ver : %d . %d . %d",APPLICATION_VERSION_MAJOR, APPLICATION_VERSION_MINOR, APPLICATION_VERSION_PATCH);
    TRACE_DEBUG("Build Date  : %s %s", __DATE__, __TIME__);
    TRACE_DEBUG("===========================================================");
}

void print_hex_line(uint8_t *pInStr, int32_t len)
{
    int32_t i, gap;
    uint8_t *ch;
    
    TRACE_DEBUG_WITHOUT_NL("%08lX   ",(uint32_t)(pInStr));
    
    ch = pInStr;
    for(i=0; i<len; i++)
    {
        TRACE_DEBUG_WITHOUT_NL("%02x ", *ch);
        ch++;
        if(i==7)
        {
            TRACE_DEBUG_WITHOUT_NL(" ");
        }
    }
    if( len < 8)
    {
        TRACE_DEBUG_WITHOUT_NL(" ");
    }
    if( len < 16)
    {
        gap = 16 - len;
        for(i=0; i<gap; i++)
        {
            TRACE_DEBUG_WITHOUT_NL("   ");
        }
    }
    TRACE_DEBUG_WITHOUT_NL("   ");
    ch = pInStr;
    for(i=0; i<len; i++)
    {
        if ( IS_PRINT(*ch) )
        {
            TRACE_DEBUG_WITHOUT_NL("%c", *ch);
        }
        else
        {
            TRACE_DEBUG_WITHOUT_NL(".");
        }
        ch++;
    }
    TRACE_DEBUG_WITHOUT_NL("\r\n");
    return;
}

void print_dump_data(uint8_t *payload, int32_t len)
{
    int32_t len_rem = len;
    int32_t line_width = 16;
    int32_t line_len;
    uint8_t *ch = payload;
    
    if ( len <= 0)
    {
        return;
    }
    
    if ( len <= line_width )
    {
        print_hex_line(ch, len);
        return;
    }
    
    for(;;)
    {
        line_len = line_width % len_rem;
        print_hex_line(ch, line_len);
        len_rem = len_rem - line_len;
        ch = ch + line_len;
        if(len_rem <= line_width)
        {
            print_hex_line(ch, len_rem);
            break;
        }
    }
    return;
}

void print_hex(uint8_t *pData, uint16_t dataLength)
{
    uint16_t i=0;

    for(i=0; i<dataLength; i++)
    {
        if(i%16)
        {
            TRACE_DEBUG("");
        }
        
        TRACE_DEBUG_WITHOUT_NL("%02X ", *(pData+i));
    }
    TRACE_DEBUG("")
}


uint32_t get_time_ms(void)
{
    uint32_t timeStamp = time_us_32() / 1000;
    
    return timeStamp;
}

uint32_t string_to_int(uint8_t *pString)
{
    return atoi((char*)pString);
}

uint32_t hex_string_to_int(uint8_t *pHexString)
{
    uint32_t num;

    num = strtoul((char*)pHexString, NULL, 16);

    return num;
}

uint8_t is_ipaddr(uint8_t *ipaddr, uint8_t *ret_ip)
{
    uint8_t i = 0;
    uint8_t dotcnt = 0;
    uint16_t tval = 0;
    uint8_t len = strlen((char *)ipaddr);

    uint8_t tmp[3] = {
        0,
    };
    uint8_t tmpcnt = 0;

    if (len > 15 || len < 7)
        return 0;

    for (i = 0; i < len; i++)
    {
        if (isdigit(ipaddr[i]))
        {
            tval = (tval * 10) + ipaddr[i] - '0';
            if (tval > 255)
                return 0;

            // added for ret_ip arrary
            tmp[tmpcnt++] = ipaddr[i];
            if (tmpcnt > sizeof(tmp))
                return 0;
        }
        else if (ipaddr[i] == '.')
        {
            if (tval > 255)
                return 0;
            if (++dotcnt > 4)
                return 0;
            tval = 0;

            // added for ret_ip arrary
            ret_ip[dotcnt - 1] = atoi((char *)tmp);
            memset(tmp, 0x00, sizeof(tmp));
            tmpcnt = 0;
        }
        else
            return 0;
    }
    // added for ret_ip arrary
    ret_ip[dotcnt] = atoi((char *)tmp);

    return 1;
}