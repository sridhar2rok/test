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

#include "UwbApi_RfTest.h"
#include "phNxpLogApis_UwbApi.h"

#include "uci_defs.h"
#include "uci_test_defs.h"
#include "AppConfigParams.h"
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary.h"
#include "UwbApi_Proprietary_Internal.h"

#include "UwbAdaptation.h"

/*******************************************************************************
 **
 ** Function:        ufaTestDeviceManagementCallback
 **
 ** Description:     Receive device management events from stack for RF tests.
 **                  dmEvent: Device-management event ID.
 **                  eventData: Data associated with event ID.
 **
 ** Returns:         None
 **
 *******************************************************************************/
void ufaTestDeviceManagementCallback(eResponse_Test_Event dmEvent, tUWA_DM_TEST_CBACK_DATA *eventData)
{
    if (eventData != NULL) {
        NXPLOG_UWBAPI_D("%s: enter; event=0x%X status %d", __func__, dmEvent, eventData->status);

        uwbContext.receivedEventId = (UINT16)dmEvent;
        switch (dmEvent) {
        case UWA_DM_TEST_SET_CONFIG_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_TEST_GET_CONFIG_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
            if (eventData->status == UWA_STATUS_OK &&
                eventData->sTest_get_config.tlv_size <= sizeof(uwbContext.rsp_data)) {
                uwbContext.rsp_len = eventData->sTest_get_config.tlv_size;
                phOsalUwb_MemCopy(
                    uwbContext.rsp_data, eventData->sTest_get_config.param_tlvs, eventData->sTest_get_config.tlv_size);
            }
            else {
                NXPLOG_UWBAPI_E("%s: UWA_DM_TEST_GET_CONFIG_EVT failed", __func__);
                uwbContext.rsp_len = 0;
            }
        } break;
        case UWA_DM_TEST_PERIODIC_TX_RSP_EVT:
        case UWA_DM_TEST_PER_RX_RSP_EVT:
        case UWA_DM_TEST_LOOPBACK_RSP_EVT:
        case UWA_DM_TEST_RX_RSP_EVT:
        case UWA_DM_TEST_STOP_SESSION_RSP_EVT: {
            uwbContext.wstatus = eventData->status;
        } break;
        case UWA_DM_TEST_PERIODIC_TX_NTF_EVT: {
            phPerTxData_t pPerTxData;

            UINT8 *ptr = eventData->rf_test_data.data;
            UWB_STREAM_TO_UINT8(pPerTxData.status, ptr);
            if (uwbContext.pAppCallback) {
                uwbContext.pAppCallback(UWBD_PER_SEND, &pPerTxData);
            }
        } break;
        case UWA_DM_TEST_PER_RX_NTF_EVT:
        case UWA_DM_TEST_LOOPBACK_NTF_EVT:
        case UWA_DM_TEST_RX_NTF_EVT: {
            phRfTestData_t pRfTestData;

            UINT8 *ptr = eventData->rf_test_data.data;
            UWB_STREAM_TO_UINT8(pRfTestData.status, ptr);
            pRfTestData.dataLength = (UINT16)(eventData->rf_test_data.length - sizeof(pRfTestData.status));

            if (pRfTestData.dataLength > 0) {
                phOsalUwb_MemCopy(pRfTestData.data, ptr, pRfTestData.dataLength);
            }
            else {
                phOsalUwb_SetMemory(&pRfTestData.data, 0, MAX_UCI_PACKET_SIZE);
            }

            if (uwbContext.pAppCallback) {
                switch (dmEvent) {
                case UWA_DM_TEST_PER_RX_NTF_EVT:
                    uwbContext.pAppCallback(UWBD_PER_RCV, &pRfTestData);
                    break;
                case UWA_DM_TEST_RX_NTF_EVT:
                    uwbContext.pAppCallback(UWBD_TEST_RX_RCV, &pRfTestData);
                    break;
                case UWA_DM_TEST_LOOPBACK_NTF_EVT:
                    uwbContext.pAppCallback(UWBD_RF_LOOPBACK_RCV, &pRfTestData);
                    break;
                default:
                    break;
                }
            }
        } break;
        default:
            break;
        }
        if (uwbContext.currentEventId == dmEvent) {
            NXPLOG_UWBAPI_D("%s: posting devMgmtSem\n", __func__);
            uwbContext.currentEventId = DEFAULT_EVENT_TYPE;
            phUwb_GKI_binary_sem_post(uwbContext.devMgmtSem);
        }
    }
}

