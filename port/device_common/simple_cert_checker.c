#include "simple_cert_checker.h"


#define BEGIN_CERTIFICATE_STR   "-----BEGIN CERTIFICATE-----"
#define END_CERTIFICATE_STR     "-----END CERTIFICATE-----"

#define BEGIN_PRIVATE_KEY_STR   "-----BEGIN RSA PRIVATE KEY-----"
#define END_PRIVATE_KEY_STR     "-----END RSA PRIVATE KEY-----"

bool is_valid_pem_certificate(uint8_t *pCertPemData)
{
    uint8_t *ptr;

    ptr = strstr(pCertPemData, BEGIN_CERTIFICATE_STR);
    if(ptr == NULL)
    {
        return false;
    }

    ptr = strstr(pCertPemData, END_CERTIFICATE_STR);
    if(ptr == NULL)
    {
        return false;
    }

    return true;
}

bool is_valid_pem_private_key(uint8_t *pPrivateKeyPemData)
{
    uint8_t *ptr;

    ptr = strstr(pPrivateKeyPemData, BEGIN_PRIVATE_KEY_STR);
    if(ptr == NULL)
    {
        return false;
    }

    ptr = strstr(pPrivateKeyPemData, END_PRIVATE_KEY_STR);
    if(ptr == NULL)
    {
        return false;
    }

    return true;
}