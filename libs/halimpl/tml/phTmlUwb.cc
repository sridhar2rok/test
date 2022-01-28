/*
 * Copyright 2012-2020 NXP.
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
#include <ctype.h>
#include "phTmlUwb.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"

#include "phOsalUwb.h"
#include "phOsalUwb_Thread.h"
#include "phOsalUwb_Timer.h"
#include "phTmlUwb_spi.h"
#include <phDal4Uwb_messageQueueLib.h>
#include "phNxpLogApis_TmlUwb.h"
#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"

#if (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_CDC_MODE)
#include "UwbHif.h"
#endif

#define MAX_RX_BUFFER 2060
extern phNxpUciHal_Control_t nxpucihal_ctrl;

/* Value to reset variables of TML  */
#define PH_TMLUWB_RESET_VALUE (0x00)

/* Indicates a Initial or offset value */
#define PH_TMLUWB_VALUE_ONE         (0x01)
#define TML_READ_WRITE_SYNC_TIMEOUT (10)

#define EXT_UCI_MSG_DBG_CIR0_LOG_NTF    0x04
#define EXT_UCI_MSG_DBG_CIR1_LOG_NTF    0x05
#define EXT_UCI_MSG_DBG_DATA_LOGGER_NTF 0x02
#define EXT_UCI_MSG_DBG_PSDU_LOG_NTF    0x09
#define UCI_GID_PROPRIETARY             0x0E /* 0111b Proprietary Group */

/* Initialize Context structure pointer used to access context structure */
phTmlUwb_Context_t *gpphTmlUwb_Context = NULL;

/* Local Function prototypes */
static UWBSTATUS phTmlUwb_StartThread(void);
static void phTmlUwb_CleanUp(void);
static void phTmlUwb_ReadDeferredCb(void *pParams);
static void phTmlUwb_WriteDeferredCb(void *pParams);
static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlReaderThread(void *pParam);
static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlWriterThread(void *pParam);

static void phTmlUwb_WaitWriteComplete(void);
static void phTmlUwb_SignalWriteComplete(void);

/* for debugging purpose, print log messages from FW */
#define SR040_PRINT_DEBUG_LOG_MESSAGES 1
#if (UWBIOT_UWBD_SR040)
/* Extended handling to print debug messages from SR040 */
static void phTmlUwb_PrintRecevedMessage(const uint8_t *const pBuffer, const uint16_t wLength);
#endif

/* Function definitions */

/*******************************************************************************
**
** Function         phTmlUwb_Init
**
** Description      Provides initialization of TML layer and hardware interface
**                  Configures given hardware interface and sends handle to the
**                  caller
**
** Parameters       pConfig - TML configuration details as provided by the upper
**                            layer
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - initialization successful
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - initialization failed (for example,
**                                     unable to open hardware interface)
**                  UWBSTATUS_INVALID_DEVICE - device has not been opened or has
**                                             been disconnected
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Init(pphTmlUwb_Config_t pConfig)
{
    UWBSTATUS wInitStatus = UWBSTATUS_SUCCESS;

    /* Check if TML layer is already Initialized */
    if (NULL != gpphTmlUwb_Context) {
        /* TML initialization is already completed */
        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_ALREADY_INITIALISED);
    }
    /* Validate Input parameters */
    else if ((NULL == pConfig) || (PH_TMLUWB_RESET_VALUE == pConfig->dwGetMsgThreadId)) {
        /*Parameters passed to TML init are wrong */
        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Allocate memory for TML context */
        gpphTmlUwb_Context = (phTmlUwb_Context_t *)phOsalUwb_GetMemory(sizeof(phTmlUwb_Context_t));

        if (NULL == gpphTmlUwb_Context) {
            wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
        }
        else {
            /* Initialise all the internal TML variables */
            phOsalUwb_SetMemory(gpphTmlUwb_Context, PH_TMLUWB_RESET_VALUE, sizeof(phTmlUwb_Context_t));
            /* Make sure that the thread runs once it is created */
            gpphTmlUwb_Context->bThreadDone = 1;

            /* Open the device file to which data is read/written */
            wInitStatus = phTmlUwb_spi_open_and_configure(pConfig, &(gpphTmlUwb_Context->pDevHandle));

            if (UWBSTATUS_SUCCESS != wInitStatus) {
                wInitStatus                    = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_DEVICE);
                gpphTmlUwb_Context->pDevHandle = NULL;
            }
            else {
                gpphTmlUwb_Context->tReadInfo.bEnable      = 0;
                gpphTmlUwb_Context->tWriteInfo.bEnable     = 0;
                gpphTmlUwb_Context->tReadInfo.bThreadBusy  = false;
                gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;
                if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->rxSemaphore, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->txSemaphore, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->postMsgSemaphore, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else if (0 != phOsalUwb_CreateSemaphore(&gpphTmlUwb_Context->wait_busy_condition, 0)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else if (0 != phOsalUwb_CreateMutex(&gpphTmlUwb_Context->wait_busy_lock)) {
                    wInitStatus = UWBSTATUS_FAILED;
                }
                else {
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
                    /* Start TML thread (to handle write and read operations) */
                    if (UWBSTATUS_SUCCESS != phTmlUwb_StartThread()) {
                        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
                    }
                    else {
                        /* Store the Thread Identifier to which Message is to be posted */
                        gpphTmlUwb_Context->dwCallbackThreadId = pConfig->dwGetMsgThreadId;
                    }
                }
            }
        }
    }
    /* Clean up all the TML resources if any error */
    if (UWBSTATUS_SUCCESS != wInitStatus) {
        /* Clear all handles and memory locations initialized during init */
        phTmlUwb_CleanUp();
    }

    return wInitStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_StartThread
