/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YMODEM_H__
#define __YMODEM_H__

#include <stdint.h>

#define YMODEM_PACKET_HEADER_SIZE      ((uint32_t)3U)    /*!<Header Size*/
#define YMODEM_PACKET_DATA_INDEX       ((uint32_t)4U)    /*!<Data Index*/
#define YMODEM_PACKET_START_INDEX      ((uint32_t)1U)    /*!<Start Index*/
#define YMODEM_PACKET_NUMBER_INDEX     ((uint32_t)2U)    /*!<Packet Number Index*/
#define YMODEM_PACKET_CNUMBER_INDEX    ((uint32_t)3U)    /*!<Cnumber Index*/
#define YMODEM_PACKET_TRAILER_SIZE     ((uint32_t)2U)    /*!<Trailer Size*/
#define YMODEM_PACKET_OVERHEAD_SIZE    (YMODEM_PACKET_HEADER_SIZE + YMODEM_PACKET_TRAILER_SIZE - 1U) /*!<Overhead Size*/
#define YMODEM_PACKET_SIZE             ((uint32_t)128U)  /*!<Packet Size*/
#define YMODEM_PACKET_1K_SIZE          ((uint32_t)1024U) /*!<Packet 1K Size*/

#define YMODEM_FILE_NAME_LENGTH        ((uint32_t)64U)   /*!< File name length*/
#define YMODEM_FILE_SIZE_LENGTH        ((uint32_t)16U)   /*!< File size length*/

#define YMODEM_SOH                     ((uint8_t)0x01U)  /*!< Start of 128-byte data packet */
#define YMODEM_STX                     ((uint8_t)0x02U)  /*!< Start of 1024-byte data packet */
#define YMODEM_EOT                     ((uint8_t)0x04U)  /*!< End of transmission */
#define YMODEM_ACK                     ((uint8_t)0x06U)  /*!< Acknowledge */
#define YMODEM_NAK                     ((uint8_t)0x15U)  /*!< Negative acknowledge */
#define YMODEM_CA                      ((uint32_t)0x18U) /*!< Two of these in succession aborts transfer */
#define YMODEM_CRC16                   ((uint8_t)0x43U)  /*!< 'C' == 0x43, request 16-bit CRC */
#define YMODEM_NEGATIVE_BYTE           ((uint8_t)0xFFU)  /*!< Negative Byte*/

#define YMODEM_ABORT1                  ((uint8_t)0x41U)  /*!< 'A' == 0x41, abort by user */
#define YMODEM_ABORT2                  ((uint8_t)0x61U)  /*!< 'a' == 0x61, abort by user */

#define YMODEM_NAK_TIMEOUT             ((uint32_t)0x100000U)  /*!< NAK Timeout*/
#define YMODEM_DOWNLOAD_TIMEOUT        ((uint32_t)5000U) /*!< Retry delay, has to be smaller than IWDG */
#define YMODEM_MAX_ERRORS              ((uint32_t)10U)    /*!< Maximum number of retry*/

#define MAX_MEMORY_BUFF_SIZE            1024

typedef enum
{
    RECEIVE_OK          = 0x00U,
    RECEIVE_ERR         = 0x01U,
    RECEIVE_BUSY        = 0x02U
} YMODEM_ReceiveStatusTypeDef;

typedef enum
{
  YMODEM_OK       = 0x00U, /*!< OK */
  YMODEM_ERROR    = 0x01U, /*!< Error */
  YMODEM_ABORT    = 0x02U, /*!< Abort */
  YMODEM_TIMEOUT  = 0x03U, /*!< Timeout */
  YMODEM_DATA     = 0x04U, /*!< Data */
  YMODEM_LIMIT    = 0x05U  /*!< Limit*/
} YMODEM_StatusTypeDef;

extern uint8_t FILE_BUFFER[MAX_MEMORY_BUFF_SIZE];
extern uint32_t FILE_BUFFER_INDEX;

int32_t YMODEM_Receive (uint32_t *pSize, uint8_t **pOutData);

YMODEM_StatusTypeDef YMODEM_HeaderPktRxCpltCallback (uint32_t uFileSize);
YMODEM_StatusTypeDef YMODEM_DataPktRxCpltCallback   (uint8_t *pData, uint32_t uSize);
YMODEM_ReceiveStatusTypeDef YModemReceivePacket     (uint8_t *pData, uint32_t *pLength, uint32_t timeout);
void        YModemUARTPutByte   (uint8_t ch);
int32_t     YModemUARTReceive   (uint8_t *pData, uint16_t dataLength, uint32_t timeout);
uint16_t    CRC16Calc           (uint8_t *buf, uint32_t count);



#endif  /* _YMODEM_H_ */

