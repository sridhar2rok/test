/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef GKI_INT_H
#define GKI_INT_H

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

//#include <pthread.h>
#include <stdbool.h>

#include "phUwb_BuildConfig.h"
#include "uwb_gki_common.h"
#include "phOsalUwb.h"

typedef struct
{
    void *GKI_mutex;
#if UWBIOT_OS_FREERTOS
    void *thread_id[GKI_MAX_TASKS];
#else
    UWBOSAL_TASK_HANDLE thread_id[GKI_MAX_TASKS];
#endif
    int no_timer_suspend; /* 1: no suspend, 0 stop calling GKI_timer_update() */
} tGKI_OS;

/* condition to exit or continue GKI_run() timer loop */
#define GKI_TIMER_TICK_RUN_COND  1
#define GKI_TIMER_TICK_STOP_COND 0
#define GKI_TIMER_TICK_EXIT_COND 2

extern void phUwb_gki_system_tick_start_stop_cback(BOOLEAN start);

/* Contains common control block as well as OS specific variables */
typedef struct
{
    tGKI_OS os;
    tGKI_COM_CB com;
} phUwb_GKI_CB_t;

#ifdef __cplusplus
extern "C" {
#endif

extern phUwb_GKI_CB_t phUwb_gki_cb;

#ifdef __cplusplus
}
#endif

#endif
