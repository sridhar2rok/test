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

#include "UwbApi_Internal.h"
#include "phNxpLogApis_UwbApi.h"

#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_defs.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"
#include "UwbAdaptation.h"
#include "UwbApi_Proprietary_Internal.h"
#include "AppConfigParams.h"
#if (UWBIOT_UWBD_SR100T)
#include "UwbApi_RfTest.h"
#endif

/* Context variable */
phUwbApiContext_t uwbContext;

tUWBAPI_STATUS recoverUWBS()
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);
    status = (UINT8)DownloadFirmwareRecovery();
    NXPLOG_UWBAPI_D("%s: DownloadFirmware status: %d", __func__, status);
    if (status == UWA_STATUS_OK) {
        if (phUwb_GKI_binary_sem_wait_timeout(uwbContext.devMgmtSem, UWB_CMD_TIMEOUT) != GKI_SUCCESS) {
            NXPLOG_UWBAPI_E("%s: Sem Timed out", __func__);
        }
        if (uwbContext.dev_state != UWBAPI_UCI_DEV_READY) {
            NXPLOG_UWBAPI_E("%s: device status is failed %d", __func__, uwbContext.dev_state);
            status = UWBAPI_STATUS_FAILED;
            return status;
        }

        status = setDefaultCoreConfigs();
        if (status != UWA_STATUS_OK) {
            NXPLOG_UWBAPI_E("%s: setDefaultCoreConfigs is failed:", __func__);
            return status;
        }

        // Update UWBC device info
        status = getDeviceInfo();
    }
    else {
        sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
        NXPLOG_UWBAPI_E("%s: DownloadFirmware is failed:", __func__);
        return status;
    }
    return status;
}

/*******************************************************************************
 **
 ** Function:        cleanUp
 **
 ** Description:     CleanUp all the Semaphores and Timers
 **
 ** Returns:         None
 **
 *******************************************************************************/
void cleanUp()
{
    phUwb_GKI_binary_sem_destroy(uwbContext.devMgmtSem);
    Finalize(); // disable GKI, UCI task, UWB task
    phOsalUwb_SetMemory(&uwbContext, 0x00, sizeof(phUwbApiContext_t));
}

/*******************************************************************************
 **
 ** Function:        uwbInit
 **
 ** Description:     Perform UwbInit with the callback
 **
 ** Returns:         Status
 **
 *******************************************************************************/
tUWBAPI_STATUS uwbInit(tUwbApi_AppCallback *pCallback)
{
    tUWBAPI_STATUS status          = UWBAPI_STATUS_FAILED;
    tHAL_UWB_ENTRY *halFuncEntries = NULL;
    phUwb_LogInit();
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == TRUE)
        return UWBAPI_STATUS_OK;
    if (pCallback == NULL) {
        NXPLOG_UWBAPI_E("%s: pCallBack is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    uwbContext.sessionInfo.state = UWBAPI_SESSION_ERROR;
    uwbContext.pAppCallback      = pCallback;
    uwbContext.receivedEventId   = DEFAULT_EVENT_TYPE;
    uwbContext.devMgmtSem        = phUwb_GKI_binary_sem_init();
    if (uwbContext.devMgmtSem == NULL)
        return status;

    NXPLOG_UWBAPI_D("UWA_Init");
    Initialize();
    halFuncEntries = GetHalEntryFuncs();
    UWA_Init(halFuncEntries);
    NXPLOG_UWBAPI_D("UfaEnable");
    sep_SetWaitEvent(UWA_DM_ENABLE_EVT);
    status = UWA_Enable(ufaDeviceManagementCallback, ufaTestDeviceManagementCallback);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            sep_SetWaitEvent(UWA_DM_REGISTER_EXT_CB_EVT);
            status = UWA_RegisterExtCallback(extDeviceManagementCallback);
            if (status == UWA_STATUS_OK) {
                phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
                uwbContext.isUfaEnabled = TRUE;
                sep_SetWaitEvent(UWA_DM_DEVICE_STATUS_NTF_EVT);

                status = (UINT8)DownloadFirmware();
                NXPLOG_UWBAPI_D("%s: DownloadFirmware status: %d", __func__, status);
                if (status == UWA_STATUS_OK) {
                    if (phUwb_GKI_binary_sem_wait_timeout(uwbContext.devMgmtSem, UWB_CMD_TIMEOUT) != GKI_SUCCESS) {
                        NXPLOG_UWBAPI_E("%s: Sem Timed out", __func__);
                    }
                    if (uwbContext.dev_state != UWBAPI_UCI_DEV_READY) {
                        NXPLOG_UWBAPI_E("%s: device status is failed %d", __func__, uwbContext.dev_state);
                        status = UWBAPI_STATUS_FAILED;
                        goto Error;
                    }

                    status = setDefaultCoreConfigs();
                    if (status != UWA_STATUS_OK)
                        goto Error;

                    // Update UWBC device info
                    status = getDeviceInfo();
                }
                else {
                    sep_SetWaitEvent(DEFAULT_EVENT_TYPE);
                    goto Error;
                }
            }
            else {
                NXPLOG_UWBAPI_D("%s: UWA_Enable status: %d", __func__, status);
                return status;
            }
        }
        else {
            return status;
        }
        return status;
    }