**
** Description      Initializes comport, reader and writer threads
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - threads initialized successfully
**                  UWBSTATUS_FAILED - initialization failed due to system error
**
*******************************************************************************/
static UWBSTATUS phTmlUwb_StartThread(void)
{
    UWBSTATUS wStartStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;

    /* Create Reader and Writer threads */
    threadparams.stackdepth = TMLREADER_STACK_SIZE;
    strcpy((char *)threadparams.taskname, "TMLREAD");
    threadparams.pContext = NULL;
    threadparams.priority = TMLREADER_PRIO;
    pthread_create_status =
        phOsalUwb_Thread_Create((void **)&gpphTmlUwb_Context->readerThread, &phTmlUwb_TmlReaderThread, &threadparams);
    if (0 != pthread_create_status) {
        wStartStatus = UWBSTATUS_FAILED;
        NXPLOG_UWB_TML_E("\n\r ---phTmlUwb_TmlReaderThread Task create failed \n");
    }
    else {
        threadparams.stackdepth = TMLWRITER_STACK_SIZE;
        strcpy((char *)threadparams.taskname, "TMLWRITE");
        threadparams.pContext = NULL;
        threadparams.priority = TMLWRITER_PRIO;
        pthread_create_status = phOsalUwb_Thread_Create(
            (void **)&gpphTmlUwb_Context->writerThread, &phTmlUwb_TmlWriterThread, &threadparams);
        if (0 != pthread_create_status) {
            wStartStatus = UWBSTATUS_FAILED;
            NXPLOG_UWB_TML_E("\n\r ---phTmlUwb_TmlWriterThread Task create failed \n");
        }
    }

    return wStartStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_TmlReaderThread
**
** Description      Read the data from the lower layer driver
**
** Parameters       pParam  - parameters for Writer thread function
**
** Returns          None
**
*******************************************************************************/
static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlReaderThread(void *pParam)
{
    UWBSTATUS wStatus     = UWBSTATUS_SUCCESS;
    int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
    static uint8_t temp[MAX_RX_BUFFER];
    static uint8_t *temp_payload = &temp[4];
    /* Transaction info 2060 to be passed to Callback Thread */
    static phTmlUwb_TransactInfo_t tTransactionInfo;
    /* Structure containing Tml callback function and parameters to be invoked
     by the callback thread */
    static phLibUwb_DeferredCall_t tDeferredInfo;
#if (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_CDC_MODE)
    uint8_t gid, oid;
#endif
    /* Initialize Message structure to post message onto Callback Thread */
    static phLibUwb_Message_t tMsg;
    PHUWB_UNUSED(pParam);
    NXPLOG_UWB_TML_D("Tml Reader Thread Started................\n");
    /* Writer thread loop shall be running till shutdown is invoked */
    while (gpphTmlUwb_Context->bThreadDone) {
        /* If Tml write is requested */
        /* Set the variable to success initially */
        wStatus = UWBSTATUS_SUCCESS;
        phOsalUwb_ConsumeSemaphore(gpphTmlUwb_Context->rxSemaphore);

        /* If Tml read is requested */
        if (1 == gpphTmlUwb_Context->tReadInfo.bEnable) {
            NXPLOG_UWB_TML_D("Read requested.....\n");
            /* Set the variable to success initially */
            wStatus = UWBSTATUS_SUCCESS;

            /* Variable to fetch the actual number of bytes read */
            dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

            /* Read the data from the file onto the buffer */
            if (NULL != gpphTmlUwb_Context->pDevHandle) {
                NXPLOG_UWB_TML_D("Invoking SPI Read.....\n");
                dwNoBytesWrRd = phTmlUwb_spi_read(gpphTmlUwb_Context->pDevHandle, temp_payload, 260);
                if (gpphTmlUwb_Context->bThreadDone == 0) {
                    return OSAL_TASK_RETURN_VALUE;
                }

                if (dwNoBytesWrRd > MAX_RX_BUFFER) {
                    NXPLOG_UWB_TML_E("Numer of bytes read exceeds the limit .....\n");
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                }
                else if (0 == dwNoBytesWrRd) {
#if UWBIOT_TML_S32UART || UWBIOT_TML_PNP
                    /* Dont' warn */
#else
                    NXPLOG_UWB_TML_D("Empty packet Read, Ignore read and try new read...\n");
#endif
                    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
                }
                else {
                    phOsalUwb_MemCopy(gpphTmlUwb_Context->tReadInfo.pBuffer, temp_payload, (uint32_t)dwNoBytesWrRd);

                    NXPLOG_UWB_TML_D("SPI Read successful.....\n");
                    /* This has to be reset only after a successful read */
                    gpphTmlUwb_Context->tReadInfo.bEnable = 0;
                    /* Update the actual number of bytes read including header */
                    gpphTmlUwb_Context->tReadInfo.wLength = (uint16_t)(dwNoBytesWrRd);

                    dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

                    /* Fill the Transaction info structure to be passed to Callback
                     * Function */
                    tTransactionInfo.wStatus = wStatus;
                    tTransactionInfo.pBuff   = gpphTmlUwb_Context->tReadInfo.pBuffer;
                    /* Actual number of bytes read is filled in the structure */
                    tTransactionInfo.wLength = gpphTmlUwb_Context->tReadInfo.wLength;

                    /* Read operation completed successfully. Post a Message onto Callback
                     * Thread*/
                    /* Prepare the message to be posted on User thread */
                    tDeferredInfo.pCallback  = &phTmlUwb_ReadDeferredCb;
                    tDeferredInfo.pParameter = &tTransactionInfo;
                    tMsg.eMsgType            = PH_LIBUWB_DEFERREDCALL_MSG;
                    tMsg.pMsgData            = &tDeferredInfo;
                    tMsg.Size                = sizeof(tDeferredInfo);
                    if ((gpphTmlUwb_Context->bWriterCbflag == FALSE) &&
                        ((gpphTmlUwb_Context->tReadInfo.pBuffer[0] & 0x60) != 0x60) &&
                        ((gpphTmlUwb_Context->tReadInfo.pBuffer[0] & 0x70) != 0x70)) {
                        phTmlUwb_WaitWriteComplete();
                    }
#if (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_CDC_MODE)

                    gid = gpphTmlUwb_Context->tReadInfo.pBuffer[0] & UCI_GID_MASK;
                    oid = gpphTmlUwb_Context->tReadInfo.pBuffer[1] & UCI_OID_MASK;
                    if ((gid == UCI_GID_PROPRIETARY) &&
                        ((oid == EXT_UCI_MSG_DBG_CIR0_LOG_NTF) || (oid == EXT_UCI_MSG_DBG_CIR1_LOG_NTF) ||
                            (oid == EXT_UCI_MSG_DBG_DATA_LOGGER_NTF) || (oid == EXT_UCI_MSG_DBG_PSDU_LOG_NTF))) {
                        UINT16 rxLen            = (UINT16)(gpphTmlUwb_Context->tReadInfo.wLength + (uint16_t)1);
                        temp[CMD_TYPE_OFFSET]   = UWB_NTF_MANAGEMENT;
                        temp[LSB_LENGTH_OFFSET] = (UINT8)(rxLen);                    // PAYLOAD SIZE LSB
                        temp[MSB_LENGTH_OFFSET] = (UINT8)(rxLen >> MSB_LENGTH_MASK); // PAYLOAD SIZE MSB
                        switch (oid) {
                        case EXT_UCI_MSG_DBG_CIR0_LOG_NTF: {
                            temp[CMD_SUB_TYPE_OFFSET] = CIR0_NTF;
                        } break;
                        case EXT_UCI_MSG_DBG_CIR1_LOG_NTF: {
                            temp[CMD_SUB_TYPE_OFFSET] = CIR1_NTF;
                        } break;
                        case EXT_UCI_MSG_DBG_DATA_LOGGER_NTF: {
                            temp[CMD_SUB_TYPE_OFFSET] = DATALOGGER_NTF;
                        } break;
                        case EXT_UCI_MSG_DBG_PSDU_LOG_NTF: {
                            temp[CMD_SUB_TYPE_OFFSET] = DEBUG_LOG_NTF;
                            temp[CMD_SUB_TYPE_OFFSET] = DEBUG_LOG_NTF;
                        } break;
                        }
                        UWB_Hif_SendRsp(temp, (UINT16)(rxLen + 3));
                    }
                    else {
                        LOG_RX("RECV ", gpphTmlUwb_Context->tReadInfo.pBuffer, gpphTmlUwb_Context->tReadInfo.wLength);
                    }
#else
#if (UWBIOT_UWBD_SR040)
                    phTmlUwb_PrintRecevedMessage(
                        gpphTmlUwb_Context->tReadInfo.pBuffer, gpphTmlUwb_Context->tReadInfo.wLength);
#else
                    LOG_RX("RECV ", gpphTmlUwb_Context->tReadInfo.pBuffer, gpphTmlUwb_Context->tReadInfo.wLength);
#endif
#endif
                    NXPLOG_UWB_TML_D("Posting read message.....\n");
                    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &tMsg);
                }
            }
            else {
                NXPLOG_UWB_TML_D("SR100 -gpphTmlUwb_Context->pDevHandle is NULL");
            }
        }
        else {
            NXPLOG_UWB_TML_D("read request NOT enabled");
            phOsalUwb_Delay(10);
        }
    } /* End of While loop */

    return OSAL_TASK_RETURN_VALUE;
}

