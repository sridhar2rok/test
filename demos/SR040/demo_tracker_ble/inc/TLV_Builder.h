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

#ifndef _TLV_BUILDER_H_
#define _TLV_BUILDER_H_

/* System includes */
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    UWB_HIF_BLE,
    UWB_HIF_UART,
} UWB_Hif_t;

bool tlvAdd_UINT8(uint8_t data);
bool tlvAdd_UINT16(uint16_t data);
bool tlvAdd_UINT32(uint32_t data);
bool tlvAdd_PTR(uint8_t *data, uint16_t len);

bool tlvStart(uint8_t type);
bool tlvSend(void);
void tlvRecv(UWB_Hif_t interface, uint8_t *tlv, uint8_t tlvSize);
void tlvFlushInput(void);

void tlvSendDoneCb(void);

bool tlvMngInit(void);
bool tlvBuilderInit(void);

#endif /* _TLV_BUILDER_H_ */