Error:
    uwbContext.isUfaEnabled = FALSE;
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    if (UWA_Disable(FALSE) == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
    }
    else {
        NXPLOG_UWBAPI_E("%s: UFA Disable is failed:", __func__);
    }
    cleanUp();
    NXPLOG_UWBAPI_D("%s: exit with status %d", __func__, status);
    return status;
}

/*******************************************************************************
 **
 ** Function:        parseRangingParams
 **
 ** Description:     Extracts Ranging Params from the given byte array and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
void parseRangingParams(UINT8 *rspPtr, UINT8 noOfParams, phRangingParams_t *pRangingParams)
{
    for (int i = 0; i < noOfParams; i++) {
        UINT8 paramId = *rspPtr++;
        UINT8 length  = *rspPtr++;
        UINT8 *value  = rspPtr;
        switch (paramId) {
        case UCI_PARAM_ID_DEVICE_ROLE:
            /*  Device Role */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceRole, value);
            break;
        case UCI_PARAM_ID_MULTI_NODE_MODE:
            /*  Multi Node Mode */
            UWB_STREAM_TO_UINT8(pRangingParams->multiNodeMode, value);
            break;
#if (UWBIOT_UWBD_SR100T)
        case UCI_PARAM_ID_MAC_ADDRESS_MODE:
            /*  Mac addr mode */
            UWB_STREAM_TO_UINT8(pRangingParams->macAddrMode, value);
            break;
#endif
        case UCI_PARAM_ID_NO_OF_CONTROLEES:
            /*  No Of Controlees */
            UWB_STREAM_TO_UINT8(pRangingParams->noOfControlees, value);
            break;
        case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
            /*  Device Mac Address */
            UWB_STREAM_TO_ARRAY(pRangingParams->deviceMacAddr, value, length);
            break;
        case UCI_PARAM_ID_DST_MAC_ADDRESS:
            UWB_STREAM_TO_ARRAY(pRangingParams->dstMacAddr, value, length);
            break;
        case UCI_PARAM_ID_DEVICE_TYPE:
            /*  Device Type */
            UWB_STREAM_TO_UINT8(pRangingParams->deviceType, value);
            break;
        default:
            break;
        }
        rspPtr += length;
    }
}