/*******************************************************************************
**
** Function         phTmlUwb_TmlWriterThread
**
** Description      Writes the requested data onto the lower layer driver
**
** Parameters       pParam  - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static OSAL_TASK_RETURN_TYPE phTmlUwb_TmlWriterThread(void *pParam)
{
    UWBSTATUS wStatus     = UWBSTATUS_SUCCESS;
    int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
    /* Transaction info buffer to be passed to Callback Thread */
    static phTmlUwb_TransactInfo_t tTransactionInfo;
    /* Structure containing Tml callback function and parameters to be invoked
       by the callback thread */
    static phLibUwb_DeferredCall_t tDeferredInfo;
    /* Initialize Message structure to post message onto Callback Thread */
    static phLibUwb_Message_t tMsg;

    PHUWB_UNUSED(pParam);
    NXPLOG_UWB_TML_D("Tml Writer Thread Started................\n");

    /* Writer thread loop shall be running till shutdown is invoked */
    while (gpphTmlUwb_Context->bThreadDone) {
        NXPLOG_UWB_TML_D("Tml Writer Thread Running................\n");
        if (phOsalUwb_ConsumeSemaphore(gpphTmlUwb_Context->txSemaphore) != 0) {
            NXPLOG_UWB_TML_E("Failed to wait semaphore ");
        }
        /* If Tml write is requested */
        if (1 == gpphTmlUwb_Context->tWriteInfo.bEnable) {
            NXPLOG_UWB_TML_D("Write requested.....\n");
            /* Set the variable to success initially */
            wStatus = UWBSTATUS_SUCCESS;
            if (NULL != gpphTmlUwb_Context->pDevHandle) {
                gpphTmlUwb_Context->tWriteInfo.bEnable = 0;
                /* Variable to fetch the actual number of bytes written */
                dwNoBytesWrRd                     = PH_TMLUWB_RESET_VALUE;
                gpphTmlUwb_Context->bWriterCbflag = FALSE;
                /* Write the data in the buffer onto the file */
                NXPLOG_UWB_TML_D("Invoking SPI Write.....\n");
                if (nxpucihal_ctrl.IsDev_suspend_enabled) {
                    NXPLOG_UWB_TML_D("writing dummy packet during standby.\n");
                    dwNoBytesWrRd = phTmlUwb_spi_write(
                        gpphTmlUwb_Context->pDevHandle, gpphTmlUwb_Context->tWriteInfo.pBuffer, NORMAL_MODE_HEADER_LEN);
                    if (-1 == dwNoBytesWrRd) {
                        NXPLOG_UWB_TML_D("Error in SPI write...\n");
                    }
                    nxpucihal_ctrl.IsDev_suspend_enabled = false;
                    phOsalUwb_Delay(40);
                }
                dwNoBytesWrRd = phTmlUwb_spi_write(gpphTmlUwb_Context->pDevHandle,
                    gpphTmlUwb_Context->tWriteInfo.pBuffer,
                    gpphTmlUwb_Context->tWriteInfo.wLength);

                /* Try SPI Write Five Times, if it fails : Raju */
                if (-1 == dwNoBytesWrRd) {
                    NXPLOG_UWB_TML_E("Error in SPI Write.....\n");
                    wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
                }
                else {
                    LOG_TX("SEND ", gpphTmlUwb_Context->tWriteInfo.pBuffer, gpphTmlUwb_Context->tWriteInfo.wLength);
                }
                // retry_cnt = 0;
                if (UWBSTATUS_SUCCESS == wStatus) {
                    // NXPLOG_UWB_TML_D("SPI Write successful.....\n");
                    dwNoBytesWrRd = PH_TMLUWB_VALUE_ONE;
                }
                /* Fill the Transaction info structure to be passed to Callback Function */
                tTransactionInfo.wStatus = wStatus;
                tTransactionInfo.pBuff   = gpphTmlUwb_Context->tWriteInfo.pBuffer;
                /* Actual number of bytes written is filled in the structure */
                tTransactionInfo.wLength = (uint16_t)dwNoBytesWrRd;

                /* Prepare the message to be posted on the User thread */
                tDeferredInfo.pCallback  = &phTmlUwb_WriteDeferredCb;
                tDeferredInfo.pParameter = &tTransactionInfo;
                /* Write operation completed successfully. Post a Message onto Callback Thread*/
                tMsg.eMsgType = PH_LIBUWB_DEFERREDCALL_MSG;
                tMsg.pMsgData = &tDeferredInfo;
                tMsg.Size     = sizeof(tDeferredInfo);

                //  NXPLOG_UWB_TML_D("Posting Fresh Write message.....\n");
                phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &tMsg);
                if (UWBSTATUS_SUCCESS == wStatus) {
                    gpphTmlUwb_Context->bWriterCbflag = TRUE;
                    phTmlUwb_SignalWriteComplete();
                }
            }
            else {
                NXPLOG_UWB_TML_D("gpphTmlUwb_Context->pDevHandle is NULL");
            }
        }
        else {
            NXPLOG_UWB_TML_D("Write request NOT enabled");
            phOsalUwb_Delay(10);
        }

    } /* End of While loop */

    return OSAL_TASK_RETURN_VALUE;
}

