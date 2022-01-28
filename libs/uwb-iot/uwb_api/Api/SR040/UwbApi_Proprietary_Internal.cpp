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

#include "uwa_api.h"
#include "phNxpLogApis_UwbApi.h"
#include "phNxpUwbConfig.h"
#include "phOsalUwb.h"
#include "uci_ext_defs.h"

#include "phNxpLogApis_UwbApi.h"
#include "UwbApi_Internal.h"
#include "AppConfigParams.h"
#include "UwbApi_Proprietary_Internal.h"
#include "UwbApi_Types.h"
#include "UwbApi.h"

/*******************************************************************************
 **
 ** Function:        getExtTLVBuffer
 **
 ** Description:     Convert one TLV Application Configuration Structure to Byte
 *Array
 **
 ** Returns:         Length of the data
 **
 *******************************************************************************/
UINT8 getExtTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer)
{
    UINT8 length        = 0;
    tlvBuffer[length++] = paramId;

    switch (paramId) {
    /* Length 2 Byte */
    case UCI_EXT_PARAM_ID_DPD_ENTRY_TIMEOUT:
    case UCI_EXT_PARAM_ID_HPD_ENTRY_TIMEOUT: {
    case UCI_EXT_PARAM_ID_RX_RADIO_CFG_IDXS:
    case UCI_EXT_PARAM_ID_TX_MAX_BLOCK_NUM:
    case UCI_EXT_PARAM_ID_TX_RADIO_CFG_IDXS:
        tlvBuffer[length++] = 2; // Param len
        UINT16 value        = *((UINT16 *)paramValue);
        tlvBuffer[length++] = (UINT8)(value);
        tlvBuffer[length++] = (UINT8)(value >> 8);
    } break;
    /* Length 1 Byte */
    case UCI_EXT_PARAM_ID_STS_INDEX_RESTART:
    case UCI_EXT_PARAM_ID_TX_POWER_ID:
    case UCI_EXT_PARAM_ID_RX_PHY_LOGGING_ENBL:
    case UCI_EXT_PARAM_ID_TX_PHY_LOGGING_ENBL:
    case UCI_EXT_PARAM_ID_NBIC_CONF_ID:
        tlvBuffer[length++] = 1; // Param len
        tlvBuffer[length++] = *((UINT8 *)paramValue);
        break;
    default:
        NXPLOG_UWBAPI_D("%s: Unknown ID\n", __func__);
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
 **                  manfacturerData:  Manufacturer Data
 **                  manufacturerLength:  Length of Manufacturer Data
 **
 ** Returns:         None
 **
 *******************************************************************************/
bool parseDeviceInfo(UINT8 *manfacturerData, UINT8 manufacturerLength)
{
    UINT8 length = 0;
    UINT8 index  = 0;

    if ((manufacturerLength <= 0) || (manfacturerData == NULL)) {
        NXPLOG_UWBAPI_E("%s: manufacturerLength is Zero or manfacturerData is NULL", __func__);
        return false;
    }

    while (index < manufacturerLength) {
        UINT8 param_id = manfacturerData[index++];
        length         = manfacturerData[index++];
        switch (param_id) {
        case UCI_EXT_PARAM_ID_DEVICE_NAME:
            phOsalUwb_MemCopy(uwbContext.devInfo.devName, &manfacturerData[index], length);
            index = index + length;
            break;
        case UCI_EXT_PARAM_ID_FIRMWARE_VERSION:
            uwbContext.devInfo.fwMajor        = manfacturerData[index++];
            uwbContext.devInfo.fwMinor        = manfacturerData[index++];
            uwbContext.devInfo.fwPatchVersion = manfacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_DEVICE_VERSION:
            uwbContext.devInfo.devMajor = manfacturerData[index++];
            uwbContext.devInfo.devMinor = manfacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_SERIAL_NUMBER:
            phOsalUwb_MemCopy(uwbContext.devInfo.serialNo, &manfacturerData[index], length);
            index = index + length;
            break;
        case UCI_EXT_PARAM_ID_DSP_VERSION:
            uwbContext.devInfo.dspMajor        = manfacturerData[index++];
            uwbContext.devInfo.dspMinor        = manfacturerData[index++];
            uwbContext.devInfo.dspPatchVersion = manfacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_RANGER4_VERSION:
            uwbContext.devInfo.bbMajor = manfacturerData[index++];
            uwbContext.devInfo.bbMinor = manfacturerData[index++];
            break;
        case UCI_EXT_PARAM_ID_CCC_VERSION:
            phOsalUwb_MemCopy(uwbContext.devInfo.cccVersion, &manfacturerData[index], length);
            index = index + length;
            break;
        }
    }
    return true;
}

/*******************************************************************************
 **
 ** Function         handle_calib_status_ntf
 **
 ** Description      This function is called to notify calibration status
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_calib_status_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.calibrationStatus, 0x00, sizeof(phCalibrationStatus_t));
    uwbContext.calibrationStatus.status = FAILURE;
    UINT8 status                        = 0xFF;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(status, p);
        uwbContext.calibrationStatus.status = (eCalibStatus)status;
    }
}

/*******************************************************************************
 **
 ** Function         handle_test_loopback_status_ntf
 **
 ** Description      This function is called to notify calibration status
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_test_loopback_status_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.testLoopbackStatus, 0x00, sizeof(phTestLoopbackData_t));
    uwbContext.testLoopbackStatus.status = FAILURE;
    if (len != 0) {
        UWB_STREAM_TO_UINT8(uwbContext.testLoopbackStatus.status, p);
        UWB_STREAM_TO_UINT32(uwbContext.testLoopbackStatus.groupDelay, p);
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWBD_TEST_MODE_LOOP_BACK_NTF, &uwbContext.testLoopbackStatus.groupDelay);
    }
}

/*******************************************************************************
 **
 ** Function         handle_msg_log_ntf
 **
 ** Description      This function is called to notify msg log
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_msg_log_ntf(UINT8 *p, UINT16 len)
{
    if (len != 0) {
        memset(&uwbContext.testPhyLogNtfnData, 0x00, sizeof(phPhyLogNtfnData_t));
        uwbContext.testPhyLogNtfnData.size = len;
        phOsalUwb_MemCopy(&uwbContext.testPhyLogNtfnData.data[0], p, len);
    }
    if (uwbContext.pAppCallback) {
        uwbContext.pAppCallback(UWB_TEST_PHY_LOG_NTF, &uwbContext.testPhyLogNtfnData);
    }
}

/*******************************************************************************
 **
 ** Function         handle_test_initiator_range_status_ntf
 **
 ** Description      This function is called to notify calibration status
 **
 ** Returns          void
 **
 *******************************************************************************/
static void handle_test_initiator_range_status_ntf(UINT8 *p, UINT16 len)
{
    memset(&uwbContext.testInitiatorRangeStatus, 0x00, sizeof(phTestInitiatorRangekData_t));
    uwbContext.testLoopbackStatus.status = FAILURE;
    if (len != 0) {
        UWB_STREAM_TO_UINT32(uwbContext.testInitiatorRangeStatus.sessionId, p);
        UWB_STREAM_TO_UINT16(uwbContext.testInitiatorRangeStatus.blockIndex, p);
        UWB_STREAM_TO_UINT32(uwbContext.testInitiatorRangeStatus.stsIndex, p);
        UWB_STREAM_TO_UINT16(uwbContext.testInitiatorRangeStatus.rrIndex, p);
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

    if ((paramLength >= UCI_RESPONSE_STATUS_OFFSET) && (pResponseBuffer != NULL)) {
        NXPLOG_UWBAPI_D("extDeviceManagementCallback: Received length data = 0x%x ", paramLength);

        responsePayloadLen         = (UINT16)(paramLength - UCI_RESPONSE_STATUS_OFFSET);
        uwbContext.receivedEventId = (UINT16)event;
        switch (event) {
        case EXT_UCI_MSG_SET_TRIM_VALUES_CMD: {
            responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];
            handle_calib_status_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_TEST_LOOPBACK_NTF: {
            NXPLOG_UWBAPI_D("%s: Test loopback Notification Received", __func__);
            responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];
            handle_test_loopback_status_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_LOG_NTF: {
            NXPLOG_UWBAPI_D("%s: Log Notification Received", __func__);
            responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];
            handle_msg_log_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_TEST_INITIATOR_RANGE_DATA_NTF: {
            NXPLOG_UWBAPI_D("%s: Test Initiator Range Notification Received", __func__);
            responsePayloadPtr = &pResponseBuffer[UCI_RESPONSE_STATUS_OFFSET];
            handle_test_initiator_range_status_ntf(responsePayloadPtr, responsePayloadLen);
        } break;
        case EXT_UCI_MSG_TEST_STOP_CMD: {
            NXPLOG_UWBAPI_D("%s: Test Stop Notification Received", __func__);
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

void formCalibrationCommand(UINT8 paramId, void *pCalibData, UINT8 *pSetCalibrationCmd, UINT16 *pCommandLength)
{
    phTxPowerDiffCalibData_t *pTxCalibData         = NULL;
    phFreqDiffCalibData_t *pFreqDiffData           = NULL;
    phAntennaDelayCalibData_t *pAntennaDelay       = NULL;
    UINT8 *pCurrLimitValue                         = NULL;
    phTxAdaptivePowerCalibData_t *pTxAdaptivePower = NULL;
    phDdfsToneConfigData_t *pDdfsToneConfigData    = NULL;
    UINT8 *pTempCompensFlag                        = NULL;
    UINT32 *pdpdTimerPenalty                       = NULL;
    UINT16 payloadLen                              = 3; // no of params + param ID + Length
    UCI_MSG_BLD_HDR0(pSetCalibrationCmd, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pSetCalibrationCmd, EXT_UCI_MSG_SET_TRIM_VALUES_CMD);
    UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 0x00);
    switch (paramId) {
    case TX_POWER_DIFF:
        pTxCalibData = (phTxPowerDiffCalibData_t *)pCalibData;
        payloadLen += TX_POWER_DIFF_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TX_POWER_DIFF);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TX_POWER_DIFF_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxCalibData->channelNo);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxCalibData->signVal);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxCalibData->absoluteVal);
        break;
    case FREQ_DIFF:
        pFreqDiffData = (phFreqDiffCalibData_t *)pCalibData;
        payloadLen += FREQ_DIFF_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, FREQ_DIFF);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, FREQ_DIFF_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pFreqDiffData->channelNo);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pFreqDiffData->signVal);
        UWB_UINT32_TO_STREAM(pSetCalibrationCmd, pFreqDiffData->absoluteFreqOffset);
        break;
    case ANTENNA_DELAY:
        pAntennaDelay = (phAntennaDelayCalibData_t *)pCalibData;
        payloadLen += ANTENNA_DELAY_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, ANTENNA_DELAY);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, ANTENNA_DELAY_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pAntennaDelay->channelNo);
        UWB_UINT16_TO_STREAM(pSetCalibrationCmd, pAntennaDelay->antennaDelay);
        break;
    case CURRENT_LIMIT_VALUE:
        pCurrLimitValue = (UINT8 *)pCalibData;
        payloadLen += CURRENT_LIMITER_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, CURRENT_LIMIT_VALUE);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, CURRENT_LIMITER_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, (*pCurrLimitValue));
        break;
    case TX_ADAPTIVE_POWER_CALC:
        pTxAdaptivePower = (phTxAdaptivePowerCalibData_t *)pCalibData;
        payloadLen += TX_ADAPTIVE_POWER_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TX_ADAPTIVE_POWER_CALC);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TX_ADAPTIVE_POWER_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxAdaptivePower->channelNo);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxAdaptivePower->powerIdRms);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pTxAdaptivePower->peakDelta);
        break;
    case DDFS_TONE_VALUES:
        pDdfsToneConfigData = (phDdfsToneConfigData_t *)pCalibData;
        payloadLen += DDFS_TONE_VALUES_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, DDFS_TONE_VALUES);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, DDFS_TONE_VALUES_LEN);
        for (UINT32 LoopCnt = 0; LoopCnt < 4; ++LoopCnt) {
            UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].channelNo);
            UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].rfu);
            UWB_UINT32_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].txDdfsTone0RegVal);
            UWB_UINT32_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].txDdfsTone1RegVal);
            UWB_UINT32_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].spurDuration);
            UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].gainvalsetRegVal);
            UWB_UINT8_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].gainByPassEnblRegVal);
            UWB_UINT16_TO_STREAM(pSetCalibrationCmd, pDdfsToneConfigData[LoopCnt].spurPeriodicity);
        }
        break;
    case TEMP_COMPENS_FLAG:
        pTempCompensFlag = (UINT8 *)pCalibData;
        payloadLen += TEMP_COMPENS_FLAG_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TEMP_COMPENS_FLAG);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, TEMP_COMPENS_FLAG_LEN);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, (*pTempCompensFlag));
        break;
    case DPD_TIMER_PENALTY_US:
        pdpdTimerPenalty = (UINT32 *)pCalibData;
        payloadLen += DPD_TIMER_PENALTY_US_LEN;
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, payloadLen);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, 1);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, DPD_TIMER_PENALTY_US);
        UWB_UINT8_TO_STREAM(pSetCalibrationCmd, DPD_TIMER_PENALTY_US_LEN);
        UWB_UINT32_TO_STREAM(pSetCalibrationCmd, (*pdpdTimerPenalty));
        break;
    default:
        break;
    }
    *pCommandLength = payloadLen + UCI_MSG_HDR_SIZE;
}

