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
#include "PrintUtility.h"
#include "AppConfigParams.h"
#include "FreeRTOS.h"
#include "task.h"
#include <uwa_api.h>
#include "phNxpLogApis_UwbApi.h"
#include "phOsalUwb.h"
#include "phNxpUwbConfig.h"
#include "phOsalUwb_Timer.h"
#include "UwbAdaptation.h"
#include "uwb_hal_int.h"
#include "UwbCoreSDK_Internal.h"
#include "UwbApi_Proprietary_Internal.h"
#include "uci_ext_defs.h"
#include "uci_test_defs.h"

#include "phNxpLogApis_UwbApi.h"
#include "UwbApi_Internal.h"
#include "uwbiot_ver.h"

/**
 * brief returns UCI, FW and MW version
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
    tUWA_STATUS status;
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

    phOsalUwb_MemCopy(pdevInfo->devName, uwbContext.devInfo.devName, sizeof(pdevInfo->devName));

    pdevInfo->fwMajor        = uwbContext.devInfo.fwMajor;
    pdevInfo->fwMinor        = uwbContext.devInfo.fwMinor;
    pdevInfo->fwPatchVersion = uwbContext.devInfo.fwPatchVersion;

    pdevInfo->devMajor = uwbContext.devInfo.devMajor;
    pdevInfo->devMinor = uwbContext.devInfo.devMinor;

    phOsalUwb_MemCopy(pdevInfo->serialNo, uwbContext.devInfo.serialNo, sizeof(pdevInfo->serialNo));

    pdevInfo->dspMajor        = uwbContext.devInfo.dspMajor;
    pdevInfo->dspMinor        = uwbContext.devInfo.dspMinor;
    pdevInfo->dspPatchVersion = uwbContext.devInfo.dspPatchVersion;

    pdevInfo->bbMajor = uwbContext.devInfo.bbMajor;
    pdevInfo->bbMinor = uwbContext.devInfo.bbMinor;

    phOsalUwb_MemCopy(pdevInfo->cccVersion, uwbContext.devInfo.cccVersion, sizeof(pdevInfo->cccVersion));

    pdevInfo->mwMajor = UWBIOTVER_STR_VER_MAJOR;
    pdevInfo->mwMinor = UWBIOTVER_STR_VER_MINOR;
    status            = UWA_STATUS_OK;

    NXPLOG_UWBAPI_D("%s: exit status %d", __func__, status);
    return status;
}

/**
 * brief API to put the device in HPD mode
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SuspendDevice()
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);
    UINT8 payloadLen = 0;
    UINT8 *pCommand  = NULL;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_DEVICE_SUSPEND_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);
    UWB_UINT8_TO_STREAM(pCommand, payloadLen);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE);
    if (status == UWBAPI_STATUS_OK) {
        phOsalUwb_Delay(200);
        NXPLOG_UWBAPI_D("%s: Suspend Command successful", __func__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Suspend Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_W("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Suspend Command failed", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_PropSR040_SetDPDTimeout(uint16_t dpd_timeout)
{
    uint8_t dpdCommand[] = {0x20,           //GID of Core_Set_Config_cmd
        0x04,                               //OID of Core_Set_Config_cmd
        0x00,                               //RFU
        0x05,                               //PayloadLength
        0x01,                               //NoOfParameters
        UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT, //HPD_TIMEOUT_ID
        0x02,                               //Length Of HPD_TIME_OUT_VALUE
        (uint8_t)(dpd_timeout),
        (uint8_t)(dpd_timeout >> 8)};
    uint8_t dpdRsp[10]   = {0};
    uint16_t length      = sizeof(dpdRsp);

    tUWBAPI_STATUS status = UwbApi_SendRawCommand(dpdCommand, sizeof(dpdCommand), dpdRsp, &length);
    if (status == UWBAPI_STATUS_OK) {
        status = dpdRsp[4];
    }
    return status;
}

EXTERNC tUWBAPI_STATUS UwbApi_PropSR040_SetHPDTimeout(uint16_t hpd_timeout)
{
    uint8_t hpdCommand[] = {0x20,           //GID of Core_Set_Config_cmd
        0x04,                               //OID of Core_Set_Config_cmd
        0x00,                               //RFU
        0x05,                               //PayloadLength
        0x01,                               //NoOfParameters
        UCI_EXT_PARAM_ID_HPD_ENTRY_TIMEOUT, //HPD_TIMEOUT_ID
        0x02,                               //Length Of HPD_TIME_OUT_VALUE
        (uint8_t)(hpd_timeout),
        (uint8_t)(hpd_timeout >> 8)};
    uint8_t hpdRsp[10]   = {0};
    uint16_t length      = sizeof(hpdRsp);

    tUWBAPI_STATUS status = UwbApi_SendRawCommand(hpdCommand, sizeof(hpdCommand), hpdRsp, &length);
    if (status == UWBAPI_STATUS_OK) {
        status = hpdRsp[4];
    }
    return status;
}
/**
 * brief API to mange the session related info in NVM
 *
 * \param sesNvmManageTag - [in] Session NVM manage tag field
 * \param sessionid       - [in] Session NVM manage session id field.
 * [Not present for the tag SESSION_NVM_MANAGE_DELETE_ALL. But pass sessionid = 0 [Any value] which is ignored ]
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if tag option is incorrect
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionNvmManage(esessionNvmManage sesNvmManageTag, UINT32 sessionid)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 payloadLen = 1;
    UINT8 *pCommand  = NULL;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_SESSION_NVM_MANAGE_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);

    switch (sesNvmManageTag) {
    case SESSION_NVM_MANAGE_PERSIST:
#if 0
    case SESSION_NVM_MANAGE_DELETE: /* Not available */
