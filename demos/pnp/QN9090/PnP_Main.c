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

#include "phUwb_BuildConfig.h"

#include <stdio.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "UWBT_PowerMode.h"
#include "LED.h"
#include "GPIO.h"
#include "UWBT_Buzzer.h"
#include "UWB_Spi_Driver_Interface.h"
#include "UWB_Evt.h"
#include "UWB_GpioIrq.h"
#include "Uwb_Read_task.h"
#include "driver_config.h"
#include "peripherals.h"
#include "UwbPnpInternal.h"
#include "../../SR040/demo_swup_update_fw/SwupPkgFW.h"

#define HIF_TASK_PRIO        3
#define PNP_APP_TASK_PRIO    3
#define UCI_READER_TASK_PRIO 1
#define HIF_WRITER_TASK_PRIO 2

#define UWB_MAX_RSP_SIZE 256

xTaskHandle mHifTask;
xTaskHandle mPnpAppTask;
xTaskHandle mUciReaderTask;
xTaskHandle mHifWriterTask;
QueueHandle_t mHifWriteMutex;
QueueHandle_t mHifSyncMutex;
QueueHandle_t mHifWriteQueue;
SemaphoreHandle_t mHifIsr_Sem;
static uint8_t mTlvBuf[TLV_RESP_SIZE];
interface_config_t swInterface_config = {kInterfaceModeUci};

extern void hardware_init(void);

/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".bss.$SRAM1"))) ucHeap[configTOTAL_HEAP_SIZE];

