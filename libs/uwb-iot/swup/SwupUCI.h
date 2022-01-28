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

#ifndef __SWUP_UCI_APIS_H__
#define __SWUP_UCI_APIS_H__

//#include <AppInternal.h>
#include <UwbApi.h>

tUWBAPI_STATUS Uci_PrintDeviceInfo(uint32_t *fwVersion, uint32_t *dspVersion);
tUWBAPI_STATUS Uci_PrintCoreCapabilities(void);
tUWBAPI_STATUS Uci_EnableSwup(void);

#endif // __APP_APIS_H__
