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

#ifndef UWB_USB_VCOM_PNP_H_
#define UWB_USB_VCOM_PNP_H_

uint32_t UWB_Usb_UciSendNtfn(uint8_t *pData, uint16_t size);
uint32_t UWB_Usb_SendUCIRsp(uint8_t *pData, uint16_t size);
uint32_t UWB_Usb_SendRsp(uint8_t *pData, uint16_t size);
void Uwb_Usb_Init(void (*rcvCb)(uint8_t *, uint32_t *));

#endif /* UWB_USB_VCOM_PNP_H_ */
