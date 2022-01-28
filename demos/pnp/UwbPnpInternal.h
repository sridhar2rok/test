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

#ifndef UWBPNPINTERNAL_H_
#define UWBPNPINTERNAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "UWB_Evt.h"
#include "app_config.h"

#define WRITER_QUEUE_SIZE 100
extern QueueHandle_t mHifWriteMutex;
extern QueueHandle_t mHifWriteQueue;
extern SemaphoreHandle_t mHifIsr_Sem;
extern QueueHandle_t mHifCommandQueue;
extern xTaskHandle mHifTask;
extern xTaskHandle mPnpAppTask;
extern xTaskHandle mUciReaderTask;
extern xTaskHandle mHifWriterTask;
extern QueueHandle_t mHifSyncMutex;
uint32_t UWB_Hif_UciSendNtfn(uint8_t *pData, uint16_t size);
uint32_t UWB_Hif_SendUCIRsp(uint8_t *pData, uint16_t size);
uint32_t UWB_Hif_SendRsp(uint8_t *pData, uint16_t size);
void Uwb_Hif_ReadDataCb(uint8_t *pData, uint32_t *pLen);
bool Uwb_Is_Hif_Active();
void Uwb_Reset_Hif_State(bool state);
void UWB_Pnp_App_Task(void *args);
#if UWBIOT_UWBD_SR100T
void UWB_Handle_SR100T_TLV(tlv_t *tlv);
#endif
#if UWBIOT_UWBD_SR040
void UWB_Handle_SR040_TLV(tlv_t *tlv);
#endif

void UWB_WriterTask(void *args);
void UWB_Hif_Handler_Task(void *args);
// void UCI_ReaderTask(void *args);
// void UWB_USBSetResponse(uint8_t type, uint8_t *data, uint16_t data_size);

// EXTERNC bool UWB_HeliosCE(bool set);

void PRINTF_WITH_TIME(const char *fmt, ...);

#define HIF_TASK_SIZE        512
#define PNP_APP_TASK_SIZE    512
#define UCI_READER_TASK_SIZE 512
#define HIF_WRITER_TASK_SIZE 512

#define TRACE_UCI           PRINTF
#define TRACE_HBCI          PRINTF
#define MAX_RSP_PACKET_SIZE 2176

#endif /* UWBPNPINTERNAL_H_ */
