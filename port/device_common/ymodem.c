#include "stdio.h"
#include "Ymodem.h"
#include "device_common.h"

uint8_t FILE_BUFFER[MAX_MEMORY_BUFF_SIZE];
uint32_t FILE_BUFFER_INDEX;
uint8_t YMODEM_PACKET_BUFF[YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_DATA_INDEX + YMODEM_PACKET_TRAILER_SIZE];
uint8_t YMODEM_FILE_NAME[YMODEM_FILE_NAME_LENGTH];

int32_t YMODEM_Receive(uint32_t *pSize, uint8_t **pOutData)
{
    uint32_t i, packetLength, sessionDone = 0U, fileDone, errors = 0U, sessionBegin = 0U;
    uint32_t ramSource = 0U, fileSize = 0U, packetsReceived = 0;
    uint8_t *file_ptr;
    uint8_t file_size[YMODEM_FILE_SIZE_LENGTH];
    YMODEM_StatusTypeDef eYModemStatus;

    if ((pSize == NULL))
    {
        return -1;
    }

    stdio_flush();
    eYModemStatus = YMODEM_OK;

    while ((sessionDone == 0U) && (eYModemStatus == YMODEM_OK))
    {
        packetsReceived = 0U;
        fileDone = 0U;

        while ((fileDone == 0U) && (eYModemStatus == YMODEM_OK))
        {
            switch (YModemReceivePacket(YMODEM_PACKET_BUFF, &packetLength, YMODEM_DOWNLOAD_TIMEOUT))
            {
            case RECEIVE_OK:
                errors = 0U;
                switch (packetLength)
                {
                case 2U:
                    /* Abort by sender */
                    YModemUARTPutByte(YMODEM_ACK);
                    eYModemStatus = YMODEM_ABORT;
                    break;

                case 0U:
                    /* End of transmission */
                    YModemUARTPutByte(YMODEM_ACK);
                    *pSize = fileSize;
                    fileDone = 1U;
                    break;

                default:
                    /* Normal packet */
                    if (YMODEM_PACKET_BUFF[YMODEM_PACKET_NUMBER_INDEX] != (packetsReceived & 0xff))
                    {
                        YModemUARTPutByte(YMODEM_NAK);
                    }
                    else
                    {
                        if (packetsReceived == 0U)
                        {
                            /* File name packet */
                            if (YMODEM_PACKET_BUFF[YMODEM_PACKET_DATA_INDEX] != 0U)
                            {
                                /* File name extraction */
                                i = 0U;
                                file_ptr = YMODEM_PACKET_BUFF + YMODEM_PACKET_DATA_INDEX;
                                while ((*file_ptr != 0U) && (i < YMODEM_FILE_NAME_LENGTH))
                                {
                                    YMODEM_FILE_NAME[i++] = *file_ptr++;
                                }

                                /* File size extraction */
                                YMODEM_FILE_NAME[i++] = '\0';
                                i = 0U;
                                file_ptr++;

                                while (((*file_ptr != ' ') && (*file_ptr != 0)) && (i < YMODEM_FILE_SIZE_LENGTH))
                                {
                                    file_size[i++] = *file_ptr++;
                                }

                                file_size[i++] = '\0';
                                fileSize = strtol(file_size, NULL, 10);

                                /* Header packet received callback call*/
                                if (YMODEM_HeaderPktRxCpltCallback((uint32_t)fileSize) == YMODEM_OK)
                                {
                                    YModemUARTPutByte(YMODEM_ACK);
                                    YModemUARTPutByte(YMODEM_CRC16);
                                }
                                else
                                {
                                    /* End session */
                                    YModemUARTPutByte(YMODEM_CA);
                                    YModemUARTPutByte(YMODEM_CA);
                                    return -1;
                                }
                            }

                            /* File header packet is empty, end session */
                            else
                            {
                                YModemUARTPutByte(YMODEM_ACK);
                                fileDone = 1U;
                                sessionDone = 1U;
                                break;
                            }
                        }
                        else /* Data packet */
                        {
                            ramSource = (uint32_t)&YMODEM_PACKET_BUFF[YMODEM_PACKET_DATA_INDEX];

                            /* Data packet received callback call*/
                            if (YMODEM_DataPktRxCpltCallback((uint8_t *)ramSource, (uint32_t)packetLength) == YMODEM_OK)
                            {
                                
                                
                                
                                YModemUARTPutByte(YMODEM_ACK);
                            }
                            else /* An error occurred while writing to Flash memory */
                            {
                                /* End session */
                                YModemUARTPutByte(YMODEM_CA);
                                YModemUARTPutByte(YMODEM_CA);
                                eYModemStatus = YMODEM_DATA;
                            }

                        }
                        packetsReceived++;
                        sessionBegin = 1;
                    }
                    break;
                }
                break;
            case RECEIVE_BUSY: /* Abort actually */
                YModemUARTPutByte(YMODEM_CA);
                YModemUARTPutByte(YMODEM_CA);
                eYModemStatus = YMODEM_ABORT;
                break;

            default:
                if (sessionBegin > 0U)
                {
                    errors++;
                }
                if (errors > YMODEM_MAX_ERRORS)
                {
                    /* Abort communication */
                    YModemUARTPutByte(YMODEM_CA);
                    YModemUARTPutByte(YMODEM_CA);
                    DELAY_MS(1000);
                    return -1;
                }
                else
                {
                    YModemUARTPutByte(YMODEM_CRC16);              /* Ask for a packet */
                    printf("\b.");                                /* Replace C char by . on display console */
                }
                break;
            }
        }
    }

    if (eYModemStatus == YMODEM_OK)
    {
        *pOutData = FILE_BUFFER;
        return 0;
    }
    else
    {
        *pOutData = NULL;
        return -1;
    }
}