/*******************************************************************************
 **
 ** Function:        parsePerAppParams
 **
 ** Description:     Extracts Per app config Params from the given byte array
 *and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
static void parsePerAppParams(UINT8 *rspPtr, UINT8 noOfParams, phRfTestParams_t *pRfTestParams)
{
    for (int i = 0; i < noOfParams; i++) {
        UINT8 paramId = *rspPtr++;
        UINT8 length  = *rspPtr++;
        UINT8 *value  = rspPtr;
        switch (paramId) {
        case UCI_PARAM_ID_MAC_CFG:
            /* MAC config */
            UWB_STREAM_TO_UINT8(pRfTestParams->macCfg, value);
            break;
        default:
            break;
        }
        rspPtr += length;
    }
}

/*******************************************************************************
 **
 ** Function:        parsePerTestParams
 **
 ** Description:     Extracts PER Test config Params from the given byte array
 *and updates the structure
 **
 ** Returns:         None
 **
 *******************************************************************************/
static void parsePerTestParams(UINT8 *rspPtr, UINT8 noOfParams, phRfTestParams_t *pRfTestParams)
{
    for (int i = 0; i < noOfParams; i++) {
        UINT8 paramId = *rspPtr++;
        UINT8 length  = *rspPtr++;
        UINT8 *value  = rspPtr;
        switch (paramId) {
        case UCI_TEST_PARAM_ID_NUM_PACKETS:
            UWB_STREAM_TO_UINT32(pRfTestParams->numOfPckts, value);
            break;
        case UCI_TEST_PARAM_ID_T_GAP:
            UWB_STREAM_TO_UINT32(pRfTestParams->tGap, value);
            break;
        case UCI_TEST_PARAM_ID_T_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->tStart, value);
            break;
        case UCI_TEST_PARAM_ID_T_WIN:
            UWB_STREAM_TO_UINT32(pRfTestParams->tWin, value);
            break;
        case UCI_TEST_PARAM_ID_RANDOMIZE_PSDU:
            UWB_STREAM_TO_UINT8(pRfTestParams->randomizedSize, value);
            break;
        case UCI_TEST_PARAM_ID_RAW_PHR:
            UWB_STREAM_TO_UINT16(pRfTestParams->rawPhr, value);
            break;
        case UCI_TEST_PARAM_ID_RMARKER_RX_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->rmarkerRxStart, value);
            break;
        case UCI_TEST_PARAM_ID_RMARKER_TX_START:
            UWB_STREAM_TO_UINT32(pRfTestParams->rmarkerTxStart, value);
            break;
        case UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR:
            UWB_STREAM_TO_UINT8(pRfTestParams->stsIndexAutoIncr, value);
            break;
        default:
            break;
        }
        rspPtr += length;
    }
}

