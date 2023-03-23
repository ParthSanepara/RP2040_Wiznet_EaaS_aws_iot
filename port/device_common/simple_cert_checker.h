#ifndef _SIMPLE_CERT_CHEKCER_H_
#define _SIMPLE_CERT_CHEKCER_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

bool is_valid_pem_certificate   (uint8_t *pCertPemData);
bool is_valid_pem_private_key   (uint8_t *pPrivateKeyPemData);

#endif