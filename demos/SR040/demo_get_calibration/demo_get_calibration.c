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

#include "UwbApi.h"
#include <AppInternal.h>
#include "AppRecovery.h"

typedef enum
{
    /** @brief Tag of TX_POWER_DIFF */
    GET_TX_POWER_DIFF_TAG = 0x00u,
    /** @brief Tag of FREQ_DIFF */
    GET_FREQ_DIFF_TAG = 0x01u,
    /** @brief Tag of ANTENNA_DELAY */
    GET_ANTENNA_DELAY_TAG = 0x02u,
    /** @brief Tag of CURRENT_LIMITER */
    GET_CURRENT_LIMITER_TAG = 0x03u,
    /** @brief Tag of GROUP_DELAY */
    GET_GROUP_DELAY_TAG = 0x04u,
    /** @brief Tag of TEMP_COMPENS_FLAG */
    GET_TEMP_COMPENS_FLAG_TAG = 0x05u,
    /** @brief Tag of TX_ADAPTIVE_POWER_CALC */
    GET_TX_ADAPTIVE_POWER_TAG = 0x06u,
    /** @brief Tag of DDFS_TONE_VALUES */
    GET_DDFS_TONE_VALUES = 0x07u,
    /** @bried Tag of DPD timer penalty in us */
    GET_DPD_TIMER_PENALTY_US = 0x08u
} demoCalibrationItem_t;

typedef struct
{
    demoCalibrationItem_t calib_param;
    union {
        /* TODO: Parse and fill values */
        uint8_t rawValue[UCI_MAX_PAYLOAD_SIZE];
    };
} demoCalibrationData_t;

static tUWBAPI_STATUS getCalibrationData(demoCalibrationItem_t a, demoCalibrationData_t *pCalibData);
static void printCalibrationData(demoCalibrationItem_t calib_param, demoCalibrationData_t *pCalibData);

/** @brief To read trim values */
#define PHSCA_UCIMSG_OID_PROPRIETARY_GET_TRIM_VALUES ((uint8_t)0x28u)

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Get Calibrated Data");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    demoCalibrationData_t calib_data;
    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init Failed");
        goto exit;
    }

    status = getCalibrationData(GET_TX_POWER_DIFF_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration TX_POWER_DIFF_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_FREQ_DIFF_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration FREQ_DIFF_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_ANTENNA_DELAY_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration ANTENNA_DELAY_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_CURRENT_LIMITER_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration CURRENT_LIMITER_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_GROUP_DELAY_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("GET_GROUP_DELAY_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_TEMP_COMPENS_FLAG_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration TEMP_COMPENS_FLAG_TAG Failed");
        goto exit;
    }
    status = getCalibrationData(GET_TX_ADAPTIVE_POWER_TAG, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration TX_ADAPTIVE_POWER_TAG Failed");
        goto exit;
    }

#if 0
    /* Get Calibration support for DDFS tone values is not yet enabled. Need to enable this once the support is provided.*/
    status = getCalibrationData(GET_DDFS_TONE_VALUES, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration DDFS_TONE_VALUES Failed");
        goto exit;
    }
#endif

#if 0
    /* Not available in this release */
    status = getCalibrationData(GET_DPD_TIMER_PENALTY_US, &calib_data);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Get Calibration DPD_TIMER_PENALTY_US Failed");
        goto exit;
    }
#endif

exit:
    UWBIOT_EXAMPLE_END(status);
}

