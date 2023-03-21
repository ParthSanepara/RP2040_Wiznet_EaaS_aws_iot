#include "main_app_task.h"
#include "fleet_provisioning_handler.h"
#include "ethernet.h"
#include "flash_control.h"
#include "system_common.h"
#include "stdlib.h"

// Temperary
#include "iot_jobs_job_list.h"
#include "http_interface.h"
#include "iot_jobs_job_ota_handler.h"


uint8_t test_string[] = "https://wizdevbucket.s3.ap-northeast-2.amazonaws.com/firmware/rp2040_wiznet_eaas.bin?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Content-Sha256=UNSIGNED-PAYLOAD&X-Amz-Credential=ASIAZLLJHA5UBBBK6YWQ%2F20230314%2Fap-northeast-2%2Fs3%2Faws4_request&X-Amz-Date=20230314T004551Z&X-Amz-Expires=9000&X-Amz-Security-Token=IQoJb3JpZ2luX2VjEBEaDmFwLW5vcnRoZWFzdC0yIkgwRgIhAKM8fwJ9AUzmrfnOJQOlvIT5u5E1kpjUe%2FoyquJ57kBKAiEA4nFolG5yQ1y6dbOrZvj9zlqo0AN9GyGL7M%2Bk2GIFiHgqgAMIyv%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FARACGgw2NDI4NTUxNDMyNzIiDKGquV5Nl%2FlsSd8vOyrUAsYuh2kfpWWKPoS%2Bn%2FAv1Jn%2BXgGH%2BkWeKCehN56Oms8I0SnKDP2tnYtArYxLQIuQlFJhQhzCnUi%2FFug2sPbNZzAKjJjmeytwuNH5BlveXv9s6UZsjwQeFTiXyQM4%2B0yaTM7ZVlrnoUkPg7uIQNVDlv8WUwJyEwWrQSosFhuITe1ifDyUDvl2nfXa31S1IgNDwbr%2BI3GjH0m7pSzHpstOf%2F2XkuT4tY2h096m26os88xxUtHNMwcAEsLWldxXgJYWIDcMsbP7tKakasIqbMCjYYy5YhwcW0vmr%2BD1lq1DujP0lKmfA1RsuP1XfSBac5dFSsWeCcH45ZgZ8t8BB%2FPI0S3iKB3atS4SfJwPwDJ3cbilgRc2IG4ucuD49PIGEi2sd%2FAgny8yd7mg%2BxrLIQY2GyJcNW%2F8dwTBY9elHZd7SZ5jsb2jZ65wZAlC6kTZTOqpiqtLFDYwvIe%2FoAY6nQFmv9qX57fbCLGXhsfD%2Fu10AQYL%2FThDRZ1HHxf21Lg8cqj5wBWQ0KhSPbsg20koU4kGAZGKD8qws%2FBWLCjqBCk%2F3D1lZSa4InxhRDz%2FsniKdp5yUvAb4XYFz3dlVkbTb5rYmdMZifQmr9hKNqw0S9ad0hTI91lnuWid38Sq6TLjGxMhwyV0ZeAl55IPFIBKH6IntOIQM7kKJN%2FiJeX5&X-Amz-Signature=862500d5baa1fcf852344124a56760c400c6459d9aa9dda313de8ab0d7309fa3&X-Amz-SignedHeaders=host&x-id=GetObject";
static tlsContext_t g_http_ota_tls_context;


void print_main_menu(void)
{
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("1  : Fleet Provisioning");
    TRACE_DEBUG("2  : Run IoT Jobs");
    TRACE_DEBUG("8  : Erase Flash");
    TRACE_DEBUG("9  : Print Flash");
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("Input Key : ");
}

void print_erase_menu(void)
{
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("1  : Erase Common Config");
    TRACE_DEBUG("2  : Erase Provisioned Certs");
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("Input Key : ");
}

void print_flash_dump_menu(void)
{
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("1  : Dump Common Config");
    TRACE_DEBUG("2  : Dump Provisioned Certs");
    TRACE_DEBUG("3  : Dump Temp Binary Flash");
    TRACE_DEBUG("==========================================");
    TRACE_DEBUG("Input Key : ");
}


