/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2018-2019 NXP
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
#ifndef GKI_TARGET_H
#define GKI_TARGET_H

#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"

/******************************************************************************
**
** Task configuration
**
******************************************************************************/
/* The number of GKI tasks in the software system. */
/*
 * At present, GKI tasks are not used.
 * But keeping for future use.
 */
#ifndef GKI_MAX_TASKS
#define GKI_MAX_TASKS 3
#endif

/* Definitions of task IDs for inter-task messaging */
#ifndef UWB_HAL_TASK
#define UWB_HAL_TASK 0
#endif

#ifndef UWB_TASK
#define UWB_TASK 3
#endif

/******************************************************************************
**
** Timer configuration
**
******************************************************************************/

/* The number of GKI timers in the software system. */
#ifndef GKI_NUM_TIMERS
#define GKI_NUM_TIMERS 3
#endif

/* A conversion value for translating ticks to calculate GKI timer.  */
#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 1000
#endif

/************************************************************************
**  Utility macros converting ticks to time with user define OS ticks per sec
**/
#ifndef GKI_MS_TO_TICKS
#define GKI_MS_TO_TICKS(x) ((x) / (1000 / TICKS_PER_SEC))
#endif

#ifndef GKI_SECS_TO_TICKS
#define GKI_SECS_TO_TICKS(x) ((x) * (TICKS_PER_SEC))
#endif

#ifndef GKI_TICKS_TO_MS
#define GKI_TICKS_TO_MS(x) ((x)*1000 / TICKS_PER_SEC)
#endif

#ifndef GKI_TICKS_TO_SECS
#define GKI_TICKS_TO_SECS(x) ((x) / TICKS_PER_SEC)
#endif

/* TICK per second from OS (OS dependent change this macro accordingly to
 * various OS) */
#ifndef OS_TICKS_PER_SEC
#define OS_TICKS_PER_SEC 1000
#endif

/************************************************************************
**  Utility macros converting ticks to time with user define OS ticks per sec
**/

#ifndef GKI_OS_TICKS_TO_MS
#define GKI_OS_TICKS_TO_MS(x) ((x)*1000 / OS_TICKS_PER_SEC)
#endif

#ifndef GKI_OS_TICKS_TO_SECS
#define GKI_OS_TICKS_TO_SECS(x)   ((x) / OS_TICKS_PER_SEC))
#endif

/* delay in ticks before stopping system tick. */
#ifndef GKI_DELAY_STOP_SYS_TICK
#define GKI_DELAY_STOP_SYS_TICK 10
#endif

/* Option to guarantee no preemption during timer expiration (most system don't
 * need this) */
#ifndef GKI_TIMER_LIST_NOPREEMPT
#define GKI_TIMER_LIST_NOPREEMPT false
#endif

/******************************************************************************
**
** Buffer configuration
**
******************************************************************************/

/* The size of the buffers in pool 0. */
#ifndef GKI_BUF0_SIZE
#define GKI_BUF0_SIZE 64
#endif

/* The number of buffers in buffer pool 0. */
#ifndef GKI_BUF0_MAX
#define GKI_BUF0_MAX 20
#endif

/* The ID of buffer pool 0. */
#ifndef GKI_POOL_ID_0
#define GKI_POOL_ID_0 0
#endif

/* The size of the buffers in pool 1. */
#ifndef GKI_BUF1_SIZE
#define GKI_BUF1_SIZE 128
#endif

/* The number of buffers in buffer pool 1. */
#ifndef GKI_BUF1_MAX
#define GKI_BUF1_MAX 10
#endif

/* The ID of buffer pool 1. */
#ifndef GKI_POOL_ID_1
#define GKI_POOL_ID_1 1
#endif

/* The size of the buffers in pool 2. */
#ifndef GKI_BUF2_SIZE
#define GKI_BUF2_SIZE 320
#endif

/* The number of buffers in buffer pool 2. */
#ifndef GKI_BUF2_MAX
#define GKI_BUF2_MAX 5
#endif

/* The ID of buffer pool 2. */
#ifndef GKI_POOL_ID_2
#define GKI_POOL_ID_2 2
#endif

/* The size of the buffers in pool 3. */
#ifndef GKI_BUF3_SIZE
#define GKI_BUF3_SIZE (0xFFB0)
#endif

/* The number of buffers in buffer pool 3. */
#ifndef GKI_BUF3_MAX
#define GKI_BUF3_MAX 0
#endif

/* The ID of buffer pool 3. */
#ifndef GKI_POOL_ID_3
#define GKI_POOL_ID_3 3
#endif

/* The size of the largest PUBLIC fixed buffer in system. */
#ifndef GKI_MAX_BUF_SIZE
#define GKI_MAX_BUF_SIZE GKI_BUF3_SIZE
#endif

/* The number of fixed GKI buffer pools.
*/
#ifndef GKI_NUM_FIXED_BUF_POOLS
#define GKI_NUM_FIXED_BUF_POOLS 3
#endif

/* The number of fixed and dynamic buffer pools */
#ifndef GKI_NUM_TOTAL_BUF_POOLS
#define GKI_NUM_TOTAL_BUF_POOLS 4
#endif

/* The buffer pool usage mask. */
#ifndef GKI_DEF_BUFPOOL_PERM_MASK
#define GKI_DEF_BUFPOOL_PERM_MASK 0xfff0
#endif

/* The buffer corruption check flag. */
#ifndef GKI_ENABLE_BUF_CORRUPTION_CHECK
#define GKI_ENABLE_BUF_CORRUPTION_CHECK TRUE
#endif

/* Maximum number of exceptions logged. */
#ifndef GKI_MAX_EXCEPTION
#define GKI_MAX_EXCEPTION 8
#endif

/* Maximum number of chars stored for each exception message. */
#ifndef GKI_MAX_EXCEPTION_MSGLEN
#define GKI_MAX_EXCEPTION_MSGLEN 64
#endif

#ifndef GKI_SEND_MSG_FROM_ISR
#define GKI_SEND_MSG_FROM_ISR FALSE
#endif

#define GKI_TRACE_0(m)
#define GKI_TRACE_1(m, p1)
#define GKI_TRACE_2(m, p1, p2)
#define GKI_TRACE_3(m, p1, p2, p3)
#define GKI_TRACE_4(m, p1, p2, p3, p4)
#define GKI_TRACE_5(m, p1, p2, p3, p4, p5)
#define GKI_TRACE_6(m, p1, p2, p3, p4, p5, p6)

#define GKI_TRACE_ERROR_0(m) phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m)
#define GKI_TRACE_ERROR_1(m, p1) \
    phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1)
#define GKI_TRACE_ERROR_2(m, p1, p2) \
    phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1, p2)
#define GKI_TRACE_ERROR_3(m, p1, p2, p3) \
    phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1, p2, p3)
#define GKI_TRACE_ERROR_4(m, p1, p2, p3, p4) \
    LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1, p2, p3, p4)
#define GKI_TRACE_ERROR_5(m, p1, p2, p3, p4, p5) \
    phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5)
#define GKI_TRACE_ERROR_6(m, p1, p2, p3, p4, p5, p6) \
    phUwb_LogMsg(TRACE_CTRL_GENERAL | TRACE_LAYER_GKI | TRACE_ORG_GKI | TRACE_TYPE_ERROR, m, p1, p2, p3, p4, p5, p6)

#endif /* GKI_TARGET_H */
