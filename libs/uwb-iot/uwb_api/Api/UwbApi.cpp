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

#include "UwbApi.h"
#include "AppConfigParams.h"
#include "uwa_api.h"
#include "phNxpLogApis_UwbApi.h"
#include "phOsalUwb.h"
#include "phOsalUwb_Timer.h"

#include "phNxpUwbConfig.h"
#include "UwbAdaptation.h"

#if UWBIOT_UWBD_SR100T
#include <Mainline_Firmware.h>
#endif

#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary_Internal.h"

/**
 * \brief Initialize the UWB Middleware stack
 *
 * \param pCallback - [in] Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications (Ranging
 * data/App Data/Per Tx & Rx) at application layer.)
 *
 * \retval #UWBAPI_STATUS_OK            - on success
 * \retval #UWBAPI_STATUS_TIMEOUT       - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED        - otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_Init(tUwbApi_AppCallback *pCallback)
{
#if UWBIOT_UWBD_SR100T
    SetFirmwareImage((UINT8 *)heliosEncryptedMainlineFwImage, sizeof(heliosEncryptedMainlineFwImage));
#endif
    return uwbInit(pCallback);
}

/**
 * \brief De-initializes the UWB Middleware stack
 *
 * \retval #UWBAPI_STATUS_OK     - on success
 * \retval #UWBAPI_STATUS_FAILED - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT- if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_ShutDown()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_OK;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_D("%s: UWB device is already  deinitialized", __func__);
        return UWBAPI_STATUS_OK;
    }
    sep_SetWaitEvent(UWA_DM_DISABLE_EVT);
    status = UWA_Disable(TRUE); /* gracefull exit */
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
    }
    else {
        NXPLOG_UWBAPI_E("%s: De-Init is failed:", __func__);
        status = UWBAPI_STATUS_FAILED;
    }
    cleanUp();
    phUwb_LogDeInit();
    return status;
}