/*******************************************************************************
**
** Function         phTmlUwb_WaitWriteComplete
**
** Description      wait function for reader thread
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_WaitWriteComplete(void)
{
    phOsalUwb_LockMutex(gpphTmlUwb_Context->wait_busy_lock);
    gpphTmlUwb_Context->bWait_busy_flag = TRUE;
    phOsalUwb_UnlockMutex(gpphTmlUwb_Context->wait_busy_lock);
    NXPLOG_UWB_TML_D("phTmlUwb_WaitWriteComplete - enter");
    if (phOsalUwb_ConsumeSemaphore_WithTimeout(gpphTmlUwb_Context->wait_busy_condition, TML_READ_WRITE_SYNC_TIMEOUT) !=
        UWBSTATUS_SUCCESS) {
        phOsalUwb_LockMutex(gpphTmlUwb_Context->wait_busy_lock);
        gpphTmlUwb_Context->bWait_busy_flag = FALSE;
        phOsalUwb_UnlockMutex(gpphTmlUwb_Context->wait_busy_lock);
        NXPLOG_UWB_TML_E("Reader Thread wait failed");
    }
    NXPLOG_UWB_TML_D("phTmlUwb_WaitWriteComplete - exit");
}

/*******************************************************************************
**
** Function         phTmlUwb_SignalWriteComplete
**
** Description      function to invoke reader thread
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_SignalWriteComplete(void)
{
    if (gpphTmlUwb_Context->bWait_busy_flag == TRUE) {
        NXPLOG_UWB_TML_D("phTmlUwb_SignalWriteComplete - enter");
        phOsalUwb_LockMutex(gpphTmlUwb_Context->wait_busy_lock);
        gpphTmlUwb_Context->bWait_busy_flag = FALSE;
        phOsalUwb_UnlockMutex(gpphTmlUwb_Context->wait_busy_lock);
        phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->wait_busy_condition);
        NXPLOG_UWB_TML_D("phTmlUwb_SignalWriteComplete - exit");
    }
}

/*******************************************************************************
**
** Function         phTmlUwb_CleanUp
**
** Description      Clears all handles opened during TML initialization
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_CleanUp(void)
{
    if (NULL == gpphTmlUwb_Context) {
        return;
    }
    if (NULL != gpphTmlUwb_Context->pDevHandle) {
        (void)phTmlUwb_spi_reset(gpphTmlUwb_Context->pDevHandle, 0);
        gpphTmlUwb_Context->bThreadDone = 0;
    }

    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->rxSemaphore);
    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->txSemaphore);
    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->postMsgSemaphore);
    phOsalUwb_DeleteSemaphore(&gpphTmlUwb_Context->wait_busy_condition);
    phOsalUwb_DeleteMutex(&gpphTmlUwb_Context->wait_busy_lock);
    gpphTmlUwb_Context->pDevHandle = NULL;
    /* Clear memory allocated for storing Context variables */
    phOsalUwb_FreeMemory((void *)gpphTmlUwb_Context);
    /* Set the pointer to NULL to indicate De-Initialization */
    gpphTmlUwb_Context = NULL;
    phTmlUwb_spi_close();
    return;
}

