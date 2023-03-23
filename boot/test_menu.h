#ifndef _TEST_PROCEDURE_H_
#define _TEST_PROCEDURE_H_

#define KEY_VALUE_ERASE_FLASH_MENU      '1'
#define KEY_VALUE_PRINT_FLASH_MENU      '2'
#define KEY_VALUE_QUIT                  'Q'

// KEY_VALUE_ERASE_FLASH_MENU
#define KEY_VALUE_ERASE_CONFIG_FLASH    '1'
#define KEY_VALUE_ERASE_APP_FLASH       '2'
#define KEY_VALUE_ERASE_FACTORY_FLASH   '3'
#define KEY_VALUE_ERASE_OTA_FLASH       '4'
#define KEY_VALUE_ERASE_PROVISONED_CERT '5'

// KEY_VALUE_PRINT_FLASH_MENU
#define KEY_VALUE_PRINT_CONFIG_FLASH            '1'
#define KEY_VALUE_PRINT_PROVISONED_CERT_FLASH   '2'
#define KEY_VALUE_PRINT_APP_FLASH               '3'
#define KEY_VALUE_PRINT_FACTORY_FLASH           '4'
#define KEY_VALUE_PRINT_OTA_FLASH               '5'


void procedure_erase_flash(void);
void procedure_print_flash(void);


#endif