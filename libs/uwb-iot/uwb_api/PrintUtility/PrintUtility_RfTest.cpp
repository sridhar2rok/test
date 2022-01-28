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

#include "PrintUtility_RfTest.h"

EXTERNC void printPerParams(const phRfTestParams_t *pRfTestParams)
{
    if (pRfTestParams != NULL) {
        PRINT_UTILITY_TRACES("pRfTestParams->numOfPckts              : %" PRIu32 " \n", pRfTestParams->numOfPckts);
        PRINT_UTILITY_TRACES("pRfTestParams->tGap                    : %" PRIu32 " \n", pRfTestParams->tGap);
        PRINT_UTILITY_TRACES("pRfTestParams->tStart                  : %" PRIu32 " \n", pRfTestParams->tStart);
        PRINT_UTILITY_TRACES("pRfTestParams->tWin                    : %" PRIu32 " \n", pRfTestParams->tWin);
        PRINT_UTILITY_TRACES("pRfTestParams->randomizedSize          : %" PRIu8 " \n", pRfTestParams->randomizedSize);
        PRINT_UTILITY_TRACES("pRfTestParams->rawPhr                  : %" PRIu16 " \n", pRfTestParams->rawPhr);
        PRINT_UTILITY_TRACES("pRfTestParams->rmarkerRxStart          : %" PRIu32 " \n", pRfTestParams->rmarkerRxStart);
        PRINT_UTILITY_TRACES("pRfTestParams->rmarkerTxStart          : %" PRIu32 " \n", pRfTestParams->rmarkerTxStart);
        PRINT_UTILITY_TRACES("pRfTestParams->stsIndexAutoIncr        : %" PRIu8 " \n", pRfTestParams->stsIndexAutoIncr);
        PRINT_UTILITY_TRACES("pRfTestParams->macCfg                  : %" PRIu8 " \n", pRfTestParams->macCfg);
    }
    else {
        PRINT_UTILITY_TRACES("pRfTestParams is NULL");
    }
}