YMODEM_StatusTypeDef YMODEM_HeaderPktRxCpltCallback(uint32_t uFileSize)
{
    int32_t err;

    if (uFileSize > sizeof(FILE_BUFFER))
    {
        err = -1;
    }
    else {
        memset(FILE_BUFFER, 0x0, sizeof(FILE_BUFFER));
        FILE_BUFFER_INDEX = 0;
        err = 0;
    }

    if (err == 0) {
        return YMODEM_OK;
    }
    else {
        return YMODEM_ABORT;
    }
}

/**
  * @brief  Ymodem Data Packet Transfer completed callback.
  * @param  pData: Pointer to the buffer.
  * @param  uSize: Packet dimension.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
YMODEM_StatusTypeDef YMODEM_DataPktRxCpltCallback(uint8_t *pData, uint32_t uSize)
{
    int32_t err;

    memcpy(&FILE_BUFFER[FILE_BUFFER_INDEX], pData, uSize);
    FILE_BUFFER_INDEX += uSize;
    err = 0;

    if (err == 0) {
        return YMODEM_OK;
    }
    else {
        return YMODEM_ABORT;
    }
}

YMODEM_ReceiveStatusTypeDef YModemReceivePacket(uint8_t *pData, uint32_t *pLength, uint32_t timeout)
{
    uint32_t    crc;
    uint32_t    packet_size = 0U;
    uint8_t     ch;
    int32_t     error;
    YMODEM_ReceiveStatusTypeDef status;

    *pLength = 0U;

    error = YModemUARTReceive(&ch, 1, timeout);

    if (error == 0)
    {
        status = RECEIVE_OK;

        switch (ch)
        {
        case YMODEM_SOH:
            packet_size = YMODEM_PACKET_SIZE;
            break;
        case YMODEM_STX:
            packet_size = YMODEM_PACKET_1K_SIZE;
            break;
        case YMODEM_EOT:
            break;
        case YMODEM_CA:
            if ((YModemUARTReceive(&ch, 1U, timeout) == 0) && (ch == YMODEM_CA)) {
                packet_size = 2U;
            }
            else {
                status = RECEIVE_ERR;
            }
            break;
        case YMODEM_ABORT1:
        case YMODEM_ABORT2:
            status = RECEIVE_BUSY;
            break;
        default:
            status = RECEIVE_ERR;
            break;
        }

        *pData = ch;

        if (packet_size >= YMODEM_PACKET_SIZE)
        {
            error = YModemUARTReceive(&pData[YMODEM_PACKET_NUMBER_INDEX], packet_size + YMODEM_PACKET_OVERHEAD_SIZE, timeout);

            /* Simple packet sanity check */
            if (error == 0)
            {
                status = RECEIVE_OK;

                if (pData[YMODEM_PACKET_NUMBER_INDEX] != ((pData[YMODEM_PACKET_CNUMBER_INDEX]) ^ YMODEM_NEGATIVE_BYTE))
                {
                    packet_size = 0U;
                    status = RECEIVE_ERR;
                }
                else
                {
                    /* Check packet CRC*/
                    crc = pData[packet_size + YMODEM_PACKET_DATA_INDEX] << 8U;
                    crc += pData[packet_size + YMODEM_PACKET_DATA_INDEX + 1U];

                    if (CRC16Calc(&pData[YMODEM_PACKET_DATA_INDEX], packet_size) != crc) {
                        packet_size = 0U;
                        status = RECEIVE_ERR;
                    }
                }
            }
            else
            {
                status = RECEIVE_ERR;
            }
        }
        else
        {
            packet_size = 0U;
        }
    }
    else
    {
        status = RECEIVE_ERR;
    }

    *pLength = packet_size;
    return status;
}

void YModemUARTPutByte(uint8_t ch)
{
    putchar(ch);
    //DebugUART_Write(ch);
}

int32_t YModemUARTReceive(uint8_t *pData, uint16_t dataLength, uint32_t timeoutMs)
{
    uint32_t timeoutCount;
    int ch;

    if (pData == NULL)
    {
        return -1;
    }

    timeoutCount = 0;

    while((timeoutCount < timeoutMs) && (dataLength > 0)) {
        ch = getchar_timeout_us(1000);
        if(ch > 0)
        {
            timeoutCount = 0;
            *pData++ = ch;
            dataLength--;
        }
        timeoutCount++;
    }

    if (dataLength == 0) {
        return 0;
    }
    else {
        return -1;
    }
}

uint16_t CRC16Calc(uint8_t *buf, uint32_t count)
{
    uint16_t crc = 0;
    uint32_t i;

    while (count--) {
        crc = crc ^ *buf++ << 8;

        for (i = 0; i < 8; i++) {
            if (crc & 0x8000) {
                crc = crc << 1 ^ 0x1021;
            }
            else {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

