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

static tUWBAPI_STATUS FactoryCalib_ResetAllToZero_CN(uint8_t channelNo);
static tUWBAPI_STATUS FactoryCalib_ResetNonZeroDefaults(void);
static tUWBAPI_STATUS FactoryCalib_ResetZeroDefaults(void);
static tUWBAPI_STATUS FactoryCalib_ResetAllToDefaults_DdfsToneConfig(void);
static tUWBAPI_STATUS FactoryCalib_TempCompensFlagDefaults(void);

OSAL_TASK_RETURN_TYPE StandaloneTask(void *args)
{
    PRINT_APP_NAME("Reset Calibration Params");

    tUWBAPI_STATUS status = UWBAPI_STATUS_FAILED;

    status = UwbApi_Init(AppCallback);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_Init Failed");
        goto exit;
    }

    status = FactoryCalib_ResetNonZeroDefaults();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("FactoryCalib_ResetNonZeroDefaults Failed");
        goto exit;
    }

    status = FactoryCalib_ResetZeroDefaults();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("FactoryCalib_ResetZeroDefaults Failed");
        goto exit;
    }

    status = FactoryCalib_ResetAllToDefaults_DdfsToneConfig();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("FactoryCalib_ResetAllToDefaults_DdfsToneConfig Failed");
        goto exit;
    }

exit:
    UWBIOT_EXAMPLE_END(status);
}

static tUWBAPI_STATUS FactoryCalib_ResetNonZeroDefaults(void)
{
    tUWBAPI_STATUS status;
    phCalibrationData calibData;
    phCalibrationStatus_t calibResp;

    /* Calibration for CURRENT_LIMIT_VALUE */
    /* Set Current limiter to 20mA */
    calibData.currLimitValue = 20;

    /* Set current limit first */
    status = UwbApi_SetCalibration(CURRENT_LIMIT_VALUE, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set CURRENT_LIMIT_VALUE");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I("CURRENT_LIMIT_VALUE Set to %dmA", calibData.currLimitValue);

    /* Calibration for DPD_TIMER_PENALTY_US */
    /* Set DPD Timer Penalty in us to 14917us */
    calibData.dpdTimerPenalty = 14917;

    /* Set DPD Timer Penalty in us */
    status = UwbApi_SetCalibration(DPD_TIMER_PENALTY_US, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set DPD_TIMER_PENALTY_US");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I("DPD_TIMER_PENALTY_US Set to %dus", calibData.dpdTimerPenalty);
    Log("Done");
cleanup:
    return status;
}

static tUWBAPI_STATUS FactoryCalib_ResetZeroDefaults(void)
{
    tUWBAPI_STATUS status;

    status = FactoryCalib_ResetAllToZero_CN(5);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Failed FactoryCalib_ResetAllToZero_CN for Channel 5");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }

    status = FactoryCalib_ResetAllToZero_CN(6);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Failed FactoryCalib_ResetAllToZero_CN for Channel 6");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }

    status = FactoryCalib_ResetAllToZero_CN(8);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Failed FactoryCalib_ResetAllToZero_CN for Channel 8");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    status = FactoryCalib_ResetAllToZero_CN(9);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Failed FactoryCalib_ResetAllToZero_CN for Channel 9");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }

    status = FactoryCalib_TempCompensFlagDefaults();
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("Failed FactoryCalib_TempCompensFlagDefaults");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }

    Log("Done");
cleanup:
    return status;
}

static tUWBAPI_STATUS FactoryCalib_ResetAllToZero_CN(uint8_t channel_no)
{
    tUWBAPI_STATUS status;
    phCalibrationData calibData;
    phCalibrationStatus_t calibResp;

    /* doc:start:tx-power-diff */
    /* Calibration for TX_POWER_DIFF */
    calibData.txPowerDiffCalibData.channelNo   = channel_no;
    calibData.txPowerDiffCalibData.signVal     = 0;
    calibData.txPowerDiffCalibData.absoluteVal = 0;
    status                                     = UwbApi_SetCalibration(TX_POWER_DIFF, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set TX_POWER_DIFF");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I(
        "TX_POWER_DIFF params set to \n \
        Channel Number    %d\n \
        Sign Value        %d\n \
        Absolute value    %d",
        calibData.txPowerDiffCalibData.channelNo,
        calibData.txPowerDiffCalibData.signVal,
        calibData.txPowerDiffCalibData.absoluteVal);
    /* doc:end:tx-power-diff */

    /* doc:start:freq-diff */
    /* Calibration for FREQ_DIFF */
    calibData.freqDiffCalibData.channelNo          = channel_no;
    calibData.freqDiffCalibData.signVal            = 0;
    calibData.freqDiffCalibData.absoluteFreqOffset = 0;
    status                                         = UwbApi_SetCalibration(FREQ_DIFF, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set FREQ_DIFF");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I(
        "FREQ_DIFF params set to \n \
        Channel Number              %d\n \
        Sign Value                  %d\n \
        Absolute Frequency Offset   0x%08X",
        calibData.freqDiffCalibData.channelNo,
        calibData.freqDiffCalibData.signVal,
        calibData.freqDiffCalibData.absoluteFreqOffset);
    /* doc:end:freq-diff */

    /* doc:start:antenna-delay */
    /* Calibration for ANTENNA_DELAY */
    calibData.antennaDelayaCalibData.channelNo    = channel_no;
    calibData.antennaDelayaCalibData.antennaDelay = 0;
    status                                        = UwbApi_SetCalibration(ANTENNA_DELAY, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set ANTENNA_DELAY");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I(
        "ANTENNA_DELAY params set to \n \
        Channel Number    %d\n \
        Antenna Delay     0x%04X\n",
        calibData.antennaDelayaCalibData.channelNo,
        calibData.antennaDelayaCalibData.antennaDelay);
    /* doc:end:antenna-delay */

    /* doc:start:tx-adaptive-power-calculation */
    /* Calibration for TX_ADAPTIVE_POWER_CALC */
    calibData.txAdaptivePowerCalibData.channelNo  = channel_no;
    calibData.txAdaptivePowerCalibData.powerIdRms = 0;
    calibData.txAdaptivePowerCalibData.peakDelta  = 0;
    status = UwbApi_SetCalibration(TX_ADAPTIVE_POWER_CALC, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set TX_ADAPTIVE_POWER_CALC");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I(
        "TX_ADAPTIVE_POWER_CALC params set to \n \
        Channel Number       %d\n \
        PowerId Rms          %d\n \
        Peak Delta           %d",
        calibData.txAdaptivePowerCalibData.channelNo,
        calibData.txAdaptivePowerCalibData.powerIdRms,
        calibData.txAdaptivePowerCalibData.peakDelta);
    /* doc:end:tx-adaptive-power-calculation */

cleanup:
    return status;
}

