/*
 * Copyright 2019,2020 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file    UWB_DEMO.c
 * @brief   Application entry point.
 */
#include "phUwb_BuildConfig.h"

#if 1 //( UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE )

#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)

#elif
#include "MKL28Z7.h"
#endif

#include "fsl_debug_console.h"

#include "UWB_Spi_Driver_Interface.h"
#include "UWB_Evt.h"
#include "UWB_GpioIrq.h"
#include "Uwb_Read_task.h"
#include "driver_config.h"
#include "peripherals.h"
#include "UwbPnpInternal.h"

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
#include "fsl_msmc.h"
#endif

#define HIF_TASK_PRIO        1
#define PNP_APP_TASK_PRIO    1
#define UCI_READER_TASK_PRIO 1
#define HIF_WRITER_TASK_PRIO 1

xTaskHandle mHifTask;
xTaskHandle mPnpAppTask;
xTaskHandle mUciReaderTask;
xTaskHandle mHifWriterTask;
QueueHandle_t mHifWriteMutex;
QueueHandle_t mHifSyncMutex;
QueueHandle_t mHifWriteQueue;
SemaphoreHandle_t mHifIsr_Sem;
static uint8_t mRxData[UWB_MAX_HELIOS_RSP_SIZE];
extern QueueHandle_t mReadTaskSyncMutex;
static uint8_t mTlvBuf[TLV_RESP_SIZE];

