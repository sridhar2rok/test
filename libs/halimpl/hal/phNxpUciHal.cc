/*
 * Copyright (C) 2012-2019 NXP Semiconductors
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

#include "phNxpUciHal.h"
#include "FreeRTOS.h"
#include "UwbCoreSDK_Internal.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUciHal_ext.h"
#include "phNxpUwb_SpiTransport.h"
#include "phOsalUwb_Thread.h"
#include "phTmlUwb.h"
#include "phTmlUwb_spi.h"
#include "task.h"
#include "UwbCore_Types.h"
#include "phNxpLogApis_HalUci.h"

#include "UWB_GpioIrq.h"

#include "phOsalUwb_Thread.h"

#define UCI_PBF_MASK  0x10
#define UCI_PBF_SHIFT 0x04
#define UCI_GID_MASK  0x0F
#define UCI_OID_MASK  0x3F

#define UCI_GID_CORE                   0x00
#define UCI_GID_RANGE_MANAGE           0x02 /* 0010b Range Management group */
#define UCI_OID_RANGE_DATA_NTF         0x00
#define UCI_MSG_CORE_DEVICE_STATUS_NTF 0x01

#define UCI_GID_PROPRIETARY             0x0E /* 1110b Proprietary Group */
#define EXT_UCI_MSG_DBG_DATA_LOGGER_NTF 0x03
#define EXT_UCI_MSG_DBG_BIN_LOG_NTF     0x06
#define EXT_UCI_MSG_DBG_CIR0_LOG_NTF    0x04
#define EXT_UCI_MSG_DBG_CIR1_LOG_NTF    0x05
#define EXT_UCI_MSG_DBG_GET_ERROR_LOG   0x09

#define UCI_NTF_PAYLOAD_OFFSET          0x04
#define NORMAL_MODE_LENGTH_OFFSET       0x03
#define EXTENDED_MODE_LEN_OFFSET        0x02
#define EXTENDED_MODE_LEN_SHIFT         0x08
#define EXTND_LEN_INDICATOR_OFFSET      0x01
#define EXTND_LEN_INDICATOR_OFFSET_MASK 0x80

/*********************** Global Variables *************************************/
/* UCI HAL Control structure */
phNxpUciHal_Control_t nxpucihal_ctrl;

/* TML Context */
extern phTmlUwb_Context_t *gpphTmlUwb_Context;

bool uwb_debug_enabled     = true;
uint32_t uwbTimeoutTimerId = 0;

static uint8_t Rx_data[UCI_MAX_DATA_LEN];
/**************** local methods used in this file only ************************/

static void phNxpUciHal_open_complete(UWBSTATUS status);
static void phNxpUciHal_write_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo);
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo);
static void phNxpUciHal_close_complete(UWBSTATUS status);
static OSAL_TASK_RETURN_TYPE phNxpUciHal_client_thread(void *arg);
extern int phNxpUciHal_fw_download();

/******************************************************************************
 * Function         phNxpUciHal_client_thread
 *
 * Description      This function is a thread handler which handles all TML and
 *                  UCI messages.
 *
 * Returns          void
 *
 ******************************************************************************/
