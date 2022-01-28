/*
 *
 * Copyright 2018-2020 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#include "UwbApi_Proprietary_Internal.h"
#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"

#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"
#include "phNxpUwbConfig.h"
#include "UwbSeApi.h"

#if defined(ENABLE_NFC) && (ENABLE_NFC == TRUE)
/*******************************************************************************
 **
 ** Function         reset_se_on_error
 **
 ** Description      This function is called to cold reset the SE.
 **
 ** Returns          void
 **
 *******************************************************************************/
void reset_se_on_error(void)
{
    UINT16 RespSize     = sizeof(uwbContext.rsp_data);
    UINT8 eSeRsetBuf[3] = {0x2F, 0x1E, 0x00}; // command to reset the eSE

    if (UwbSeApi_NciRawCmdSend(sizeof(eSeRsetBuf), eSeRsetBuf, &RespSize, uwbContext.rsp_data) != 0) {
        NXPLOG_UWBAPI_E("eSE reset failure\n");
    }
    else {
        NXPLOG_UWBAPI_D("eSE reset success\n");
    }
}
#endif /*ENABLE_NFC*/

/*******************************************************************************
 **
 ** Function:        getExtAppConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended application related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getExtAppConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = EXTENDED_APP_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
    case UCI_EXT_PARAM_ID_TOA_MODE:
    case UCI_EXT_PARAM_ID_CIR_CAPTURE_MODE:
    case UCI_EXT_PARAM_ID_MAC_PAYLOAD_ENCRYPTION:
    case UCI_EXT_PARAM_ID_SESSION_SYNC_ATTEMPTS:
    case UCI_EXT_PARAM_ID_SESSION_SCHED_ATTEMPTS:
    case UCI_EXT_PARAM_ID_SCHED_STATUS_NTF:
    case UCI_EXT_PARAM_ID_TX_POWER_DELTA_FCC: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    default:
        break;
    }

    return length;
}

/*******************************************************************************
 **
 ** Function:        getExtTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended test related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getExtTestConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = EXTENDED_TEST_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_EXT_TEST_PARAM_ID_RSSI_AVG_FILT_CNT: {
        tlvBuffer[length++] = 4; // Param len
        UINT32 value        = *((UINT32 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
        tlvBuffer[length++] = (UINT8)(value >> 16);
        tlvBuffer[length++] = (UINT8)(value >> 24);
    } break;
    case UCI_EXT_TEST_PARAM_ID_RSSI_CALIBRATION_OPTION: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    case UCI_EXT_TEST_PARAM_ID_AGC_GAIN_VAL_RX: {
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;
    default:
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        getExtTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for extended test related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getExtDeviceConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = EXTENDED_DEVICE_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT: {
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;
    case UCI_EXT_PARAM_ID_DPD_WAKEUP_SRC:
    case UCI_EXT_PARAM_ID_WTX_COUNT: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;
    default:
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        getExtDebugConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Extended Debug Configs only.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getExtDebugConfigTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = EXTENDED_DEBUG_CONFIG_ID;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
    case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF:
    case UCI_EXT_PARAM_ID_CIR_LOG_NTF:
    case UCI_EXT_PARAM_ID_PSDU_LOG_NTF:
    case UCI_EXT_PARAM_ID_RFRAME_LOG_NTF: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;
        /* Length 2 Byte */
    case UCI_EXT_PARAM_ID_THREAD_SECURE:
    case UCI_EXT_PARAM_ID_THREAD_SECURE_ISR:
    case UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR:
    case UCI_EXT_PARAM_ID_THREAD_SHELL:
    case UCI_EXT_PARAM_ID_THREAD_PHY:
    case UCI_EXT_PARAM_ID_THREAD_RANGING: {
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;
    default:
        break;
    }

    return length;
}

/*******************************************************************************
 **
 ** Function:        parseDeviceInfo
 **
 ** Description:     Parse Manufacturer Specific Device Information from the
 **                  Given buffer as per UCI
 **                  manufacturerData:  Manufacturer Data
 **                  manufacturerLength:  Length of Manufacturer Data
 **
 ** Returns:         boolean, parse success or failure
 **
 *******************************************************************************/