/**
 * \brief Set session specific PER parameters

 * \param sessionId - [In] Initialized Session ID
 * \param perParams - [In] Pointer to \ref phRfTestParams_t
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRfTestParams(UINT32 sessionId, const phRfTestParams_t *pRfTestParams)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    UINT8 offset          = 0;
    UINT8 noOfPerParams   = 0;

    UINT8 noOfControlees   = 1;
    UINT8 deviceMacAddr[2] = {0x11, 0x11};
    UINT8 dstMacAddr[2]    = {0x22, 0x22};

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pRfTestParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pPerParam is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_NUM_PACKETS,
                                  sizeof(pRfTestParams->numOfPckts),
                                  (void *)&pRfTestParams->numOfPckts,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_GAP,
                                  sizeof(pRfTestParams->tGap),
                                  (void *)&pRfTestParams->tGap,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_START,
                                  sizeof(pRfTestParams->tStart),
                                  (void *)&pRfTestParams->tStart,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_T_WIN,
                                  sizeof(pRfTestParams->tWin),
                                  (void *)&pRfTestParams->tWin,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RANDOMIZE_PSDU,
                                  sizeof(pRfTestParams->randomizedSize),
                                  (void *)&pRfTestParams->randomizedSize,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RAW_PHR,
                                  sizeof(pRfTestParams->rawPhr),
                                  (void *)&pRfTestParams->rawPhr,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RMARKER_TX_START,
                                  sizeof(pRfTestParams->rmarkerTxStart),
                                  (void *)&pRfTestParams->rmarkerTxStart,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_RMARKER_RX_START,
                                  sizeof(pRfTestParams->rmarkerRxStart),
                                  (void *)&pRfTestParams->rmarkerRxStart,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getTestConfigTLVBuffer(UCI_TEST_PARAM_ID_STS_INDEX_AUTO_INCR,
                                  sizeof(pRfTestParams->stsIndexAutoIncr),
                                  (void *)&pRfTestParams->stsIndexAutoIncr,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    sep_SetWaitEvent(UWA_DM_TEST_SET_CONFIG_RSP_EVT);
    status = UWA_TestSetConfig(sessionId, noOfPerParams, offset, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    offset        = 0;
    noOfPerParams = 0;
    offset        = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_NO_OF_CONTROLEES,
                                  UCI_PARAM_LEN_NO_OF_CONTROLEES,
                                  (void *)&noOfControlees,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_MAC_ADDRESS,
                                  UCI_PARAM_LEN_DEVICE_MAC_ADDRESS,
                                  (void *)&deviceMacAddr,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DST_MAC_ADDRESS,
                                  (UINT8)(UCI_PARAM_LEN_DEST_MAC_ADDRESS * noOfControlees),
                                  (void *)&dstMacAddr,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_MAC_CFG,
                                  sizeof(pRfTestParams->macCfg),
                                  (void *)&pRfTestParams->macCfg,
                                  &uwbContext.snd_data[offset]));
    ++noOfPerParams; // Increment the number of debug params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfPerParams, offset, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    if ((status == UWA_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __func__);
            status = UWBAPI_STATUS_FAILED;
            return status;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Get session specific PER parameters
 *
 * \param sessionId  - [in] Initialized Session ID
 * \param pPerParams - [out] Pointer to \ref phRfTestParams_t . Valid only if API
 * status is success.
 *
 * \retval #UWBAPI_STATUS_OK              - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetRfTestParams(UINT32 sessionId, phRfTestParams_t *pRfTestParams)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    UINT8 *pNxpConfigCommand = NULL;
    UINT16 index             = 0;
    UINT16 paramId           = 0;
    UINT8 noOfParams;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRfTestParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pPerParams is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Get App config */
    noOfParams        = sizeof(uciRfTest_AppParamIds) / sizeof(UINT8);
    pNxpConfigCommand = uwbContext.snd_data;
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRfTest_AppParamIds[index];
        UWB_UINT8_TO_STREAM(pNxpConfigCommand, paramId);
        NXPLOG_UWBAPI_D("%s: App ID: %02X", __func__, paramId);
    }
    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    status = UWA_GetAppConfig(sessionId, noOfParams, noOfParams, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Success UWA_GetAppConfig", __func__);
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            UINT8 *rspPtr = &uwbContext.rsp_data[0];
            parsePerAppParams(rspPtr, noOfParams, pRfTestParams);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: Failed UWA_GetAppConfig", __func__);
        return status;
    }

    /* Get Test Config */
    noOfParams        = sizeof(uciRfTest_TestParamIds) / sizeof(UINT8);
    pNxpConfigCommand = uwbContext.snd_data;
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRfTest_TestParamIds[index];
        UWB_UINT8_TO_STREAM(pNxpConfigCommand, paramId);
        NXPLOG_UWBAPI_D("%s: App ID: %02X", __func__, paramId);
    }
    sep_SetWaitEvent(UWA_DM_TEST_GET_CONFIG_RSP_EVT);
    status = UWA_TestGetConfig(sessionId, noOfParams, noOfParams, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Success UWA_TestGetConfig", __func__);
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            UINT8 *rspPtr = &uwbContext.rsp_data[0];
            parsePerTestParams(rspPtr, noOfParams, pRfTestParams);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: Failed UWA_TestGetConfig", __func__);
        return status;
    }

    NXPLOG_UWBAPI_E("%s: Exit", __func__);
    return status;
}