/** This interface reset the DDFS Tone Config values to their default values of all channels */
static tUWBAPI_STATUS FactoryCalib_ResetAllToDefaults_DdfsToneConfig(void)
{
    tUWBAPI_STATUS status;
    phCalibrationData calibData;
    phCalibrationStatus_t calibResp;
    const uint8_t channelList[] = {5, 6, 8, 9};

    /* doc:start:tx-adaptive-power-calculation */
    /* Calibration for DDFS_TONE_VALUES for all channels */
    for (UINT32 LoopCnt = 0; LoopCnt < 4; ++LoopCnt) {
        calibData.ddfsToneConfigData[LoopCnt].channelNo            = channelList[LoopCnt];
        calibData.ddfsToneConfigData[LoopCnt].rfu                  = 0x00;
        calibData.ddfsToneConfigData[LoopCnt].txDdfsTone0RegVal    = 0x0000010E;
        calibData.ddfsToneConfigData[LoopCnt].txDdfsTone1RegVal    = 0x0000013E;
        calibData.ddfsToneConfigData[LoopCnt].spurDuration         = 0x000001F4;
        calibData.ddfsToneConfigData[LoopCnt].gainvalsetRegVal     = 0x26;
        calibData.ddfsToneConfigData[LoopCnt].gainByPassEnblRegVal = 0x00;
        calibData.ddfsToneConfigData[LoopCnt].spurPeriodicity      = 0x0064;
    }
    status = UwbApi_SetCalibration(DDFS_TONE_VALUES, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set DDFS_TONE_VALUES");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
#if (APP_LOG_LEVEL >= UWB_LOG_INFO_LEVEL)
    NXPLOG_APP_I(" DDFS_TONE_VALUES params set to");
    for (UINT32 LoopCnt = 0; LoopCnt < 4; ++LoopCnt)
#endif
    {
        NXPLOG_APP_I(
            "Channel Number              %d\n \
              Ddfs Tone0 RegVal           0x%08X\n \
              Ddfs Tone1 RegVal           0x%08X\n \
              Spur Duration               0x%08X\n \
              Gain val set RegVal         0x%02X\n \
              Gain ByPass Enbl RegVal     0x%02X\n \
              Spur Periodicity            0x%04X\n ",
            calibData.ddfsToneConfigData[LoopCnt].channelNo,
            calibData.ddfsToneConfigData[LoopCnt].rfu,
            calibData.ddfsToneConfigData[LoopCnt].txDdfsTone0RegVal,
            calibData.ddfsToneConfigData[LoopCnt].txDdfsTone1RegVal,
            calibData.ddfsToneConfigData[LoopCnt].spurDuration,
            calibData.ddfsToneConfigData[LoopCnt].gainvalsetRegVal,
            calibData.ddfsToneConfigData[LoopCnt].gainByPassEnblRegVal,
            calibData.ddfsToneConfigData[LoopCnt].spurPeriodicity);
    }
    /* doc:end:ddfs-tone-values-calcualtion */
cleanup:
    return status;
}

/** This interface to reset the TEMP_COMPENS_FLAG */
static tUWBAPI_STATUS FactoryCalib_TempCompensFlagDefaults(void)
{
    tUWBAPI_STATUS status;
    phCalibrationData calibData;
    phCalibrationStatus_t calibResp;

    /* doc:start:temp-compens-flag */
    /* Calibration for TEMP_COMPENS_FLAG */
    calibData.tempCompensFlag = 0; //enable

    /* Set tempcompens first */
    status = UwbApi_SetCalibration(TEMP_COMPENS_FLAG, &calibData, &calibResp);
    if ((status != UWBAPI_STATUS_OK) || (calibResp.status != APPLIED)) {
        NXPLOG_APP_E("Failed to set TEMP_COMPENS_FLAG");
        status = UWBAPI_STATUS_FAILED;
        goto cleanup;
    }
    NXPLOG_APP_I("TEMP_COMPENS_FLAG set to %d", calibData.tempCompensFlag);
    /* doc:end:temp-compens-flag */
cleanup:
    return status;
}