bool parseDeviceInfo(UINT8 *manufacturerData, UINT8 manufacturerLength)
{
    UINT8 index = 0;
    UINT8 fwVersionMinor;
    UINT8 paramId;
    UINT8 length;

    if ((manufacturerLength == 0) || (manufacturerData == NULL)) {
        NXPLOG_UWBAPI_E("%s: manufacturerLength is zero or manufacturerData is NULL", __func__);
        return false;
    }

    while (index < manufacturerLength) {
        index++; // Ignore Extended Parameter Id 0xE3 from the UCI Response
        paramId = manufacturerData[index++];
        length  = manufacturerData[index++];

        NXPLOG_UWBAPI_D("Extended Device Param Id = %d\n", paramId);
        switch (paramId) {
        case UCI_EXT_PARAM_ID_DEVICE_NAME:
            if (length > sizeof(uwbContext.devInfo.devName)) {
                NXPLOG_UWBAPI_E("device name data size is more than response buffer", __func__);
                return false;
            }
            uwbContext.devInfo.devNameLen = length;
            if (length != 0) {
                phOsalUwb_MemCopy(uwbContext.devInfo.devName, &manufacturerData[index], length);
            }
            index = (UINT8)(index + length);
            break;
        case UCI_EXT_PARAM_ID_FW_VERSION:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return false;
            }
            uwbContext.devInfo.fwMajor = manufacturerData[index++];
            fwVersionMinor             = manufacturerData[index++];
            uwbContext.devInfo.fwMinor = (UINT8)(fwVersionMinor >> 4); // most significant nibble says FW minor version
            uwbContext.devInfo.fwRc    = manufacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_NXP_UCI_VER:
            if (length != UWBD_VERSION_LENGTH_MAX) {
                return false;
            }
            uwbContext.devInfo.nxpUciMajor = manufacturerData[index++];
            uwbContext.devInfo.nxpUciMinor = manufacturerData[index++];
            uwbContext.devInfo.nxpUciPatch = manufacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_NXP_CHIP_ID:
            if (length > sizeof(uwbContext.devInfo.nxpChipId)) {
                NXPLOG_UWBAPI_E("nxp chip id size is more than response buffer", __func__);
                return false;
            }
            if (length != 0) {
                phOsalUwb_MemCopy(uwbContext.devInfo.nxpChipId, &manufacturerData[index], length);
            }
            index = (UINT8)(index + length);
            break;
        default:
            NXPLOG_UWBAPI_E("unknown param Id", __func__);
            return false;
        }
    }
    return true;
}