#endif
        payloadLen += sizeof(sessionid);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, sesNvmManageTag);
        UWB_UINT32_TO_STREAM(pCommand, sessionid);
        break;
    case SESSION_NVM_MANAGE_DELETE_ALL:
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, sesNvmManageTag);
        break;
    default:
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE + payloadLen);
    if (status == UWBAPI_STATUS_OK) {
        NXPLOG_UWBAPI_D("%s: Session NVM Manage Command successful", __func__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Session NVM Manage Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_W("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Session NVM Manage Command failed", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}
/**
 * brief API to start the device in Continuous Wave Mode
 *
 * \param testMode - [In] Test Mode to be started
 * \param testModeParams - [In] param used for the corresponding test mode
 * \param testModeOutputData - [Out] param used for test mode output values
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if any of the input parameter is NULL or invalid value
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartTestMode(eTestMode testMode, const phTestModeParams *testModeParams)
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT16 commandLength = 0;
    UINT8 *pCommand      = NULL;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (testMode == RECEIVE_MODE || testMode == TRANSMIT_MODE) {
        if (testModeParams == NULL) {
            NXPLOG_UWBAPI_E("%s: testModeParams is NULL", __func__);
            return UWBAPI_STATUS_INVALID_PARAM;
        }

        if (testMode == TRANSMIT_MODE) {
            if (testModeParams->transmitModeParams.psduData == NULL ||
                testModeParams->transmitModeParams.psduLen == 0) {
                NXPLOG_UWBAPI_E("%s: psduData is NULL or psduLen is Zero", __func__);
                return UWBAPI_STATUS_INVALID_PARAM;
            }
            if (testModeParams->transmitModeParams.enableLogging == true) {
                // Perform Set App Config to Enable logging while in Transmit Mode
                UwbApi_SetAppConfig(SESSION_ID_RFTEST, TX_PHY_LOGGING_ENABLE, 1);
            }
        }
        else {
            if (testModeParams->receiveModeParams.enableLogging == true) {
                // Perform Set App Config to Enable logging while in Receive Mode
                UwbApi_SetAppConfig(SESSION_ID_RFTEST, RX_PHY_LOGGING_ENABLE, 1);
            }
        }
    }

    pCommand = uwbContext.snd_data;
    if (formTestModeCommand(testMode, testModeParams, pCommand, &commandLength) == false) {
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    /* Start Test Mode */
    status = sendRawUci(uwbContext.snd_data, commandLength);
    if (status == UWBAPI_STATUS_OK) {
        status = UWBAPI_STATUS_OK;
        NXPLOG_UWBAPI_D("%s: Test Mode Cmd successful", __func__);
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_D("%s: Test Mode Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_D("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_D("%s: Test Mode Command failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * brief API to stop Test Mode
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopTestMode()
{
    tUWBAPI_STATUS status;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    UINT8 payloadLen = 0;
    UINT8 *pCommand  = NULL;

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    pCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_TEST_STOP_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);
    UWB_UINT8_TO_STREAM(pCommand, payloadLen);

    /* Start Test Mode */
    status = sendRawUci(uwbContext.snd_data, (UINT16)(payloadLen + UCI_MSG_HDR_SIZE));
    if (status == UWBAPI_STATUS_OK) {
        if (waitforNotification(EXT_UCI_MSG_TEST_STOP_CMD, UWBD_TEST_MODE_NTF_TIMEOUT) == UWBAPI_STATUS_OK) {
            NXPLOG_UWBAPI_D("%s: Test Mode Cmd successful", __func__);
        }
        else {
            NXPLOG_UWBAPI_D("%s: Test Mode Ntf Wait Failed", __func__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Test Mode Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_E("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Test Mode Command failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * brief Set calibration parameters.
 *
 * \param paramId     - [In] Calibration parameter ID
 * \param pCalibData  -  [In] Calibration Data
 * \param calibResp   -  [Out] Calibration Status
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(
    eCalibParam paramId, phCalibrationData *pCalibData, phCalibrationStatus_t *calibResp)
{
    tUWBAPI_STATUS status;
    UINT8 *pSetCalibrationCmd = NULL;
    UINT16 commandLength      = 0;
    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }
    if (pCalibData == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }
    pSetCalibrationCmd = uwbContext.snd_data;
    switch (paramId) {
    case TX_POWER_DIFF:
        formCalibrationCommand(
            TX_POWER_DIFF, (phTxPowerDiffCalibData_t *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case FREQ_DIFF:
        formCalibrationCommand(FREQ_DIFF, (phFreqDiffCalibData_t *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case ANTENNA_DELAY:
        formCalibrationCommand(
            ANTENNA_DELAY, (phAntennaDelayCalibData_t *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case CURRENT_LIMIT_VALUE:
        formCalibrationCommand(CURRENT_LIMIT_VALUE, (UINT8 *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case TX_ADAPTIVE_POWER_CALC:
        formCalibrationCommand(
            TX_ADAPTIVE_POWER_CALC, (phTxAdaptivePowerCalibData_t *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case DDFS_TONE_VALUES:
        formCalibrationCommand(
            DDFS_TONE_VALUES, (phDdfsToneConfigData_t *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case TEMP_COMPENS_FLAG:
        formCalibrationCommand(TEMP_COMPENS_FLAG, (UINT8 *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    case DPD_TIMER_PENALTY_US:
        formCalibrationCommand(DPD_TIMER_PENALTY_US, (UINT32 *)pCalibData, pSetCalibrationCmd, &commandLength);
        break;
    default:
        NXPLOG_UWBAPI_E("%s:    Invalid Param ", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    status = sendRawUci(uwbContext.snd_data, (UINT16)commandLength);
    if (status == UWBAPI_STATUS_OK) {
        status = waitforNotification(EXT_UCI_MSG_SET_TRIM_VALUES_CMD, UWBD_CALIB_NTF_TIMEOUT);
        if (status == UWBAPI_STATUS_OK) {
            calibResp->status = uwbContext.calibrationStatus.status;
            if (uwbContext.calibrationStatus.status == APPLIED) {
                NXPLOG_UWBAPI_D("%s: Set Calibration notification successful", __func__);
            }
            else {
                NXPLOG_UWBAPI_E("%s: Set Calibration notification failed", __func__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Set Calibration notification time out", __func__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Set Calibration Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_E("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Set Calibration failed", __func__);
        status = UWBAPI_STATUS_FAILED;
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * brief Radio Config Get CRC api.
 *
 * \param radioConfigIndex - [In] Radio Configuration index value. 0x00 - 0x0F for Rx and 0x10 - 0x1F for Tx
 * \param pCrc - [Out] Radio Config CRC
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW   - if response buffer is not sufficient to hold the response
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_RadioConfigGetCrc(UINT8 radioConfigIndex, UINT16 *pCrc)
{
    tUWBAPI_STATUS status;
    UINT8 payloadLen              = 2; // One byte download sequence followed by Rx or Tx Radio config index
    UINT8 *pCommand               = NULL;
    edownloadSequence downloadSeq = GET_CRC;
    UINT8 *pResponse              = NULL;
    UINT16 reponseLen             = 0;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pCrc == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_RADIO_CONFIG_DOWNLOAD_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);
    UWB_UINT8_TO_STREAM(pCommand, payloadLen);
    UWB_UINT8_TO_STREAM(pCommand, downloadSeq);
    UWB_UINT8_TO_STREAM(pCommand, radioConfigIndex);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE + payloadLen);
    if (status == UWBAPI_STATUS_OK) {
        /* Need to exclude status in the response length */
        /* Shall receive status followed by CRC. Exclude the header part */
        pResponse  = &uwbContext.rsp_data[UCI_MSG_HDR_SIZE];
        reponseLen = (UCI_MSG_HDR_SIZE + sizeof(status) + sizeof(UINT16));
        UWB_STREAM_TO_UINT8(status, pResponse);
        if (status == UWBAPI_STATUS_OK) {
            if (uwbContext.rsp_len > reponseLen) {
                NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __func__);
                status = UWBAPI_STATUS_BUFFER_OVERFLOW;
            }
            else {
                /* Fetch the CRC bytes */
                UWB_STREAM_TO_UINT16(*pCrc, pResponse);
            }
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Radio Config Get CRC Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_W("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Radio Config Get CRC Command failed", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}

/**
 * brief Radio Config Get Version api.
 *
 * \param radioConfigIndex - [In] Radio Configuration index value. 0x00 - 0x0F for Rx and 0x10 - 0x1F for Tx
 * \param pRadioConfigGetVersionResp - [Out] Radio Config Get Version Response
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW   - if response buffer is not sufficient to hold the response
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_RadioConfigGetVersion(
    UINT8 radioConfigIndex, phRadioConfigGetVersionResp_t *pRadioConfigGetVersionResp)
{
    tUWBAPI_STATUS status;
    UINT8 payloadLen              = 2; // One byte download sequence followed by Rx or Tx Radio config index
    UINT8 *pCommand               = NULL;
    edownloadSequence downloadSeq = GET_VERSION;
    UINT8 *pResponse              = NULL;

    NXPLOG_UWBAPI_D("%s: Enter", __func__);

    if (uwbContext.isUfaEnabled == FALSE) {
        NXPLOG_UWBAPI_E("%s: UWB device is not initialized", __func__);
        return UWBAPI_STATUS_NOT_INITIALIZED;
    }

    if (pRadioConfigGetVersionResp == NULL) {
        NXPLOG_UWBAPI_E("%s: data is invalid", __func__);
        return UWBAPI_STATUS_INVALID_PARAM;
    }

    pCommand = uwbContext.snd_data;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_RADIO_CONFIG_DOWNLOAD_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);
    UWB_UINT8_TO_STREAM(pCommand, payloadLen);
    UWB_UINT8_TO_STREAM(pCommand, downloadSeq);
    UWB_UINT8_TO_STREAM(pCommand, radioConfigIndex);

    status = sendRawUci(uwbContext.snd_data, UCI_MSG_HDR_SIZE + payloadLen);
    if (status == UWBAPI_STATUS_OK) {
        /* Need to exclude status in the response length */
        if (uwbContext.rsp_len > UWB_MAX_RADIO_CONFIG_VERSION_LEN) {
            NXPLOG_UWBAPI_E("%s: Response data size is more than response buffer", __func__);
            status = UWBAPI_STATUS_BUFFER_OVERFLOW;
        }
        else {
            /* Skip the first 2 bytes Component id */
            pResponse = (uwbContext.rsp_data + 2);
            /* [TBD] : To be tested once the FW is available */
            UWB_STREAM_TO_UINT16(pRadioConfigGetVersionResp->patch, pResponse);
            UWB_STREAM_TO_UINT16(pRadioConfigGetVersionResp->minor, pResponse);
            UWB_STREAM_TO_UINT16(pRadioConfigGetVersionResp->major, pResponse);
            NXPLOG_UWBAPI_D("%s: Radio Config Get Version Command successful", __func__);
        }
    }
    else if (status == UWBAPI_STATUS_TIMEOUT) {
        NXPLOG_UWBAPI_E("%s: Radio Config Get Version Command Timed Out", __func__);
    }
    else if (status == UWBAPI_STATUS_HPD_WAKEUP) {
        NXPLOG_UWBAPI_W("%s: Device Woken up from HPD", __func__);
    }
    else {
        NXPLOG_UWBAPI_E("%s: Radio Config Get Version Command failed", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit", __func__);
    return status;
}
