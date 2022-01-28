/*
 * Copyright (C) 2012-2020 NXP Semiconductors
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
 *  OSAL header files related to memory, debug, random, semaphore and mutex
 * functions.
 */

#ifndef PHUWBCOMMON_H
#define PHUWBCOMMON_H

/*
************************* Include Files ****************************************
*/

#include <phDal4Uwb_messageQueueLib.h>
#include <phOsalUwb.h>
#include <phOsalUwb_Timer.h>
#include <phUwbCompId.h>
#include <phUwbStatus.h>

/*
 *  information to configure OSAL
 */
typedef struct phOsalUwb_Config
{
    uint8_t *pLogFile;            /* Log File Name*/
    uintptr_t dwCallbackThreadId; /* Client ID to which message is posted */
} phOsalUwb_Config_t, *pphOsalUwb_Config_t /* Pointer to #phOsalUwb_Config_t */;

/*
 * Deferred call declaration.
 * This type of API is called from ClientApplication (main thread) to notify
 * specific callback.
 */
typedef void (*pphOsalUwb_DeferFuncPointer_t)(void *);

/*
 * Deferred message specific info declaration.
 */
typedef struct phOsalUwb_DeferedCallInfo
{
    pphOsalUwb_DeferFuncPointer_t pDeferedCall; /* pointer to Deferred callback */
    void *pParam;                               /* contains timer message specific details*/
} phOsalUwb_DeferedCallInfo_t;

/*
 * States in which a OSAL timer exist.
 */
typedef enum
{
    eTimerIdle    = 0,     /* Indicates Initial state of timer */
    eTimerRunning = 1,     /* Indicate timer state when started */
    eTimerStopped = 2      /* Indicates timer state when stopped */
} phOsalUwb_TimerStates_t; /* Variable representing State of timer */

/*
 **Timer Handle structure containing details of a timer.
 */
typedef struct phOsalUwb_TimerHandle
{
    uint32_t TimerId;   /* ID of the timer */
    void *hTimerHandle; /* Handle of the timer */
    /* Timer callback function to be invoked */
    pphOsalUwb_TimerCallbck_t Application_callback;
    void *pContext;                 /* Parameter to be passed to the callback function */
    phOsalUwb_TimerStates_t eState; /* Timer states */
    /* Osal Timer message posted on User Thread */
    phLibUwb_Message_t tOsalMessage;
    /* Deferred Call structure to Invoke Callback function */
    phOsalUwb_DeferedCallInfo_t tDeferedCallInfo;
    /* Variables for Structure Instance and Structure Ptr */
} phOsalUwb_TimerHandle_t, *pphOsalUwb_TimerHandle_t;

#endif /*  PHOSALUWB_H  */