/*******************************************************************************
 **
 ** Function:        parseDebugParams
 **
 ** Description:     Extracts Debug Params from the given byte array and updates
 *the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseDebugParams(UINT8 *rspPtr, UINT8 noOfParams, phDebugParams_t *pDebugParams)
{
    for (int i = 0; i < noOfParams; i++) {
        rspPtr++; // Ignore Extended Parameter Id from the UCI Response
        UINT8 paramId = *rspPtr++;
        UINT8 length  = *rspPtr++;
        UINT8 *value  = rspPtr;
        switch (paramId) {
        case UCI_EXT_PARAM_ID_THREAD_SECURE:
            UWB_STREAM_TO_UINT16(pDebugParams->secureThread, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_SECURE_ISR:
            UWB_STREAM_TO_UINT16(pDebugParams->secureIsrThread, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR:
            UWB_STREAM_TO_UINT16(pDebugParams->nonSecureIsrThread, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_SHELL:
            UWB_STREAM_TO_UINT16(pDebugParams->shellThread, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_PHY:
            UWB_STREAM_TO_UINT16(pDebugParams->phyThread, value);
            break;
        case UCI_EXT_PARAM_ID_THREAD_RANGING:
            UWB_STREAM_TO_UINT16(pDebugParams->rangingThread, value);
            break;
        case UCI_EXT_PARAM_ID_CIR_LOG_NTF:
            UWB_STREAM_TO_UINT8(pDebugParams->cirLogNtf, value);
            break;
        case UCI_EXT_PARAM_ID_DATA_LOGGER_NTF:
            UWB_STREAM_TO_UINT8(pDebugParams->dataLoggerNtf, value);
            break;
        case UCI_EXT_PARAM_ID_PSDU_LOG_NTF:
            UWB_STREAM_TO_UINT8(pDebugParams->psduLogNtf, value);
            break;
        case UCI_EXT_PARAM_ID_RFRAME_LOG_NTF:
            UWB_STREAM_TO_UINT8(pDebugParams->rframeLogNtf, value);
        default:
            break;
        }
        rspPtr += length;
    }
}

/*******************************************************************************
 **
 ** Function:        parseUwbSessionParams
 **
 ** Description:     Extracts All Sessions Data Parameters and updates the
 *structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseUwbSessionParams(UINT8 *rspPtr, phUwbSessionsContext_t *pUwbSessionsContext)
{
    // Validation of all the parameters needs to be added.
    UWB_STREAM_TO_UINT8(pUwbSessionsContext->sessioncnt, rspPtr);

    for (UINT8 i = 0; i < pUwbSessionsContext->sessioncnt; i++) {
        UWB_STREAM_TO_UINT32(pUwbSessionsContext->pUwbSessionData[i].session_id, rspPtr);
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_type, rspPtr);
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->pUwbSessionData[i].session_state, rspPtr);
    }

    printUwbSessionData(pUwbSessionsContext);
}

/*******************************************************************************
 **
 ** Function         handle_rframe_log_ntf
 **
 ** Description      This function is called to process rframe log notifications
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_rframe_log_ntf(UINT8 *p, UINT16 len)
{
    phRframeData_t sRframe_log_data;

    if (len > 0) {
        sRframe_log_data.dataLength = len;
        phOsalUwb_MemCopy(sRframe_log_data.data, p, len);
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_RFRAME_DATA, &sRframe_log_data);
    }
}

/*******************************************************************************
 **
 ** Function         handle_schedstatus_ntf
 **
 ** Description      This function is called to process Scheduler Status
 **                  notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_schedstatus_ntf(UINT8 *p, UINT16 len)
{
    phSchedStatusNtfData_t sSchedStatusNtf_data;

    if (len > 0) {
        sSchedStatusNtf_data.dataLength = len;
        phOsalUwb_MemCopy(sSchedStatusNtf_data.data, p, len);
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_SCHEDULER_STATUS_NTF, &sSchedStatusNtf_data);
    }
}

/*******************************************************************************
 **
 ** Function         handle_connectivity_test_ntf
 **
 ** Description      This function is called to process connectivity test
 **                  notifications
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_connectivity_test_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.testConnectivityData, 0x00, sizeof(phTestConnectivityData_t));
    uwbContext.testConnectivityData.status = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.testConnectivityData.status, p);
        UWB_STREAM_TO_UINT8(uwbContext.testConnectivityData.wtx, p);
    }
}

/*******************************************************************************
 **
 ** Function         handle_loop_test_ntf
 **
 ** Description      This function is called to process loop test notifications
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_loop_test_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.testLoopData, 0x00, sizeof(phTestLoopData_t));
    uwbContext.testLoopData.status = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.testLoopData.status, p);
        UWB_STREAM_TO_UINT16(uwbContext.testLoopData.loop_cnt, p);
        UWB_STREAM_TO_UINT16(uwbContext.testLoopData.loop_pass_count, p);
    }
}

/*******************************************************************************
 **
 ** Function         handle_se_do_bind_ntf
 **
 ** Description      This function is called to notify do bind status
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_se_do_bind_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.doBindStatus, 0x00, sizeof(phSeDoBindStatus_t));
    uwbContext.doBindStatus.status = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.doBindStatus.status, p);
        UWB_STREAM_TO_UINT8(uwbContext.doBindStatus.count_remaining, p);
        UWB_STREAM_TO_UINT8(uwbContext.doBindStatus.binding_state, p);
    }
}

/*******************************************************************************
 **
 ** Function         handle_se_get_binding_status_ntf
 **
 ** Description      This function is called to notify get binding status
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_se_get_binding_status_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.getBindingStatus, 0x00, sizeof(phSeGetBindingStatus_t));
    uwbContext.getBindingStatus.status = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.getBindingStatus.status, p);
        UWB_STREAM_TO_UINT8(uwbContext.getBindingStatus.se_binding_count, p);
        UWB_STREAM_TO_UINT8(uwbContext.getBindingStatus.uwbd_binding_count, p);
    }
}

/*******************************************************************************
 **
 ** Function         handle_se_com_err_ntf
 **
 ** Description      This function is called to notify se comm err
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_se_com_err_ntf(UINT8 *p, UINT16 len)
{
    phSeCommError_t seCommError;
    memset(&seCommError, 0, sizeof(phSeCommError_t));

    if (len != 0) {
        UWB_STREAM_TO_UINT8(seCommError.status, p);
        UWB_STREAM_TO_UINT16(seCommError.cla_ins, p);
        UWB_STREAM_TO_UINT16(seCommError.t_eq_1_status, p);
        uwbContext.wstatus = seCommError.status;
    }
    NXPLOG_UWBAPI_E("%s: SE_COMM_ERR, status %d", __func__, seCommError.status);
}

/*******************************************************************************
 **
 ** Function         handle_do_calibration_ntf
 **
 ** Description      This function is called to process do calibration
 *notification
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_do_calibration_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.doCalibrationStatus, 0x00, sizeof(phCalibRespStatus_t));
    uwbContext.doCalibrationStatus.status = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.doCalibrationStatus.status, p);
        --len;
        uwbContext.doCalibrationStatus.length = (UINT16)len;
        UWB_STREAM_TO_ARRAY(uwbContext.doCalibrationStatus.calibValueOut, p, len);
    }
}

/*******************************************************************************
 **
 ** Function:        extDeviceManagementCallback
 **
 ** Description:     Receive device management events from stack for
 **                  Properiatary Extensions
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/
void extDeviceManagementCallback(UINT8 event, UINT16 paramLength, UINT8 *pResponseBuffer)
{
    UINT16 responsePayloadLen = 0;
    UINT8 *responsePayloadPtr = NULL;

    if ((paramLength > UCI_RESPONSE_STATUS_OFFSET) && (pResponseBuffer != NULL)) {
        NXPLOG_UWBAPI_D(
            "extDeviceManagementCallback: Received length data = 0x%x "
            "status = 0x%x",
            paramLength,
            pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET]);

        responsePayloadLen         = (UINT16)(paramLength - UCI_RESPONSE_STATUS_OFFSET);
        responsePayloadPtr         = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];
        uwbContext.receivedEventId = (UINT16)event;
        switch (event) {
        case EXT_UCI_MSG_DBG_RFRAME_LOG_NTF: {
            handle_rframe_log_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SE_DO_BIND: /* SE_DO_BIND_EVT */
        {
            handle_se_do_bind_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SE_DO_TEST_CONNECTIVITY: {
            handle_connectivity_test_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SE_DO_TEST_LOOP: {
            handle_loop_test_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SE_GET_BINDING_STATUS: {
            handle_se_get_binding_status_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SE_COMM_ERROR_NTF: {
            handle_se_com_err_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_SCHEDULER_STATUS_NTF: {
            handle_schedstatus_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_DO_CALIBRATION: {
            handle_do_calibration_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        default:
            NXPLOG_UWBAPI_E("%s: unhandled event", __func__);
            break;
        }
        if (uwbContext.currentEventId == event || event == UWA_DM_UWBD_RESP_TIMEOUT_EVT) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __func__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            phUwb_GKI_binary_sem_post(uwbContext.devMgmtSem);
        }
    }
    else {
        NXPLOG_UWBAPI_E(
            "%s: pResponseBuffer is NULL or paramLength is less than "
            "UCI_RESPONSE_STATUS_OFFSET\n",
            __func__);
    }
}

/*******************************************************************************
 **
 ** Function:        setDefaultCoreConfigs
 **
 ** Description:     Sets all core configs. Default values are picked from
 **                  config file(phNxpUciHal_RhodesConfig.h)
 **
 ** Returns:         None
 **
 *******************************************************************************/
tUWBAPI_STATUS setDefaultCoreConfigs(void)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    void *p_cmd           = NULL;
    long cmd_len          = 0;
    UINT8 config          = 0;
    UINT16 dpdTimeout     = 0;
    UINT8 offset          = 0;

    NXPLOG_UWBAPI_D("%s: Enter ", __func__);
    if ((phNxpUciHal_GetNxpByteArrayValue(UWB_CORE_CONFIG_PARAM, &p_cmd, &cmd_len) == TRUE) && cmd_len > 0) {
        status = sendRawUci((UINT8 *)p_cmd, (UINT16)cmd_len);
        if (status == UWBAPI_STATUS_TIMEOUT) {
            return status;
        }
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_LOW_POWER_MODE, &config, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_LOW_POWER_MODE value %d ", __func__, (UINT8)config);

        offset = (UINT8)getCoreDeviceConfigTLVBuffer(
            UCI_PARAM_ID_LOW_POWER_MODE, sizeof(config), (void *)&config, &uwbContext.snd_data[offset]);

        sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
        status = UWA_SetCoreConfig(1, offset, uwbContext.snd_data);
        if (status == UWA_STATUS_OK) {
            phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
            status = uwbContext.wstatus;
        }
        else if (status == UWBAPI_STATUS_TIMEOUT) {
            return status;
        }
        else {
            NXPLOG_UWBAPI_E("%s: low power mode config is failed", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: low power mode config not found", __func__);
    }
    if (phNxpUciHal_GetNxpNumValue(UWB_DPD_ENTRY_TIMEOUT, &dpdTimeout, 0x02) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_DPD_ENTRY_TIMEOUT value %d ", __func__, dpdTimeout);

        offset = 0; //Reset offset to zero for next use
        if ((dpdTimeout >= UWBD_DPD_TIMEOUT_MIN) && (dpdTimeout <= UWBD_DPD_TIMEOUT_MAX)) {
            offset = (UINT8)getExtDeviceConfigTLVBuffer(
                UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT, (void *)&dpdTimeout, &uwbContext.snd_data[offset]);

            sep_SetWaitEvent(UWA_DM_CORE_SET_CONFIG_RSP_EVT);
            status = UWA_SetCoreConfig(1, offset, uwbContext.snd_data);
            if (status == UWA_STATUS_OK) {
                phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
                status = uwbContext.wstatus;
            }
            else if (status == UWBAPI_STATUS_TIMEOUT) {
                return status;
            }
            else {
                NXPLOG_UWBAPI_E("%s: low power mode config is failed", __func__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for DPD Entry Timeout in ConfigFile", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: low power mode config not found", __func__);
    }
    NXPLOG_UWBAPI_D("%s: Exit ", __func__);
    return status;
}