/*******************************************************************************
**
** Function         phTmlUwb_Shutdown
**
** Description      Uninitializes TML layer and hardware interface
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - TML configuration released successfully
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - un-initialization failed (example: unable
**                                     to close interface)
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Shutdown(void)
{
    UWBSTATUS wShutdownStatus = UWBSTATUS_SUCCESS;

    /* Check whether TML is Initialized */
    if (NULL != gpphTmlUwb_Context) {
        /* Reset thread variable to terminate the thread */
        gpphTmlUwb_Context->bThreadDone = 0;

        /* Clear All the resources allocated during initialization */
        phTmlUwb_spi_reset(gpphTmlUwb_Context->pDevHandle, ABORT_READ_PENDING);
        phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore);
        phOsalUwb_Delay(1);
        phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->txSemaphore);
        phOsalUwb_Delay(1);
        phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
        phOsalUwb_Delay(1);

        phOsalUwb_Thread_Delete(gpphTmlUwb_Context->readerThread);
        phOsalUwb_Thread_Delete(gpphTmlUwb_Context->writerThread);

        NXPLOG_UWB_TML_D("bThreadDone == 0");

        phTmlUwb_CleanUp();
    }
    else {
        wShutdownStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
    }

    return wShutdownStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Write
**
** Description      Asynchronously writes given data block to hardware
**                  interface/driver. Enables writer thread if there are no
**                  write requests pending. Returns successfully once writer
**                  thread completes write operation. Notifies upper layer using
**                  callback mechanism.
**
**                  NOTE:
**                  * it is important to post a message with id
**                    PH_TMLUWB_WRITE_MESSAGE to IntegrationThread after data
**                    has been written to SR100
**                  * if CRC needs to be computed, then input buffer should be
**                    capable to store two more bytes apart from length of
**                    packet
**
** Parameters       pBuffer - data to be sent
**                  wLength - length of data buffer
**                  pTmlWriteComplete - pointer to the function to be invoked
**                                      upon completion
**                  pContext - context provided by upper layer
**
** Returns          UWB status:
**                  UWBSTATUS_PENDING - command is yet to be processed
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_BUSY - write request is already in progress
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Write(
    uint8_t *pBuffer, uint16_t wLength, pphTmlUwb_TransactCompletionCb_t pTmlWriteComplete, void *pContext)
{
    UWBSTATUS wWriteStatus;

    /* Check whether TML is Initialized */

    if (NULL != gpphTmlUwb_Context) {
        if ((NULL != gpphTmlUwb_Context->pDevHandle) && (NULL != pBuffer) && (PH_TMLUWB_RESET_VALUE != wLength) &&
            (NULL != pTmlWriteComplete)) {
            if (!gpphTmlUwb_Context->tWriteInfo.bThreadBusy) {
                /* Setting the flag marks beginning of a Write Operation */
                gpphTmlUwb_Context->tWriteInfo.bThreadBusy = true;
                /* Copy the buffer, length and Callback function,
                   This shall be utilized while invoking the Callback function in thread
                   */
                gpphTmlUwb_Context->tWriteInfo.pBuffer          = pBuffer;
                gpphTmlUwb_Context->tWriteInfo.wLength          = wLength;
                gpphTmlUwb_Context->tWriteInfo.pThread_Callback = pTmlWriteComplete;
                gpphTmlUwb_Context->tWriteInfo.pContext         = pContext;

                wWriteStatus = UWBSTATUS_PENDING;
                /* Set event to invoke Writer Thread */
                gpphTmlUwb_Context->tWriteInfo.bEnable = 1;
                phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->txSemaphore);
            }
            else {
                NXPLOG_UWB_TML_D("Writer Busy");
                wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
            }
        }
        else {
            wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
    }

    return wWriteStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Read
**
** Description      Asynchronously reads data from the driver
**                  Number of bytes to be read and buffer are passed by upper
**                  layer.
**                  Enables reader thread if there are no read requests pending
**                  Returns successfully once read operation is completed
**                  Notifies upper layer using callback mechanism
**
** Parameters       pBuffer - location to send read data to the upper layer via
**                            callback
**                  wLength - length of read data buffer passed by upper layer
**                  pTmlReadComplete - pointer to the function to be invoked
**                                     upon completion of read operation
**                  pContext - context provided by upper layer
**
** Returns          UWB status:
**                  UWBSTATUS_PENDING - command is yet to be processed
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_BUSY - read request is already in progress
**
*******************************************************************************/
UWBSTATUS phTmlUwb_Read(
    uint8_t *pBuffer, uint16_t wLength, pphTmlUwb_TransactCompletionCb_t pTmlReadComplete, void *pContext)
{
    UWBSTATUS wReadStatus;

    /* Check whether TML is Initialized */
    if (NULL != gpphTmlUwb_Context) {
        if ((gpphTmlUwb_Context->pDevHandle != NULL) && (NULL != pBuffer) && (PH_TMLUWB_RESET_VALUE != wLength) &&
            (NULL != pTmlReadComplete)) {
            if (!gpphTmlUwb_Context->tReadInfo.bThreadBusy) {
                /* Setting the flag marks beginning of a Read Operation */
                gpphTmlUwb_Context->tReadInfo.bThreadBusy = true;
                /* Copy the buffer, length and Callback function,
                   This shall be utilized while invoking the Callback function in thread
                   */
                gpphTmlUwb_Context->tReadInfo.pBuffer          = pBuffer;
                gpphTmlUwb_Context->tReadInfo.wLength          = wLength;
                gpphTmlUwb_Context->tReadInfo.pThread_Callback = pTmlReadComplete;
                gpphTmlUwb_Context->tReadInfo.pContext         = pContext;
                wReadStatus                                    = UWBSTATUS_PENDING;

                /* Set event to invoke Reader Thread */
                gpphTmlUwb_Context->tReadInfo.bEnable = 1;                   // To be enabled later
                phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->rxSemaphore); // To be enabled later
            }
            else {
                wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
            }
        }
        else {
            wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
        }
    }
    else {
        wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
    }

    return wReadStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadAbort
**
** Description      Aborts pending read request (if any)
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_ReadAbort(void)
{
    gpphTmlUwb_Context->tReadInfo.bEnable = 0;

    /*Reset the flag to accept another Read Request */
    gpphTmlUwb_Context->tReadInfo.bThreadBusy = false;
}

/*******************************************************************************
**
** Function         phTmlUwb_WriteAbort
**
** Description      Aborts pending write request (if any)
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_WriteAbort(void)
{
    gpphTmlUwb_Context->tWriteInfo.bEnable = 0;

    /* Reset the flag to accept another Write Request */
    gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;
}

