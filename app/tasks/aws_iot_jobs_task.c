#include "aws_iot_jobs_task.h"
#include "iot_jobs_handler.h"
#include "iot_jobs_job_list.h"
#include "device_common.h"
#include "system_common.h"
#include "os_common.h"

#define IOT_JOBS_GET_JOB_TIME_MS                10000
#define IOT_JOBS_START_JOB_DOCUMENT_TIME_MS      1000

#define IOT_JOBS_MAX_FAIL_COUNT_GET_JOB         6


void aws_iot_jobs_task(void *pParam)
{
    bool retStatus=false;
    int qos=1;
    uint32_t startGetJobTimeoutMs, startStartJobDocumentTimeoutMs, currentTimeoutMs;
    uint8_t tempJobId[MAX_AWS_JOB_ID_SIZE];    
    uint16_t tempJobIdLength;
    
    uint32_t get_job_fail_count = 0;

    APP_COMMON_t *pAppCommon = (APP_COMMON_t *)pParam;

    do{
        OS_DELAY_MS(1000);
    }while(pAppCommon->isWizChipLinkUp == false || pAppCommon->isDhcpDone == false);

    TRACE_DEBUG("AWS IoT Jobs Task");
    pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
    pAppCommon->isRunAwsIotJobs = false;

    while(1)
    {
        OS_DELAY_MS(100);

        if(pAppCommon->isRunAwsIotJobs == false)
        {
            if(pAppCommon->AWS_IOT_JOBS_STATUS == AWS_IOT_JOBS_CONNECTED)
            {
                TRACE_DEBUG("Close connection for IOT JOBS");
                iot_jobs_close_procedure(&pAppCommon->DEVICE_INFO);
                pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
            }
            
            continue;
        }

        if(pAppCommon->AWS_IOT_JOBS_STATUS == AWS_IOT_JOBS_DISCONNECTED)
        {
            TRACE_DEBUG("Connect for IOT JOBS");
            retStatus = iot_jobs_connect_procedure(&pAppCommon->DEVICE_INFO);
            if(retStatus == false)
            {
                iot_jobs_close_procedure(&pAppCommon->DEVICE_INFO);
                pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
                OS_DELAY_MS(1000);
                continue;
            }

            pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_CONNECTED;
        
            startGetJobTimeoutMs = get_time_ms();
            startStartJobDocumentTimeoutMs = get_time_ms();
        }
        
        if(pAppCommon->AWS_IOT_JOBS_STATUS == AWS_IOT_JOBS_CONNECTED)
        {
            currentTimeoutMs = get_time_ms();

            if( (currentTimeoutMs - startGetJobTimeoutMs) > IOT_JOBS_GET_JOB_TIME_MS )
            {
                startGetJobTimeoutMs = get_time_ms();
                
                retStatus = iot_jobs_get_procedure(&pAppCommon->DEVICE_INFO);
                if(retStatus == false)
                {
                    iot_jobs_close_procedure(&pAppCommon->DEVICE_INFO);
                    pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
                    continue;
                }

                // if(retStatus == false)
                // {
                //     get_job_fail_count++;
                // }

                // if(get_job_fail_count > IOT_JOBS_MAX_FAIL_COUNT_GET_JOB)
                // {
                //     iot_jobs_close_procedure(&pAppCommon->DEVICE_INFO);
                //     pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
                //     continue;
                // }
            }

            if( (currentTimeoutMs - startStartJobDocumentTimeoutMs) > IOT_JOBS_START_JOB_DOCUMENT_TIME_MS)
            {
                retStatus = iot_jobs_start_job_document_procedure (&pAppCommon->DEVICE_INFO);
                if(retStatus == false)
                {
                    iot_jobs_close_procedure(&pAppCommon->DEVICE_INFO);
                    pAppCommon->AWS_IOT_JOBS_STATUS = AWS_IOT_JOBS_DISCONNECTED;
                    continue;
                }
            }

            


        }
    }
}