bool formTestModeCommand(
    eTestMode testMode, const phTestModeParams *testModeParams, UINT8 *pCommand, UINT16 *pCommandLength)
{
    UINT8 payloadLen = 0;
    UCI_MSG_BLD_HDR0(pCommand, UCI_MT_CMD, UCI_GID_PROPRIETARY);
    UCI_MSG_BLD_HDR1(pCommand, EXT_UCI_MSG_TEST_START_CMD);
    UWB_UINT8_TO_STREAM(pCommand, 0x00);

    switch (testMode) {
    case RECEIVE_MODE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_RECEIVE_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN +
                             UCI_EXT_TEST_MODE_ID_DELAY_LEN + UCI_EXT_TEST_MODE_ID_SLOT_TYPE_LEN +
                             UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX_LEN + UCI_EXT_TEST_MODE_ID_TIME_OUT_LEN);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_RECEIVE_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_RECEIVE_MODE);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_SLOT_TYPE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_SLOT_TYPE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, testModeParams->receiveModeParams.commonParams.slotType);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_DELAY);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_DELAY_LEN);
        UWB_UINT16_TO_STREAM(pCommand, testModeParams->receiveModeParams.commonParams.startDelay);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX_LEN);
        UWB_UINT32_TO_STREAM(pCommand, testModeParams->receiveModeParams.commonParams.eventCounterMax);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TIME_OUT);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TIME_OUT_LEN);
        UWB_UINT16_TO_STREAM(pCommand, testModeParams->receiveModeParams.timeOut);
        break;
    case TRANSMIT_MODE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_TRANSMIT_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN +
                             UCI_EXT_TEST_MODE_ID_DELAY_LEN + UCI_EXT_TEST_MODE_ID_SLOT_TYPE_LEN +
                             UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX_LEN + UCI_EXT_TEST_MODE_ID_TX_CYCLE_TIME_LEN +
                             testModeParams->transmitModeParams.psduLen);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TRANSMIT_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_TRANSMIT_MODE);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_SLOT_TYPE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_SLOT_TYPE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, testModeParams->transmitModeParams.commonParams.slotType);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_DELAY);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_DELAY_LEN);
        UWB_UINT16_TO_STREAM(pCommand, testModeParams->transmitModeParams.commonParams.startDelay);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_EVENT_COUNTER_MAX_LEN);
        UWB_UINT32_TO_STREAM(pCommand, testModeParams->transmitModeParams.commonParams.eventCounterMax);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TX_CYCLE_TIME);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TX_CYCLE_TIME_LEN);
        UWB_UINT32_TO_STREAM(pCommand, testModeParams->transmitModeParams.txCycleTime);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_PSDU);
        UWB_UINT8_TO_STREAM(pCommand, testModeParams->transmitModeParams.psduLen);
        UWB_ARRAY_TO_STREAM(
            pCommand, testModeParams->transmitModeParams.psduData, testModeParams->transmitModeParams.psduLen);

        break;
    case CONTINUOUS_WAVE_MODE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_CONTINUOUS_WAVE_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_CONTINUOUS_WAVE_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_CONTINUOUS_WAVE_MODE);
        break;
    case LOOP_BACK_MODE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_LOOP_BACK_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_LOOP_BACK_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_LOOP_BACK_MODE);
        break;
    case INITIATOR_MODE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_INITIATOR_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN +
                             UCI_EXT_TEST_MODE_ID_ENCRYPTED_PSDU_LEN);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_INITIATOR_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_INITIATOR_MODE);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_ENCRYPTED_PSDU);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_ENCRYPTED_PSDU_LEN);
        UWB_UINT8_TO_STREAM(pCommand, testModeParams->initiatorModeParams.isPsduEncrypted);
        break;
    case LOOP_BACK_MODE_NVM_AND_SAVE:
        // PayloadLength = size of Number of Params(1 Byte) + No of Params * 2 Byte (1 Byte Type, 1 Byte Length) + Length of all N parameters
        payloadLen = (UINT8)(1 + (UCI_EXT_LOOP_BACK_MODE_NO_OF_PARAMS * 2) + UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, payloadLen);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_LOOP_BACK_MODE_NO_OF_PARAMS);

        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_ID_TEST_MODE_LEN);
        UWB_UINT8_TO_STREAM(pCommand, UCI_EXT_TEST_MODE_LOOP_BACK_AND_SAVE_MODE);
        break;
    default:
        NXPLOG_UWBAPI_E("%s: Invalid Test Mode", __func__);
        return false;
    }
    *pCommandLength = payloadLen + UCI_MSG_HDR_SIZE;
    return true;
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
    /* By default PASS, if nothing to set */
    tUWBAPI_STATUS status = UWBAPI_STATUS_OK;
    UINT8 config          = 0;
    UINT16 dpdTimeout     = 0;
    UINT16 hpdTimeout     = 0;
    UINT8 offset          = 0;
    UINT32 telec_val      = 0;

    NXPLOG_UWBAPI_D("%s: Enter ", __func__);
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
        NXPLOG_UWBAPI_D("%s: low power mode config not found", __func__);
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_DPD_ENTRY_TIMEOUT, &dpdTimeout, 0x02) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_DPD_ENTRY_TIMEOUT value %d ", __func__, dpdTimeout);

        offset = 0; //Reset offset to zero for next use
        if ((dpdTimeout >= UWBD_DPD_TIMEOUT_MIN) && (dpdTimeout <= UWBD_DPD_TIMEOUT_MAX)) {
            offset = (UINT8)getExtTLVBuffer(
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
        NXPLOG_UWBAPI_D("%s: low power mode config not found", __func__);
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_HPD_ENTRY_TIMEOUT, &hpdTimeout, 0x02) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_HPD_ENTRY_TIMEOUT value %d ", __func__, hpdTimeout);

        offset = 0; //Reset offset to zero;
        if ((hpdTimeout >= UWBD_HPD_TIMEOUT_MIN) && (hpdTimeout <= UWBD_HPD_TIMEOUT_MAX)) {
            offset = (UINT8)getExtTLVBuffer(
                UCI_EXT_PARAM_ID_HPD_ENTRY_TIMEOUT, (void *)&hpdTimeout, &uwbContext.snd_data[offset]);

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
                NXPLOG_UWBAPI_E("%s: Hpd Entry Timeout set config is failed", __func__);
            }
        }
        else {
            NXPLOG_UWBAPI_E("%s: Invalid Range for Hpd Entry Timeout in ConfigFile", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: Hpd Entry Timeout config not found", __func__);
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_MHR_IN_CCM, &config, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_MHR_IN_CCM value %d ", __func__, (UINT8)config);

        offset = (UINT8)getCoreDeviceConfigTLVBuffer(
            UCI_EXT_PARAM_ID_MHR_IN_CCM, sizeof(config), (void *)&config, &uwbContext.snd_data[offset]);

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
            NXPLOG_UWBAPI_E("%s: MHR in CCM config is failed", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: MHR in CCM config not found", __func__);
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_DDFS_TONE_CONFIG_ENABLE, &config, 0x01) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_DDFS_TONE_CONFIG_ENABLE value %d ", __func__, (UINT8)config);

        offset = (UINT8)getCoreDeviceConfigTLVBuffer(
            UCI_EXT_PARAM_ID_DDFS_TONE_CONFIG_ENABLE, sizeof(config), (void *)&config, &uwbContext.snd_data[offset]);

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
            NXPLOG_UWBAPI_E("%s: DDFS TONE CONFIG ENABLE is failed", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s: DDFS TONE CONFIG ENABLE config not found", __func__);
    }

    if (phNxpUciHal_GetNxpNumValue(UWB_TELEC_CONFIG, &telec_val, 0x04) == TRUE) {
        NXPLOG_UWBAPI_D("%s: UWB_TELEC_CONFIG value %0x ", __func__, (UINT32)telec_val);
        offset = 0; //Reset offset to zero for next use
        offset = (UINT8)getCoreDeviceConfigTLVBuffer(
            UCI_EXT_PARAM_ID_TELC_CONFIG, sizeof(telec_val), (void *)&telec_val, &uwbContext.snd_data[offset]);

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
            NXPLOG_UWBAPI_E("%s: UWB_TELEC_CONFIG failed", __func__);
        }
    }
    else {
        NXPLOG_UWBAPI_D("%s:UWB_TELEC_CONFIG  not found", __func__);
    }

    NXPLOG_UWBAPI_D("%s: Exit ", __func__);
    return status;
}