static tUWBAPI_STATUS getCalibrationData(demoCalibrationItem_t calib_param, demoCalibrationData_t *pCalibData)
{
    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;
    UINT8 calibResponse[UCI_MAX_PAYLOAD_SIZE];
    memset(pCalibData, 0, sizeof(*pCalibData));

    UINT16 respLen                   = sizeof(calibResponse);
    UINT8 UCI_GetCalibrationParams[] = {
        0x2E,
        PHSCA_UCIMSG_OID_PROPRIETARY_GET_TRIM_VALUES,
        0x00,
        0x02,        /* len */
        0x01,        /* n parameters */
        calib_param, /* which parameter */
    };

    status = UwbApi_SendRawCommand(
        (UINT8 *)&UCI_GetCalibrationParams[0], sizeof(UCI_GetCalibrationParams), &calibResponse[0], &respLen);
    if (respLen > sizeof(*pCalibData)) {
        status = UWBAPI_STATUS_BUFFER_OVERFLOW;
    }
    if (respLen < 8) {
        status = UWBAPI_STATUS_FAILED;
    }
    else if (status == UWBAPI_STATUS_OK) {
        pCalibData->calib_param = calib_param;
        memcpy(pCalibData->rawValue, calibResponse + 8, respLen - 8);
        printCalibrationData(calib_param, pCalibData);
    }
    return status;
}