/*******************************************************************************
 **
 ** Function:        ufaDeviceManagementCallback
 **
 ** Description:     Receive device management events from stack.
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/
void ufaDeviceManagementCallback(eResponse_Event dmEvent, tUWA_DM_CBACK_DATA *eventData)
{
    if (eventData != NULL) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __func__, dmEvent, eventData->status);

        uwbContext.receivedEventId = (UINT16)dmEvent;
        switch (dmEvent) {
        case UWA_DM_ENABLE_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_DISABLE_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_DEVICE_STATUS_NTF_EVT: {
            uwbContext.dev_state = (eUWBD_DEVICE_STATUS_t)eventData->dev_status.status;
            if (uwbContext.dev_state == UWBD_STATUS_ERROR) {
                if (isCmdRespPending()) {
                    uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
                }
                else {
                    // in case of uwb device err firmware crash, send recovery signal to app
                    if (uwbContext.pAppCallback) {
                        uwbContext.pAppCallback(UWBD_RECOVERY_NTF, NULL);
                    }
                }
            }
            else if (uwbContext.dev_state == 0xFC) {
                uwbContext.wstatus = UWBAPI_STATUS_HPD_WAKEUP;
                /* Keeping below code for future use in case we want to change the handling of HPD wakeup*/
                /* If Device Status Notification is 0xFC, then inform application to perform clean up */
                // if (uwbContext.pAppCallback) {
                //     uwbContext.pAppCallback(UWBD_ACTION_APP_CLEANUP, NULL);
                // }
            }
        } break;
        case UWA_DM_CORE_GEN_ERR_STATUS_EVT: {
            NXPLOG_UWBAPI_E(
                "%s: UWA_DM_CORE_GEN_ERR_STATUS_EVT status %d", __func__, eventData->sCore_gen_err_status.status);
#if (UWBIOT_UWBD_SR100T)
            /*
             * Notify application if STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY is received.
             */
            if (UCI_STATUS_DEVICE_TEMP_REACHED_THERMAL_RUNAWAY == eventData->sCore_gen_err_status.status) {
                if (uwbContext.pAppCallback) {
                    uwbContext.pAppCallback(UWBD_OVER_TEMP_REACHED, NULL);
                }
            }
#endif
        } break;
        case UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK) {
                uwbContext.devInfo.uciVersion = eventData->sGet_device_info.uci_version;
                if (parseDeviceInfo(eventData->sGet_device_info.device_info,
                        eventData->sGet_device_info.device_info_len) == false) {
                    NXPLOG_UWBAPI_E("%s: Parsing Device Information Failed", __func__);
                }
            }
        } break;
        case UWA_DM_DEVICE_RESET_RSP_EVT: {
            uwbContext.wstatus = eventData->sDevice_reset.status;
        } break;
        case UWA_DM_CORE_GET_CONFIG_RSP_EVT: {
            /* Result of UWA_GetCoreConfig */
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK &&
                eventData->sCore_get_config.tlv_size <= sizeof(uwbContext.rsp_data)) {
                uwbContext.rsp_len = eventData->sCore_get_config.tlv_size;
                phOsalUwb_MemCopy(
                    uwbContext.rsp_data, eventData->sCore_get_config.param_tlvs, eventData->sCore_get_config.tlv_size);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_CORE_CONFIG failed", __func__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UWA_DM_CORE_SET_CONFIG_RSP_EVT: /* Result of UWA_SetCoreConfig */
        {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_SESSION_GET_COUNT_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK) {
                uwbContext.sessionCnt = eventData->sGet_session_cnt.count;
            }
            else {
                NXPLOG_UWBAPI_E("%s: get session count Request is failed", __func__);
                uwbContext.sessionCnt = -1;
            }
        } break;
        case UWA_DM_SESSION_GET_STATE_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK) {
                uwbContext.sessionState = eventData->sGet_session_state.session_state;
            }
            else {
                NXPLOG_UWBAPI_E("%s: get session state Request is failed", __func__);
                uwbContext.sessionState = UWBAPI_SESSION_ERROR;
            }
        } break;
        case UWA_DM_GET_RANGE_COUNT_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK) {
                uwbContext.rangingCnt = eventData->sGet_range_cnt.count;
            }
            else {
                NXPLOG_UWBAPI_E("%s: get ranging count Request is failed", __func__);
                uwbContext.rangingCnt = -1;
            }
        } break;
        case UWA_DM_SESSION_INIT_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_SESSION_DEINIT_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_SESSION_GET_CONFIG_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK &&
                eventData->sApp_get_config.tlv_size <= sizeof(uwbContext.rsp_data)) {
                uwbContext.rsp_len = eventData->sApp_get_config.tlv_size;
                phOsalUwb_MemCopy(
                    uwbContext.rsp_data, eventData->sApp_get_config.param_tlvs, eventData->sApp_get_config.tlv_size);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_GET_APP_CONFIG failed", __func__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UWA_DM_SESSION_SET_CONFIG_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_RANGE_START_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_RANGE_STOP_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_SESSION_STATUS_NTF_EVT: {
            uwbContext.sessionInfo.session_id  = eventData->sSessionStatus.session_id;
            uwbContext.sessionInfo.state       = eventData->sSessionStatus.state;
            uwbContext.sessionInfo.reason_code = eventData->sSessionStatus.reason_code;

            if (uwbContext.sessionInfo.reason_code != UWB_SESSION_STATE_CHANGED) {
                if (uwbContext.pAppCallback) {
                    uwbContext.pAppCallback(UWBD_SESSION_DATA, &uwbContext.sessionInfo);
                }
            }
        } break;
        case UWA_DM_RANGE_INTERVAL_UPDATE_REQ_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_UWBD_RESP_TIMEOUT_EVT:
            uwbContext.wstatus = UWBAPI_STATUS_TIMEOUT;
            break;