/**
 * \brief API to recover from crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverUWBS()
{
#if UWBIOT_UWBD_SR100T
    SetFirmwareImage((UINT8 *)heliosEncryptedMainlineFwImage, sizeof(heliosEncryptedMainlineFwImage));
#endif
    return recoverUWBS();
}

/**
 * \brief Resets UWBD device to Ready State
 *
 * \param resetConfig - [in] Supported Value: UWBD_RESET
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_UwbdReset(UINT8 resetConfig)
{
    tUWBAPI_STATUS status;

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_DEVICE_RESET_RSP_EVT);
    uwbContext.dev_state = UWBAPI_UCI_DEV_ERROR;
    status               = UWA_SendDeviceReset(resetConfig);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_DEVICE_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.dev_state != UWBAPI_UCI_DEV_READY) {
            status = UWA_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Gets UWB Device State
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if parameter is invalid
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetUwbDevState(UINT8 *pDeviceState)
{
    tUWA_STATUS status;
    tUWA_PMID configParam[] = {UCI_PARAM_ID_DEVICE_STATE};

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (pDeviceState == NULL) {
        NXPLOG_UWBAPI_E("%s: pDeviceState is NULL\n", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB Device is not initialized\n", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    sep_SetWaitEvent(UWA_DM_CORE_GET_CONFIG_RSP_EVT);
    status = UWA_GetCoreConfig(1, sizeof(configParam), configParam);

    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK && uwbContext.rsp_data[0] == UCI_PARAM_ID_DEVICE_STATE) {
            *pDeviceState = uwbContext.rsp_data[2];
        }
        else {
            NXPLOG_UWBAPI_E("%s: Get UWB DEV state is failed\n", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get UWB DEV state is failed\n", __func__);
        status = UWBAPI_STATUS_FAILED;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Initializes session for a Type(Ranging/Data/Per)
 *
 * \param sessionId   - [In] Unique Session ID
 * \param sessionType - [In] Type of Session(Ranging/Data/Per)
 *
 * \retval #UWBAPI_STATUS_OK                    - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED       - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_DUPLICATE     - if session with the session ID
 *                                               passed already exists
 * \retval #UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED - if more than 5 sessions are exceeded
 * \retval #UWBAPI_STATUS_TIMEOUT               - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionInit(UINT32 sessionId, eSessionType sessionType)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (sessionType == UWBD_RFTEST) {
        if (sessionId != 0x00) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }
    sep_SetWaitEvent(UWA_DM_SESSION_INIT_RSP_EVT);
    status = UWA_SendSessionInit(sessionId, sessionType);
    if (UWA_STATUS_OK == status) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_INITIALIZED) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_INITIALIZED notification", __func__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);

    return status;
}

/**
 * \brief De-initialize based on Session ID
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionDeinit(UINT32 sessionId)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_SESSION_DEINIT_RSP_EVT);
    status = UWA_SendSessionDeInit(sessionId);
    if (UWA_STATUS_OK == status) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId ||
            uwbContext.sessionInfo.state != UWB_SESSION_DEINITIALIZED) {
            return UWBAPI_STATUS_FAILED;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Set session specific ranging parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param pRangingParam - [In] Pointer to \ref phRangingParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRangingParams(UINT32 sessionId, const phRangingParams_t *pRangingParam)
{
    UINT8 offset = 0;
    tUWBAPI_STATUS status;
    UINT8 addrLen           = 0;
    UINT8 noOfRangingParams = 0;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParam == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParam is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_ROLE,
                                  sizeof(pRangingParam->deviceRole),
                                  (void *)&pRangingParam->deviceRole,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_MULTI_NODE_MODE,
                                  sizeof(pRangingParam->multiNodeMode),
                                  (void *)&pRangingParam->multiNodeMode,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count
#if (UWBIOT_UWBD_SR100T)
    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_MAC_ADDRESS_MODE,
                                  sizeof(pRangingParam->macAddrMode),
                                  (void *)&pRangingParam->macAddrMode,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count
#endif

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_NO_OF_CONTROLEES,
                                  sizeof(pRangingParam->noOfControlees),
                                  (void *)&pRangingParam->noOfControlees,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count
#if (UWBIOT_UWBD_SR100T)
    if (pRangingParam->macAddrMode == SHORT_MAC_ADDRESS) {
        addrLen = (UINT8)MAC_SHORT_ADD_LEN;
    }
    else if (pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS ||
             pRangingParam->macAddrMode == EXTENDED_MAC_ADDRESS_AND_HEADER) {
        addrLen = (UINT8)MAC_EXT_ADD_LEN;
    }
#else
    addrLen = (UINT8)MAC_SHORT_ADD_LEN;
#endif
    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_MAC_ADDRESS,
                                  addrLen,
                                  (void *)&pRangingParam->deviceMacAddr,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DST_MAC_ADDRESS,
                                  (UINT8)(addrLen * (pRangingParam->noOfControlees)),
                                  (void *)&pRangingParam->dstMacAddr,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_DEVICE_TYPE,
                                  sizeof(pRangingParam->deviceType),
                                  (void *)&pRangingParam->deviceType,
                                  &uwbContext.snd_data[offset]));
    ++noOfRangingParams; // Increment the number of ranging params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfRangingParams, offset, uwbContext.snd_data);
#if UWBIOT_OS_NATIVE
    phOsalUwb_Delay(100);
#endif
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    if ((status == UWA_STATUS_OK) && (uwbContext.sessionInfo.state != UWB_SESSION_IDLE)) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            NXPLOG_UWBAPI_E("%s: Failed to get SESSION_IDLE notification", __func__);
            status = UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Get session specific ranging parameters

 * \param sessionId     - [In] Initialized Session ID
 * \param pRangingParam - [Out] Pointer to \ref phRangingParams_t
 *
 * \retval #UWBAPI_STATUS_OK on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */

