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
#ifndef GKI_COMMON_H
#define GKI_COMMON_H

#include "uwb_gki_target.h"

/************************************************************************
** Mailbox definitions. Each task has 4 mailboxes that are used to
** send buffers to the task.
*/
#define TASK_MBOX_0 0
#define TASK_MBOX_2 2

#define NUM_TASK_MBOX 4

/* Task States: (For OSRdyTbl) */
#define TASK_DEAD  0 /* b0000 */
#define TASK_READY 1 /* b0001 */

/********************************************************************
**  Internal Error codes
*********************************************************************/
#define GKI_ERROR_BUF_CORRUPTED       0xFFFF
#define GKI_ERROR_NOT_BUF_OWNER       0xFFFE
#define GKI_ERROR_FREEBUF_BAD_QID     0xFFFD
#define GKI_ERROR_FREEBUF_BUF_LINKED  0xFFFC
#define GKI_ERROR_SEND_MSG_BAD_DEST   0xFFFB
#define GKI_ERROR_SEND_MSG_BUF_LINKED 0xFFFA
#define GKI_ERROR_ENQUEUE_BUF_LINKED  0xFFF9
#define GKI_ERROR_DELETE_POOL_BAD_QID 0xFFF8
#define GKI_ERROR_BUF_SIZE_TOOBIG     0xFFF7
#define GKI_ERROR_BUF_SIZE_ZERO       0xFFF6

/********************************************************************
**  Misc constants
*********************************************************************/

#define GKI_MAX_INT32 (0x7fffffffL)

/********************************************************************
**  Buffer Management Data Structures
*********************************************************************/

typedef struct _buffer_hdr
{
    struct _buffer_hdr *p_next; /* next buffer in the queue */
    uint8_t q_id;               /* id of the queue */
    uint8_t task_id;            /* task which allocated the buffer*/
    uint8_t status;             /* FREE, UNLINKED or QUEUED */
    uint8_t Type;
} BUFFER_HDR_T;

typedef struct _free_queue
{
    BUFFER_HDR_T *p_first; /* first buffer in the queue */
    BUFFER_HDR_T *p_last;  /* last buffer in the queue */
    uint16_t size;         /* size of the buffers in the pool */
    uint16_t total;        /* toatal number of buffers */
    uint16_t cur_cnt;      /* number of  buffers currently allocated */
    uint16_t max_cnt;      /* maximum number of buffers allocated at any time */
} FREE_QUEUE_T;

/* Buffer related defines
 */
#define ALIGN_POOL(pl_size) ((((pl_size) + 3) / sizeof(uint32_t)) * sizeof(uint32_t))
/* Offset past header */
#define BUFFER_HDR_SIZE (sizeof(BUFFER_HDR_T))
/* Header + Magic Number */
#define BUFFER_PADDING_SIZE (sizeof(BUFFER_HDR_T) + sizeof(uint32_t))
/* pool size must allow for header */
#define MAX_USER_BUF_SIZE ((uint16_t)0xffff - BUFFER_PADDING_SIZE)
#define MAGIC_NO          0xDDBADDBA

#define BUF_STATUS_FREE     0
#define BUF_STATUS_UNLINKED 1
#define BUF_STATUS_QUEUED   2

/* Put all GKI variables into one control block
 */
typedef struct
{
    /* Task management variables
   */
    /* The stack and stack size are not used on Windows
   */

#if (GKI_NUM_FIXED_BUF_POOLS > 0)
    uint8_t bufpool0[(ALIGN_POOL(GKI_BUF0_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF0_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 1)
    uint8_t bufpool1[(ALIGN_POOL(GKI_BUF1_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF1_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 2)
    uint8_t bufpool2[(ALIGN_POOL(GKI_BUF2_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF2_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 3)
    uint8_t bufpool3[(ALIGN_POOL(GKI_BUF3_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF3_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 4)
    uint8_t bufpool4[(ALIGN_POOL(GKI_BUF4_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF4_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 5)
    uint8_t bufpool5[(ALIGN_POOL(GKI_BUF5_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF5_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 6)
    uint8_t bufpool6[(ALIGN_POOL(GKI_BUF6_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF6_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 7)
    uint8_t bufpool7[(ALIGN_POOL(GKI_BUF7_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF7_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 8)
    uint8_t bufpool8[(ALIGN_POOL(GKI_BUF8_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF8_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 9)
    uint8_t bufpool9[(ALIGN_POOL(GKI_BUF9_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF9_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 10)
    uint8_t bufpool10[(ALIGN_POOL(GKI_BUF10_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF10_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 11)
    uint8_t bufpool11[(ALIGN_POOL(GKI_BUF11_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF11_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 12)
    uint8_t bufpool12[(ALIGN_POOL(GKI_BUF12_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF12_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 13)
    uint8_t bufpool13[(ALIGN_POOL(GKI_BUF13_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF13_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 14)
    uint8_t bufpool14[(ALIGN_POOL(GKI_BUF14_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF14_MAX];
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 15)
    uint8_t bufpool15[(ALIGN_POOL(GKI_BUF15_SIZE) + BUFFER_PADDING_SIZE) * GKI_BUF15_MAX];
#endif

    int8_t *OSTName[GKI_MAX_TASKS]; /* name of the task */

    uint8_t OSRdyTbl[GKI_MAX_TASKS]; /* current state of the task */
    /* Timer related variables  */
    uint32_t OSTicks;

    /* Buffer related variables
   */
    BUFFER_HDR_T *OSTaskQFirst[GKI_MAX_TASKS][NUM_TASK_MBOX]; /* array of pointers to the first
                                                event in the task mailbox */
    BUFFER_HDR_T *OSTaskQLast[GKI_MAX_TASKS][NUM_TASK_MBOX];  /* array of pointers to the last
                                               event in the task mailbox */

    /* Define the buffer pool management variables
   */
    FREE_QUEUE_T freeq[GKI_NUM_TOTAL_BUF_POOLS];

    /* Define the buffer pool start addresses
   */
    uint8_t *pool_start[GKI_NUM_TOTAL_BUF_POOLS]; /* array of pointers to the
                                                   start of each buffer pool */
    uint8_t *pool_end[GKI_NUM_TOTAL_BUF_POOLS];   /* array of pointers to the end of
                                                 each buffer pool */
    uint16_t pool_size[GKI_NUM_TOTAL_BUF_POOLS];  /* actual size of the buffers in
                                                  a pool */

    /* Define the buffer pool access control variables */
    uint16_t pool_access_mask;                  /* Bits are set if the corresponding buffer pool is
                                a restricted pool */
    uint8_t pool_list[GKI_NUM_TOTAL_BUF_POOLS]; /* buffer pools arranged in the
                                                 order of size */
    uint8_t curr_total_no_of_pools;             /* number of fixed buf pools + current number
                                     of dynamic pools */
} tGKI_COM_CB;

/* Internal GKI function prototypes
 */
extern bool phUwb_gki_chk_buf_damage(void *);
extern bool phUwb_gki_chk_buf_owner(void *);
extern void phUwb_gki_buffer_init(void);
#endif
