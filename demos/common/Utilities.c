/*
 * Copyright 2019,2020 NXP
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
#include "phUwb_BuildConfig.h"

#if 1 // UWB_BUILD_STANDALONE_CDC_MODE

#include "Utilities.h"
#include "uci_defs.h"
#include "uwb_types.h"

void serializeDataFromRangingParams(phRangingParams_t *pRangingParam, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingParam->deviceRole);
    offset = (UINT16)(offset + sizeof(pRangingParam->deviceRole));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingParam->multiNodeMode);
    offset = (UINT16)(offset + sizeof(pRangingParam->multiNodeMode));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingParam->noOfControlees);
    offset = (UINT16)(offset + sizeof(pRangingParam->noOfControlees));
#if UWBIOT_UWBD_SR100T
    UWB_UINT8_TO_STREAM(pRespBuf, pRangingParam->macAddrMode);
    offset = (UINT16)(offset + sizeof(pRangingParam->macAddrMode));

    UINT16 addrLen = MAC_SHORT_ADD_LEN;
    if (pRangingParam->macAddrMode != SHORT_MAC_ADDRESS) {
        addrLen = MAC_EXT_ADD_LEN;
    }
#elif UWBIOT_UWBD_SR040
    UINT16 addrLen = MAC_SHORT_ADD_LEN;
#endif

    UWB_ARRAY_TO_STREAM(pRespBuf, pRangingParam->deviceMacAddr, addrLen);
    offset = (UINT16)(offset + addrLen);

    UWB_ARRAY_TO_STREAM(pRespBuf, pRangingParam->dstMacAddr, addrLen * pRangingParam->noOfControlees);
    offset = (UINT16)(offset + addrLen * pRangingParam->noOfControlees);

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingParam->deviceType);
    offset = (UINT16)(offset + sizeof(pRangingParam->deviceType));

    *pRespSize = offset;
}

#if UWBIOT_UWBD_SR100T
void serializeDataFromDebugParams(phDebugParams_t *pDebugParams, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->secureThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->secureThread));

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->secureIsrThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->secureIsrThread));

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->nonSecureIsrThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->nonSecureIsrThread));

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->shellThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->shellThread));

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->phyThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->phyThread));

    UWB_UINT16_TO_STREAM(pRespBuf, pDebugParams->rangingThread);
    offset = (UINT16)(offset + sizeof(pDebugParams->rangingThread));

    UWB_UINT8_TO_STREAM(pRespBuf, pDebugParams->dataLoggerNtf);
    offset = (UINT16)(offset + sizeof(pDebugParams->dataLoggerNtf));

    UWB_UINT8_TO_STREAM(pRespBuf, pDebugParams->cirLogNtf);
    offset = (UINT16)(offset + sizeof(pDebugParams->cirLogNtf));

    UWB_UINT8_TO_STREAM(pRespBuf, pDebugParams->psduLogNtf);
    offset = (UINT16)(offset + sizeof(pDebugParams->psduLogNtf));

    UWB_UINT8_TO_STREAM(pRespBuf, pDebugParams->rframeLogNtf);
    offset = (UINT16)(offset + sizeof(pDebugParams->rframeLogNtf));

    *pRespSize = offset;
}
#endif

void serializeDataFromPerParams(phRfTestParams_t *pRfTestParam, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;
    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->numOfPckts);
    offset = (UINT16)(offset + sizeof(pRfTestParam->numOfPckts));

    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->tGap);
    offset = (UINT16)(offset + sizeof(pRfTestParam->tGap));

    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->tStart);
    offset = (UINT16)(offset + sizeof(pRfTestParam->tStart));

    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->tWin);
    offset = (UINT16)(offset + sizeof(pRfTestParam->tWin));

    UWB_UINT8_TO_STREAM(pRespBuf, pRfTestParam->randomizedSize);
    offset = (UINT16)(offset + sizeof(pRfTestParam->randomizedSize));

    UWB_UINT16_TO_STREAM(pRespBuf, pRfTestParam->rawPhr);
    offset = (UINT16)(offset + sizeof(pRfTestParam->rawPhr));

    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->rmarkerTxStart);
    offset = (UINT16)(offset + sizeof(pRfTestParam->rmarkerTxStart));

    UWB_UINT32_TO_STREAM(pRespBuf, pRfTestParam->rmarkerRxStart);
    offset = (UINT16)(offset + sizeof(pRfTestParam->rmarkerRxStart));

    UWB_UINT8_TO_STREAM(pRespBuf, pRfTestParam->stsIndexAutoIncr);
    offset = (UINT16)(offset + sizeof(pRfTestParam->stsIndexAutoIncr));

    UWB_UINT8_TO_STREAM(pRespBuf, pRfTestParam->macCfg);
    offset     = (UINT16)(offset + sizeof(pRfTestParam->macCfg));
    *pRespSize = offset;
}

void serializeDataFromRangingDataNtf(phRangingData_t *pRangingData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;
    UWB_UINT32_TO_STREAM(pRespBuf, pRangingData->seq_ctr);
    offset = (UINT16)(offset + sizeof(pRangingData->seq_ctr));

    UWB_UINT32_TO_STREAM(pRespBuf, pRangingData->sessionId);
    offset = (UINT16)(offset + sizeof(pRangingData->sessionId));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->rcr_indication);
    offset = (UINT16)(offset + sizeof(pRangingData->rcr_indication));

    UWB_UINT32_TO_STREAM(pRespBuf, pRangingData->curr_range_interval);
    offset = (UINT16)(offset + sizeof(pRangingData->curr_range_interval));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->ranging_measure_type);
    offset = (UINT16)(offset + sizeof(pRangingData->ranging_measure_type));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->antenna_pair_sel);
    offset = (UINT16)(offset + sizeof(pRangingData->antenna_pair_sel));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->mac_addr_mode_indicator);
    offset = (UINT16)(offset + sizeof(pRangingData->mac_addr_mode_indicator));

    UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->no_of_measurements);
    offset = (UINT16)(offset + sizeof(pRangingData->no_of_measurements));

    if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_TWOWAY) {
        for (int i = 0; i < pRangingData->no_of_measurements; i++) {
            if (pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
                UWB_ARRAY_TO_STREAM(pRespBuf, pRangingData->range_meas[i].mac_addr, MAC_SHORT_ADD_LEN);
                offset = (UINT16)(offset + MAC_SHORT_ADD_LEN);
            }
            else {
                UWB_ARRAY_TO_STREAM(pRespBuf, pRangingData->range_meas[i].mac_addr, MAC_EXT_ADD_LEN);
                offset = (UINT16)(offset + MAC_EXT_ADD_LEN);
            }
            UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas[i].status);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].status));

            UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas[i].nLos);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].nLos));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].distance);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].distance));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].aoaFirst);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].aoaFirst));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].aoaSecond);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].aoaSecond));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].pdoaFirst);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].pdoaFirst));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].pdoaSecond);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].pdoaSecond));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].pdoaFirstIndex);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].pdoaFirstIndex));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].pdoaSecondIndex);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].pdoaSecondIndex));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].aoaDestFirst);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].aoaDestFirst));

            UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas[i].aoaDestSecond);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].aoaDestSecond));

            UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas[i].slot_index);
            offset = (UINT16)(offset + sizeof(pRangingData->range_meas[i].slot_index));
        }
    }
    else if (pRangingData->ranging_measure_type == MEASUREMENT_TYPE_ONEWAY) {
        if (pRangingData->mac_addr_mode_indicator == SHORT_MAC_ADDRESS) {
            UWB_ARRAY_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.mac_addr, MAC_SHORT_ADD_LEN);
            offset = (UINT16)(offset + MAC_SHORT_ADD_LEN);
        }
        else {
            UWB_ARRAY_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.mac_addr, MAC_EXT_ADD_LEN);
            offset = (UINT16)(offset + MAC_EXT_ADD_LEN);
        }

        UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.frame_type);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.frame_type));

        UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.nLos);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.nLos));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.aoaFirst);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.aoaFirst));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.aoaSecond);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.aoaSecond));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.pdoaFirst);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.pdoaFirst));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.pdoaSecond);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.pdoaSecond));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.pdoaFirstIndex);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.pdoaFirstIndex));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.pdoaSecondIndex);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.pdoaSecondIndex));

        UWB_UINT64_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.timestamp);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.timestamp));

        UWB_UINT32_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.blink_frame_number);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.blink_frame_number));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.rssiRX1);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.rssiRX1));

        UWB_UINT16_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.rssiRX2);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.rssiRX2));

        UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.device_info_size);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.device_info_size));

        UWB_ARRAY_TO_STREAM(
            pRespBuf, pRangingData->range_meas_tdoa.device_info, pRangingData->range_meas_tdoa.device_info_size);
        offset = (UINT16)(offset + pRangingData->range_meas_tdoa.device_info_size);

        UWB_UINT8_TO_STREAM(pRespBuf, pRangingData->range_meas_tdoa.blink_payload_size);
        offset = (UINT16)(offset + sizeof(pRangingData->range_meas_tdoa.blink_payload_size));

        UWB_ARRAY_TO_STREAM(pRespBuf,
            pRangingData->range_meas_tdoa.blink_payload_data,
            pRangingData->range_meas_tdoa.blink_payload_size);
        offset = (UINT16)(offset + pRangingData->range_meas_tdoa.blink_payload_size);
    }
    *pRespSize = offset;
}

void serializeStackInfo(phUwbDevInfo_t *pStackInfo, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;
    UWB_UINT16_TO_STREAM(pRespBuf, pStackInfo->uciVersion);
    offset = (UINT16)(offset + sizeof(pStackInfo->uciVersion));
#if UWBIOT_UWBD_SR100T
    UWB_UINT8_TO_STREAM(pRespBuf, pStackInfo->devNameLen);
    offset = (UINT16)(offset + sizeof(pStackInfo->devNameLen));
    UWB_ARRAY_TO_STREAM(pRespBuf, pStackInfo->devName, pStackInfo->devNameLen);
    offset = (UINT16)(offset + pStackInfo->devNameLen);
#endif
#if UWBIOT_UWBD_SR040
    UWB_ARRAY8_TO_STREAM(pRespBuf, pStackInfo->devName);
    offset = (UINT16)(offset + sizeof(pStackInfo->devName));
#endif
    UWB_UINT8_TO_STREAM(pRespBuf, pStackInfo->fwMajor);
    offset = (UINT16)(offset + sizeof(pStackInfo->fwMajor));
    UWB_UINT8_TO_STREAM(pRespBuf, pStackInfo->fwMinor);
    offset = (UINT16)(offset + sizeof(pStackInfo->fwMinor));
    UWB_UINT8_TO_STREAM(pRespBuf, pStackInfo->mwMajor);
    offset = (UINT16)(offset + sizeof(pStackInfo->mwMajor));
    UWB_UINT8_TO_STREAM(pRespBuf, pStackInfo->mwMinor);
    offset     = (UINT16)(offset + sizeof(pStackInfo->mwMinor));
    *pRespSize = offset;
}

void serializeDataFromPerTxDataNtf(phPerTxData_t *pPerTxData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pPerTxData->status);
    offset = (UINT16)(offset + sizeof(pPerTxData->status));

    *pRespSize = offset;
}

#if UWBIOT_UWBD_SR100T
void serializeDataFromRcvDataNtf(phRcvData_t *pRcvData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT32_TO_STREAM(pRespBuf, pRcvData->session_id);
    offset = (UINT16)(offset + sizeof(pRcvData->session_id));

    UWB_UINT8_TO_STREAM(pRespBuf, pRcvData->status);
    offset = (UINT16)(offset + sizeof(pRcvData->status));

    UWB_UINT8_TO_STREAM(pRespBuf, pRcvData->data_len);
    offset = (UINT16)(offset + sizeof(pRcvData->data_len));

    UWB_ARRAY_TO_STREAM(pRespBuf, pRcvData->data, pRcvData->data_len);
    offset = (UINT16)(offset + pRcvData->data_len);

    *pRespSize = offset;
}

void serializeDataFromSendDataNtf(phSendData_t *pSendData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT32_TO_STREAM(pRespBuf, pSendData->sessionId);
    offset = (UINT16)(offset + sizeof(pSendData->sessionId));

    UWB_UINT8_TO_STREAM(pRespBuf, pSendData->status);
    offset = (UINT16)(offset + sizeof(pSendData->status));

    *pRespSize = offset;
}

void serializeDataFromLoopTestNtf(phTestLoopData_t *pTestLoopData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pTestLoopData->status);
    offset = (UINT16)(offset + sizeof(pTestLoopData->status));

    UWB_UINT16_TO_STREAM(pRespBuf, pTestLoopData->loop_cnt);
    offset = (UINT16)(offset + sizeof(pTestLoopData->loop_cnt));

    UWB_UINT16_TO_STREAM(pRespBuf, pTestLoopData->loop_pass_count);
    offset = (UINT16)(offset + sizeof(pTestLoopData->loop_pass_count));

    *pRespSize = offset;
}
#endif

void serializeDataFromSessionStatusNtf(phUwbSessionInfo_t *pSessionStatusData, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT32_TO_STREAM(pRespBuf, pSessionStatusData->session_id);
    offset = (UINT16)(offset + sizeof(pSessionStatusData->session_id));

    UWB_UINT8_TO_STREAM(pRespBuf, pSessionStatusData->state);
    offset = (UINT16)(offset + sizeof(pSessionStatusData->state));

    *pRespSize = offset;
}

#if UWBIOT_UWBD_SR100T
void serializeDataFromDoBindStatusNtf(phSeDoBindStatus_t *pSeDoBindStatus, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pSeDoBindStatus->status);
    offset = (UINT16)(offset + sizeof(pSeDoBindStatus->status));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeDoBindStatus->count_remaining);
    offset = (UINT16)(offset + sizeof(pSeDoBindStatus->count_remaining));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeDoBindStatus->binding_state);
    offset = (UINT16)(offset + sizeof(pSeDoBindStatus->binding_state));

    *pRespSize = offset;
}
#endif

#if UWBIOT_UWBD_SR100T
void serializeDataFromGetBindingStatusNtf(
    phSeGetBindingStatus_t *pSeGetBindingStatus, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingStatus->status);
    offset = (UINT16)(offset + sizeof(pSeGetBindingStatus->status));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingStatus->se_binding_count);
    offset = (UINT16)(offset + sizeof(pSeGetBindingStatus->se_binding_count));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingStatus->uwbd_binding_count);
    offset = (UINT16)(offset + sizeof(pSeGetBindingStatus->uwbd_binding_count));

    *pRespSize = offset;
}

void serializeDataFromGetBindingCountResponse(
    phSeGetBindingCount_t *pSeGetBindingCount, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingCount->bindingStatus);
    offset = (UINT16)(offset + sizeof(pSeGetBindingCount->bindingStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingCount->uwbdBindingCount);
    offset = (UINT16)(offset + sizeof(pSeGetBindingCount->uwbdBindingCount));

    UWB_UINT8_TO_STREAM(pRespBuf, pSeGetBindingCount->seBindingCount);
    offset = (UINT16)(offset + sizeof(pSeGetBindingCount->seBindingCount));

    *pRespSize = offset;
}

void serializeDataFromFactoryTestStatus(factoryFwTestStatus_t factoryFwTestStatus, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.shutDownStatus);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.shutDownStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.fwDlStatus);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.fwDlStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.seInitStatus);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.seInitStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.connectivityTestStatus);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.connectivityTestStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.wtxCount);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.wtxCount));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.doBindStatus.status);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.doBindStatus.status));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.doBindStatus.count_remaining);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.doBindStatus.count_remaining));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.doBindStatus.binding_state);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.doBindStatus.binding_state));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.getBindingCount.bindingStatus);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.getBindingCount.bindingStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.getBindingCount.uwbdBindingCount);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.getBindingCount.uwbdBindingCount));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.getBindingCount.seBindingCount);
    offset = (UINT16)(offset + sizeof(factoryFwTestStatus.getBindingCount.seBindingCount));

    UWB_UINT8_TO_STREAM(pRespBuf, factoryFwTestStatus.testStatus);
    offset     = (UINT16)(offset + sizeof(factoryFwTestStatus.testStatus));
    *pRespSize = offset;
}

void serializeDataFromMainlineTestStatus(
    mainLineFwTestStatus_t mainLineFwTestStatus, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.shutDownStatus);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.shutDownStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.fwDlStatus);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.fwDlStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.seInitStatus);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.seInitStatus));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.testLoopData.status);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.testLoopData.status));

    UWB_UINT16_TO_STREAM(pRespBuf, mainLineFwTestStatus.testLoopData.loop_cnt);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.testLoopData.loop_cnt));

    UWB_UINT16_TO_STREAM(pRespBuf, mainLineFwTestStatus.testLoopData.loop_pass_count);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.testLoopData.loop_pass_count));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.getBindingStatus.status);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.getBindingStatus.status));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.getBindingStatus.uwbd_binding_count);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.getBindingStatus.uwbd_binding_count));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.getBindingStatus.se_binding_count);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.getBindingStatus.se_binding_count));

    UWB_UINT8_TO_STREAM(pRespBuf, mainLineFwTestStatus.testStatus);
    offset = (UINT16)(offset + sizeof(mainLineFwTestStatus.testStatus));

    *pRespSize = offset;
}

void serializeDataFromDoCalibrationNtf(const phCalibRespStatus_t *pCalibResp, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT8_TO_STREAM(pRespBuf, pCalibResp->status);
    offset = (UINT16)(offset + sizeof(pCalibResp->status));

    UWB_ARRAY_TO_STREAM(pRespBuf, pCalibResp->calibValueOut, pCalibResp->length);
    offset = (UINT16)(offset + pCalibResp->length);

    *pRespSize = offset;
}
#endif

void serializeDataFromMulticastControleeListUpdateNtf(
    const phMulticastControleeListNtfContext_t *pControleeNtfContext, UINT8 *pRespBuf, UINT16 *pRespSize)
{
    UINT16 offset = 0;

    UWB_UINT32_TO_STREAM(pRespBuf, pControleeNtfContext->session_id);
    offset = (UINT16)(offset + sizeof(pControleeNtfContext->session_id));

    UWB_UINT8_TO_STREAM(pRespBuf, pControleeNtfContext->remaining_list);
    offset = (UINT16)(offset + sizeof(pControleeNtfContext->remaining_list));

    UWB_UINT8_TO_STREAM(pRespBuf, pControleeNtfContext->no_of_controlees);
    offset = (UINT16)(offset + sizeof(pControleeNtfContext->no_of_controlees));

    for (UINT8 i = 0; i < pControleeNtfContext->no_of_controlees; i++) {
        UWB_UINT32_TO_STREAM(pRespBuf, pControleeNtfContext->subsession_id_list[i]);
        offset = (UINT16)(offset + sizeof(pControleeNtfContext->subsession_id_list[i]));

        UWB_UINT8_TO_STREAM(pRespBuf, pControleeNtfContext->status_list[i]);
        offset = (UINT16)(offset + sizeof(pControleeNtfContext->status_list[i]));
    }

    *pRespSize = offset;
}
#endif
