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

#ifndef _PRINT_UTILITY_
#define _PRINT_UTILITY_

#include "UwbCore_Types.h"
#include <inttypes.h>
#include "fsl_debug_console.h"
#include "phUwb_BuildConfig.h"
#include "UwbApi_Types.h"
#include "PrintUtility_Proprietary.h"

#if (ENABLE_PRINT_UTILITY_TRACES == TRUE)
#define PRINT_UTILITY_TRACES PRINTF
#else
#define PRINT_UTILITY_TRACES(...)
#endif

EXTERNC void printGenericErrorStatus(const phGenericError_t *pGenericError);
EXTERNC void printSessionStatusData(const phUwbSessionInfo_t *pSessionInfo);
EXTERNC void printUwbSessionData(const phUwbSessionsContext_t *pUwbSessionsContext);
EXTERNC void printMulticastListStatus(const phMulticastControleeListNtfContext_t *pControleeNtfContext);
EXTERNC void printRangingData(const phRangingData_t *pRangingData);
EXTERNC void printRangingParams(const phRangingParams_t *pRangingParams);

#endif
