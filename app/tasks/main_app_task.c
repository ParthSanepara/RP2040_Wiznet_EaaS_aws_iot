#include "main_app_task.h"
#include "fleet_provisioning_handler.h"
#include "ethernet.h"
#include "flash_control.h"
#include "system_common.h"
#include "stdlib.h"
#include "os_common.h"
#include "simple_cert_checker.h"


void main_app_task(void *pParam)
{
    APP_COMMON_t *pAppCommon = (APP_COMMON_t *)pParam;
    AWS_FP_DEVICE_CERT_INFO_t provisonedCertInfo;
    bool retStatus;


    init_ethernet(&pAppCommon->DEVICE_INFO.ETH_NET_INFO);

    do{
        OS_DELAY_MS(1000);
    }while(pAppCommon->isWizChipLinkUp == false || pAppCommon->isDhcpDone == false);

    TRACE_INFO("Start Main App Task");
    
    pAppCommon->isValidClaimCertInfo = false;
    pAppCommon->isValidProvisionedCertInfo = true;
    pAppCommon->enableRefreshProvisionedCert = false;

    load_flash_provisioned_cert_info(&provisonedCertInfo);

    if( is_valid_pem_certificate(provisonedCertInfo.PROVISONED_CERT_INFO.DEVICE_PROVISIONED_CERT) == false )
    {
        TRACE_WARN("Invalid Provisoned Cert");
        pAppCommon->enableRefreshProvisionedCert = true;
        pAppCommon->isValidProvisionedCertInfo = false;
    }

    if( is_valid_pem_private_key(provisonedCertInfo.PROVISONED_CERT_INFO.DEVICE_PROVISIONED_PRIVATE_KEY) == false )
    {
        TRACE_WARN("Invalid Provisoned Private Key");
        pAppCommon->enableRefreshProvisionedCert = true;
        pAppCommon->isValidProvisionedCertInfo = false;
    }

    while(1)
    {
        OS_DELAY_MS(100);

        // Provisoned 인증서가 없거나, AWS IoT Job 연결이 연속적으로 끊어지면 fleet_provsioning 수행
        if(pAppCommon->enableRefreshProvisionedCert == true)
        {
            pAppCommon->isRunAwsIotJobs = false;

            retStatus = fleet_provisioning_handle(&pAppCommon->DEVICE_INFO);
            if(retStatus == false)
            {
                OS_DELAY_MS(10000);
                continue;
            }
            
            // fleet_provisioning 성공 시, AWS IoT Jobs 시작
            pAppCommon->enableRefreshProvisionedCert = false;
            pAppCommon->isRunAwsIotJobs = true;
        }
        else if(pAppCommon->isValidProvisionedCertInfo == true)
        {
            pAppCommon->isRunAwsIotJobs = true;
        }
    }
}
