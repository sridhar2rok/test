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

#include "PrintUtility.h"

EXTERNC void printStackCapabilities(const phUwbDevInfo_t *pdevInfo)
{
    if (pdevInfo != NULL) {
        PRINT_UTILITY_TRACES("--------------Stack Capabilities--------------\n");
        PRINT_UTILITY_TRACES("UCI Version                        : %" PRIu16 ".%" PRIu16 "\n",
            (UINT8)(pdevInfo->uciVersion >> 8),
            (UINT8)pdevInfo->uciVersion);
        PRINT_UTILITY_TRACES("Device Name                        : %s\n", pdevInfo->devName);
        PRINT_UTILITY_TRACES(" FW Version             : %" PRIu8 ".%" PRIu8 ".%" PRIu8 " \n",
            pdevInfo->fwMajor,
            pdevInfo->fwMinor,
            pdevInfo->fwPatchVersion);
        PRINT_UTILITY_TRACES(
            "Device Version                     : %" PRIu8 ".%" PRIu8 " \n", pdevInfo->devMajor, pdevInfo->devMinor);
        PRINT_UTILITY_TRACES("R4 SerialNo                        : %s \n", pdevInfo->serialNo);
        PRINT_UTILITY_TRACES("DSP Version                        : %" PRIu8 ".%" PRIu8 ".%" PRIu8 " \n",
            pdevInfo->dspMajor,
            pdevInfo->dspMinor,
            pdevInfo->dspPatchVersion);
        PRINT_UTILITY_TRACES(
            "Base Band  Version                : %" PRIu8 ".%" PRIu8 " \n", pdevInfo->bbMajor, pdevInfo->bbMinor);
        PRINT_UTILITY_TRACES("CCC Version                        : %s \n", pdevInfo->cccVersion);
        PRINT_UTILITY_TRACES(
            "Middleware Version                 : %" PRIu8 ".%" PRIu8 "\n", pdevInfo->mwMajor, pdevInfo->mwMinor);
    }
}