static void printCalibrationData(demoCalibrationItem_t calib_param, demoCalibrationData_t *pCalibData)
{
    UINT32 LoopCnt;
    UINT8 *pCalib_Data = NULL;
    phCalibrationData Calib_Data;

    pCalib_Data = (pCalibData->rawValue);

    switch (calib_param) {
    case GET_TX_POWER_DIFF_TAG:
        NXPLOG_APP_I("TX_POWER_DIFF params set to");
        /* Including Channel 7 Related Info received by FW !!! */
        for (LoopCnt = 0; LoopCnt < 5; ++LoopCnt) {
            UWB_STREAM_TO_UINT8(Calib_Data.txPowerDiffCalibData.channelNo, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.txPowerDiffCalibData.signVal, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.txPowerDiffCalibData.absoluteVal, pCalib_Data);
            NXPLOG_APP_I(
                "Channel Number    %d\n \
              Sign Value        %d\n \
              Absolute value    %d",
                Calib_Data.txPowerDiffCalibData.channelNo,
                Calib_Data.txPowerDiffCalibData.signVal,
                Calib_Data.txPowerDiffCalibData.absoluteVal);
        }
        break;

    case GET_FREQ_DIFF_TAG:
        NXPLOG_APP_I("FREQ_DIFF params set to");
        /* Including Channel 7 Related Info received by FW !!! */
        for (LoopCnt = 0; LoopCnt < 5; ++LoopCnt) {
            UWB_STREAM_TO_UINT8(Calib_Data.freqDiffCalibData.channelNo, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.freqDiffCalibData.signVal, pCalib_Data);
            UWB_STREAM_TO_UINT32(Calib_Data.freqDiffCalibData.absoluteFreqOffset, pCalib_Data);
            NXPLOG_APP_I(
                "Channel Number              %d\n \
              Sign Value                  %d\n \
              Absolute Frequency Offset   0x%08X",
                Calib_Data.freqDiffCalibData.channelNo,
                Calib_Data.freqDiffCalibData.signVal,
                Calib_Data.freqDiffCalibData.absoluteFreqOffset);
        }
        break;
    case GET_ANTENNA_DELAY_TAG:
        NXPLOG_APP_I("ANTENNA_DELAY params set to");
        /* Including Channel 7 Related Info received by FW !!! */
        for (LoopCnt = 0; LoopCnt < 5; ++LoopCnt) {
            UWB_STREAM_TO_UINT8(Calib_Data.antennaDelayaCalibData.channelNo, pCalib_Data);
            UWB_STREAM_TO_UINT16(Calib_Data.antennaDelayaCalibData.antennaDelay, pCalib_Data);
            NXPLOG_APP_I(
                "Channel Number    %d\n \
              Antenna Delay     0x%04X",
                Calib_Data.antennaDelayaCalibData.channelNo,
                Calib_Data.antennaDelayaCalibData.antennaDelay);
        }
        break;
    case GET_CURRENT_LIMITER_TAG:
        UWB_STREAM_TO_UINT8(Calib_Data.currLimitValue, pCalib_Data);
        NXPLOG_APP_I("CURRENT_LIMIT_VALUE Set to %d", Calib_Data.currLimitValue);
        break;
    case GET_GROUP_DELAY_TAG:
        /* [TODO] To be added */
        break;
    case GET_TEMP_COMPENS_FLAG_TAG:
        UWB_STREAM_TO_UINT8(Calib_Data.tempCompensFlag, pCalib_Data);
        NXPLOG_APP_I("TEMP_COMPENS_FLAG set to %d", Calib_Data.tempCompensFlag);
        break;
    case GET_TX_ADAPTIVE_POWER_TAG:
        NXPLOG_APP_I("TX_ADAPTIVE_POWER_CALC params set to");
        /* Including Channel 7 Related Info received by FW !!! */
        for (LoopCnt = 0; LoopCnt < 5; ++LoopCnt) {
            UWB_STREAM_TO_UINT8(Calib_Data.txAdaptivePowerCalibData.channelNo, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.txAdaptivePowerCalibData.powerIdRms, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.txAdaptivePowerCalibData.peakDelta, pCalib_Data);
            NXPLOG_APP_I(
                "Channel Number       %d\n \
              PowerId Rms          %d\n \
              Peak Delta           %d",
                Calib_Data.txAdaptivePowerCalibData.channelNo,
                Calib_Data.txAdaptivePowerCalibData.powerIdRms,
                Calib_Data.txAdaptivePowerCalibData.peakDelta);
        }
        break;
    case GET_DDFS_TONE_VALUES:
        NXPLOG_APP_I("DDFS_TONE_VALUES params set to");
        for (LoopCnt = 0; LoopCnt < 4; ++LoopCnt) {
            UWB_STREAM_TO_UINT8(Calib_Data.ddfsToneConfigData[LoopCnt].channelNo, pCalib_Data);
            ++pCalib_Data; // Skip RFU
            UWB_STREAM_TO_UINT32(Calib_Data.ddfsToneConfigData[LoopCnt].txDdfsTone0RegVal, pCalib_Data);
            UWB_STREAM_TO_UINT32(Calib_Data.ddfsToneConfigData[LoopCnt].txDdfsTone1RegVal, pCalib_Data);
            UWB_STREAM_TO_UINT32(Calib_Data.ddfsToneConfigData[LoopCnt].spurDuration, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.ddfsToneConfigData[LoopCnt].gainvalsetRegVal, pCalib_Data);
            UWB_STREAM_TO_UINT8(Calib_Data.ddfsToneConfigData[LoopCnt].gainByPassEnblRegVal, pCalib_Data);
            UWB_STREAM_TO_UINT16(Calib_Data.ddfsToneConfigData[LoopCnt].spurPeriodicity, pCalib_Data);
            NXPLOG_APP_I(
                "Channel Number              %d\n \
              Ddfs Tone0 RegVal           0x%08X\n \
              Ddfs Tone1 RegVal           0x%08X\n \
              Spur Duration               0x%08X\n \
              Gain val set RegVal         0x%02X\n \
              Gain ByPass Enbl RegVal     0x%02X\n \
              Spur Periodicity            0x%04X\n ",
                Calib_Data.ddfsToneConfigData[LoopCnt].channelNo,
                Calib_Data.ddfsToneConfigData[LoopCnt].txDdfsTone0RegVal,
                Calib_Data.ddfsToneConfigData[LoopCnt].txDdfsTone1RegVal,
                Calib_Data.ddfsToneConfigData[LoopCnt].spurDuration,
                Calib_Data.ddfsToneConfigData[LoopCnt].gainvalsetRegVal,
                Calib_Data.ddfsToneConfigData[LoopCnt].gainByPassEnblRegVal,
                Calib_Data.ddfsToneConfigData[LoopCnt].spurPeriodicity);
        }
        break;
    case GET_DPD_TIMER_PENALTY_US:
        UWB_STREAM_TO_UINT32(Calib_Data.dpdTimerPenalty, pCalib_Data);
        NXPLOG_APP_I("DPD Timer Penalty (us) param set to %dus", Calib_Data.dpdTimerPenalty);
        break;
    default:
        NXPLOG_APP_E("Get Calibration parameter error !!!");
    }
}