#ifdef __DEPRECATED
#if (UWBIOT_UWBD_SR100T)
        case UWA_DM_APP_DATA_SEND_RSP_EVT: {
            uwbContext.wstatus = eventData->sApp_data_tx.status;
        } break;
        case UWA_DM_APP_DATA_RCVE_RSP_EVT: {
            uwbContext.wstatus = eventData->sApp_data_rx.status;
        } break;
        case UWA_DM_APP_DATA_SEND_STATUS_NTF_EVT: {
            phSendData_t pSendData;
            pSendData.sessionId = eventData->sApp_data_tx_status.session_id;
            pSendData.status    = eventData->sApp_data_tx_status.status;
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_DATA_TRANSFER_SEND, &pSendData);
            }
        } break;
        case UWA_DM_APP_DATA_RCVE_STATUS_NTF_EVT: {
            phRcvData_t pRcvData;
            pRcvData.session_id = eventData->sApp_data_rx_status.session_id;
            pRcvData.status     = eventData->sApp_data_rx_status.status;
            pRcvData.data_len   = eventData->sApp_data_rx_status.data_len;
            phOsalUwb_MemCopy(pRcvData.data, eventData->sApp_data_rx_status.data, pRcvData.data_len);
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_DATA_TRANSFER_RCV, &pRcvData);
            }
        } break;