/*USB header is handled here as per designed doc*/
void UWB_Handle_SR040_TLV(tlv_t *tlv)
{
    switch (tlv->type) {
    case UCI_CMD: {
#if (ENABLE_UCI_CMD_LOGGING == ENABLED)
        PRINTF_WITH_TIME("UCI cmd:");
        for (int i = 0; i < tlv->size; i++) {
            DEBUGOUT(" %02x", tlv->value[i]);
        }
        DEBUGOUT("\n");
#endif
        if (UWB_SpiUciWrite(tlv->value, tlv->size)) {
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }

    } break;

    case RCI_CMD:
        if (UWB_SpiRciTransceive(tlv->value, tlv->size)) {
        }
        else {
            PRINTF_WITH_TIME("ERROR: error processing UCI command\n");
        }

        break;

    case RESET: {
        PRINTF_WITH_TIME("Reset Received\n");
        /*Acquire mReadTaskSyncMutex to make sure there is no pending
             * operation in the read task and then suspend read task*/
        PRINTF_WITH_TIME("After mReadTaskSyncMutex\n");
        /*Acquire mUsbSyncMutex to make sure there is no pending
             * operation in the mUsbWriterTask task and then suspend mUsbWriterTask task*/
        xSemaphoreTake(mHifSyncMutex, portMAX_DELAY);
        PRINTF_WITH_TIME("After mUsbSyncMutex\n");

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
        phNxpUwb_sr040Reset();
        PRINTF_WITH_TIME("Chip Enable Completed\n");
        xSemaphoreGive(mHifSyncMutex);
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
        // BOARD_GetMCUUid(&mTlvBuf[2], &len);
        UWB_Hif_SendRsp(mTlvBuf, (len + 2));
    } break;
    case MCU_RESET: {
        PRINTF_WITH_TIME("Issuing NVIC_SystemReset, MCU going to reboot!\n");
        // NVIC_SystemReset(); // No prints after this line wont appear
    } break;

    case SWITCH_PROTOCOL_CMD: {
        mTlvBuf[0] = SWITCH_PROTOCOL_CMD;
        mTlvBuf[1] = 0x01;
        mTlvBuf[2] = kInterfaceModeUci;
        if (*(tlv->value) == kInterfaceModeSwup) {
            xSemaphoreTake(mHifSyncMutex, portMAX_DELAY);
            swInterface_config.interface_mode = kInterfaceModeSwup;
            vTaskSuspend(mUciReaderTask);
            vTaskSuspend(mHifWriterTask);
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
            mTlvBuf[2] = swInterface_config.interface_mode;
            xSemaphoreGive(mHifSyncMutex);
            UWB_Hif_SendRsp(mTlvBuf, 3);
        }
        else {
            swInterface_config.interface_mode = kInterfaceModeUci;
            mTlvBuf[2]                        = swInterface_config.interface_mode;
            UWB_Hif_SendRsp(mTlvBuf, 3);
            vTaskResume(mUciReaderTask);
            vTaskResume(mHifWriterTask);
        }
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
    hardware_init();

    UWBT_PowerModesInit();
    BOARD_InitHeliosPins();

    /* Call UWB_GpioInit() and LED_Init() in this order */
    UWB_GpioInit();
    UWBT_BuzzerInit();

    BOARD_InitBuzzerPins();

    LED_Init();
    GPIO_InitTimer();
    /* Override pin configurations with SWO, if in debug session */
    //BOARD_InitSwoPins();
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    swInterface_config.interface_mode = kInterfaceModeUci;
    PRINTF_WITH_TIME("UWB PNP Major Version: 0x%d\n", RHODES_MW_MAJOR_VERSION);
    PRINTF_WITH_TIME("UWB PNP Minor Version: 0x%d\n", RHODES_MW_MINOR_VERSION);

    /* Init Helios subsystem */
    UWB_GpioInit();
    PRINTF_WITH_TIME("main(): GPIO/IRQ module initialized\n");

    /* Init Helios subsystem */
    if (UWB_HeliosSpiInit()) {
        PRINTF_WITH_TIME("main(): Helios initialized\n");
    }
    else {
        PRINTF_WITH_TIME("CRITICAL: error initializing Helios\n");
        while (1)
            ;
    }

    /* This mutex is used to make USB write operations(Bulkin) from Rhodes mutually exclusive . */
    mHifWriteMutex = xSemaphoreCreateMutex();
    if (!mHifWriteMutex) {
        PRINTF_WITH_TIME("Error: UWB_HeliosInit(), could not create mutex mUsbWriteMutex\n");
        while (1)
            ;
    }
    /* This mutex is used to make Reset operation and USART write operation from UWB_WriterTask mutually exclusive
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

tUWBAPI_STATUS UpdateFirmware(void)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    uint32_t fwVersion = 0, dspVersion = 0;

    /*Reset SR040 */
    phNxpUwb_HeliosReset();
    status = Uci_PrintDeviceInfo(&fwVersion, &dspVersion);
    if (status != UWBAPI_STATUS_OK) {
        fwVersion = 0xFFFFFF;
    }

    /*indicate FW update using BLUE LED*/
    GPIO_PinWrite(GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, 0);

    /** Check if Device has SWUP enabled.
     *  Send GetDeviceInfo command.
     *  If this function fails, the device is in
     *  UCI mode. Send raw SwupActivate command
     *  to activate SWUP mode.
     */

    phNxpUwb_HeliosReset();

    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    if (fwVersion != 0xFFFFFF) {
        /* Device is in UCI mode */
        if (fwVersion == MAC_RANGING_FW_PKG_VERSION) {
            NXPLOG_APP_W("Same package version found, exiting");
            goto exit;
        }
        else {
            /* Device is in UCI mode. Enable SWUP and move forward */
            status = Uci_EnableSwup();
            phOsalUwb_Delay(500);
            if (status != UWBAPI_STATUS_OK) {
                /* Enable SWUP failed. Device is in UCI mode. Exit */
                goto exit;
            }
        }
    }
    /* If code reaches here, device is in SWUP mode or fwVersion failed */
    SwupDeviceId_t swupDeviceId;
    phNxpUwb_SwitchProtocol(kInterfaceModeSwup);
    if (Swup_ReadDeviceId(&swupDeviceId) != STATUS_CMD_SUCCESS) {
        /* Not in SWUP. Exit. */
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }

    GPIO_PinWrite(GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, 0);
    SwupResponseStatus_t swup_response;
    swup_response = SwupUpdate(MAC_RANGING_FW_PKG, MAC_RANGING_FW_PKG_LEN, kSWUP_KEY_VESRION_UNKNOWN);

    GPIO_PinWrite(GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, 1);
    if (STATUS_CMD_SUCCESS != swup_response) {
        status = UWBAPI_STATUS_FAILED;
        goto exit;
    }
    else {
        status = UWBAPI_STATUS_OK;
    }

exit:
    phOsalUwb_Delay(2000);
    if (status == UWBAPI_STATUS_OK) {
        GPIO_PinWrite(GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, 1);
    }
    phNxpUwb_SwitchProtocol(kInterfaceModeUci);
    phNxpUwb_HeliosReset();
    return status;
}