void main_app_task(void *pParam)
{
    APP_COMMON_t *pAppCommon = (APP_COMMON_t *)pParam;
    uint32_t loopCount = 0;
    uint32_t timeStamp;
    uint8_t cmd;
    bool testResult;
    uint8_t *pFlashAddr;
    //uint8_t tempIpAddr[100];
    //uint8_t tempDnsBuf[512];

    init_ethernet(&pAppCommon->DEVICE_INFO.ETH_NET_INFO);


    while(1)
    {
        print_main_menu();
        scanf("%c",&cmd);

        if(cmd == '1')
        {
            if(pAppCommon->isWizChipLinkUp == false || pAppCommon->isDhcpDone == false)
            {
                TRACE_DEBUG("Wait until ethernet link-up");
                do{
                    loopCount++;

                    if((loopCount % 16) == 0)
                    {
                        TRACE_DEBUG("");
                    }
                    
                    TRACE_DEBUG_WITHOUT_NL("#");
                    DELAY_MS(1000);
                }while(pAppCommon->isWizChipLinkUp == false || pAppCommon->isDhcpDone == false);
            }
            
            fleet_provisioning_handle(&pAppCommon->DEVICE_INFO);
        }
        else if(cmd == '2')
        {
            pAppCommon->isRunAwsIotJobs = true;
        }
        
        else if(cmd == '6')
        {
            // uint8_t temp_buffer[4096];
            // HTTPResponse_t response;

            // g_http_ota_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_NONE; // not used Root CA verify
            // g_http_ota_tls_context.clica_option = 0;                        // not used client certificate

            // memset(&response, 0x00, sizeof(response));
            // http_get(SOCKET_HTTP, temp_buffer, test_string, &g_http_ota_tls_context, &response);

            // TRACE_DEBUG("")

            //ota_download_firmware(test_string, 3000, 300);


            HTTPResponse_t response;

            http_get(SOCKET_HTTP, test_string, &g_http_ota_tls_context, true);




        }
        
        else if(cmd == '7')
        {
            uint8_t tempStr[64];
            uint16_t tempStrLength;

            if( is_empty_job_id_list() == false )
            {
                TRACE_DEBUG("Job Id List is empty");
            }

            add_job_id("TEST JOB ID1",strlen("TEST JOB ID1"));
            add_job_id("TEST JOB ID2",strlen("TEST JOB ID2"));
            add_job_id("TEST JOB ID3",strlen("TEST JOB ID3"));
            add_job_id("TEST JOB ID4",strlen("TEST JOB ID4"));

            get_first_job_id(tempStr, &tempStrLength);
            del_first_job_id();
            TRACE_DEBUG("JOB ID : %s",tempStr);

            get_first_job_id(tempStr, &tempStrLength);
            del_first_job_id();
            TRACE_DEBUG("JOB ID : %s",tempStr);

            get_first_job_id(tempStr, &tempStrLength);
            del_first_job_id();
            TRACE_DEBUG("JOB ID : %s",tempStr);

            get_first_job_id(tempStr, &tempStrLength);
            del_first_job_id();
            TRACE_DEBUG("JOB ID : %s",tempStr);

        }

        else if(cmd == '8')
        {
            print_erase_menu();
            scanf("%c", &cmd);
            if(cmd == '1')
            {
                erase_flash_common_config_info();
            }
            else if(cmd == '2')
            {
                erase_flash_provisioned_cert_info();
            }
            else if(cmd == '3')
            {
                erase_flash_ota_binary(330716);
            }
        }

        else if(cmd == '9')
        {
            print_flash_dump_menu();
            scanf("%c", &cmd);
            if(cmd == '1')
            {
                TRACE_DEBUG("THING NAME : %s",pAppCommon->DEVICE_INFO.THING_NAME);
                
                print_flash_common_config_info();
            }
            else if(cmd == '2')
            {
                print_flash_provioned_cert_info();
            }
            else if(cmd == '3')
            {
                print_flash_ota_binary();
            }
            else if(cmd == '4')
            {
                pFlashAddr = get_flash_ota_binary();
                
                TRACE_DEBUG("Addr : %x, Size : %d",pFlashAddr, APPLICATION_SIZE );
                //printf("%.*s",APPLICATION_SIZE, pFlashAddr);
            }
        }
        else if(cmd == '0')
        {
            device_reboot();
        }


        DELAY_MS(10);
    }
}
