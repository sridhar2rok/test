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

#if 1 //( UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE )

#ifndef UWB_EVT_H
#define UWB_EVT_H

#include "fsl_gpio.h"

typedef enum
{
    USB_TLV_EVT,
} UWB_EvtType_t;

typedef struct
{
    uint8_t type;
    uint16_t size;
    uint8_t *value;
} tlv_t;

typedef struct
{
    UWB_EvtType_t type;
    void *data;
} UWB_Evt_t;

/* TLV types */
#define UCI_CMD              0x01
#define HBCI_CMD             0x02
#define HBCI_LAST_CMD        0x03
#define RESET                0x04
#define HBCI_QUERY_CMD       0x05
#define GET_SOFTWARE_VERSION 0x06
#define GET_BOARD_ID         0x07
#define MCU_RESET            0x09 //MCU nvReset will be called.
#define RCI_CMD              0x0A
#define SWITCH_PROTOCOL_CMD  0x0B

#define TLV_RESP_SIZE               0x20
#define RESET_SOFTWARE_VERSION_SIZE 0x04

void UWB_HandleEvt(UWB_EvtType_t ev, void *args);

#endif

#endif