/*******************************************************************************
**
** Function         phTmlUwb_DeferredCall
**
** Description      Posts message on upper layer thread
**                  upon successful read or write operation
**
** Parameters       dwThreadId  - id of the thread posting message
**                  ptWorkerMsg - message to be posted
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_DeferredCall(uintptr_t dwThreadId, phLibUwb_Message_t *ptWorkerMsg)
{
    PHUWB_UNUSED(dwThreadId);
    /* Post message on the user thread to invoke the callback function */
    if (phOsalUwb_ConsumeSemaphore(gpphTmlUwb_Context->postMsgSemaphore) != 0) {
        NXPLOG_UWB_TML_E("phTmlUwb_DeferredCall: semaphore error");
    }
    phDal4Uwb_msgsnd(gpphTmlUwb_Context->dwCallbackThreadId, ptWorkerMsg, 0);
    phOsalUwb_ProduceSemaphore(gpphTmlUwb_Context->postMsgSemaphore);
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadDeferredCb
**
** Description      Read thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_ReadDeferredCb(void *pParams)
{
    /* Transaction info buffer to be passed to Callback Function */
    phTmlUwb_TransactInfo_t *pTransactionInfo = (phTmlUwb_TransactInfo_t *)pParams;

    /* Reset the flag to accept another Read Request */
    gpphTmlUwb_Context->tReadInfo.bThreadBusy = false;
    gpphTmlUwb_Context->tReadInfo.pThread_Callback(gpphTmlUwb_Context->tReadInfo.pContext, pTransactionInfo);

    return;
}

