/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef UWBADAPT_H
#define UWBADAPT_H

#include "phNxpUciHal.h"
#include "phUwb_BuildConfig.h"
#include "uwb_api.h"
#include "uwb_hal_api.h"
#include "uwb_target.h"

void Initialize();
tUWB_STATUS DownloadFirmware();
tUWB_STATUS DownloadFirmwareRecovery();
void Finalize();
tHAL_UWB_ENTRY *GetHalEntryFuncs();
bool SetFirmwareImage(uint8_t *fwImgPtr, uint32_t fwSize);
bool isCmdRespPending();

#endif
