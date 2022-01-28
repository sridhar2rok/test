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

#ifndef __TEST_MODE_CONFIG_H__
#define __TEST_MODE_CONFIG_H__

#include <UwbApi.h>

extern void *testLoopBackNtfSem;

tUWBAPI_STATUS do_CONTINUOUS_WAVE(void);
tUWBAPI_STATUS do_LOOP_BACK(void);
tUWBAPI_STATUS do_LOOP_BACK_AND_SAVE(void);
tUWBAPI_STATUS do_TX_ONLY(UWB_SR040_TxRxSlotType_t slotType, UINT8 psdu_length);
tUWBAPI_STATUS do_RX_ONLY(UWB_SR040_TxRxSlotType_t slotType);
tUWBAPI_STATUS do_EnablePhyLogs(UWB_SR040_TxRxSlotType_t slotType);
tUWBAPI_STATUS do_EnableHPRFConfigs(void);
tUWBAPI_STATUS do_EnableDDFSConfigs(void);
tUWBAPI_STATUS do_StoreSTSKeys(void);

#endif // __TEST_MODE_CONFIG_H__
