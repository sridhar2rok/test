/* Copyright 2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#ifndef _UWB_USART_VCOM_PNP_H_
#define _UWB_USART_VCOM_PNP_H_

#include <stdint.h>
#include "FreeRTOS.h"

uint32_t transmitToUsart(uint8_t *pData, size_t size);
void Uwb_USART_Init(void (*rcvCb)(uint8_t *, uint32_t *));
#define UWB_USART_UciSendNtfn transmitToUsart
#define UWB_USART_SendUCIRsp  transmitToUsart
#define UWB_USART_SendRsp     transmitToUsart

#define USART_DEVICE_INTERRUPT_PRIORITY (3U)

#endif /* _UWB_USART_VCOM_PNP_H_ */