static OSAL_TASK_RETURN_TYPE phNxpUciHal_client_thread(void *arg)
{
    phNxpUciHal_Control_t *p_nxpucihal_ctrl = (phNxpUciHal_Control_t *)arg;
    phLibUwb_Message_t msg;

    NXPLOG_UCIHAL_D("thread started");

    p_nxpucihal_ctrl->thread_running = 1;

    /* Initialize the message */
    memset(&msg, 0, sizeof(msg));

    while (p_nxpucihal_ctrl->thread_running == 1) {
        /* Fetch next message from the UWB stack message queue */
        if (phDal4Uwb_msgrcv(p_nxpucihal_ctrl->gDrvCfg.nClientId, &msg, 0, 0) == -1) {
            NXPLOG_UCIHAL_E("UWB client received bad message");
            continue;
        }

        if (p_nxpucihal_ctrl->thread_running == 0) {
            break;
        }

        switch (msg.eMsgType) {
        case PH_LIBUWB_DEFERREDCALL_MSG: {
            phLibUwb_DeferredCall_t *deferCall = (phLibUwb_DeferredCall_t *)(msg.pMsgData);

            REENTRANCE_LOCK();
            deferCall->pCallback(deferCall->pParameter);
            REENTRANCE_UNLOCK();

            break;
        }

        case UCI_HAL_OPEN_CPLT_MSG: {
            REENTRANCE_LOCK();
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_STATUS_OK);
            }
            REENTRANCE_UNLOCK();
            break;
        }

        case UCI_HAL_CLOSE_CPLT_MSG: {
            REENTRANCE_LOCK();
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_CLOSE_CPLT_EVT, HAL_UWB_STATUS_OK);
                phOsalUwb_ProduceSemaphore(p_nxpucihal_ctrl->halClientSemaphore);
            }
            REENTRANCE_UNLOCK();
            break;
        }

        case UCI_HAL_ERROR_MSG: {
            REENTRANCE_LOCK();
            if (nxpucihal_ctrl.p_uwb_stack_cback != NULL) {
                /* Send the event */
                (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_ERROR_EVT, HAL_UWB_ERROR_EVT);
            }
            REENTRANCE_UNLOCK();
            break;
        }
        }
    }

    NXPLOG_UCIHAL_D("NxpUciHal thread stopped");
    // vTaskDelete(NULL);

    return OSAL_TASK_RETURN_VALUE;
}

/******************************************************************************
 * Function         phNxpUciHal_open
 *
 * Description      This function is called by libuwb-uci during the
 *                  initialization of the UWBC. It opens the physical connection
 *                  with UWBC (SR100) and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to libuwb-uci
 *                  through callback function.
 *
 * Returns          This function return UWBSTATUS_SUCCES (0) in case of success
 *                  In case of failure returns other failure value.
 *
 ******************************************************************************/
int phNxpUciHal_open(uwb_stack_callback_t *p_cback, uwb_stack_data_callback_t *p_data_cback)
{
    phOsalUwb_Config_t tOsalConfig;
    phTmlUwb_Config_t tTmlConfig;
    UWBSTATUS wConfigStatus = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t threadparams;
    UWBSTATUS status = UWBSTATUS_SUCCESS;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_OPEN) {
        NXPLOG_UCIHAL_E("phNxpUciHal_open already open");
        return UWBSTATUS_SUCCESS;
    }

    /*Create the timer for extns write response*/
    uwbTimeoutTimerId = phOsalUwb_Timer_Create(FALSE);

    if (phNxpUciHal_init_monitor() == NULL) {
        NXPLOG_UCIHAL_E("Init monitor failed");
        return UWBSTATUS_FAILED;
    }

    CONCURRENCY_LOCK();

    phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));
    phOsalUwb_SetMemory(&tOsalConfig, 0x00, sizeof(tOsalConfig));
    phOsalUwb_SetMemory(&tTmlConfig, 0x00, sizeof(tTmlConfig));

    status = phOsalUwb_CreateSemaphore(&nxpucihal_ctrl.halClientSemaphore, 0);
    if (status != UWBSTATUS_SUCCESS) {
        CONCURRENCY_UNLOCK();
        phNxpUciHal_cleanup_monitor();
        phOsalUwb_Timer_Delete(uwbTimeoutTimerId);
        return status;
    }
    /* By default HAL status is HAL_STATUS_OPEN */
    nxpucihal_ctrl.halStatus = HAL_STATUS_OPEN;

    nxpucihal_ctrl.p_uwb_stack_cback      = p_cback;
    nxpucihal_ctrl.p_uwb_stack_data_cback = p_data_cback;

    nxpucihal_ctrl.IsDev_suspend_enabled  = false;
    nxpucihal_ctrl.IsFwDebugDump_enabled  = false;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = false;
    nxpucihal_ctrl.fw_dwnld_mode          = false;

    /* Configure hardware link */
    nxpucihal_ctrl.gDrvCfg.nClientId = phDal4Uwb_msgget();
    nxpucihal_ctrl.gDrvCfg.nLinkType = ENUM_LINK_TYPE_SPI; /* For SR100 */
    tOsalConfig.dwCallbackThreadId   = (uintptr_t)nxpucihal_ctrl.gDrvCfg.nClientId;
    tOsalConfig.pLogFile             = NULL;
    tTmlConfig.dwGetMsgThreadId      = (uintptr_t)nxpucihal_ctrl.gDrvCfg.nClientId;

    /* Initialize TML layer */
    wConfigStatus = phTmlUwb_Init(&tTmlConfig);
    if (wConfigStatus != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("phTmlUwb_Init Failed");
        goto clean_and_return;
    }
    threadparams.stackdepth = CLIENT_STACK_SIZE;
    strcpy((char *)threadparams.taskname, "CLIENT");
    threadparams.pContext = &nxpucihal_ctrl;
    threadparams.priority = CLIENT_PRIO;
    if (phOsalUwb_Thread_Create((void **)&nxpucihal_ctrl.client_thread, &phNxpUciHal_client_thread, &threadparams) !=
        UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("\n\r ---client_thread Task create failed \n");
        goto clean_and_return;
    }

    CONCURRENCY_UNLOCK();
    /* Call open complete */
    phNxpUciHal_open_complete(wConfigStatus);
    return wConfigStatus;