/*******************************************************************************
**
** Function         phTmlUwb_WriteDeferredCb
**
** Description      Write thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_WriteDeferredCb(void *pParams)
{
    /* Transaction info buffer to be passed to Callback Function */
    phTmlUwb_TransactInfo_t *pTransactionInfo = (phTmlUwb_TransactInfo_t *)pParams;

    /* Reset the flag to accept another Write Request */
    gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;
    gpphTmlUwb_Context->tWriteInfo.pThread_Callback(gpphTmlUwb_Context->tWriteInfo.pContext, pTransactionInfo);

    return;
}
#if (UWBIOT_UWBD_SR040)
/* Print ASCII messages coming from FW */
static void phTmlUwb_PrintRecevedMessage(const uint8_t *const pBuffer, const uint16_t wLength)
{
#if defined(DEBUG) && UWBIOT_UWBD_SR040 && SR040_PRINT_DEBUG_LOG_MESSAGES && defined(_MSC_VER)
    if ((wLength >= 5)                       /* */
        && (pBuffer[0] == 0x6E)              /* */
        && (pBuffer[1] == 0x00)              /* */
        && (pBuffer[2] == 0x00)              /* */
        && (pBuffer[3] == (wLength - 4))     /* */
        && (pBuffer[4] == 0x01)              /* */
        && (pBuffer[5] == 0x0A)              /* */
        && (pBuffer[6] == (wLength - 4 - 3)) /* */
    ) {
        for (int i = 7; i < wLength; i++) {
            if (isspace(pBuffer[i])) {
                /* No new lines */
                PUTCHAR(' ');
            }
            else {
                PUTCHAR(pBuffer[i]);
            }
        }
        PRINTF("\n");
    }
    else
#endif
    {
        LOG_RX("RECV ", pBuffer, wLength);
    }
}
#endif
