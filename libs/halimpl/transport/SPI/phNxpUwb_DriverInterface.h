/*
  * Copyright (C) 2012-2020 NXP Semiconductors
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

#ifndef UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPIDRIVER_H_
#define UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPIDRIVER_H_

#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

int phNxpUwb_SpiInit(void);
int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len);
int phNxpUwb_SpiRead(uint8_t *rsp, uint16_t rspLen);
int phNxpUwb_SpiDeInit(void);

#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
void phNxpUwb_GpioIrqEnable(void (*cb)(void));
void phNxpUwb_GpioIrqDisable(void (*cb)(void));
#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
void phNxpUwb_GpioIrqEnable(void (*cb)(void *args));
void phNxpUwb_GpioIrqDisable(void (*cb)(void *args));
#endif

#if UWBIOT_UWBD_SR100T
bool phNxpUwb_RtcSyncWrite(bool value);
bool phNxpUwb_RtcSyncRead(void);
#endif

#if UWBIOT_UWBD_SR040
bool phNxpUwb_ReadyStatus(void);
void phNxpUwb_sr040Reset(void);
bool phNxpUwb_RdyRead(void);
void phNxpUwb_RdyGpioIrqEnable(void (*cb)(void *args));
void phNxpUwb_RdyGpioIrqDisable(void (*cb)(void *args));
#endif

bool phNxpUwb_HeliosInteruptStatus(void);

#ifdef __cplusplus
} /* __cplusplus extern "C" */
#endif

#endif /* UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPITRANSPORT_H_ */