/*USB header is handled here as per designed doc*/
void UWB_Handle_SR100T_TLV(tlv_t *tlv)
{
    switch (tlv->type) {
    case UCI_CMD: {
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF_WITH_TIME("UCI cmd:");
        for (int i = 0; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif

        if (UWB_SpiUciWrite(tlv->value, tlv->size)) {
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }

    } break;

    case HBCI_CMD: {
        uint16_t rspLen;

#if (ENABLE_HBCI_CMD_LOGGING == ENABLED)
        PRINTF_WITH_TIME("HBCI cmd:");
        for (int i = 0; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif

        if (UWB_SpiHbciXfer(tlv->value, tlv->size, mRxData, &rspLen)) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }
    } break;

    case HBCI_QUERY_CMD: {
#if (ENABLE_HBCI_CMD_LOGGING == ENABLED)
        PRINTF_WITH_TIME("HBCI cmd:");
        for (int i = 1; i < tlv->size; i++) {
            PRINTF(" %02x", tlv->value[i]);
        }
        PRINTF("\n");
#endif
        UINT8 readLen = tlv->value[0];
        if (UWB_SpiHbciXferWithLen(&tlv->value[1], (tlv->size - 1), mRxData, readLen)) {
            if (readLen > 0) {
                UWB_Hif_SendRsp(mRxData, readLen);
            }
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }
    } break;

    case HBCI_LAST_CMD: {
        uint16_t rspLen;

#if (ENABLE_HBCI_CMD_LOGGING == ENABLED)
        PRINTF_WITH_TIME("HBCI cmd:");
        for (int i = 0; i < tlv->size; i++) {
            PRINTF_WITH_TIME(" %02x", tlv->value[i]);
        }
        PRINTF_WITH_TIME("\n");
#endif
        /* For the query command after HBCI image DND, host needs to wait till IRQ is received from Helios */
        if (!UWB_GpioRead(HELIOS_IRQ)) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (UWB_SpiHbciXfer(tlv->value, tlv->size, mRxData, &rspLen)) {
            UWB_Hif_SendRsp(mRxData, rspLen);
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }
        /*After Query command Spurious Interrupt is expected, Add 100msec delay before resuming UCI UCI read task*/
        vTaskDelay(pdMS_TO_TICKS(10));
        /* After HBCI DND resume UCI mode by resuming tasks required for UCI*/

        /* Note: at this point UCI reader task is blocked waiting for HELIOS IRQ interrupt. After FW download interrupt
             * may be disabled which will cause UCI reader task to block forever. Enable back the interrupt to prevent that
             */
        UWB_HeliosIrqEnable();
        vTaskResume(mUciReaderTask);
        vTaskResume(mHifWriterTask);
    } break;
    case RESET: {
        PRINTF_WITH_TIME("Reset Received\n");
        /*Acquire mReadTaskSyncMutex to make sure there is no pending
             * operation in the read task and then suspend read task*/
        xSemaphoreTake(mReadTaskSyncMutex, portMAX_DELAY);
        PRINTF_WITH_TIME("After mReadTaskSyncMutex\n");
        /*Acquire mUsbSyncMutex to make sure there is no pending
             * operation in the mUsbWriterTask task and then suspend mUsbWriterTask task*/
        xSemaphoreTake(mHifSyncMutex, portMAX_DELAY);
        PRINTF_WITH_TIME("After mUsbSyncMutex\n");
        vTaskSuspend(mUciReaderTask);
        vTaskSuspend(mHifWriterTask);

        /*Delete all the elements from the USB Write Queue before going in to Bootrom mode*/
        int itemsInQueue = (WRITER_QUEUE_SIZE - uxQueueSpacesAvailable(mHifWriteQueue));
        PRINTF_WITH_TIME("items in Queue : %d\n", itemsInQueue);
        for (int i = 0; i < itemsInQueue; i++) {
            tlv_t tlv;
            if (xQueueReceive(mHifWriteQueue, &tlv, 0) == pdFALSE) {
                PRINTF_WITH_TIME("Failed to Receive an item\n");
                continue;
            }
            if ((uint8_t *)tlv.value != NULL) {
                vPortFree((uint8_t *)tlv.value);
                tlv.value = NULL;
                PRINTF_WITH_TIME("FREE\r\n");
            }
        }
        PRINTF_WITH_TIME("Chip Enable Started\n");
        // Assert/deassert pin
        UWB_GpioSet(HELIOS_ENABLE, 0);
        UWB_GpioSet(HELIOS_RTC_SYNC, 0);
        USLEEP(5000);
        UWB_GpioSet(HELIOS_ENABLE, 1);
        UWB_GpioSet(HELIOS_RTC_SYNC, 1);
        USLEEP(50000);
        PRINTF_WITH_TIME("Chip Enable Completed\n");
        xSemaphoreGive(mHifSyncMutex);
        xSemaphoreGive(mReadTaskSyncMutex);
        mTlvBuf[0] = 0x01;
        mTlvBuf[1] = 0x02;
        mTlvBuf[2] = 0x03;
        mTlvBuf[3] = 0x04;

        // Send response over USB to HOST upper layer
        UWB_Hif_SendRsp(mTlvBuf, RESET_SOFTWARE_VERSION_SIZE);
    } break;
    case GET_SOFTWARE_VERSION: {
        mTlvBuf[0] = GET_SOFTWARE_VERSION;
        mTlvBuf[1] = 0x02;
        mTlvBuf[2] = RHODES_MW_MAJOR_VERSION;
        mTlvBuf[3] = RHODES_MW_MINOR_VERSION;
        UWB_Hif_SendRsp(mTlvBuf, RESET_SOFTWARE_VERSION_SIZE);
    } break;
    case GET_BOARD_ID: {
        uint8_t len = 0;
        mTlvBuf[0]  = GET_BOARD_ID;
        mTlvBuf[1]  = 0x10;
        BOARD_GetMCUUid(&mTlvBuf[2], &len);
        UWB_Hif_SendRsp(mTlvBuf, (len + 2));
    } break;
    case MCU_RESET: {
        PRINTF_WITH_TIME("Issuing NVIC_SystemReset, MCU going to reboot!\n");
        NVIC_SystemReset(); // No prints after this line wont appear
    } break;
    default:
        PRINTF_WITH_TIME("ERROR: invalid TLV type %02x\n", tlv->type);
    }
}

/*
 * @brief   Application entry point.
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
    BOARD_InitBootPeripherals();

    /* From USB sample app */
    SCG->FIRCCSR |= SCG_FIRCCSR_FIRCSTEN_MASK | SCG_FIRCCSR_FIRCLPEN_MASK;
    SPM->CORELPCNFG |= SPM_CORELPCNFG_BGEN_MASK | SPM_CORELPCNFG_BGBDS_MASK;
    SMC_SetPowerModeProtection(SMC0, kSMC_AllowPowerModeVlp);

#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
    BOARD_BootClockHSRUN();
    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_IpSrcFircAsync);
