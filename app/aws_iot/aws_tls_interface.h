#ifndef _AWS_TLS_INTERFACE_H_
#define _AWS_TLS_INTERFACE_H_

#include <stdio.h>

#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/entropy.h"


typedef struct{
    int32_t fdSocket;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	
    mbedtls_x509_crt rootCA;
}AWS_TLS_CONTEXT;


#endif