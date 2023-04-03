#include "simple_cert_checker.h"
#include "mbedtls/x509_crt.h"
#include "logger.h"

bool is_valid_pem_certificate(uint8_t *pCertPemData)
{
    int resultCode, certLength=0;
    mbedtls_x509_crt certificate;

    mbedtls_x509_crt_init(&certificate);

    certLength = strlen(pCertPemData) + 1;
    resultCode = mbedtls_x509_crt_parse(&certificate, (const unsigned char *)pCertPemData, certLength);
    if(resultCode < 0)
    {
        TRACE_ERROR("Failed. mbedtls_x509_crt_parse -0x%x\r\n", -resultCode);
        return false;
    }

    return true;
}

bool is_valid_pem_private_key(uint8_t *pPrivateKeyPemData)
{
    mbedtls_pk_context privateKey;
    int resultCode, privateKeyLength = 0;

    privateKeyLength = strlen(pPrivateKeyPemData) + 1;
    
    resultCode = mbedtls_pk_parse_key(&privateKey, (const unsigned char *)pPrivateKeyPemData, privateKeyLength, NULL, 0);
    if(resultCode < 0)
    {
        TRACE_ERROR("Failed. mbedtls_pk_parse_key -0x%x\r\n", -resultCode);
        return false;
    }

    return true;
}