EXTERNC tUWBAPI_STATUS UwbApi_GetRangingParams(UINT32 sessionId, phRangingParams_t *pRangingParams)
{
    tUWBAPI_STATUS status;
    UINT8 *pNxpConfigCommand = NULL;
    UINT16 index             = 0;
    UINT8 paramId            = 0;
    UINT8 noOfParams;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRangingParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pRangingParams is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    noOfParams        = sizeof(uciRangingParamIds) / sizeof(UINT8);
    pNxpConfigCommand = uwbContext.snd_data;
    for (index = 0; index < noOfParams; index++) {
        paramId = uciRangingParamIds[index];
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
            parseRangingParams(rspPtr, noOfParams, pRangingParams);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: Failed UWA_GetAppConfig", __func__);
        return status;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Set session specific app config parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param param_id - [In] App Config Parameter Id
 * \param param_value - [In] Param value for App config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */

EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfig(UINT32 sessionId, eAppConfig param_id, UINT32 param_value)
{
    tUWBAPI_STATUS status;
    UINT8 ext_param_id = 0;
    UINT8 noOfParams   = 1;
    UINT8 paramLen     = 0;

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    ext_param_id = (UINT8)(param_id >> 8);

    if (!((param_id >= RANGING_METHOD && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            (ext_param_id == EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS)))
        return UWBAPI_STATUS_INVALID_PARAM;

    if (ext_param_id == EXTENDED_APP_CONFIG_ID) {
#if (UWBIOT_UWBD_SR100T)
        paramLen = getExtAppConfigTLVBuffer(
            (UINT8)(ext_app_config_mapping[(UINT8)param_id] >> 8), (void *)&param_value, &uwbContext.snd_data[0]);
#else
        paramLen =
            getExtTLVBuffer(ext_app_config_mapping[(UINT8)param_id], (void *)&param_value, &uwbContext.snd_data[0]);
#endif
    }
    else {
        paramLen =
            getAppConfigTLVBuffer(app_config_mapping[param_id], 0, (void *)&param_value, &uwbContext.snd_data[0]);
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfParams, paramLen, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Host shall use this API to set multiple application confifuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * \param sessionId      - [In] Initialized Session ID
 * \param noOfparams     - [In] Number of App Config Parameters
 * \param AppParams_List - [In] Application parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
tUWBAPI_STATUS UwbApi_SetAppConfigMultipleParams(
    UINT32 sessionId, UINT8 noOfparams, const SetAppParams_List_t *AppParams_List)
{
    UINT16 cmdLen = 0;
    UINT8 ext_param_id;
    eAppConfig paramId;
    UINT8 cmdBuffer[UCI_MAX_PAYLOAD_SIZE];
    tUWBAPI_STATUS status;
    SetAppParams_value_au8_t output_param_value;

    /* Assign the buffer for storing all the configs */
    output_param_value.param_value = uwbContext.snd_data;

    for (UINT32 LoopCnt = 0; LoopCnt < noOfparams; ++LoopCnt) {
        ext_param_id = (AppParams_List[LoopCnt].param_id >> 8);
        paramId      = AppParams_List[LoopCnt].param_id;

        if (!((paramId >= RANGING_METHOD && paramId < END_OF_SUPPORTED_APP_CONFIGS) ||
                (ext_param_id == EXTENDED_APP_CONFIG_ID && paramId < END_OF_SUPPORTED_EXT_CONFIGS))) {
            return UWBAPI_STATUS_INVALID_PARAM;
        }

        /* parse and get input length and pointer */
        if (AppConfig_TlvParser(&AppParams_List[LoopCnt], &output_param_value) != UWBAPI_STATUS_OK) {
            return UWBAPI_STATUS_FAILED;
        }

        if (ext_param_id == EXTENDED_APP_CONFIG_ID) {
#if (UWBIOT_UWBD_SR100T)
            cmdLen += getExtAppConfigTLVBuffer((UINT8)(ext_app_config_mapping[(UINT8)paramId] >> 8),
                (void *)(output_param_value.param_value),
                cmdBuffer + cmdLen);
#else
            /* Safe check for array indexing. Coverity issue fix. */
            if ((UINT8)paramId < sizeof(ext_app_config_mapping)) {
                cmdLen += getExtTLVBuffer(ext_app_config_mapping[(UINT8)paramId],
                    (void *)(output_param_value.param_value),
                    cmdBuffer + cmdLen);
            }
            else {
                NXPLOG_UWBAPI_E("%s: param_id is outside the range of ext_app_config_mapping", __func__);
                return UWBAPI_STATUS_INVALID_PARAM;
            }
#endif
        }
        else {
            cmdLen += getAppConfigTLVBuffer(app_config_mapping[paramId],
                output_param_value.param_len,
                output_param_value.param_value,
                cmdBuffer + cmdLen);
        }
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfparams, cmdLen, cmdBuffer);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Get session specific app config parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param param_id - [In] App Config Parameter Id
 * \param param_value - [Out] Param value for App config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfig(UINT32 sessionId, eAppConfig param_id, UINT32 *param_value)
{
    UINT8 len          = 0;
    UINT8 offset       = 0;
    UINT8 noOfParams   = 1;
    UINT8 paramLen     = 1;
    UINT8 ext_param_id = 0;

    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (param_value == NULL) {
        NXPLOG_UWBAPI_E("%s: param_value is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    ext_param_id = (UINT8)(param_id >> 8);
    if (!((param_id >= RANGING_METHOD && param_id < END_OF_SUPPORTED_APP_CONFIGS) ||
            (ext_param_id == EXTENDED_APP_CONFIG_ID && param_id < END_OF_SUPPORTED_EXT_CONFIGS)))
        return UWBAPI_STATUS_INVALID_PARAM;

    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    if ((UINT8)(param_id >> 8) == EXTENDED_APP_CONFIG_ID) {
#if (UWBIOT_UWBD_SR100T)
        paramLen++;
#endif
        /* Safe check for array indexing. Coverity issue fix. */
        if ((UINT8)param_id < sizeof(ext_app_config_mapping)) {
            status =
                UWA_GetAppConfig(sessionId, noOfParams, paramLen, (UINT8 *)&ext_app_config_mapping[(UINT8)param_id]);
        }
        else {
            NXPLOG_UWBAPI_E("%s: param_id is outside the range of ext_app_config_mapping", __func__);
            return UWBAPI_STATUS_INVALID_PARAM;
        }
    }
    else {
        status = UWA_GetAppConfig(sessionId, noOfParams, paramLen, (UINT8 *)&app_config_mapping[(UINT8)param_id]);
    }
    phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

    if (status == UWA_STATUS_OK) {
        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            offset++;
            if ((UINT8)(param_id >> 8) == EXTENDED_APP_CONFIG_ID) {
                offset++;
            }
            /* rsp_data contains complete rsp, we have to skip Header + status + 1
             * byte which contains noOfParams */
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
 * \brief Sets session specific app config parameters Vendor ID and Static STS
 * IV.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param vendorId      - [In] App Config Parameter Vendor Id
 * \param staticStsIv   - [In] Param value for App config param static Sts Iv
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetStaticSts(UINT32 sessionId, UINT16 vendorId, UINT8 *staticStsIv)
{
    UINT8 offset = 0;
    tUWBAPI_STATUS status;
    UINT8 noOfParams = 0;

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (staticStsIv == NULL) {
        NXPLOG_UWBAPI_E("%s: Static Sts Iv is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    offset = getAppConfigTLVBuffer(
        UCI_PARAM_ID_VENDOR_ID, UCI_PARAM_LEN_VENDOR_ID, (void *)&vendorId, &uwbContext.snd_data[offset]);
    ++noOfParams; // Increment the number of params count

    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_STATIC_STS_IV,
                                  UCI_PARAM_LEN_STATIC_STS_IV,
                                  (void *)staticStsIv,
                                  &uwbContext.snd_data[offset]));
    ++noOfParams; // Increment the number of params count

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfParams, offset, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Start Ranging for a session. Before Invoking Start ranging its
 * mandatory to set all the ranging configurations.
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRangingSession(UINT32 sessionId)
{
    tUWBAPI_STATUS status;
#if (UWBIOT_UWBD_SR100T)
#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
    UINT8 KeyFetchErrorRetryCnt = 0U;
#endif
#endif
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
#if (UWBIOT_UWBD_SR100T)
#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
RetryUponKeyFetchError:
#endif
#endif
    sep_SetWaitEvent(UWA_DM_RANGE_START_RSP_EVT);
    status = UWA_StartRangingSession(sessionId);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
#if (UWBIOT_UWBD_SR100T)
#if defined(ENABLE_NFC) && ENABLE_NFC == TRUE
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWBD_SE_RANGING_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
            /*
         * Hanlde the Key Fetch Error Handling. Only till Key Fetch error retry
         * count.
         */
            if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_KEY_FETCH_ERROR) {
                status = UCI_STATUS_ESE_RECOVERY_FAILURE;
                while (KeyFetchErrorRetryCnt < UWB_KEY_FETCH_ERROR_RETRY_COUNT) {
                    /*
                   * Increment the Key Fetch Error Retry Count.
                   */
                    ++KeyFetchErrorRetryCnt;
                    /*
                     * Wait for SE COMM ERROR NTF.
                     */
                    status = waitforNotification(EXT_UCI_MSG_SE_COMM_ERROR_NTF, UWB_NTF_TIMEOUT);
                    if (status == UWBAPI_STATUS_OK) {
                        /*
                         * If the Recovery is successful during se_comm_error notification
                         * then Resend start ranging command.
                         */
                        if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_SUCCESS) {
                            goto RetryUponKeyFetchError;
                        }
                        else if (uwbContext.wstatus == UCI_STATUS_ESE_RECOVERY_FAILURE) {
                            /*
                             * Reset the eSE.[Do the cold reset of eSE]
                             */
                            reset_se_on_error();
                            status = UWBAPI_STATUS_ESE_RESET;
                            break;
                        }
                    }
                }
            }
            else if (uwbContext.sessionInfo.state == UCI_SESSION_FAILED_WITH_NO_RNGDATA_IN_SE) {
                status = UWBAPI_STATUS_SESSION_NOT_EXIST;
            }
            else {
                status = UWBAPI_STATUS_FAILED;
            }
        }
    }
#endif /* defined(ENABLE_NFC) && ENABLE_NFC == TRUE */
#else  /* UWBIOT_UWBD_SR100T */
    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_ACTIVE) {
            status = UWBAPI_STATUS_FAILED;
        }
    }
#endif /* ENABLE_NFC */

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Stop Ranging for a session
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopRangingSession(UINT32 sessionId)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    sep_SetWaitEvent(UWA_DM_RANGE_STOP_RSP_EVT);
    status = UWA_StopRangingSession(sessionId);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    if (status == UWA_STATUS_OK) {
        status = waitforNotification(UWA_DM_SESSION_STATUS_NTF_EVT, UWB_NTF_TIMEOUT);
        if (uwbContext.sessionInfo.session_id != sessionId || uwbContext.sessionInfo.state != UWB_SESSION_IDLE) {
            return UWBAPI_STATUS_FAILED;
        }
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Enable Ranging Data Notifications different options.

 * \param sessionId            - [In] Initialized Session ID
 * \param enableRangingDataNtf - [In] Enable Ranging data notification - 0/1/2.
 option 2 is not allowed when ranging is ongoing.
 * \param proximityNear        - [In] Proximity Near value valid if
 enableRangingDataNtf sets to 2
 * \param proximityFar         - [In] Proximity far value valid if
 enableRangingDataNtf sets to 2
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_EnableRangingDataNtf(
    UINT32 sessionId, UINT8 enableRangingDataNtf, UINT16 proximityNear, UINT16 proximityFar)
{
    UINT8 noOfParam = 1;
    UINT8 offset    = 0;
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (enableRangingDataNtf > 2) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    offset = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_RNG_DATA_NTF,
                                  UCI_PARAM_LEN_RNG_DATA_NTF,
                                  &enableRangingDataNtf,
                                  &uwbContext.snd_data[offset]));
    if (enableRangingDataNtf == 2) {
        noOfParam = (UINT8)(noOfParam + 2);
        offset    = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_NEAR,
                                      UCI_PARAM_LEN_RNG_DATA_NTF_PROXIMITY_NEAR,
                                      &proximityNear,
                                      &uwbContext.snd_data[offset]));
        offset    = (UINT8)(offset + getAppConfigTLVBuffer(UCI_PARAM_ID_RNG_DATA_NTF_PROXIMITY_FAR,
                                      UCI_PARAM_LEN_RNG_DATA_NTF_PROXIMITY_FAR,
                                      &proximityFar,
                                      &uwbContext.snd_data[offset]));
    }

    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfParam, offset, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Send UCI RAW command.
 *
 * \param data     - [in] UCI Command to be sent
 * \param data_len - [in] Length of the UCI Command
 * \param resp     - [out] Response Received
 * \param respLen  - [out] Response length
 *
 * \retval #UWBAPI_STATUS_OK              - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if wrong parameter is passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW - if response buffer is not sufficient
 *                                          to hold the response
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendRawCommand(UINT8 data[], UINT16 data_len, UINT8 *pResp, UINT16 *pRespLen)
{
    tUWA_STATUS status;

    NXPLOG_UWBAPI_D("%s: enter; ", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        *pRespLen = 0;
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (data == NULL || data_len <= 0 || pResp == NULL || pRespLen == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (*pRespLen == 0) {
        NXPLOG_UWBAPI_E("%s: pRespLen is Zero", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(data, data_len);
    if (uwbContext.rsp_len > *pRespLen) {
        NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __func__);
        status = UWBAPI_STATUS_BUFFER_OVERFLOW;
    }
    else {
        *pRespLen = uwbContext.rsp_len;
        phOsalUwb_MemCopy(pResp, uwbContext.rsp_data, uwbContext.rsp_len);
    }
    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Get Session State
 *
 * \param sessionID    - [in] Initialized Session ID
 * \param sessionState - [out] Session Status
 *
 * \retval #UWBAPI_STATUS_OK               - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED           - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT          - if command is timeout
 *
 * if API returns UWBAPI_STATUS_OK, Session State would be one of the below
 * values
 * \n #UWBAPI_SESSION_INIT_SUCCESS     - Session is Initialized
 * \n #UWBAPI_SESSION_DEINIT_SUCCESS   - Session is De-initialized
 * \n #UWBAPI_SESSION_ACTIVATED        - Session is Busy
 * \n #UWBAPI_SESSION_IDLE             - Session is Idle
 * \n #UWBAPI_SESSION_ERROR            - Session Not Found
 *
 * if API returns not UWBAPI_STATUS_OK, Session State is set to
 * UWBAPI_SESSION_ERROR
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetSessionState(UINT32 sessionId, UINT8 *sessionState)
{
    tUWA_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter; ", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (sessionState == NULL) {
        NXPLOG_UWBAPI_E("%s: sessionState is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_STATE_RSP_EVT);
    status = UWA_GetSessionStatus(sessionId);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

        status = uwbContext.wstatus;
        if (status != UWA_STATUS_OK) {
            *sessionState = UWBAPI_SESSION_ERROR;
        }
        else {
            *sessionState = uwbContext.sessionState;
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Update Controller Multicast List.
 *
 * \param pControleeContext - [In] Controlee Context
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_UpdateControllerMulticastList(phMulticastControleeListContext_t *pControleeContext)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pControleeContext == NULL) {
        NXPLOG_UWBAPI_E("%s: pControleeContext is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    if (pControleeContext->action == MULTICAST_LIST_DEL_CONTROLEE) {
        for (UINT8 i = 0; i < pControleeContext->no_of_controlees; i++) {
            if (pControleeContext->subsession_id_list[i] != 0x0) {
                return UWBAPI_STATUS_INVALID_PARAM; //for deletion, sub session id must be zero
            }
        }
    }

    sep_SetWaitEvent(UWA_DM_SESSION_MC_LIST_UPDATE_RSP_EVT);
    status = UWA_ControllerMulticastListUpdate(pControleeContext->session_id,
        pControleeContext->action,
        pControleeContext->no_of_controlees,
        pControleeContext->short_address_list,
        pControleeContext->subsession_id_list);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);
        status = uwbContext.wstatus;
    }

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}