#endif

    /* Init FSL debug console. */
    BOARD_InitDebugConsole();

    PRINTF_WITH_TIME("UWB PNP Major Version: 0x%d\n", RHODES_MW_MAJOR_VERSION);
    PRINTF_WITH_TIME("UWB PNP Minor Version: 0x%d\n", RHODES_MW_MINOR_VERSION);

    /* Init Helios subsystem */
    if (UWB_HeliosSpiInit()) {
        PRINTF_WITH_TIME("main(): Helios initialized\n");
    }
    else {
        PRINTF_WITH_TIME("CRITICAL: error initializing Helios\n");
        while (1)
            ;
    }

    /* Init Helios subsystem */
    UWB_GpioInit();
    PRINTF_WITH_TIME("main(): GPIO/IRQ module initialized\n");

    /* Enable power domains */
#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
    UWB_GpioSet(UWB_ENABLE_1V8, true);
    UWB_GpioSet(DIG_ENABLE_1V8, true);
#endif

    /* This mutex is used to make USB write operations(Bulkin) from Rhodes mutually exclusive . */
    mHifWriteMutex = xSemaphoreCreateMutex();
    if (!mHifWriteMutex) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbWriteMutex\n");
        while (1)
            ;
    }
    /* This mutex is used to make Reset operation and USB write operation from UWB_WriterTask mutually exclusive
     * anytime Host can send a Reset command when ranging is ongoing*/
    mHifSyncMutex = xSemaphoreCreateMutex();
    if (!mHifSyncMutex) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbSyncMutex\n");
        while (1)
            ;
    }
    /* This semaphore is signaled in the USB CDC ISR context when any command is received from Host*/
    mHifIsr_Sem = xSemaphoreCreateBinary();
    if (!mHifIsr_Sem) {
        PRINTF_WITH_TIME("Error: main, could not create semaphore mSem\n");
        while (1)
            ;
    }
    /* This Queue is used to store the notifications received from helios
     * Currently it can store WRITER_QUEUE_SIZE elements*/
    mHifWriteQueue = xQueueCreate(WRITER_QUEUE_SIZE, sizeof(tlv_t));
    if (!mHifWriteQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mUsbWriteQueue\n");
        while (1)
            ;
    }
    /* This Queue is used to store the commands received from Host
     * Currently it can store MAX 1 element at a time*/
    mHifCommandQueue = xQueueCreate(1, sizeof(UWB_Evt_t));
    if (!mHifCommandQueue) {
        PRINTF_WITH_TIME("Error: main, could not create queue mUsbCommandQueue\n");
        while (1)
            ;
    }
    /*This is the PNP Rhodes Application task which receives all the command sent by Host*/
    xTaskCreate(UWB_Pnp_App_Task, "UWB_Pnp_App_Task", PNP_APP_TASK_SIZE, NULL, PNP_APP_TASK_PRIO, &mPnpAppTask);
    /*This task is waiting on a CDC USB interrupt. Once USB command is received, it is forwarded to UWB_HeliosTask queue for the
     *further processing */
    xTaskCreate(UWB_Hif_Handler_Task, "UWB_HIF_Task", HIF_TASK_SIZE, NULL, HIF_TASK_PRIO, &mHifTask);
    /*This task is used for reading UCI resp and notifications from Helios. its blocked on a helios IRQ interrupt*/
    xTaskCreate(UCI_ReaderTask, "UCI_ReaderTask", UCI_READER_TASK_SIZE, NULL, UCI_READER_TASK_PRIO, &mUciReaderTask);
    /*This task is used for sending all notifications received from helios to Host via CDC USB interface.*/
    xTaskCreate(UWB_WriterTask, "UWB_WriterTask", HIF_WRITER_TASK_SIZE, NULL, HIF_WRITER_TASK_PRIO, &mHifWriterTask);

    vTaskStartScheduler();
    return 0;
}

void PRINTF_WITH_TIME(const char *fmt, ...)
{
    /* No printing here */
}

#endif
