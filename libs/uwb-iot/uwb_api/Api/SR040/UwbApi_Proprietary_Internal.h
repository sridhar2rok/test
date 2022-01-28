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

#ifndef UWBAPI_PROPRIETARY_INTERNALH
#define UWBAPI_PROPRIETARY_INTERNALH

#include "UwbCore_Types.h"
#include "UwbApi_Types_Proprietary.h"

#define TX_POWER_DIFF_LEN        3u
#define FREQ_DIFF_LEN            6u
#define ANTENNA_DELAY_LEN        3u
#define CURRENT_LIMITER_LEN      1u
#define TEMP_COMPENS_FLAG_LEN    1u
#define TX_ADAPTIVE_POWER_LEN    3u
#define DDFS_TONE_VALUES_LEN     72u
#define DPD_TIMER_PENALTY_US_LEN 4u

UINT8 getExtTLVBuffer(UINT8 paramId, void *paramValue, UINT8 *tlvBuffer);
void extDeviceManagementCallback(UINT8 event, uint16_t paramLength, UINT8 *pResponseBuffer);
bool parseDeviceInfo(UINT8 *manfacturerData, UINT8 manufacturerLength);
void formCalibrationCommand(UINT8 paramId, void *pCalibData, UINT8 *pSetCalibrationCmd, UINT16 *pCommandLength);
bool formTestModeCommand(
    eTestMode testMode, const phTestModeParams *testModeParams, UINT8 *pCommand, UINT16 *pCommandLength);
tUWBAPI_STATUS setDefaultCoreConfigs(void);

#endif // UWBAPI_PROPRIETARY_INTERNALH