#endif
#endif
        case UWA_DM_RANGE_DATA_NTF_EVT: {
            phRangingData_t pRangingData;
            UINT8 i;
            pRangingData.seq_ctr                 = eventData->sRange_data.seq_ctr;
            pRangingData.sessionId               = eventData->sRange_data.session_id;
            pRangingData.rcr_indication          = eventData->sRange_data.rcr_indication;
            pRangingData.curr_range_interval     = eventData->sRange_data.curr_range_interval;
            pRangingData.ranging_measure_type    = eventData->sRange_data.ranging_measure_type;
            pRangingData.antenna_pair_sel        = eventData->sRange_data.antenna_pair_sel;
            pRangingData.mac_addr_mode_indicator = eventData->sRange_data.mac_addr_mode_indicator;
            pRangingData.no_of_measurements      = eventData->sRange_data.no_of_measurements;

            if (pRangingData.ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
                for (i = 0; i < pRangingData.no_of_measurements; i++) {
                    if (eventData->sRange_data.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
                        phOsalUwb_MemCopy(pRangingData.range_meas[i].mac_addr,
                            eventData->sRange_data.twr_range_measr[i].mac_addr,
                            MAC_SHORT_ADD_LEN);
                    }
                    else {
                        phOsalUwb_MemCopy(pRangingData.range_meas[i].mac_addr,
                            eventData->sRange_data.twr_range_measr[i].mac_addr,
                            MAC_EXT_ADD_LEN);
                    }

                    pRangingData.range_meas[i].status   = eventData->sRange_data.twr_range_measr[i].status;
                    pRangingData.range_meas[i].nLos     = eventData->sRange_data.twr_range_measr[i].nLos;
                    pRangingData.range_meas[i].distance = eventData->sRange_data.twr_range_measr[i].distance;

                    UINT8 *ptr;
                    ptr = (UINT8 *)&eventData->sRange_data.twr_range_measr[i].aoa[0];
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].aoaFirst, ptr)
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].aoaSecond, ptr);

                    ptr = (UINT8 *)&eventData->sRange_data.twr_range_measr[i].pdoa[0];
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].pdoaFirst, ptr);
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].pdoaSecond, ptr);

                    ptr = (UINT8 *)&eventData->sRange_data.twr_range_measr[i].pdoaIndex[0];
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].pdoaFirstIndex, ptr);
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].pdoaSecondIndex, ptr);

                    ptr = (UINT8 *)&eventData->sRange_data.twr_range_measr[i].aoa_dest[0];
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].aoaDestFirst, ptr);
                    UWB_STREAM_TO_UINT16(pRangingData.range_meas[i].aoaDestSecond, ptr);

                    pRangingData.range_meas[i].slot_index = eventData->sRange_data.twr_range_measr[i].slot_index;
                }
            }
            else if (pRangingData.ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
                if (eventData->sRange_data.mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
                    phOsalUwb_MemCopy(pRangingData.range_meas_tdoa.mac_addr,
                        eventData->sRange_data.tdoa_range_measr.mac_addr,
                        MAC_SHORT_ADD_LEN);
                }
                else {
                    phOsalUwb_MemCopy(pRangingData.range_meas_tdoa.mac_addr,
                        eventData->sRange_data.tdoa_range_measr.mac_addr,
                        MAC_EXT_ADD_LEN);
                }
                pRangingData.range_meas_tdoa.frame_type = eventData->sRange_data.tdoa_range_measr.frame_type;
                pRangingData.range_meas_tdoa.nLos       = eventData->sRange_data.tdoa_range_measr.nLos;

                UINT8 *ptr;
                ptr = (UINT8 *)&eventData->sRange_data.tdoa_range_measr.aoa[0];
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.aoaFirst, ptr)
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.aoaSecond, ptr);

                ptr = (UINT8 *)&eventData->sRange_data.tdoa_range_measr.pdoa[0];
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.pdoaFirst, ptr);
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.pdoaSecond, ptr);

                ptr = (UINT8 *)&eventData->sRange_data.tdoa_range_measr.pdoaIndex[0];
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.pdoaFirstIndex, ptr);
                UWB_STREAM_TO_UINT16(pRangingData.range_meas_tdoa.pdoaSecondIndex, ptr);

                pRangingData.range_meas_tdoa.timestamp = eventData->sRange_data.tdoa_range_measr.timestamp;
                pRangingData.range_meas_tdoa.blink_frame_number =
                    eventData->sRange_data.tdoa_range_measr.blink_frame_number;
                pRangingData.range_meas_tdoa.rssiRX1 = eventData->sRange_data.tdoa_range_measr.rssiRX1;
                pRangingData.range_meas_tdoa.rssiRX2 = eventData->sRange_data.tdoa_range_measr.rssiRX2;

                pRangingData.range_meas_tdoa.device_info_size =
                    eventData->sRange_data.tdoa_range_measr.device_info_size;
                if (eventData->sRange_data.tdoa_range_measr.device_info_size != 0) {
                    phOsalUwb_MemCopy(pRangingData.range_meas_tdoa.device_info,
                        eventData->sRange_data.tdoa_range_measr.device_info,
                        eventData->sRange_data.tdoa_range_measr.device_info_size);
                }

                pRangingData.range_meas_tdoa.blink_payload_size =
                    eventData->sRange_data.tdoa_range_measr.blink_payload_size;
                if (eventData->sRange_data.tdoa_range_measr.blink_payload_size != 0) {
                    phOsalUwb_MemCopy(pRangingData.range_meas_tdoa.blink_payload_data,
                        eventData->sRange_data.tdoa_range_measr.blink_payload_data,
                        eventData->sRange_data.tdoa_range_measr.blink_payload_size);
                }
            }
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_RANGING_DATA, &pRangingData);
            }

        } break;

        case UWA_DM_REGISTER_EXT_CB_EVT: {
            uwbContext.wstatus = eventData->status;
            NXPLOG_UWBAPI_D("%s: Received Ext Callback Status.\n", __func__);
        } break;
        case UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            NXPLOG_UWBAPI_D("%s: Received Multicast List Status.\n", __func__);
        } break;
        case UWA_DM_SESSION_MC_LIST_UPDATE_NTF_EVT: {
            phMulticastControleeListNtfContext_t pControleeNtfContext;

            pControleeNtfContext.session_id       = eventData->sMulticast_list_ntf.session_id;
            pControleeNtfContext.remaining_list   = eventData->sMulticast_list_ntf.remaining_list;
            pControleeNtfContext.no_of_controlees = eventData->sMulticast_list_ntf.no_of_controlees;
            phOsalUwb_MemCopy(pControleeNtfContext.subsession_id_list,
                eventData->sMulticast_list_ntf.subsession_id_list,
                (eventData->sMulticast_list_ntf.no_of_controlees * SESSION_ID_LEN));
            phOsalUwb_MemCopy(pControleeNtfContext.status_list,
                eventData->sMulticast_list_ntf.status_list,
                eventData->sMulticast_list_ntf.no_of_controlees);

            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_MULTICAST_LIST_NTF, &pControleeNtfContext);
            }

            uwbContext.wstatus = eventData->status;
            NXPLOG_UWBAPI_D("%s: Received Multicast List data.\n", __func__);
        } break;
        default:
            break;
        }
        if (uwbContext.currentEventId == dmEvent || dmEvent == UWA_DM_UWBD_RESP_TIMEOUT_EVT ||
            (isCmdRespPending() && uwbContext.dev_state == UWBD_STATUS_ERROR) ||
            (uwbContext.wstatus == UWBAPI_STATUS_HPD_WAKEUP)) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __func__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            phUwb_GKI_binary_sem_post(uwbContext.devMgmtSem);
        }
    }
}

/*******************************************************************************
 **
 ** Function:        sep_SetWaitEvent
 **
 ** Description:     Update the current event ID in Context with given event ID
 **                  eventID:  event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/
void sep_SetWaitEvent(UINT16 eventID)
{
    uwbContext.currentEventId = eventID;
}

/*******************************************************************************
 **
 ** Function:        RawCommandResponse_Cb
 **
 ** Description:     Receive response from the stack for raw command sent from
 **                  UWB API.
 **                  event:  event ID.
 **                  param_len: length of the response
 **                  p_param: pointer to data
 **
 ** Returns:         None
 **
 *******************************************************************************/
