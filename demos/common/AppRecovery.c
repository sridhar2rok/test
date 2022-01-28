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

/*
 * Recovery.c
 *
 *  Created on: Mar 9, 2020
 *      Author: nxf50460
 */
#include "phUwb_BuildConfig.h"

#if (UWBCORE_SDK_BUILDCONFIG != UWB_BUILD_PLUG_AND_PLAY_MODE)

#include "AppRecovery.h"
#include "AppInternal.h"
#include "phOsalUwb.h"
#include "phOsalUwb_Thread.h"
#include "phDal4Uwb_messageQueueLib.h"

extern UWBOSAL_TASK_HANDLE testTaskHandle;
static UWBOSAL_TASK_HANDLE recoveryTaskHandle;

static intptr_t mErrorHandlerQueue;
phLibUwb_Message_t receive_message;
errorScenario_t receive_scenario;

extern OSAL_TASK_RETURN_TYPE StandaloneTask(void *args);
static OSAL_TASK_RETURN_TYPE AppRecoveryTask(void *args);
static OSAL_TASK_RETURN_TYPE recovery_handler(errorScenario_t error_scenario);

static OSAL_TASK_RETURN_TYPE AppRecoveryTask(void *args)
{
    Log("Started AppRecoveryTask\n");

    mErrorHandlerQueue = phDal4Uwb_msgget();
    if (!mErrorHandlerQueue) {
        Log("Error: main, could not create queue mErrorHandlerQueue\n");
        while (1)
            ;
    }

    while (1) {
        phLibUwb_Message_t message;
        errorScenario_t scenario;
        Log("Waiting for Error Scenario\n");
        if (phDal4Uwb_msgrcv(mErrorHandlerQueue, &message, 0, 0) == -1) {
            continue;
        }
        memcpy(&scenario, message.pMsgData, sizeof(scenario));
        recovery_handler(scenario);
    }
}

static OSAL_TASK_RETURN_TYPE recovery_handler(errorScenario_t error_scenario)
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;

    Log("Recovery Started for Scenario : %d\n", error_scenario);
    switch (error_scenario) {
    case TIME_OUT:
    case APP_CLEANUP:
    case FW_CRASH:
#if (UWBIOT_UWBD_SR100T)
#if (FACTORY_MODE == ENABLED)
        UwbApi_RecoverFactoryUWBS();
#else
        UwbApi_RecoverUWBS();
#endif /* FACTORY_MODE */
#endif /* UWBIOT_UWBD_SR100T */
        phOsalUwb_Thread_Delete(testTaskHandle);

        threadparams.stackdepth = TEST_TASK_STACK_SIZE;
        strcpy((char *)threadparams.taskname, TEST_TASK_NAME);
        threadparams.pContext = NULL;
        threadparams.priority = TEST_TASK_PRIORITY;
        pthread_create_status = phOsalUwb_Thread_Create((void **)&testTaskHandle, &StandaloneTask, &threadparams);
        if (0 != pthread_create_status) {
            LOG_E("Task created Failed");
        }
        break;
    default:
        break;
    }
}

void Start_AppRecoveryTask()
{
    phOsalUwb_ThreadCreationParams_t threadparams;
    int pthread_create_status = 0;

    threadparams.stackdepth = RECOVERY_TASK_STACK_SIZE;
    strcpy((char *)threadparams.taskname, RECOVERY_TASK_NAME);
    threadparams.pContext = NULL;
    threadparams.priority = RECOVERY_TASK_PRIORITY;
    pthread_create_status = phOsalUwb_Thread_Create((void **)&recoveryTaskHandle, &AppRecoveryTask, &threadparams);
    if (0 != pthread_create_status) {
        Log("AppRecoveryTask created Failed");
    }
}

void Handle_ErrorScenario(errorScenario_t scenario)
{
    receive_message.eMsgType = 0;
    receive_message.pMsgData = &receive_scenario;
    memcpy(&receive_scenario, &scenario, sizeof(receive_scenario));
    receive_message.Size = sizeof(scenario);
    phDal4Uwb_msgrcv(mErrorHandlerQueue, &receive_message, 0, 0);
}

#endif
