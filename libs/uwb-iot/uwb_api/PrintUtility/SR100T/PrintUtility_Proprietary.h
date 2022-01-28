/*
 *
 * Copyright 2018-2020 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#ifndef _PRINT_UTILITY_PROPRIETARY_H
#define _PRINT_UTILITY_PROPRIETARY_H

#include "UwbApi_Types_Proprietary.h"

EXTERNC void printStackCapabilities(const phUwbDevInfo_t *pdevInfo);
EXTERNC void printRcvData(const phRcvData_t *pRcvData);
EXTERNC void printDistance_Aoa(const phRangingData_t *pRangingData);
EXTERNC void printDebugParams(const phDebugParams_t *pDebugParams);
EXTERNC void printStackCapabilities(const phUwbDevInfo_t *pdevInfo);
EXTERNC void printDoBindStatus(const phSeDoBindStatus_t *pDoBindStatus);
EXTERNC void printGetBindingStatus(const phSeGetBindingStatus_t *pGetBindingStatus);
EXTERNC void printTestLoopNtfData(const phTestLoopData_t *pTestLoopData);

#endif