clean_and_return:
    CONCURRENCY_UNLOCK();

    /* Report error status */
    (*nxpucihal_ctrl.p_uwb_stack_cback)(HAL_UWB_OPEN_CPLT_EVT, HAL_UWB_ERROR_EVT);

    nxpucihal_ctrl.p_uwb_stack_cback      = NULL;
    nxpucihal_ctrl.p_uwb_stack_data_cback = NULL;
    phNxpUciHal_cleanup_monitor();
    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;
    return UWBSTATUS_FAILED;
}

/******************************************************************************
 * Function         phNxpUciHal_open_complete
 *
 * Description      This function inform the status of phNxpUciHal_open
 *                  function to libuwb-uci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_open_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType                   = UCI_HAL_OPEN_CPLT_MSG;
        nxpucihal_ctrl.hal_open_status = true;
        nxpucihal_ctrl.halStatus       = HAL_STATUS_OPEN;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }

    msg.pMsgData = NULL;
    msg.Size     = 0;

    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, (phLibUwb_Message_t *)&msg);

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_write
 *
 * Description      This function write the data to UWBC through physical
 *                  interface (e.g. SPI) using the SR100 driver interface.
 *
 * Returns          It returns number of bytes successfully written to UWBC.
 *
 ******************************************************************************/
int phNxpUciHal_write(uint16_t data_len, const uint8_t *p_data)
{
    uint16_t len;

    if (nxpucihal_ctrl.halStatus != HAL_STATUS_OPEN) {
        return UWBSTATUS_FAILED;
    }

    CONCURRENCY_LOCK();
    len = phNxpUciHal_write_unlocked(data_len, p_data);
    CONCURRENCY_UNLOCK();

    /* No data written */
    return len;
}

/******************************************************************************
 * Function         phNxpUciHal_write_unlocked
 *
 * Description      This is the actual function which is being called by
 *                  phNxpUciHal_write. This function writes the data to UWBC.
 *                  It waits till write callback provide the result of write
 *                  process.
 *
 * Returns          It returns number of bytes successfully written to UWBC.
 *
 ******************************************************************************/
