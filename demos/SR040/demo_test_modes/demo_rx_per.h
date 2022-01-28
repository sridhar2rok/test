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

#ifndef DEMOS_SR040_DEMO_TEST_MODES_DEMO_RX_PER_H_
#define DEMOS_SR040_DEMO_TEST_MODES_DEMO_RX_PER_H_

#include <UwbApi_Types.h>

#include "demo_test_modes.h"

void demo_create_test_file(void);
void demo_close_per_log_file(void);
void demo_rx_per_AppCallback(eNotificationType opType, void *pData);
void demo_rx_per_Init(
    const UWB_SR040_TxRxSlotType_t slotType, uint8_t const *const expectedRxFrame, size_t expectedRxFrameLen);
void demo_rx_per_PrintSummary(UWB_SR040_TxRxSlotType_t slotType);
uint16_t demo_rx_per_GetNtfsCount(void);

#endif /* DEMOS_SR040_DEMO_TEST_MODES_DEMO_RX_PER_H_ */
