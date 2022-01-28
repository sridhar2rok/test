/*
 * Copyright 2012-2020 NXP.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file  phOsalUwb_Thread.c
 * \brief OSAL Implementation.
 */

/** \addtogroup grp_osal_uwb
    @{
 */
/*
************************* Header Files ****************************************
*/
#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include "phOsalUwb_Thread.h"
#include "FreeRTOSConfig.h"
#include "phOsalUwb_Internal.h"

#if defined(ANDROID_MW) || defined(COMPANION_DEVICE) || UWBIOT_OS_NATIVE
#include <pthread.h>
#endif

#if defined(ANDROID_MW) || defined(COMPANION_DEVICE) || UWBIOT_OS_NATIVE
/*
****************************** Macro Definitions ******************************
*/
/** \ingroup grp_osal_uwb
    Value indicates Failure to suspend/resume thread */
#define PH_OSALUWB_THREADPROC_FAILURE (0xFFFFFFFF)

/*
*************************** Function Definitions ******************************
*/

static void *phOsalUwb_ThreadProcedure(void *lpParameter)
{
    phOsalUwb_sOsalThreadHandle_t *pThreadHandle = (phOsalUwb_sOsalThreadHandle_t *)lpParameter;
    pThreadHandle->ThreadFunction(pThreadHandle->Params);
    return PH_OSALUWB_RESET_VALUE;
}

UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam)
{
    UWBSTATUS wCreateStatus = UWBSTATUS_SUCCESS;

    phOsalUwb_sOsalThreadHandle_t *pThreadHandle;
    phOsalUwb_ThreadCreationParams_t *threadparams = (phOsalUwb_ThreadCreationParams_t *)pParam;

    printf("Creating thread - %s \n", threadparams->taskname);

    if ((NULL == hThread) || (NULL == pThreadFunction)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        pThreadHandle = (phOsalUwb_sOsalThreadHandle_t *)phOsalUwb_GetMemory(sizeof(phOsalUwb_sOsalThreadHandle_t));
        if (pThreadHandle == NULL) {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
            return wCreateStatus;
        }
        /* Fill the Threadhandle structure which is needed for threadprocedure
     * function */
        pThreadHandle->ThreadFunction = pThreadFunction;
        pThreadHandle->Params         = threadparams->pContext;
        /* Indicate the thread is created without message queue */
        if (pthread_create(&pThreadHandle->ObjectHandle, NULL, pThreadFunction, threadparams->pContext) == 0) {
            /* Assign the Thread handle structure to the pointer provided by the user */
            *hThread = (void *)&pThreadHandle->ObjectHandle;
            phOsalUwb_Delay(10);
        }
        else {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
        }
    }

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread)
{
    UWBSTATUS wDeletionStatus = UWBSTATUS_SUCCESS;
    pthread_cancel(hThread);
    return wDeletionStatus;
}

UWBOSAL_TASK_HANDLE phOsalUwb_GetTaskHandle(void)
{
    return (pthread_self());
}

/** @} */
#else
/*
****************************** Macro Definitions ******************************
*/
/** \ingroup grp_osal_uwb
    Value indicates Failure to suspend/resume thread */
#define PH_OSALUWB_THREADPROC_FAILURE (0xFFFFFFFF)

/*
*************************** Function Definitions ******************************
*/

UWBSTATUS phOsalUwb_Thread_Create(void **hThread, pphOsalUwb_ThreadFunction_t pThreadFunction, void *pParam)
{
    UWBSTATUS wCreateStatus                         = UWBSTATUS_SUCCESS;
    phOsalUwb_ThreadCreationParams_t *pThreadParams = (phOsalUwb_ThreadCreationParams_t *)pParam;

    if ((NULL == hThread) || (NULL == pThreadFunction) || (NULL == pThreadParams)) {
        wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, UWBSTATUS_INVALID_PARAMETER);
    }
    else {
        /* Indicate the thread is created without message queue */
        if (pdPASS == xTaskCreate(pThreadFunction,
                          (const char *)&(pThreadParams->taskname[0]),
                          pThreadParams->stackdepth,
                          pThreadParams->pContext,
                          (tskIDLE_PRIORITY + pThreadParams->priority),
                          (TaskHandle_t *)hThread)) {
            configASSERT(*hThread);
        }
        else {
            wCreateStatus = PHUWBSTVAL(CID_UWB_OSAL, PH_OSALUWB_THREAD_CREATION_ERROR);
        }
    }
#if !(configUSE_PREEMPTION)
    if (uxTaskPriorityGet(NULL) < tskIDLE_PRIORITY + pThreadParams->priority) {
        taskYIELD();
    }
#endif

    return wCreateStatus;
}

UWBSTATUS phOsalUwb_Thread_Delete(UWBOSAL_TASK_HANDLE hThread)
{
    UWBSTATUS wDeletionStatus = UWBSTATUS_SUCCESS;
    vTaskDelete((tskTaskControlBlock *)hThread);
    return wDeletionStatus;
}

UWBOSAL_TASK_HANDLE phOsalUwb_GetTaskHandle(void)
{
    return (xTaskGetCurrentTaskHandle());
}

#endif
/** @} */
