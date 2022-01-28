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

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#include "UwbApi_Types.h"
#include "SwupApi.h"
#include "Swup_update.h"
#include "SwupUCI.h"
#include "AppInternal.h"

#define DEBUGOUT(...)
tUWBAPI_STATUS UpdateFirmware(void);

#endif // __APP_CONFIG_H__
