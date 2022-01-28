/* Copyright 2019,2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#ifndef APP_INTERNAL_H_
#define APP_INTERNAL_H_

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include <uwb_port.h>
//#include "TestAppConfig.h"
#include "AppStateManagement.h"
#include "PrintUtility.h"
#include "UwbApi.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#include "phNxpLogApis_App.h"

#define TEST_TASK_PRIORITY   4
#define TEST_TASK_STACK_SIZE 1024
#define TEST_TASK_NAME       "TestTask"

#define RECOVERY_TASK_PRIORITY   5
#define RECOVERY_TASK_STACK_SIZE 300
#define RECOVERY_TASK_NAME       "RecoveryTask"

#define Log LOG_I

extern void *perSem;
extern void *rangingDataSem;
extern void *inBandterminationSem;
extern UINT8 data_buff[];
extern void AppCallback(eNotificationType opType, void *pData);

#define PRINT_APP_NAME(szAPP_NAME)                                   \
    PRINTF("#################################################\r\n"); \
    PRINTF("## " szAPP_NAME "\r\n");                                 \
    PRINTF("## " UWBIOTVER_STR_PROD_NAME_VER_FULL "\r\n");           \
    PRINTF("#################################################\r\n")

/**
 * @brief      End of the example.
 *
 * @param      status MW status from tUWBAPI_STATUS
 *
 * @return     NA
 *
 * If running on MSVC, this program will exit.
 */

#if UWBIOT_OS_FREERTOS
#define RET_VALUE return
#else
#define RET_VALUE return 0
#endif

#if defined(_MSC_VER) || defined(__linux__) || defined(__MINGW32__) || defined(__MINGW64__)
#define UWBIOT_EXAMPLE_END(status)                      \
    if (status == UWBAPI_STATUS_OK) {                   \
        LOG_I("\r\nFinished %s : Succes!\n", __FILE__); \
        exit(0);                                        \
    }                                                   \
    else {                                              \
        LOG_E("\nFinished %s : Failed!\n", __FILE__);   \
        exit(1);                                        \
    }                                                   \
    RET_VALUE;

#else /* Non windows linux */

#define UWBIOT_EXAMPLE_END(status)                \
    if (status == UWBAPI_STATUS_OK) {             \
        LOG_I("Finished %s : Succes!", __FILE__); \
    }                                             \
    else {                                        \
        LOG_E("Finished %s : Failed!", __FILE__); \
    }                                             \
    __disable_irq();                              \
    while (1) {                                   \
        __WFI();                                  \
    }                                             \
    RET_VALUE
#endif // ! _MSC_VER

#endif //APP_INTERNAL_H_