uint16_t phNxpUciHal_write_unlocked(uint16_t data_len, const uint8_t *p_data)
{
    UWBSTATUS status;
    phNxpUciHal_Sem_t cb_data;

    /* Create the local semaphore */
    if (phNxpUciHal_init_cb_data(&cb_data, NULL) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_D("phNxpUciHal_write_unlocked Create cb data failed");
        data_len = 0;
        goto clean_and_return;
    }

    /* Create local copy of cmd_data */
    phOsalUwb_MemCopy(nxpucihal_ctrl.p_cmd_data, p_data, data_len);
    nxpucihal_ctrl.cmd_len = data_len;

    data_len = nxpucihal_ctrl.cmd_len;

    status = phTmlUwb_Write((uint8_t *)nxpucihal_ctrl.p_cmd_data,
        (uint16_t)nxpucihal_ctrl.cmd_len,
        (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_write_complete,
        (void *)&cb_data);
    if (status != UWBSTATUS_PENDING) {
        NXPLOG_UCIHAL_E("write_unlocked status error");
        data_len = 0;
        goto clean_and_return;
    }

    /* Wait for callback response */
    if (SEM_WAIT(cb_data)) {
        NXPLOG_UCIHAL_E("write_unlocked semaphore error");
        data_len = 0;
        goto clean_and_return;
    }

clean_and_return:
    phNxpUciHal_cleanup_cb_data(&cb_data);
    return data_len;
}

/******************************************************************************
 * Function         phNxpUciHal_write_complete
 *
 * Description      This function handles write callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_write_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo)
{
    phNxpUciHal_Sem_t *p_cb_data = (phNxpUciHal_Sem_t *)pContext;

    if (pInfo->wStatus == UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_D("write successful status = 0x%x", pInfo->wStatus);
    }
    else {
        NXPLOG_UCIHAL_E("write error status = 0x%x", pInfo->wStatus);
    }
    p_cb_data->status = pInfo->wStatus;

    SEM_POST(p_cb_data);

    return;
}
#if (NXP_UWB_EXTNS == TRUE)
/******************************************************************************
 * Function         phNxpUciHal_dump_log
 *
 * Description      This function is called whenever there is an debug logs
 *                  needs to be collected
 *
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_dump_log(uint8_t gid, uint8_t oid, uint8_t pbf)
{
    if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_BIN_LOG_NTF)) {
        NXPLOG_UCIHAL_D("debug bin ntf samples received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_DATA_LOGGER_NTF)) {
        NXPLOG_UCIHAL_D("debug data logger ntf samples received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else if ((gid == UCI_GID_PROPRIETARY) &&
             ((oid == EXT_UCI_MSG_DBG_CIR0_LOG_NTF) || (oid == EXT_UCI_MSG_DBG_CIR1_LOG_NTF))) {
        NXPLOG_UCIHAL_D("CIR samples received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else if ((gid == UCI_GID_PROPRIETARY) && (oid == EXT_UCI_MSG_DBG_GET_ERROR_LOG)) {
        NXPLOG_UCIHAL_D(" error log received. ntf received");
        nxpucihal_ctrl.isSkipPacket = 1;
    }
    else {
        if ((nxpucihal_ctrl.IsCIRDebugDump_enabled) && (gid == UCI_GID_RANGE_MANAGE) &&
            (oid == UCI_OID_RANGE_DATA_NTF)) {
            NXPLOG_UCIHAL_D(" range data ntf received");
        }
    }
    return;
}
#endif
/******************************************************************************
 * Function         phNxpUciHal_read_complete
 *
 * Description      This function is called whenever there is an UCI packet
 *                  received from UWBC. It could be RSP or NTF packet. This
 *                  function provide the received UCI packet to libuwb-uci
 *                  using data callback of libuwb-uci.
 *                  There is a pending read called from each
 *                  phNxpUciHal_read_complete so each a packet received from
 *                  UWBC can be provide to libuwb-uci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_read_complete(void *pContext, phTmlUwb_TransactInfo_t *pInfo)
{
    UWBSTATUS status;
    uint8_t gid = 0, oid = 0, pbf;
    PHUWB_UNUSED(pContext);
    if (nxpucihal_ctrl.read_retry_cnt == 1) {
        nxpucihal_ctrl.read_retry_cnt = 0;
    }
    if (pInfo->wStatus == UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_D("read successful status = 0x%x", pInfo->wStatus);
        nxpucihal_ctrl.p_rx_data   = pInfo->pBuff;
        nxpucihal_ctrl.rx_data_len = pInfo->wLength;

        gid = nxpucihal_ctrl.p_rx_data[0] & UCI_GID_MASK;
        oid = nxpucihal_ctrl.p_rx_data[1] & UCI_OID_MASK;
        pbf = (nxpucihal_ctrl.p_rx_data[0] & UCI_PBF_MASK) >> UCI_PBF_SHIFT;

#if (NXP_UWB_EXTNS == TRUE)
        nxpucihal_ctrl.isSkipPacket = 0;
        if ((nxpucihal_ctrl.IsCIRDebugDump_enabled) || (nxpucihal_ctrl.IsFwDebugDump_enabled) ||
            (oid == EXT_UCI_MSG_DBG_GET_ERROR_LOG || oid == EXT_UCI_MSG_DBG_BIN_LOG_NTF)) {
            phNxpUciHal_dump_log(gid, oid, pbf);
        }
#endif

        if (nxpucihal_ctrl.hal_ext_enabled == 1) {
            nxpucihal_ctrl.isSkipPacket = 1;
            if (nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UWBSTATUS_SUCCESS) {
                nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
            }
            else {
                nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_FAILED;
                NXPLOG_UCIHAL_E(
                    "set baord config failed status = 0x%x", nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET]);
            }
            SEM_POST(&(nxpucihal_ctrl.ext_cb_data));
        }
        /* if Debug Notification, then skip sending to application */
        if (nxpucihal_ctrl.isSkipPacket == 0) {
            /* Read successful, send the event to higher layer */
            if ((nxpucihal_ctrl.p_uwb_stack_data_cback != NULL) && (nxpucihal_ctrl.rx_data_len <= UCI_MAX_PACKET_LEN)) {
                (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len, nxpucihal_ctrl.p_rx_data);
            }
        }
    }
    else {
        NXPLOG_UCIHAL_E("read error status = 0x%x", pInfo->wStatus);
    }

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
        return;
    }
    /* Disable junk data check for each UCI packet*/
    if (nxpucihal_ctrl.fw_dwnld_mode) {
        if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF)) {
            nxpucihal_ctrl.fw_dwnld_mode = false;
        }
    }
    /* Read again because read must be pending always.*/
    status =
        phTmlUwb_Read(Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
    if (status != UWBSTATUS_PENDING) {
        NXPLOG_UCIHAL_E("read status error status = %x", status);
        /* TODO: Not sure how to handle this ? */
    }

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_close
 *
 * Description      This function close the UWBC interface and free all
 *                  resources.This is called by libuwb-uci on UWB service stop.
 *
 * Returns          Always return UWBSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpUciHal_close()
{
    UWBSTATUS status = UWBSTATUS_FAILED;

    if (nxpucihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
        NXPLOG_UCIHAL_E("phNxpUciHal_close is already closed, ignoring close");
        return UWBSTATUS_FAILED;
    }

    nxpucihal_ctrl.IsFwDebugDump_enabled  = false;
    nxpucihal_ctrl.IsCIRDebugDump_enabled = false;

    CONCURRENCY_LOCK();

    nxpucihal_ctrl.halStatus = HAL_STATUS_CLOSE;

    if (NULL != gpphTmlUwb_Context->pDevHandle) {
        phNxpUciHal_close_complete(UWBSTATUS_SUCCESS);
        // wait till message NCI_HAL_CLOSE_CPLT_MSG is posted to HAL Client Task
        phOsalUwb_ConsumeSemaphore(nxpucihal_ctrl.halClientSemaphore);
        /* Abort any pending read and write */
        phTmlUwb_ReadAbort();
        phTmlUwb_WriteAbort();

        // phOsalUwb_Timer_Cleanup();
        if (uwbTimeoutTimerId != 0) {
            phOsalUwb_Timer_Stop(uwbTimeoutTimerId);
            phOsalUwb_Timer_Delete(uwbTimeoutTimerId);
            uwbTimeoutTimerId = 0;
        }
        status = phTmlUwb_Shutdown();

        phDal4Uwb_msgrelease(nxpucihal_ctrl.gDrvCfg.nClientId);
        phOsalUwb_Thread_Delete(nxpucihal_ctrl.client_thread);
        phOsalUwb_DeleteSemaphore(&nxpucihal_ctrl.halClientSemaphore);
        phOsalUwb_SetMemory(&nxpucihal_ctrl, 0x00, sizeof(nxpucihal_ctrl));

        NXPLOG_UCIHAL_D("phNxpUciHal_close - phOsalUwb_DeInit completed");
    }

    CONCURRENCY_UNLOCK();

    phNxpUciHal_cleanup_monitor();

    /* Return success always */
    return status;
}
/******************************************************************************
 * Function         phNxpUciHal_close_complete
 *
 * Description      This function inform libuwb-uci about result of
 *                  phNxpUciHal_close.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpUciHal_close_complete(UWBSTATUS status)
{
    static phLibUwb_Message_t msg;

    if (status == UWBSTATUS_SUCCESS) {
        msg.eMsgType = UCI_HAL_CLOSE_CPLT_MSG;
    }
    else {
        msg.eMsgType = UCI_HAL_ERROR_MSG;
    }
    msg.pMsgData = NULL;
    msg.Size     = 0;

    phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &msg);

    return;
}

/******************************************************************************
 * Function         phNxpUciHal_ioctl
 *
 * Description      This function is called by jni when wired mode is
 *                  performed.First SR100 driver will give the access
 *                  permission whether wired mode is allowed or not
 *                  arg (0):
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
int phNxpUciHal_ioctl(long arg, void *p_data)
{
    NXPLOG_UCIHAL_D("%s : enter - arg = %ld", __func__, arg);

    int status = UWBSTATUS_FAILED;
    switch (arg) {
    case HAL_UWB_IOCTL_FW_DWNLD: {
        NXPLOG_UCIHAL_D(" Start FW download");
        nxpucihal_ctrl.fw_dwnld_mode = true; /* system in FW download mode*/

        InputOutputData_t *io_data_ptr = (InputOutputData_t *)p_data;
        if (io_data_ptr->recovery == true) {
            phTmlUwb_spi_reset(NULL, 0);
        }
#if (UWBIOT_UWBD_SR040) && UWBIOT_TML_SPI
        status = UWBSTATUS_SUCCESS;
        UWB_GpioSet(RST_N, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        UWB_GpioSet(RST_N, 1);
#endif // (UWBIOT_UWBD_SR040) && UWBIOT_TML_SPI

#if UWBIOT_UWBD_SR100T
        status = phNxpUciHal_fw_download();
#if UWBIOT_TML_PNP
        if (status != 0) {
            /* Retry, just once more...
             * This failure is seen in PNP PC Windows mode, where if there was no clean
             * shut down, above call seems to fail, so sending again.
             */
            status = phNxpUciHal_fw_download();
        }
#endif
        phNxpUwb_HeliosIrqEnable();
        if (status == UWBSTATUS_SUCCESS)
#endif
        {
            status = phTmlUwb_Read(
                Rx_data, UCI_MAX_DATA_LEN, (pphTmlUwb_TransactCompletionCb_t)&phNxpUciHal_read_complete, NULL);
            if (status != UWBSTATUS_PENDING) {
                NXPLOG_UCIHAL_E("read status error status = %x", status);
            }
            else {
                status = UWBSTATUS_SUCCESS; // Reader thread started successfully
            }
        }
#if (UWBIOT_UWBD_SR100T)
        else {
            NXPLOG_UCIHAL_E("FW download is failed: status= %x", status);
            status = UWBSTATUS_FAILED;
        }
#endif
    } break;

    case HAL_UWB_IOCTL_DUMP_FW_CRASH_LOG:
        status = phNxpUciHal_dump_fw_crash_log();
        break;

    case HAL_UWB_IOCTL_SET_SUSPEND_STATE:
        nxpucihal_ctrl.IsDev_suspend_enabled = true;
        break;
#if (NXP_UWB_EXTNS == TRUE)
    case HAL_UWB_IOCTL_DUMP_FW_LOG: {
        InputOutputData_t *ioData = (InputOutputData_t *)(p_data);
        if (p_data == NULL) {
            NXPLOG_UCIHAL_E("%s : p_data is NULL", __func__);
            return UWBSTATUS_FAILED;
        }
        nxpucihal_ctrl.IsFwDebugDump_enabled = ioData->enableFwDump;
        NXPLOG_UCIHAL_E("%s : Fw Dump is enabled status is %d", __func__, ioData->enableFwDump);
        nxpucihal_ctrl.IsCIRDebugDump_enabled = ioData->enableCirDump;
        NXPLOG_UCIHAL_E("%s : Cir Dump is enabled status is %d", __func__, ioData->enableCirDump);
        status = UWBSTATUS_SUCCESS;
    } break;
    case HAL_UWB_IOCTL_SET_BOARD_CONFIG:
        status = phNxpUciHal_set_board_config();
        break;
#endif
    default:
        NXPLOG_UCIHAL_E("%s : Wrong arg = %ld", __func__, arg);
        break;
    }
    return status;
}