/**
 * \brief Set Test specific config parameters.
 *
 * \param param_id - [In] Test Config Parameter Id
 * \param param_value - [In] Param value for Test config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetTestConfig(eTestConfig param_id, UINT32 param_value)
{
    UINT8 paramLen       = 0;
    UINT8 noOfParams     = 1;
    UINT32 sessionIdTest = 0x0;

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((UINT8)(param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
        paramLen = getExtTestConfigTLVBuffer(
            (UINT8)(ext_test_config_mapping[(UINT8)param_id] >> 8), (void *)&param_value, &uwbContext.snd_data[0]);
    }
    else {
        paramLen =
            getTestConfigTLVBuffer(uciRfTest_TestParamIds[param_id], 0, (void *)&param_value, &uwbContext.snd_data[0]);
    }

    sep_SetWaitEvent(UWA_DM_TEST_SET_CONFIG_RSP_EVT);
    status = UWA_TestSetConfig(sessionIdTest, noOfParams, paramLen, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Get Test specific config parameters.
 *
 * \param param_id      - [In]  Test Config Parameter Id
 * \param param_value   - [In]  Param value for Test config param id
 * \param param_value   - [Out] Param value for Test config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetTestConfig(eTestConfig param_id, UINT32 *param_value)
{
    UINT8 len            = 0;
    UINT8 offset         = 0;
    UINT8 noOfParams     = 1;
    UINT8 paramLen       = 1;
    UINT32 sessionIdTest = 0x0;

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_TEST_GET_CONFIG_RSP_EVT);
    if ((UINT8)(param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
        paramLen++;
        status =
            UWA_TestGetConfig(sessionIdTest, noOfParams, paramLen, (UINT8 *)&ext_test_config_mapping[(UINT8)param_id]);
    }
    else {
        status =
            UWA_TestGetConfig(sessionIdTest, noOfParams, paramLen, (UINT8 *)&uciRfTest_TestParamIds[(UINT8)param_id]);
    }

    phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

    if (status == UWA_STATUS_OK) {
        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            offset++;
            if ((UINT8)(param_id >> 8) == EXTENDED_TEST_CONFIG_ID) {
                offset++;
            }
            len           = uwbContext.rsp_data[offset++];
            UINT8 *rspPtr = &uwbContext.rsp_data[offset];
            if (len == sizeof(UINT8)) {
                UWB_STREAM_TO_UINT8(*param_value, rspPtr);
            }
            else if (len == sizeof(UINT16)) {
                UWB_STREAM_TO_UINT16(*param_value, rspPtr);
            }
            else if (len == sizeof(UINT32)) {
                UWB_STREAM_TO_UINT32(*param_value, rspPtr);
            }
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief RF Start test
 *
 * \param paramId      - [in] Param ID
 * \param pStartData   - [in] Start data
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if input data in null
 * \retval #UWBAPI_STATUS_REJECTED        - if rf parameters are not set
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRfTest(eStartRfParam paramId, phRfStartData_t *pStartData)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if ((paramId != RF_TEST_RX) && pStartData == NULL) { //for Single Rx RF test psdu data is NUL
        NXPLOG_UWBAPI_E("%s: Input data is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    switch (paramId) {
    case RF_START_PER_TX:
        sep_SetWaitEvent(UWA_DM_TEST_PERIODIC_TX_RSP_EVT);
        status = UWA_PeriodicTxTest(
            ((phStartPerTxData_t *)pStartData)->txDataLength, ((phStartPerTxData_t *)pStartData)->txData);
        break;
    case RF_START_PER_RX:
        sep_SetWaitEvent(UWA_DM_TEST_PER_RX_RSP_EVT);
        status =
            UWA_PerRxTest(((phStartPerRxData_t *)pStartData)->rxDataLength, ((phStartPerRxData_t *)pStartData)->rxData);
        break;
    case RF_LOOPBACK_TEST:
        sep_SetWaitEvent(UWA_DM_TEST_LOOPBACK_RSP_EVT);
        status = UWA_UwbLoopBackTest(((phLoopbackTestData_t *)pStartData)->loopbackDataLength,
            ((phLoopbackTestData_t *)pStartData)->loopbackData);
        break;
    case RF_TEST_RX:
        sep_SetWaitEvent(UWA_DM_TEST_RX_RSP_EVT);
        status = UWA_RxTest();
        break;
    default:
        NXPLOG_UWBAPI_E("%s:    Invalid Param ", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief RF Test stop
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_REJECTED        - if per parameters are not set
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_Stop_RfTest(void)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_TEST_STOP_SESSION_RSP_EVT);
    status = UWA_TestStopSession();
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}
