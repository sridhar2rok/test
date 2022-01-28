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

#ifndef _UWB_HELIOS_H_
#define _UWB_HELIOS_H_

#include "UwbCore_Types.h"
#include "phUwb_BuildConfig.h"

#if 1 //( UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE )
EXTERNC uint8_t UWB_HeliosSpiInit(void);
EXTERNC void UWB_HeliosSpiDeInit(void);
EXTERNC uint8_t UWB_SpiHbciXfer(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen);
EXTERNC uint8_t UWB_SpiHbciXferWithLen(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t rspLen);
EXTERNC uint16_t UWB_SpiUciWrite(uint8_t *data, uint16_t len);
EXTERNC uint16_t UWB_SpiUciRead(uint8_t *rsp);
EXTERNC void UWB_HeliosIrqEnable(void);
#endif

#endif
