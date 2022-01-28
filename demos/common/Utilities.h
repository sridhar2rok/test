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

#if 1 // (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_CDC_MODE)

#include "UwbApi.h"

#if UWBIOT_UWBD_SR100T
#include "Binding_SE_Test.h"
#endif

#ifndef APP_CDC_APP_UTILITIES_H_
#define APP_CDC_APP_UTILITIES_H_
#if UWBIOT_UWBD_SR100T
void serializeDataFromRangingParams(phRangingParams_t *pRangingParam, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromDebugParams(phDebugParams_t *pDebugParam, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromPerParams(phRfTestParams_t *pRfTestParam, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromRangingDataNtf(phRangingData_t *pRangingData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromPerTxDataNtf(phPerTxData_t *pPerTxData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromRcvDataNtf(phRcvData_t *pRcvData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromSendDataNtf(phSendData_t *pSendData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeStackInfo(phUwbDevInfo_t *pStackInfo, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromLoopTestNtf(phTestLoopData_t *pTestLoopData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromSessionStatusNtf(phUwbSessionInfo_t *pSessionStatusData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromDoBindStatusNtf(phSeDoBindStatus_t *pSeDoBindStatus, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromGetBindingStatusNtf(
    phSeGetBindingStatus_t *pSeGetBindingStatus, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromGetBindingCountResponse(
    phSeGetBindingCount_t *pSeGetBindingCount, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromFactoryTestStatus(factoryFwTestStatus_t factoryFwTestStatus, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromMainlineTestStatus(
    mainLineFwTestStatus_t mainLineFwTestStatus, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromDoCalibrationNtf(const phCalibRespStatus_t *pCalibResp, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromMulticastControleeListUpdateNtf(
    const phMulticastControleeListNtfContext_t *pControleeNtfContext, UINT8 *pRespBuf, UINT16 *pRespSize);
void PRINTF_WITH_TIME(const char *fmt, ...);
#elif UWBIOT_UWBD_SR040
void serializeDataFromRangingParams(phRangingParams_t *pRangingParam, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromPerParams(phRfTestParams_t *pRfTestParam, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromRangingDataNtf(phRangingData_t *pRangingData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromPerTxDataNtf(phPerTxData_t *pPerTxData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeStackInfo(phUwbDevInfo_t *pStackInfo, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromSessionStatusNtf(phUwbSessionInfo_t *pSessionStatusData, UINT8 *pRespBuf, UINT16 *pRespSize);
void serializeDataFromMulticastControleeListUpdateNtf(
    const phMulticastControleeListNtfContext_t *pControleeNtfContext, UINT8 *pRespBuf, UINT16 *pRespSize);
#endif

#endif /* _UTILITIES_H_ */
#endif /* UWBCORE_SDK_BUILDCONFIG */
