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

#ifndef UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPITRANSPORT_H_
#define UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPITRANSPORT_H_

#include "UwbCore_Types.h"

/** interface handler */
typedef enum
{
    /* UCI application mode */
    kInterfaceModeUci = 0x00u,
    /* RCI/SWUP application mode */
    kInterfaceModeSwup = 0x01u,
} interface_handler_t;

EXTERNC int phNxpUwb_HeliosInit(void);
EXTERNC void phNxpUwb_HeliosDeInit(void);
EXTERNC int phNxpUwb_UciWrite(uint8_t *data, uint16_t len);
EXTERNC uint16_t phNxpUwb_UciRead(uint8_t *rsp);
EXTERNC int phNxpUwb_RciWrite(uint8_t *data, uint16_t len);
EXTERNC uint16_t phNxpUwb_RciRead(uint8_t *rsp);
EXTERNC int phNxpUwb_HbciTransceive(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen);
EXTERNC void phNxpUwb_HeliosIrqEnable(void);
EXTERNC void phNxpUwb_SwitchProtocol(interface_handler_t protocolType);
EXTERNC bool phNxpUwb_HeliosReset(void);

#endif /* UWB_CORE_HALIMPL_TRANSPORT_SPI_PHNXPUWB_SPITRANSPORT_H_ */
