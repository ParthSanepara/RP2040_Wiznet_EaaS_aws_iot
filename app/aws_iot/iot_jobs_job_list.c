#include "iot_jobs_job_list.h"
#include "aws_iot_config.h"
#include "config.h"

uint8_t g_aws_job_id_list[MAX_AWS_JOB_LIST_SIZE][MAX_AWS_JOB_ID_SIZE] = {"",};

bool is_empty_job_id_list(void)
{
    uint8_t *pJobId;
    uint32_t jobIdStrLength = 0;

    uint8_t temp_job_id[MAX_AWS_JOB_ID_SIZE] = {0,};
    pJobId = g_aws_job_id_list[0];

    jobIdStrLength = strlen(pJobId);
    if(jobIdStrLength > 0)
    {
        return true;
    }

    return false;
}

bool is_exist_job_id(uint8_t *pJobId, uint16_t jobIdLength)
{
    uint8_t i;

    for(i=0; i<MAX_AWS_JOB_LIST_SIZE; i++)
    {
        if( strncmp( g_aws_job_id_list[i], pJobId, jobIdLength ) == 0 )
        {
            return true;
        }
    }

    return false;
}

bool get_available_index(uint8_t *pIndex)
{
    uint8_t i;
    uint8_t *pJobId;
    uint32_t jobIdStrLength = 0;    

    for(i=0; i<MAX_AWS_JOB_LIST_SIZE; i++)
    {
        pJobId = g_aws_job_id_list[i];
        jobIdStrLength = strlen(pJobId);
        
        if( jobIdStrLength == 0 )
        {
            *pIndex = i;
            return true;
        }
    }

    return false;
}

bool get_first_job_id(uint8_t *pJobId, uint16_t *jobIdLength)
{
    if( is_empty_job_id_list() == false )
    {
        //TRACE_DEBUG("There is no job id");
        return false;
    }

    strncpy(pJobId, g_aws_job_id_list[0], strlen(g_aws_job_id_list[0]));
    *jobIdLength = strlen(g_aws_job_id_list[0]);
    pJobId[*jobIdLength] = '\0';

    return true;
}

bool del_first_job_id(void)
{
    uint8_t i;

    memset(g_aws_job_id_list[0], 0x00, sizeof(g_aws_job_id_list[0]));

    for(i=1; i<MAX_AWS_JOB_LIST_SIZE; i++)
    {
        memcpy(g_aws_job_id_list[i-1], g_aws_job_id_list[i], sizeof(g_aws_job_id_list[i]));
        memset(g_aws_job_id_list[i], 0x00, sizeof(g_aws_job_id_list[i]));
    }

    return true;
}

bool add_job_id(uint8_t *pJobId, uint16_t jobIdLength)
{
    uint8_t idx;

    if(get_available_index(&idx) == false)
    {
        return false;
    }

    memset(g_aws_job_id_list[idx], 0x00, sizeof(g_aws_job_id_list[idx]));
    strncpy( g_aws_job_id_list[idx], pJobId, jobIdLength );
    g_aws_job_id_list[idx][jobIdLength] = '\0';

    return true;
}