static void rawCommandResponse_Cb(UINT8 event, UINT16 param_len, UINT8 *p_param)
{
    (void)event;
    NXPLOG_UWBAPI_D(
        "NxpResponse_Cb Received length data = 0x%x status = 0x%x", param_len, p_param[UCI_RESPONSE_STATUS_OFFSET]);
    uwbContext.wstatus = UWBAPI_STATUS_FAILED;
    uwbContext.rsp_len = param_len;
    if (param_len > 0 && p_param != NULL) {
        uwbContext.wstatus = p_param[UCI_RESPONSE_STATUS_OFFSET];
        phOsalUwb_MemCopy(uwbContext.rsp_data, p_param, param_len);
    }
    NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __func__);
    uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
    phUwb_GKI_binary_sem_post(uwbContext.devMgmtSem);
}

/*******************************************************************************
 **
 ** Function:        sendRawUci
 **
 ** Description:     Internal function to Send Raw Command
 **
 ** Returns:         Status
 **
 *******************************************************************************/
tUWBAPI_STATUS sendRawUci(UINT8 *p_cmd_params, UINT16 cmd_params_len)
{
    UINT8 cmd_gid, cmd_oid, rsp_gid, rsp_oid;

    cmd_gid            = p_cmd_params[0] & UCI_GID_MASK;
    cmd_oid            = p_cmd_params[1] & UCI_OID_MASK;
    uwbContext.wstatus = UWBAPI_STATUS_FAILED;

    sep_SetWaitEvent(UWA_DM_UWBD_RESP_TIMEOUT_EVT);
    tUWBAPI_STATUS status = UWA_SendRawCommand(cmd_params_len, p_cmd_params, rawCommandResponse_Cb);
    if (status == UWBAPI_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        NXPLOG_UWBAPI_D("%s: Success UWA_SendRawCommand", __func__);
        status = uwbContext.wstatus;
    }

    if (uwbContext.wstatus == UWBAPI_STATUS_TIMEOUT) {
        status = UWBAPI_STATUS_TIMEOUT;
    }
    else if (uwbContext.wstatus == UWBAPI_STATUS_HPD_WAKEUP) {
        status = UWBAPI_STATUS_HPD_WAKEUP;
    }
    else {
        rsp_gid = uwbContext.rsp_data[0] & UCI_GID_MASK;
        rsp_oid = uwbContext.rsp_data[1] & UCI_OID_MASK;
        if ((cmd_gid != rsp_gid) || (cmd_oid != rsp_oid)) {
            LOG_E(
                "Error, Received gid/oid other than what is sent, sent %x%x recv "
                "%x%x\n",
                cmd_gid,
                cmd_oid,
                rsp_gid,
                rsp_oid);
            uwbContext.wstatus = UWBAPI_STATUS_FAILED;
            status             = UWBAPI_STATUS_FAILED;
        }
    }
    return status;
}

/*******************************************************************************
 **
 ** Function:        waitforNotification
 **
 ** Description:     waits for the notification for the specified event time out value.
 **                  waitEventId: Device-management event ID.
 **                  waitEventNtftimeout: Event associated time out value.
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS waitforNotification(UINT16 waitEventId, UINT32 waitEventNtftimeout)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    /*
     * Check the waiting notification is already received or not.
     */
    if (uwbContext.receivedEventId != waitEventId) {
        /*
         *  Wait for the event notification
         */
        sep_SetWaitEvent(waitEventId);

        if (phUwb_GKI_binary_sem_wait_timeout(uwbContext.devMgmtSem, waitEventNtftimeout) == GKI_SUCCESS) {
            status = UWBAPI_STATUS_OK;
        }
        else {
            /*
             * A scenario can happen when waiting for session status notification.
             * Session status notification comes prior to device status notification.
             * In that case, status to be set to ok since the notification is
             * already received. In any case, session state checking is done in the
             * Session init/deinit related API's.
             */
            if (UWA_DM_SESSION_STATUS_NTF_EVT == waitEventId) {
                status = UWBAPI_STATUS_OK;
            }
        }
    }
    else {
        status = UWBAPI_STATUS_OK;
    }
    /*
     * Reset the received event id to default event.
     */
    uwbContext.receivedEventId = DEFAULT_EVENT_TYPE;

    return status;
}

