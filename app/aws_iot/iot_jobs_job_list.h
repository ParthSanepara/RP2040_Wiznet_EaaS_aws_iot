#ifndef AWS_IOT_IOT_JOBS_JOB_LIST_H_
#define AWS_IOT_IOT_JOBS_JOB_LIST_H_

#include "aws_iot_config.h"
#include "config.h"

typedef struct 
{
    uint8_t JOB_COUNT;
    uint8_t JOB_ID_LIST[MAX_AWS_JOB_LIST_SIZE][MAX_AWS_JOB_ID_SIZE];
} AWS_JOB_ID_LIST_PARAM_t;

bool is_empty_job_id_list   (void);
bool is_exist_job_id        (uint8_t *pJobId, uint16_t jobIdLength);
bool get_available_index    (uint8_t *pIndex);
bool get_first_job_id       (uint8_t *pJobId, uint16_t *jobIdLength);
bool del_first_job_id       (void);
bool add_job_id             (uint8_t *pJobId, uint16_t jobIdLength);

#endif