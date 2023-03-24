#ifndef _TEST_PROCEDURE_H_
#define _TEST_PROCEDURE_H_

#define KEY_VALUE_ERASE_FLASH_MENU      '1'
#define KEY_VALUE_PRINT_FLASH_MENU      '2'
#define KEY_VALUE_FACTORY_FW_MENU       '3'
#define KEY_VALUE_QUIT                  'q'

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

// KEY_VALUE_FACTORY_FW_MENU
#define KEY_VALUE_COPY_APP_AREA_TO_FACTORY_AREA     '1'
#define KEY_VALUE_COPY_FACTORY_AREA_TO_APP_AREA     '2'


void display_menu(void);

void procedure_erase_flash(void);
void procedure_print_flash(void);
void procedure_factory_fw_(void);


#endif