/*******************************************************************************
 **
 ** Function:        getAppConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Application related configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getAppConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
        /* Length 1 Byte */
    case UCI_PARAM_ID_DEVICE_ROLE:
    case UCI_PARAM_ID_RANGING_METHOD:
    case UCI_PARAM_ID_STS_CONFIG:
    case UCI_PARAM_ID_MULTI_NODE_MODE:
    case UCI_PARAM_ID_CHANNEL_NUMBER:
    case UCI_PARAM_ID_NO_OF_CONTROLEES:
    case UCI_PARAM_ID_RNG_DATA_NTF:
    case UCI_PARAM_ID_DEVICE_TYPE:
    case UCI_PARAM_ID_MAC_FCS_TYPE:
    case UCI_PARAM_ID_RANGING_ROUND_PHASE_CONTROL:
    case UCI_PARAM_ID_AOA_RESULT_REQ:
    case UCI_PARAM_ID_RFRAME_CONFIG:
    case UCI_PARAM_ID_RX_MODE:
    case UCI_PARAM_ID_PREAMBLE_CODE_INDEX:
    case UCI_PARAM_ID_SFD_ID:
    case UCI_PARAM_ID_PSDU_DATA_RATE:
    case UCI_PARAM_ID_PREAMBLE_DURATION:
    case UCI_PARAM_ID_RX_ANTENNA_PAIR_SELECTION:
    case UCI_PARAM_ID_MAC_CFG:
    case UCI_PARAM_ID_RANGING_TIME_STRUCT:
    case UCI_PARAM_ID_SLOTS_PER_RR:
    case UCI_PARAM_ID_TX_ADAPTIVE_PAYLOAD_POWER:
    case UCI_PARAM_ID_TX_ANTENNA_SELECTION:
    case UCI_PARAM_ID_RESPONDER_SLOT_INDEX:
    case UCI_PARAM_ID_PRF_MODE:
    case UCI_PARAM_ID_MAX_CONTENTION_PHASE_LEN:
    case UCI_PARAM_ID_CONTENTION_PHASE_UPDATE_LEN:
    case UCI_PARAM_ID_SCHEDULED_MODE:
    case UCI_PARAM_ID_KEY_ROTATION:
    case UCI_PARAM_ID_KEY_ROTATION_RATE:
    case UCI_PARAM_ID_SESSION_PRIORITY:
    case UCI_PARAM_ID_MAC_ADDRESS_MODE:
    case UCI_PARAM_ID_NUMBER_OF_STS_SEGMENTS:
    case UCI_PARAM_ID_RANGING_ROUND_HOPPING:
    case UCI_PARAM_ID_IN_BAND_TERMINATION_ATTEMPT_COUNT:
    case UCI_PARAM_ID_RESULT_REPORT_CONFIG: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;

        /* Length 2 Bytes */
    case UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_NEAR:
    case UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_FAR:
    case UCI_PARAM_ID_SLOT_DURATION:
    case UCI_PARAM_ID_MAX_RR_RETRY:
    case UCI_PARAM_ID_MAX_NUMBER_OF_BLOCKS:
    case UCI_PARAM_ID_BLINK_RANDOM_INTERVAL:
    case UCI_PARAM_ID_VENDOR_ID: {
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;

        /* Length 4 Byte */
    case UCI_PARAM_ID_STS_INDEX:
    case UCI_PARAM_ID_UWB_INITIATION_TIME:
    case UCI_PARAM_ID_SUB_SESSION_ID:
    case UCI_PARAM_ID_RANGING_INTERVAL: {
        tlvBuffer[length++] = 4; // Param len
        UINT32 value        = *((UINT32 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
        tlvBuffer[length++] = (UINT8)(value >> 16);
        tlvBuffer[length++] = (UINT8)(value >> 24);
    } break;

        /* Length Array of 1 Bytes */
    case UCI_PARAM_ID_STATIC_STS_IV:
    case UCI_PARAM_ID_DEVICE_MAC_ADDRESS:
    case UCI_PARAM_ID_DST_MAC_ADDRESS: {
        UINT8 *value        = (UINT8 *)paramValue;
        tlvBuffer[length++] = paramLen; // Param len
        for (UINT8 i = 0; i < (paramLen / sizeof(UINT8)); i++) {
            tlvBuffer[length++] = *value++;
        }
    } break;
    default:
        break;
    }

    return length;
}

/*******************************************************************************
 **
 ** Function:        getTestConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for test configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getTestConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 1 Byte */
    case UCI_TEST_PARAM_ID_RANDOMIZE_PSDU:
    case UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR: {
        tlvBuffer[length++] = 1; // Param len
        UINT8 value         = *((UINT8 *)paramValue);
        tlvBuffer[length++] = value;
    } break;

    /* Length 2 byte */
    case UCI_TEST_PARAM_ID_RAW_PHR: {
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;

    /* Length 4 Byte */
    case UCI_TEST_PARAM_ID_NUM_PACKETS:
    case UCI_TEST_PARAM_ID_T_GAP:
    case UCI_TEST_PARAM_ID_T_START:
    case UCI_TEST_PARAM_ID_T_WIN:
    case UCI_TEST_PARAM_ID_RMARKER_TX_START:
    case UCI_TEST_PARAM_ID_RMARKER_RX_START: {
        tlvBuffer[length++] = 4; // Param len
        UINT32 value        = *((UINT32 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
        tlvBuffer[length++] = (UINT8)(value >> 16);
        tlvBuffer[length++] = (UINT8)(value >> 24);
    } break;
    default:
        break;
    }
    return length;
}

/*******************************************************************************
 **
 ** Function:        getCoreDeviceConfigTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 **                  Array for Core Device configs.
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getCoreDeviceConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length = 0;

    tlvBuffer[length++] = paramId;

    switch (paramId) {
#if (UWBIOT_UWBD_SR040)
    case UCI_EXT_PARAM_ID_TELC_CONFIG: {
        tlvBuffer[length++] = 4; // Param len
        UINT32 value        = *((UINT32 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
        tlvBuffer[length++] = (UINT8)(value >> 16);
        tlvBuffer[length++] = (UINT8)(value >> 24);
    } break;
#endif
    case UCI_PARAM_ID_DEVICE_STATE:
    case UCI_PARAM_ID_LOW_POWER_MODE:
#if (UWBIOT_UWBD_SR040)
    case UCI_EXT_PARAM_ID_MHR_IN_CCM:
    case UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE:
#endif
    {
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
 ** Function:        setdeviceInfo
 **
 ** Description:     Gets Device Info from FW and sets in to context
 **
 ** Returns:         None
 **
 *******************************************************************************/
tUWBAPI_STATUS getDeviceInfo(void)
{
    tUWBAPI_STATUS status;
    sep_SetWaitEvent(UWA_DM_CORE_GET_DEVICE_INFO_RSP_EVT);
    status = UWA_GetDeviceInfo();
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    return status;
}

/*******************************************************************************
 **
 ** Function:        AppConfig_TlvParser
 **
 ** Description:     Application configuration Tlv parser
 **
 ** Returns:         status
 **
 *******************************************************************************/
tUWBAPI_STATUS AppConfig_TlvParser(
    const SetAppParams_List_t *pAppParams_List, SetAppParams_value_au8_t *pOutput_param_value)
{
    tUWBAPI_STATUS status;
    UINT8 *param_value = pAppParams_List->param_value.au8.param_value;

    switch (pAppParams_List->param_type) {
    case kAPPPARAMS_Type_u32:
        pOutput_param_value->param_len = 4;
        UWB_UINT32_TO_FIELD(pOutput_param_value->param_value, pAppParams_List->param_value.vu32);
        status = UWBAPI_STATUS_OK;
        break;
    case kAPPPARAMS_Type_au8:
        pOutput_param_value->param_len = pAppParams_List->param_value.au8.param_len;
        UWB_STREAM_TO_ARRAY(pOutput_param_value->param_value, param_value, pOutput_param_value->param_len);
        status = UWBAPI_STATUS_OK;
        break;
    default:
        status = UWBAPI_STATUS_FAILED;
        break;
    }
    return status;
}

UINT8 tlvSet_appConfigBuf(eAppConfig paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 tlvLen       = 0;
    UINT8 ext_param_id = (UINT8)(paramId >> 8);
    if (!((paramId >= RANGING_METHOD && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
            (ext_param_id == EXTENDED_APP_CONFIG_ID && paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
        return 0;
    }
    if (ext_param_id == EXTENDED_APP_CONFIG_ID) {
#if (UWBIOT_UWBD_SR100T)
        tlvLen = getExtAppConfigTLVBuffer(ext_app_config_mapping[(UINT8)(paramId >> 8)], paramValue, tlvBuffer);
#else
        tlvLen = getExtTLVBuffer(ext_app_config_mapping[(UINT8)paramId], paramValue, tlvBuffer);
#endif
    }
    else {
        tlvLen = getAppConfigTLVBuffer(app_config_mapping[paramId], paramLen, paramValue, tlvBuffer);
    }
    return tlvLen;
}
#if (UWBIOT_UWBD_SR040)
void ufaTestDeviceManagementCallback(eResponse_Test_Event dmEvent, tUWA_DM_TEST_CBACK_DATA *eventData)
{
    //dummy call back
}
#endif
