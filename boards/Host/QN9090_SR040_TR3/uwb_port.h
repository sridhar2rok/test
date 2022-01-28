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

#ifndef PORT_H
#define PORT_H

#include "phUwb_BuildConfig.h"

#include <QN9090.h>
// #if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V1 || UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
// #include "MKL28Z7.h"
// #include "core_cm0plus.h"

// #elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
// //#include <K32W042S1M2_cm4.h>
// #endif

// TODO: Check if this is valid for QN9090
#define phPlatform_Is_Irq_Context() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)
#endif
