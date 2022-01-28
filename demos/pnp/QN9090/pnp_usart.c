/* Copyright 2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "board.h"
#include "fsl_usart.h"
#include <pnp_usart.h>
#include <stdbool.h>
#include <semphr.h>
#include <UwbPnpInternal.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART          USART0
#define DEMO_USART_CLK_SRC  kCLOCK_Fro32M
#define DEMO_USART_CLK_FREQ CLOCK_GetFreq(DEMO_USART_CLK_SRC)
#define DEMO_USART_IRQn     USART0_IRQn

#define UART_HEADER_SIZE 3

#define USART_RCV_HDR_NON_BLOCKING(BUFFER, SIZE, RCVD_BYTES_PTR) \
    rcvXfer.data     = BUFFER;                                   \
    rcvXfer.dataSize = SIZE;                                     \
    USART_TransferReceiveNonBlocking(DEMO_USART, &g_uartHandle, &rcvXfer, RCVD_BYTES_PTR);

usart_handle_t g_uartHandle;
SemaphoreHandle_t mHifNtfnSem, mHifRspSem;
static uint8_t buffer[4096];
static usart_transfer_t sendXfer;
static usart_transfer_t rcvXfer;
volatile bool txOnGoing = false;
volatile bool mError    = false;

static void (*usartRcvCb)(uint8_t *, uint32_t *);
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *))
{
    size_t rxReceivedBytes = 0;
    rcvXfer.data           = buffer;
    rcvXfer.dataSize       = UART_HEADER_SIZE;

    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending Notifications from Rhodes*/
    mHifNtfnSem = xSemaphoreCreateBinary();
    if (!mHifNtfnSem) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifNtfnSem\n");
        while (1)
            ;
    }
    /* This semaphore is signaled when ACK is received for the Bulkin Operations(USB Write) for sending UCI resp from Rhodes*/
    mHifRspSem = xSemaphoreCreateBinary();
    if (!mHifRspSem) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mHifRspSem\n");
        while (1)
            ;
    }

    usart_config_t config;

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx     = true;
    config.enableRx     = true;
    usartRcvCb          = rcvCb;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_TransferCreateHandle(DEMO_USART, &g_uartHandle, USART_UserCallback, NULL);
    /* Enable RX interrupt. */
    USART_EnableInterrupts(DEMO_USART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    NVIC_SetPriority(DEMO_USART_IRQn, USART_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ(DEMO_USART_IRQn);
    USART_RCV_HDR_NON_BLOCKING(buffer, UART_HEADER_SIZE, &rxReceivedBytes);
}

void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status) {
        txOnGoing = false;
    }

    if (kStatus_USART_RxIdle == status) {
        size_t rxReceivedBytes = 0;
        uint32_t receivedBytes = handle->rxDataSizeAll - handle->rxDataSize;
        status                 = USART_TransferGetReceiveCount(base, handle, &receivedBytes);

        uint32_t payloadLength = (buffer[UART_HEADER_SIZE - 2] << (8 * 1)) | (buffer[UART_HEADER_SIZE - 1] << (8 * 0));

        status = USART_ReadBlocking(base, (&buffer[UART_HEADER_SIZE]), payloadLength);
        payloadLength += UART_HEADER_SIZE;
        if (usartRcvCb) {
            usartRcvCb(buffer, (uint32_t *)&payloadLength);
        }
        memset(buffer, 0, sizeof(buffer));
        USART_RCV_HDR_NON_BLOCKING(buffer, UART_HEADER_SIZE, &rxReceivedBytes);
    }
}

uint32_t transmitToUsart(uint8_t *pData, size_t size)
{
    static uint8_t txBuffer[256] = {0};
    if (size == 0) {
        return 1;
    }
    while (g_uartHandle.txState != 0) {
    }
    memcpy(txBuffer, pData, size);
    sendXfer.data     = &txBuffer[0];
    sendXfer.dataSize = size;
    txOnGoing         = true;
    USART_TransferSendNonBlocking(DEMO_USART, &g_uartHandle, &sendXfer);
    return (uint32_t)kStatus_Success;
}
