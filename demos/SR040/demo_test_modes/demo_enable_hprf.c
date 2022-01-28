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

#include "demo_test_modes.h"
#include <phNxpLogApis_App.h>

/**  [0,2]:BPRF, [1,2,3]:HPRF */
#define SFD_ID_VALUE 2
/**  [9-24]:BPRF, [25-32]:HPRF */
#define PREAMBLE_ID_VALUE 25
/**  0: BPRF, 1:HPRF */
#define PRF_MODE_VALUE 1
/**  0:32 symbols, 1:64 symbols */
#define PREAMBLE_DURATION_VALUE 1
/** Number of STS segments in the frame */
#define NUMBER_OF_STS_SEGMENTS_VALUE 1
/**  0:6.81MBPS, 1:7.80MBPS */
#define PSDU_DATA_RATE_VALUE 0

/* very basic compile time check of invalid combinations */

/* clang-format off */
#if PRF_MODE_VALUE == 0
#   if PREAMBLE_ID_VALUE >= 24
#       error Invalid Combination
#   endif
#   if PREAMBLE_DURATION_VALUE == 0
#       error Invalid Combination
#   endif
#   if SFD_ID_VALUE == 0
#       error Invalid Combination
#   endif
#   if SFD_ID_VALUE == 1
#       error Invalid Combination
#   endif
#   if PSDU_DATA_RATE_VALUE == 1
#       error Invalid Combination
#   endif
#endif

#if PRF_MODE_VALUE == 1
#   if PREAMBLE_ID_VALUE <= 24
#       error Invalid Combination
#   endif
#   if SFD_ID_VALUE == 0
#       error Invalid Combination
#   endif
#endif
/* clang-format on */

tUWBAPI_STATUS do_EnableHPRFConfigs(void)
{
    tUWBAPI_STATUS status;
    LOG_I("DO:%s", __FUNCTION__);

    /* Values are set above. */

#if defined(SFD_ID_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, SFD_ID, SFD_ID_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::SFD_ID Failed");
        goto exit;
    }
#endif

#if defined(PREAMBLE_ID_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, PREAMBLE_CODE_INDEX, PREAMBLE_ID_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::PREAMBLE_CODE_INDEX Failed");
        goto exit;
    }
#endif

#if defined(PRF_MODE_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, PRF_MODE, PRF_MODE_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::PRF_MODE Failed");
        goto exit;
    }
#endif

#if defined(PREAMBLE_DURATION_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, PREAMBLE_DURATION, PREAMBLE_DURATION_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::PREAMBLE_DURATION Failed");
        goto exit;
    }
#endif

#if defined(NUMBER_OF_STS_SEGMENTS_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, NUMBER_OF_STS_SEGMENTS, NUMBER_OF_STS_SEGMENTS_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::NUMBER_OF_STS_SEGMENTS Failed");
        goto exit;
    }
#endif

#if defined(PSDU_DATA_RATE_VALUE)
    status = UwbApi_SetAppConfig(SESSION_ID_RFTEST, PSDU_DATA_RATE, PSDU_DATA_RATE_VALUE);
    if (status != UWBAPI_STATUS_OK) {
        NXPLOG_APP_E("UwbApi_SetAppConfig::PSDU_DATA_RATE Failed");
        goto exit;
    }
#endif

exit:
    return status;
}
