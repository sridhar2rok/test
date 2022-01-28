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

/*
 * \file  phOsalUwb.h
 * \brief OSAL header files related to memory, debug, random, semaphore and
 * mutex functions.
 */

/** \addtogroup grp_osal_uwb
    @{
 */

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#ifndef PHOSALUWB_INTERNAL_H
#define PHOSALUWB_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 *\note: API listed here encompasses Operating System Abstraction Layer
 *interfaces which are used internal to the module.
 *
 */

/*
************************* Include Files ****************************************
*/
#include "phOsalUwb.h"
#if defined(ANDROID_MW) || defined(COMPANION_DEVICE) || UWBIOT_OS_NATIVE
#include <pthread.h>
#include <semaphore.h>
#else
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif

/*
****************************** Macro Definitions ******************************
*/

/**
 * Maximum Number of timers available with OSAL module */
#define PH_OSALUWB_MAX_TIMER (5U)

/**
 * Maximum Number of threads available with OSAL module */
#define PH_OSALUWB_MAX_THREAD (5U)

/**
 * Indicates whether thread has created its message queue or not */
#define PH_OSALUWB_MSG_QUEUE_NOT_CREATED (0x0)

/**
 * Maximum Number of semaphores available with OSAL module */
#define PH_OSALUWB_MAX_SEMAPHORE (5U)

/**
 * Maximum Number of Mutexes available with OSAL module */
#define PH_OSALUWB_MAX_MUTEX (5U)

/**
 * Continuable exception type  */
#define PH_OSALUWB_CONTINUABLE_EXCEPTION_TYPE (0x00)

/**
 * Unknown exception type  */
#define PH_OSALUWB_UNKNOWN_EXCEPTION_TYPE (0x02)

/**
 * Value to reset variables of OSAL  */
#define PH_OSALUWB_RESET_VALUE (0x00)

/**
 * Value to reset variables of OSAL  */
#define PH_OSALUWB_VALUE_ONE (0x01)

/* Max task name length for freeRTOS */
#if UWBIOT_OS_NATIVE
#define configMAX_TASK_NAME_LEN 20
#endif
#define TASK_NAME_MAX_LENGTH configMAX_TASK_NAME_LEN

/*
***************************Globals,Structure and Enumeration ******************
*/

#if defined(ANDROID_MW) || defined(COMPANION_DEVICE) || UWBIOT_OS_NATIVE

/*
***************************Globals,Structure and Enumeration ******************
*/

/**
 **OSAL Handle structure containing details of a thread.
 *
 */
typedef struct phOsalUwb_sOsalThreadHandle
{
    pphOsalUwb_ThreadFunction_t ThreadFunction; /**<Thread function to be invoked */
    void *Params;                               /**<Parameters to the thread function */
    pthread_t ObjectHandle;                     /**<Handle of the thread object */
} phOsalUwb_sOsalThreadHandle_t;

/**
 * OSAL structure contains details of a Mutex
 */
typedef struct phOsalUwb_sOsalMutex
{
    pthread_mutex_t ObjectHandle; /**<Handle of the mutex object */
} phOsalUwb_sOsalMutex_t;

/**
 * OSAL structure contains details of a semaphore
 */
typedef struct phOsalUwb_sOsalSemaphore
{
    sem_t ObjectHandle;       /**<Handle of the semaphore object */
} phOsalUwb_sOsalSemaphore_t; /**< Variable for Structure Instance*/

#else

/**
 **OSAL Handle structure containing details of a thread.
 *
 */
typedef struct phOsalUwb_sOsalThreadHandle
{
    pphOsalUwb_ThreadFunction_t ThreadFunction; /**<Thread function to be invoked */
    void *Params;                               /**<Parameters to the thread function */
    TaskHandle_t ObjectHandle;                  /**<Handle of the Task object */
} phOsalUwb_sOsalThreadHandle_t;

/**
 * OSAL structure contains details of a Mutex
 */
typedef struct phOsalUwb_sOsalMutex
{
    QueueHandle_t ObjectHandle; /**<Handle of the mutex object */
} phOsalUwb_sOsalMutex_t;

/**
 * OSAL structure contains details of a semaphore
 */
typedef struct phOsalUwb_sOsalSemaphore
{
    SemaphoreHandle_t ObjectHandle; /**<Handle of the semaphore object */
} phOsalUwb_sOsalSemaphore_t;       /**< Variable for Structure Instance*/

#endif
#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
#endif /*  PHOSALUWB_INTERNAL_H  */
/** @} */
