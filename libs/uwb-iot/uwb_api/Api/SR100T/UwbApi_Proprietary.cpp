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
#include "UwbApi_Proprietary_Internal.h"
#include "phNxpLogApis_UwbApi.h"

#include "uwa_api.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"
#include "UwbAdaptation.h"
#include "Factory_Firmware.h"
#include "AppConfigParams.h"
#include "PrintUtility.h"

/**
 * \brief Initialize the UWB Middleware stack with Factory Firmware
 *
 * \param pCallback - [in] Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications at
 * application layer.)
 *
 * \retval #UWBAPI_STATUS_OK            - on success
 * \retval #UWBAPI_STATUS_FAILED        - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT       - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_FactoryInit(tUwbApi_AppCallback *pCallback)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
#if (FACTORY_MODE == ENABLED)

    SetFirmwareImage((UINT8 *)heliosEncryptedFactoryFwImage, sizeof(heliosEncryptedFactoryFwImage));
    return uwbInit(pCallback);
#endif
    return status;
}

/**
 * \brief API to recover from Factory Firmware crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverFactoryUWBS()
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
#if (FACTORY_MODE == ENABLED)
    SetFirmwareImage((UINT8 *)heliosEncryptedFactoryFwImage, sizeof(heliosEncryptedFactoryFwImage));
    return recoverUWBS();
#endif
    return status;
}

/**
 * \brief returns UCI, FW and MW version
 *
 * \param pdevInfo - [out] Pointer to \ref phUwbDevInfo_t
 *
 * \retval UWBAPI_STATUS_OK              - if successful
 * \retval UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
 * \retval UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval UWBAPI_STATUS_FAILED          - otherwise
 * \retval UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetStackCapabilities(phUwbDevInfo_t *pdevInfo)
{
    tUWA_STATUS status = UWA_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: enter; ", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pdevInfo == NULL) {
        NXPLOG_UWBAPI_E("%s: pdevInfo is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pdevInfo->uciVersion = uwbContext.devInfo.uciVersion;
    pdevInfo->devNameLen = uwbContext.devInfo.devNameLen;
    phOsalUwb_MemCopy(pdevInfo->devName, uwbContext.devInfo.devName, uwbContext.devInfo.devNameLen);
    pdevInfo->fwMajor     = uwbContext.devInfo.fwMajor;
    pdevInfo->fwMinor     = uwbContext.devInfo.fwMinor;
    pdevInfo->fwRc        = uwbContext.devInfo.fwRc;
    pdevInfo->nxpUciMajor = uwbContext.devInfo.nxpUciMajor;
    pdevInfo->nxpUciMinor = uwbContext.devInfo.nxpUciMinor;
    pdevInfo->nxpUciPatch = uwbContext.devInfo.nxpUciPatch;
    phOsalUwb_MemCopy(pdevInfo->nxpChipId, uwbContext.devInfo.nxpChipId, sizeof(pdevInfo->nxpChipId));
    pdevInfo->mwMajor = RHODES_MW_MAJOR_VERSION;
    pdevInfo->mwMinor = RHODES_MW_MINOR_VERSION;
    status            = UWA_STATUS_OK;
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Do calibration parameters.
 *
 * \param paramId     - [In] Channel
 * \param paramValue  - [In] Do Calibration param Id \ref eDoCalibParam
 * \param paramcalibResp - [Out] Pointer to \ref phCalibRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoCalibration(UINT8 channel, eDoCalibParam paramId, phCalibRespStatus_t *calibResp)
{
    tUWBAPI_STATUS status    = UWBAPI_STATUS_FAILED;
    UINT8 payloadLen         = (UINT8)(sizeof(channel) + sizeof(UINT8));
    UINT8 *pDoCalibrationCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    switch (paramId) {
    case DO_CALIB_VCO_PLL:
    case DO_CALIB_PAPPPA_CALIB_CTRL:
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid do calibration parameter ", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pDoCalibrationCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pDoCalibrationCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pDoCalibrationCmd, EXT_UCI_MSG_DO_CALIBRATION);
    UWB_UINT8_TO_STREAM(pDoCalibrationCmd, 0x00);
    UWB_UINT8_TO_STREAM(pDoCalibrationCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pDoCalibrationCmd, channel);
    UWB_UINT8_TO_STREAM(pDoCalibrationCmd, (UINT8)paramId);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));

    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(EXT_UCI_MSG_DO_CALIBRATION, UWBD_DO_CALIB_NTF_TIMEOUT);

        if (status == UWBAPI_STATUS_OK) {
            calibResp->status = uwbContext.doCalibrationStatus.status;
            if (uwbContext.doCalibrationStatus.status == UWBAPI_STATUS_OK) {
                calibResp->length = uwbContext.doCalibrationStatus.length;

                if (uwbContext.rsp_len > sizeof(calibResp->calibValueOut)) {
                    NXPLOG_UWBAPI_E("doCalib response data size is more than response buffer", __func__);
                    return UWBAPI_STATUS_BUFFER_OVERFLOW;
                }
                phOsalUwb_MemCopy(
                    calibResp->calibValueOut, uwbContext.doCalibrationStatus.calibValueOut, calibResp->length);
                NXPLOG_UWBAPI_D("%s: Do Calibration notification successful", __func__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Do Calibration notification failed", __func__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Do Calibration notification time out", __func__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Do Calibration Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Do Calibration failed", __func__);
    }

    if (status != UWBAPI_STATUS_OK) {
        calibResp->length = 0;
    }
    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Set calibration parameters.
 *
 * \param channel     - [In] channel
 * \param paramId     - [In] Calibration parameter ID
 * \param paramValue  -  [In] Calibration value
 * \param length      -  [In] Calibration value array length
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(UINT8 channel, eCalibParam paramId, UINT8 *calibrationValue, UINT8 length)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    UINT8 payloadLen          = (UINT8)(sizeof(channel) + sizeof(UINT8) + length);
    UINT8 *pSetCalibrationCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    switch (paramId) {
    case VCO_PLL:
    case TX_POWER:
    case RF_XTAL_CAP:
    case RSSI_CALIB_CONST1:
    case RSSI_CALIB_CONST2:
    case SNR_CALIB_CONST:
    case MANUAL_TX_POW_CTRL:
    case PDOA_OFFSET:
    case PAPPPA_CALIB_CTRL:
    case TX_TEMPARATURE_COMP:
    case AOA_FINE_CALIB:
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    if (calibrationValue == NULL || length == 0) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pSetCalibrationCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pSetCalibrationCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pSetCalibrationCmd, EXT_UCI_MSG_SET_CALIBRATION);
    UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 0x00);
    UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pSetCalibrationCmd, channel);
    UWB_UINT8_TO_STREAM(pSetCalibrationCmd, (UINT8)paramId);
    UWB_ARRAY_TO_STREAM(pSetCalibrationCmd, calibrationValue, length);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));

    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Set Calibration successful", __func__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Set Calibration Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_INVALID_RANGE) {
        NXPLOG_UWBAPI_E("%s: Set Calibration Command failed, Invalid value range", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Set Calibration failed", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Get calibration parameters.
 *
 * \param paramId     - [In] Channel
 * \param paramValue  - [In] Calibration param Id
 * \param paramcalibResp - [Out] Pointer to \ref phCalibRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetCalibration(UINT8 channel, eCalibParam paramId, phCalibRespStatus_t *calibResp)
{
    tUWBAPI_STATUS status     = UWBAPI_STATUS_FAILED;
    UINT8 payloadLen          = sizeof(channel) + sizeof(UINT8);
    UINT8 *pGetCalibrationCmd = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    switch (paramId) {
    case VCO_PLL:
    case TX_POWER:
    case RF_XTAL_CAP:
    case RSSI_CALIB_CONST1:
    case RSSI_CALIB_CONST2:
    case SNR_CALIB_CONST:
    case MANUAL_TX_POW_CTRL:
    case PDOA_OFFSET:
    case PAPPPA_CALIB_CTRL:
    case TX_TEMPARATURE_COMP:
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid calibration parameter ", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pGetCalibrationCmd = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pGetCalibrationCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pGetCalibrationCmd, EXT_UCI_MSG_GET_CALIBRATION);
    UWB_UINT8_TO_STREAM(pGetCalibrationCmd, 0x00);
    UWB_UINT8_TO_STREAM(pGetCalibrationCmd, payloadLen);
    UWB_UINT8_TO_STREAM(pGetCalibrationCmd, channel);
    UWB_UINT8_TO_STREAM(pGetCalibrationCmd, (UINT8)paramId);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));

    if (status == UWBAPI_STATUS_OK) {
        UINT8 *calibrationPtr = &uwbContext.rsp_data[UCI_RESPONSE_STATUS_OFFSET];

        if (uwbContext.rsp_len > sizeof(calibResp->calibValueOut)) {
            NXPLOG_UWBAPI_E("Response data size is more than response buffer", __func__);
            status = UWBAPI_STATUS_BUFFER_OVERFLOW;
        }
        else {
            /*
       * Exclude Calibration state.
       */
            calibResp->length =
                (UINT16)(uwbContext.rsp_len - ((UINT16)UCI_RESPONSE_PAYLOAD_OFFSET + sizeof(calibResp->rfu)));
            ++calibrationPtr;
            UWB_STREAM_TO_UINT8(calibResp->rfu, calibrationPtr);
            phOsalUwb_MemCopy(calibResp->calibValueOut, calibrationPtr, (UINT8)calibResp->length);
            NXPLOG_UWBAPI_D("%s: Get Calibration successful", __func__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Calibration Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get Calibration value failed", __func__);
    }
    if (status != UWBAPI_STATUS_OK)
        calibResp->length = 0;
    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Set Uwb Debug Configuration Parameters
 *
 * \param sessionId    - [in] Initialized Session ID
 * \param pDebugParams - [in] Pointer to \ref phDebugParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDebugParams(UINT32 sessionId, const phDebugParams_t *pDebugParams)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    UINT8 offset          = 0;
    UINT8 noOfDebugParams = 0;

    NXPLOG_UWBAPI_D("%s: enter", __func__);
    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pDebugParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pDebugParams is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_THREAD_SECURE,
                                  (void *)&pDebugParams->secureThread,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_THREAD_SECURE_ISR,
                                  (void *)&pDebugParams->secureIsrThread,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_THREAD_NON_SECURE_ISR,
                                  (void *)&pDebugParams->nonSecureIsrThread,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(
        offset + getExtDebugConfigTLVBuffer(
                     UCI_EXT_PARAM_ID_THREAD_SHELL, (void *)&pDebugParams->shellThread, &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(
        offset + getExtDebugConfigTLVBuffer(
                     UCI_EXT_PARAM_ID_THREAD_PHY, (void *)&pDebugParams->phyThread, &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_THREAD_RANGING,
                                  (void *)&pDebugParams->rangingThread,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_DATA_LOGGER_NTF,
                                  (void *)&pDebugParams->dataLoggerNtf,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(
        offset + getExtDebugConfigTLVBuffer(
                     UCI_EXT_PARAM_ID_CIR_LOG_NTF, (void *)&pDebugParams->cirLogNtf, &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(
        offset + getExtDebugConfigTLVBuffer(
                     UCI_EXT_PARAM_ID_PSDU_LOG_NTF, (void *)&pDebugParams->psduLogNtf, &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    offset = (UINT8)(offset + getExtDebugConfigTLVBuffer(UCI_EXT_PARAM_ID_RFRAME_LOG_NTF,
                                  (void *)&pDebugParams->rframeLogNtf,
                                  &uwbContext.snd_data[offset]));
    ++noOfDebugParams; // Increment the number of debug params count

    NXPLOG_UWBAPI_D("%s: tlv created", __func__);
    sep_SetWaitEvent(UWA_DM_SESSION_SET_CONFIG_RSP_EVT);
    status = UWA_SetAppConfig(sessionId, noOfDebugParams, offset, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            if ((pDebugParams->cirLogNtf) || (pDebugParams->dataLoggerNtf)) {
                InputOutputData_t ioData;
                ioData.enableFwDump  = FALSE;
                ioData.enableCirDump = FALSE;
                if (pDebugParams->dataLoggerNtf) {
                    ioData.enableFwDump = (UINT8)TRUE;
                }
                if (pDebugParams->cirLogNtf) {
                    ioData.enableCirDump = (UINT8)TRUE;
                }
                tHAL_UWB_ENTRY *halFuncEntries = NULL;
                halFuncEntries                 = GetHalEntryFuncs();
                if (halFuncEntries != NULL) {
                    halFuncEntries->ioctl(HAL_UWB_IOCTL_DUMP_FW_LOG, &ioData);
                }
            }
        }
    }
    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * \brief Get Uwb Debug Configuration Parameters
 *
 * \param sessionId    - [in] Initialized Session ID
 * \param pDebugParams - [out] Pointer to \ref phDebugParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise \return
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDebugParams(UINT32 sessionId, phDebugParams_t *pDebugParams)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    UINT8 *pTlv           = NULL;
    UINT8 tlvLength;
    UINT16 index   = 0;
    UINT16 paramId = 0;
    UINT8 noOfParams;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pDebugParams == NULL) {
        NXPLOG_UWBAPI_E("%s: pDebugParams is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    phOsalUwb_SetMemory(&uwbContext.snd_data[0], 0x00, sizeof(uwbContext.snd_data));
    noOfParams = (UINT8)(sizeof(uciDebugParamIds) / sizeof(UINT16));
    tlvLength  = (UINT8)(sizeof(UINT16) * noOfParams);
    pTlv       = uwbContext.snd_data;
    for (index = 0; index < noOfParams; index++) {
        paramId = uciDebugParamIds[index];
        UWB_UINT8_TO_STREAM(pTlv, (UINT8)(paramId >> 8));
        UWB_UINT8_TO_STREAM(pTlv, (UINT8)(paramId));
        NXPLOG_UWBAPI_D("%s: Ext App ID: %02X", __func__, paramId);
    }

    sep_SetWaitEvent(UWA_DM_SESSION_GET_CONFIG_RSP_EVT);
    status = UWA_GetAppConfig(sessionId, noOfParams, tlvLength, uwbContext.snd_data);
    if (status == UWA_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Success UWA_GetAppConfig", __func__);
        phUwb_GKI_binary_sem_wait(uwbContext.devMgmtSem);

        status = uwbContext.wstatus;
        if (status == UWA_STATUS_OK) {
            /* rsp_data contains complete rsp, we have to skip Header + status + 1
       * byte which contains noOfParams */
            UINT8 *rspPtr = &uwbContext.rsp_data[0];
            parseDebugParams(rspPtr, noOfParams, pDebugParams);
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
 * \brief DoBind Perform Factory Binding using this API
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoBind(phSeDoBindStatus_t *doBindStatus)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    UINT8 *pDoBindCommand = NULL;
    UINT8 payloadLen      = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pDoBindCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pDoBindCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pDoBindCommand, EXT_UCI_MSG_SE_DO_BIND);
    UWB_UINT8_TO_STREAM(pDoBindCommand, 0x00);
    UWB_UINT8_TO_STREAM(pDoBindCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        sep_SetWaitEvent(EXT_UCI_MSG_SE_DO_BIND);
        if (phUwb_GKI_binary_sem_wait_timeout(uwbContext.devMgmtSem, UWBD_SE_TIMEOUT) == GKI_SUCCESS) {
            NXPLOG_UWBAPI_D("%s: Binding is successful", __func__);
            if (uwbContext.doBindStatus.status == STATUS_BINDING_SUCCESS) {
                phOsalUwb_MemCopy(doBindStatus, &uwbContext.doBindStatus, sizeof(uwbContext.doBindStatus));
            }
            else {
                NXPLOG_UWBAPI_E("%s: DoBind status is failed", __func__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            uwbContext.doBindStatus.status = 0xFF;
            NXPLOG_UWBAPI_E("%s: Binding is failed", __func__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Do Binding Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Binding is failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief Get the binding count using this API
 *
 * \param getBindingCount    - [out] getBindingCount data. valid only if API
 * status is success
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingCount(phSeGetBindingCount_t *getBindingCount)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    UINT8 *pBindingCountCommand = NULL;
    UINT8 payloadLen            = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingCount == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingCount is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pBindingCountCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pBindingCountCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pBindingCountCommand, EXT_UCI_MSG_SE_GET_BINDING_COUNT);
    UWB_UINT8_TO_STREAM(pBindingCountCommand, 0x00);
    UWB_UINT8_TO_STREAM(pBindingCountCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Binding Count is successful", __func__);
        /* rsp_data contains complete rsp, we have to skip Header */
        UINT8 *rspPtr = &uwbContext.rsp_data[UCI_RESPONSE_STATUS_OFFSET];
        UWB_STREAM_TO_UINT8(status, rspPtr);
        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(getBindingCount->bindingStatus, rspPtr);
            UWB_STREAM_TO_UINT8(getBindingCount->uwbdBindingCount, rspPtr);
            UWB_STREAM_TO_UINT8(getBindingCount->seBindingCount, rspPtr);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get Binding Count Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Binding Count is failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief TestConnectivity Perform SE connectivity test using this API
 *
 * \param wtxCount                       - [out] WTX count
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_TestConnectivity(UINT8 *wtxCount)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    UINT8 *pConnectivityCommand = NULL;
    UINT8 payloadLen            = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (wtxCount == NULL) {
        NXPLOG_UWBAPI_E("%s: wtxCount is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pConnectivityCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pConnectivityCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pConnectivityCommand, EXT_UCI_MSG_SE_DO_TEST_CONNECTIVITY);
    UWB_UINT8_TO_STREAM(pConnectivityCommand, 0x00);
    UWB_UINT8_TO_STREAM(pConnectivityCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(EXT_UCI_MSG_SE_DO_TEST_CONNECTIVITY, UWB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            if (uwbContext.testConnectivityData.status == UWBAPI_STATUS_OK) {
                *wtxCount = uwbContext.testConnectivityData.wtx;
                NXPLOG_UWBAPI_D("%s: Connectivity test is successful", __func__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Connectivity test is failed", __func__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            if (uwbContext.testConnectivityData.status == UWBAPI_STATUS_OK) {
                *wtxCount = uwbContext.testConnectivityData.wtx;
                status    = UWBAPI_STATUS_OK;
                NXPLOG_UWBAPI_E("%s: Connectivity Status OK", __func__);
            }
            else {
                uwbContext.testConnectivityData.status = 0xFF;
                status                                 = UWBAPI_STATUS_FAILED;
                NXPLOG_UWBAPI_E("%s: Connectivity test is failed for timeout", __func__);
            }
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Connectivity test Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Connectivity test response is failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief TestLoop Perform SE loop test using this API
 *
 * \param loopCnt       - [In] No of times test to be run
 * \param timeInterval  - [In] time interval in ms
 * \param testLoopData  - [out] Test loop notification data
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SeTestLoop(UINT16 loopCnt, UINT16 timeInterval, phTestLoopData_t *testLoopData)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 *pTestLoopCommand = NULL;
    UINT8 payloadLen        = sizeof(loopCnt) + sizeof(timeInterval);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (testLoopData == NULL) {
        NXPLOG_UWBAPI_E("%s: testLoopData is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pTestLoopCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pTestLoopCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pTestLoopCommand, EXT_UCI_MSG_SE_DO_TEST_LOOP);
    UWB_UINT8_TO_STREAM(pTestLoopCommand, 0x00);
    UWB_UINT8_TO_STREAM(pTestLoopCommand, payloadLen);
    UWB_UINT16_TO_STREAM(pTestLoopCommand, loopCnt);
    UWB_UINT16_TO_STREAM(pTestLoopCommand, timeInterval);

    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(
            EXT_UCI_MSG_SE_DO_TEST_LOOP, (UINT32)((UINT32)(loopCnt * timeInterval) + UWBD_SE_TIMEOUT));
        /*
     * Increasing the delay as it is not sufficient to get the notification.
     */
        if (status == UWBAPI_STATUS_OK) {
            if (uwbContext.testLoopData.status == UWBAPI_STATUS_OK) {
                phOsalUwb_MemCopy(testLoopData, &uwbContext.testLoopData, sizeof(uwbContext.testLoopData));
                NXPLOG_UWBAPI_D("%s: Loop test is successful", __func__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Loop test is failed", __func__);
                status = UWBAPI_STATUS_FAILED;
            }
        }
        else {
            uwbContext.testLoopData.status = 0xFF;
            NXPLOG_UWBAPI_E("%s: Loop test is failed", __func__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Loop test Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Loop test is failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief API to get current binding status
 * Use of this API will lock binding status if UWBD is in unlock state
 * \param  getBindingStatus  - [In] Binding status notification
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingStatus(phSeGetBindingStatus_t *getBindingStatus)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 *pBindingStatusCommand = NULL;
    UINT8 payloadLen             = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (getBindingStatus == NULL) {
        NXPLOG_UWBAPI_E("%s: getBindingStatus is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pBindingStatusCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pBindingStatusCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pBindingStatusCommand, EXT_UCI_MSG_SE_GET_BINDING_STATUS);
    UWB_UINT8_TO_STREAM(pBindingStatusCommand, 0x00);
    UWB_UINT8_TO_STREAM(pBindingStatusCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(EXT_UCI_MSG_SE_GET_BINDING_STATUS, UWBD_SE_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            phOsalUwb_MemCopy(getBindingStatus, &uwbContext.getBindingStatus, sizeof(uwbContext.getBindingStatus));
            NXPLOG_UWBAPI_D("%s: Get binding status cmd passed", __func__);
        }
        else {
            uwbContext.getBindingStatus.status = 0xFF;
            NXPLOG_UWBAPI_E("%s: Get binding status cmd failed", __func__);
            status = UWBAPI_STATUS_FAILED;
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Get binding status Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Get binding status cmd failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief API to get all uwb sessions.
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAllUwbSessions(phUwbSessionsContext_t *pUwbSessionsContext)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 payloadLen           = 0;
    UINT8 *pGetSessionsCommand = NULL;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pUwbSessionsContext == NULL) {
        NXPLOG_UWBAPI_E("%s: UwbSessionsContext is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pGetSessionsCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pGetSessionsCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pGetSessionsCommand, EXT_UCI_MSG_GET_ALL_UWB_SESSIONS);
    UWB_UINT8_TO_STREAM(pGetSessionsCommand, 0x00);
    UWB_UINT8_TO_STREAM(pGetSessionsCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: GetAllUWBSessions successful", __func__);
        /* rsp_data contains complete rsp, we have to skip Header */
        UINT8 *rspPtr = &uwbContext.rsp_data[UCI_RESPONSE_STATUS_OFFSET];
        UWB_STREAM_TO_UINT8(pUwbSessionsContext->status, rspPtr);

        if (pUwbSessionsContext->status == UWBAPI_STATUS_OK) {
            /*
             * Parse all the response parameters are correct or not.
             */
            parseUwbSessionParams(rspPtr, pUwbSessionsContext);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions Command Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: GetAllUWBSessions failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * \brief API to get the current temperature
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryTemperature(UINT8 *pTemperatureValue)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 *pQueryTemperatureCommand = NULL;
    UINT8 payloadLen                = 0;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pTemperatureValue == NULL) {
        NXPLOG_UWBAPI_E("%s: pTemperatureValue is NULL", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pQueryTemperatureCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pQueryTemperatureCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pQueryTemperatureCommand, EXT_UCI_MSG_QUERY_TEMPERATURE);
    UWB_UINT8_TO_STREAM(pQueryTemperatureCommand, 0x00);
    UWB_UINT8_TO_STREAM(pQueryTemperatureCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Query temperature cmd successful", __func__);
        /* rsp_data contains complete rsp, we have to skip Header */
        UINT8 *rspPtr = &uwbContext.rsp_data[UCI_RESPONSE_STATUS_OFFSET];
        UWB_STREAM_TO_UINT8(status, rspPtr);

        if (status == UWBAPI_STATUS_OK) {
            UWB_STREAM_TO_UINT8(*pTemperatureValue, rspPtr);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Query temperature cmd Timed Out", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Query temperature cmd failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}
