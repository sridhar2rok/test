/*
*
* Copyright 2020 NXP.
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

#ifndef UWBAPI_RADIOCONFIG_H
#define UWBAPI_RADIOCONFIG_H

#include <UwbApi_Types.h>

/**
 * brief Checks Crc's of all indexes and conditional update of radio configs.
 *
 * \retval #UWBAPI_STATUS_OK       - Returns SUCCESS if radio config crc's are correct
 *                                   or update is fine.
 * \retval #UWBAPI_STATUS_FAILED   - otherwise
 */
EXTERNC tUWBAPI_STATUS RadioConfig_tdoaAndTracker(bool force_update);

/**
 * brief Checks Crc's of all indexes.
 *
 * \retval #UWBAPI_STATUS_OK       - Returns SUCCESS if values are "SAME" as
 *                                   expected as per the current release.
 * \retval #UWBAPI_STATUS_FAILED   - otherwise
 */
EXTERNC tUWBAPI_STATUS RadioConfig_CheckCrc(void);

#endif // UWBAPI_RADIOCONFIG_H
