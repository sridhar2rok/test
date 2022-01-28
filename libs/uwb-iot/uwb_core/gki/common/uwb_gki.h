/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *  Copyright 2019 NXP
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
#ifndef GKI_H
#define GKI_H

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#ifndef UWB_STANDALONE
#define UWB_STANDALONE FALSE
#endif

#include "phUwb_BuildConfig.h"
#include "uwb_gki_int.h"
#include "uwb_target.h"

/* Error codes */
#define GKI_SUCCESS      0x00
#define GKI_FAILURE      0x01
#define GKI_INVALID_TASK 0xF0
#define GKI_INVALID_POOL 0xFF

/************************************************************************
** Event definitions.
**
** There are 4 reserved events used to signal messages rcvd in task mailboxes.
** There are 4 reserved events used to signal timeout events.
** There are 8 general purpose events available for applications.
*/

#define TASK_MBOX_0_EVT_MASK 0x0001
#define TASK_MBOX_1_EVT_MASK 0x0002
#define TASK_MBOX_2_EVT_MASK 0x0004
#define TASK_MBOX_3_EVT_MASK 0x0008

#define TIMER_0 0
#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 3

#define TIMER_0_EVT_MASK 0x0010
#define TIMER_1_EVT_MASK 0x0020
#define TIMER_2_EVT_MASK 0x0040
#define TIMER_3_EVT_MASK 0x0080

#define APPL_EVT_0 8
#define APPL_EVT_7 15

#define EVENT_MASK(evt) ((UINT16)(0x0001 << (evt)))
#ifndef GKI_SHUTDOWN_EVT_MASK
#define GKI_SHUTDOWN_EVT_MASK EVENT_MASK(APPL_EVT_7)
#endif

/* Timer list entry callback type
 */
typedef struct TIMER_LIST_ENT TIMER_LIST_ENT;
typedef void(TIMER_CBACK)(TIMER_LIST_ENT *p_tle);

/* Define a timer list entry
 */
struct TIMER_LIST_ENT
{
    TIMER_LIST_ENT *p_next;
    TIMER_LIST_ENT *p_prev;
    TIMER_CBACK *p_cback;
    INT32 ticks;
    uintptr_t param;
    UINT16 event;
    UINT8 in_use;
    UINT32 id;
};

/* Define a timer list queue
 */
typedef struct
{
    TIMER_LIST_ENT *p_first;
    TIMER_LIST_ENT *p_last;
    INT32 last_ticks;
} TIMER_LIST_Q;

/***********************************************************************
** This queue is a general purpose buffer queue, for application use.
*/
typedef struct
{
    void *p_first;
    void *p_last;
    UINT16 count;
} BUFFER_Q;

/* Task constants
 */
#ifndef TASKPTR
#if UWBIOT_OS_FREERTOS
typedef void (*TASKPTR)(void *);
#elif UWBIOT_OS_NATIVE
typedef void *(*TASKPTR)(void *);
#else
#error "Invalid OS Type"
#endif
#endif //#ifndef TASKPTR

/* General pool accessible to GKI_getbuf() */
#define GKI_RESTRICTED_POOL 1 /* Inaccessible pool to GKI_getbuf() */

#define GKI_TASK_PRIORITY 5

typedef void *GKI_tBinarySem;

/***********************************************************************
** Function prototypes
*/

/* Task management
 */
#ifdef __cplusplus
extern "C" {
#endif

void phUwb_GKI_exit_task(UWBOSAL_TASK_HANDLE taskhandle);
void phUwb_GKI_init(void);
UINT8 phUwb_GKI_get_taskid(void);
void phUwb_GKI_shutdown(void);

/* To send buffers and events between tasks
 */
void phUwb_GKI_send_msg(UINT8, UINT16, void *);

UINT8 phUwb_GKI_send_event(UINT8, UINT16);

/* To get and release buffers, change owner and get size
 */
UINT8 phUwb_GKI_create_pool(UINT16, UINT16, UINT8, void *);
void phUwb_GKI_delete_pool(UINT8);
void *phUwb_GKI_find_buf_start(void *);
void phUwb_GKI_freebuf(void *);
void *phUwb_GKI_getbuf(UINT16);
UINT16 phUwb_GKI_get_buf_size(void *);
void *phUwb_GKI_getpoolbuf(UINT8);
void *phUwb_GKI_find_get_poolBuf(UINT16 size);

UINT8 phUwb_GKI_set_pool_permission(UINT8, UINT8);

/* User buffer queue management
 */
void *phUwb_GKI_dequeue(BUFFER_Q *);
void phUwb_GKI_enqueue(BUFFER_Q *, void *);

/* Timer management
 */
void phUwb_GKI_init_timer_list(TIMER_LIST_Q *);
UINT32 phUwb_GKI_start_timer(UINT8, UINT32, UINT16);
void phUwb_GKI_stop_timer(UINT8, UINT32);

/* Start and Stop system time tick callback
 * true for start system tick if time queue is not empty
 * false to stop system tick if time queue is empty
 */
typedef void(SYSTEM_TICK_CBACK)(bool);

/* Disable Interrupts, Enable Interrupts */
void phUwb_GKI_enable(void);
void phUwb_GKI_disable(void);

/* Allocate (Free) memory from an OS */
void *phUwb_GKI_os_malloc(UINT32);
void phUwb_GKI_os_free(void *);

/* Exception handling */
void phUwb_GKI_exception(UINT16, const char *);
GKI_tBinarySem phUwb_GKI_binary_sem_init(void);
void phUwb_GKI_binary_sem_destroy(GKI_tBinarySem semaphore);
void phUwb_GKI_binary_sem_post(GKI_tBinarySem semaphore);
void phUwb_GKI_binary_sem_wait(GKI_tBinarySem semaphore);
UINT8 phUwb_GKI_binary_sem_wait_timeout(GKI_tBinarySem semaphore, UINT32 timeout);
#ifdef __cplusplus
}
#endif
#endif
