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
#include "phUwb_BuildConfig.h"
#include "uwb_logging.h"
#include "phDal4Uwb_messageQueueLib.h"
#include "phUwbTypes.h"
#include "uwb_gki.h"
#include "uwb_int.h"

#if (GKI_NUM_TOTAL_BUF_POOLS > 16)
#error Number of pools out of range (16 Max)!
#endif

#if (BTU_STACK_LITE_ENABLED == FALSE)
static void phUwb_gki_add_to_pool_list(UINT8 pool_id);
static void gki_remove_from_pool_list(UINT8 pool_id);
#endif /*  BTU_STACK_LITE_ENABLED == FALSE */

extern phUwbtask_Control_t *gp_uwbtask_ctrl;

/*******************************************************************************
**
** Function         phUwb_gki_init_free_queue
**
** Description      Internal function called at startup to initialize a free
**                  queue. It is called once for each free queue.
**
** Returns          void
**
*******************************************************************************/
static void phUwb_gki_init_free_queue(UINT8 id, UINT16 size, UINT16 total, void *p_mem)
{
    UINT16 i;
    UINT16 act_size;
    BUFFER_HDR_T *hdr;
    BUFFER_HDR_T *hdr1 = NULL;
    UINT32 *magic;
    INT32 tempsize    = size;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    /* Ensure an even number of longwords */
    tempsize = (INT32)ALIGN_POOL(size);
    act_size = (UINT16)(tempsize + BUFFER_PADDING_SIZE);

    /* Remember pool start and end addresses */
    if (p_mem) {
        p_cb->pool_start[id] = (UINT8 *)p_mem;
        p_cb->pool_end[id]   = (UINT8 *)p_mem + (act_size * total);
    }

    p_cb->pool_size[id] = act_size;

    p_cb->freeq[id].size    = (UINT16)tempsize;
    p_cb->freeq[id].total   = total;
    p_cb->freeq[id].cur_cnt = 0;
    p_cb->freeq[id].max_cnt = 0;

    /* Initialize  index table */
    if (p_mem) {
        hdr                     = (BUFFER_HDR_T *)p_mem;
        p_cb->freeq[id].p_first = hdr;
        for (i = 0; i < total; i++) {
            hdr->task_id = GKI_INVALID_TASK;
            hdr->q_id    = id;
            hdr->status  = BUF_STATUS_FREE;
            magic        = (UINT32 *)((UINT8 *)hdr + BUFFER_HDR_SIZE + tempsize);
            *magic       = MAGIC_NO;
            hdr1         = hdr;
            hdr          = (BUFFER_HDR_T *)((UINT8 *)hdr + act_size);
            hdr1->p_next = hdr;
        }
        if (hdr1 != NULL)
            hdr = hdr1;
        hdr->p_next            = NULL;
        p_cb->freeq[id].p_last = hdr;
    }
    return;
}

static bool phUwb_gki_alloc_free_queue(UINT8 id)
{
    FREE_QUEUE_T *Q;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    Q = &p_cb->freeq[p_cb->pool_list[id]];

    if (Q->p_first == 0) {
        void *p_mem = phUwb_GKI_os_malloc((Q->size + BUFFER_PADDING_SIZE) * Q->total);
        if (p_mem) {
            // re-initialize the queue with allocated memory
            phUwb_gki_init_free_queue(id, Q->size, Q->total, p_mem);
            return true;
        }
        phUwb_GKI_exception(GKI_ERROR_BUF_SIZE_TOOBIG, "phUwb_gki_alloc_free_queue: Not enough memory");
    }
    return false;
}

/*******************************************************************************
**
** Function         phUwb_gki_buffer_init
**
** Description      Called once internally by GKI at startup to initialize all
**                  buffers and free buffer pools.
**
** Returns          void
**
*******************************************************************************/
void phUwb_gki_buffer_init(void)
{
    UINT8 i, tt, mb;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    /* Initialize mailboxes */
    for (tt = 0; tt < GKI_MAX_TASKS; tt++) {
        for (mb = 0; mb < NUM_TASK_MBOX; mb++) {
            p_cb->OSTaskQFirst[tt][mb] = NULL;
            p_cb->OSTaskQLast[tt][mb]  = NULL;
        }
    }

    for (tt = 0; tt < GKI_NUM_TOTAL_BUF_POOLS; tt++) {
        p_cb->pool_start[tt] = NULL;
        p_cb->pool_end[tt]   = NULL;
        p_cb->pool_size[tt]  = 0;

        p_cb->freeq[tt].p_first = 0;
        p_cb->freeq[tt].p_last  = 0;
        p_cb->freeq[tt].size    = 0;
        p_cb->freeq[tt].total   = 0;
        p_cb->freeq[tt].cur_cnt = 0;
        p_cb->freeq[tt].max_cnt = 0;
    }

    /* Use default from target.h */
    p_cb->pool_access_mask = GKI_DEF_BUFPOOL_PERM_MASK;

#if (GKI_NUM_FIXED_BUF_POOLS > 0)
    phUwb_gki_init_free_queue(0, GKI_BUF0_SIZE, GKI_BUF0_MAX, p_cb->bufpool0);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 1)
    phUwb_gki_init_free_queue(1, GKI_BUF1_SIZE, GKI_BUF1_MAX, p_cb->bufpool1);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 2)
    phUwb_gki_init_free_queue(2, GKI_BUF2_SIZE, GKI_BUF2_MAX, p_cb->bufpool2);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 3)
    phUwb_gki_init_free_queue(3, GKI_BUF3_SIZE, GKI_BUF3_MAX, p_cb->bufpool3);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 4)
    phUwb_gki_init_free_queue(4, GKI_BUF4_SIZE, GKI_BUF4_MAX, p_cb->bufpool4);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 5)
    phUwb_gki_init_free_queue(5, GKI_BUF5_SIZE, GKI_BUF5_MAX, p_cb->bufpool5);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 6)
    phUwb_gki_init_free_queue(6, GKI_BUF6_SIZE, GKI_BUF6_MAX, p_cb->bufpool6);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 7)
    phUwb_gki_init_free_queue(7, GKI_BUF7_SIZE, GKI_BUF7_MAX, p_cb->bufpool7);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 8)
    phUwb_gki_init_free_queue(8, GKI_BUF8_SIZE, GKI_BUF8_MAX, p_cb->bufpool8);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 9)
    phUwb_gki_init_free_queue(9, GKI_BUF9_SIZE, GKI_BUF9_MAX, p_cb->bufpool9);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 10)
    phUwb_gki_init_free_queue(10, GKI_BUF10_SIZE, GKI_BUF10_MAX, p_cb->bufpool10);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 11)
    phUwb_gki_init_free_queue(11, GKI_BUF11_SIZE, GKI_BUF11_MAX, p_cb->bufpool11);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 12)
    phUwb_gki_init_free_queue(12, GKI_BUF12_SIZE, GKI_BUF12_MAX, p_cb->bufpool12);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 13)
    phUwb_gki_init_free_queue(13, GKI_BUF13_SIZE, GKI_BUF13_MAX, p_cb->bufpool13);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 14)
    phUwb_gki_init_free_queue(14, GKI_BUF14_SIZE, GKI_BUF14_MAX, p_cb->bufpool14);
#endif

#if (GKI_NUM_FIXED_BUF_POOLS > 15)
    phUwb_gki_init_free_queue(15, GKI_BUF15_SIZE, GKI_BUF15_MAX, p_cb->bufpool15);
#endif

    /* add pools to the pool_list which is arranged in the order of size */
    for (i = 0; i < GKI_NUM_FIXED_BUF_POOLS; i++) {
        p_cb->pool_list[i] = i;
    }

    p_cb->curr_total_no_of_pools = GKI_NUM_FIXED_BUF_POOLS;

    return;
}

/*******************************************************************************
**
** Function         GKI_getbuf
**
** Description      Called by an application to get a free buffer which
**                  is of size greater or equal to the requested size.
**
**                  Note: This routine only takes buffers from public pools.
**                        It will not use any buffers from pools
**                        marked GKI_RESTRICTED_POOL.
**
** Parameters       size - (input) number of bytes needed.
**
** Returns          A pointer to the buffer, or NULL if none available
**
*******************************************************************************/
void *phUwb_GKI_getbuf(UINT16 size)
{
    UINT8 i;
    FREE_QUEUE_T *Q;
    BUFFER_HDR_T *p_hdr;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    if (size == 0) {
        phUwb_GKI_exception(GKI_ERROR_BUF_SIZE_ZERO, "getbuf: Size is zero");
        return (NULL);
    }

    /* Find the first buffer pool that is public that can hold the desired size */
    for (i = 0; i < p_cb->curr_total_no_of_pools; i++) {
        if (size <= p_cb->freeq[p_cb->pool_list[i]].size)
            break;
    }

    if (i == p_cb->curr_total_no_of_pools) {
        phUwb_GKI_exception(GKI_ERROR_BUF_SIZE_TOOBIG, "getbuf: Size is too big");
        return (NULL);
    }

    /* Make sure the buffers aren't disturbed til finished with allocation */
    phUwb_GKI_disable();

    /* search the public buffer pools that are big enough to hold the size
   * until a free buffer is found */
    for (; i < p_cb->curr_total_no_of_pools; i++) {
        /* Only look at PUBLIC buffer pools (bypass RESTRICTED pools) */
        if (((UINT16)1 << p_cb->pool_list[i]) & p_cb->pool_access_mask)
            continue;

        Q = &p_cb->freeq[p_cb->pool_list[i]];
        if (Q->cur_cnt < Q->total) {
            if (Q->p_first == 0 && phUwb_gki_alloc_free_queue(i) != true) {
                ALOGE("out of buffer");
                phUwb_GKI_enable();
                return NULL;
            }

            if (Q->p_first == 0) {
                /* gki_alloc_free_queue() failed to alloc memory */
                ALOGE("fail alloc free queue");
                phUwb_GKI_enable();
                return NULL;
            }

            p_hdr      = Q->p_first;
            Q->p_first = p_hdr->p_next;

            if (!Q->p_first)
                Q->p_last = NULL;

            if (++Q->cur_cnt > Q->max_cnt)
                Q->max_cnt = Q->cur_cnt;

            phUwb_GKI_enable();

            p_hdr->task_id = phUwb_GKI_get_taskid();

            p_hdr->status = BUF_STATUS_UNLINKED;
            p_hdr->p_next = NULL;
            p_hdr->Type   = 0;
            return ((void *)((UINT8 *)p_hdr + BUFFER_HDR_SIZE));
        }
    }

    ALOGE("unable to allocate buffer!!!!!");

    phUwb_GKI_enable();

    return (NULL);
}

/*******************************************************************************
**
** Function         GKI_getpoolbuf
**
** Description      Called by an application to get a free buffer from
**                  a specific buffer pool.
**
**                  Note: If there are no more buffers available from the pool,
**                        the public buffers are searched for an available
**                        buffer.
**
** Parameters       pool_id - (input) pool ID to get a buffer out of.
**
** Returns          A pointer to the buffer, or NULL if none available
**
*******************************************************************************/
void *phUwb_GKI_getpoolbuf(UINT8 pool_id)
{
    FREE_QUEUE_T *Q;
    BUFFER_HDR_T *p_hdr;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    if (pool_id >= GKI_NUM_TOTAL_BUF_POOLS)
        return (NULL);

    /* Make sure the buffers aren't disturbed til finished with allocation */
    phUwb_GKI_disable();

    Q = &p_cb->freeq[pool_id];
    if (Q->cur_cnt < Q->total) {
        if (Q->p_first == 0 && phUwb_gki_alloc_free_queue(pool_id) != true) {
            phUwb_GKI_enable();
            return NULL;
        }

        if (Q->p_first == 0) {
            /* gki_alloc_free_queue() failed to alloc memory */
            ALOGE("fail alloc free queue");
            phUwb_GKI_enable();
            return NULL;
        }

        p_hdr      = Q->p_first;
        Q->p_first = p_hdr->p_next;

        if (!Q->p_first)
            Q->p_last = NULL;

        if (++Q->cur_cnt > Q->max_cnt)
            Q->max_cnt = Q->cur_cnt;

        phUwb_GKI_enable();

        p_hdr->task_id = phUwb_GKI_get_taskid();

        p_hdr->status = BUF_STATUS_UNLINKED;
        p_hdr->p_next = NULL;
        p_hdr->Type   = 0;

        return ((void *)((UINT8 *)p_hdr + BUFFER_HDR_SIZE));
    }

    /* If here, no buffers in the specified pool */
    phUwb_GKI_enable();

    /* try for free buffers in public pools */
    return (phUwb_GKI_getbuf(p_cb->freeq[pool_id].size));
}

/*******************************************************************************
**
** Function         phUwb_GKI_get_poolId
**
** Description      Called by an application to get a pool ID based on the requested size
**
**
** Parameters       length - (input) number of bytes needed.
**
** Returns          Pool ID
**
*******************************************************************************/
UINT8 phUwb_GKI_get_poolId(UINT16 length)
{
    UINT8 pool_id;
    if (length <= GKI_BUF0_SIZE)
        pool_id = GKI_POOL_ID_0;
    else if (length <= GKI_BUF1_SIZE)
        pool_id = GKI_POOL_ID_1;
    else if (length <= GKI_BUF2_SIZE)
        pool_id = GKI_POOL_ID_2;
    else
        pool_id = 0xFF;
    return pool_id;
}

/*******************************************************************************
**
** Function         phUwb_GKI_find_get_poolBuf
**
** Description      Called by an application to get a gki buffer based on size
**
**
** Parameters       length - (input) number of bytes needed.
**
** Returns          gki buffer based on the size
**
*******************************************************************************/
void *phUwb_GKI_find_get_poolBuf(UINT16 size)
{
    UINT8 *p_buf  = NULL;
    UINT8 pool_id = phUwb_GKI_get_poolId(size);
    if (pool_id != 0xFF) {
        if ((p_buf = (UINT8 *)phUwb_GKI_getpoolbuf(pool_id)) == NULL) {
        retry:
            if (pool_id != (GKI_NUM_FIXED_BUF_POOLS - 1)) {
                pool_id++;
                if ((p_buf = (UINT8 *)phUwb_GKI_getpoolbuf(pool_id)) == NULL)
                    goto retry;
            }
            else {
                ALOGE("unable to allocate buffer!!!!!");
            }
        }
    }
    return (void *)p_buf;
}

/*******************************************************************************
**
** Function         GKI_freebuf
**
** Description      Called by an application to return a buffer to the free
**                  pool.
**
** Parameters       p_buf - (input) address of the beginning of a buffer.
**
** Returns          void
**
*******************************************************************************/
void phUwb_GKI_freebuf(void *p_buf)
{
    FREE_QUEUE_T *Q;
    BUFFER_HDR_T *p_hdr;

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (!p_buf || phUwb_gki_chk_buf_damage(p_buf)) {
        phUwb_GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Free - Buf Corrupted");
        return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED) {
        phUwb_GKI_exception(GKI_ERROR_FREEBUF_BUF_LINKED, "Freeing Linked Buf");
        return;
    }

    if (p_hdr->q_id >= GKI_NUM_TOTAL_BUF_POOLS) {
        phUwb_GKI_exception(GKI_ERROR_FREEBUF_BAD_QID, "Bad Buf QId");
        return;
    }

    phUwb_GKI_disable();

    /*
  ** Release the buffer
  */
    Q = &phUwb_gki_cb.com.freeq[p_hdr->q_id];
    if (Q->p_last)
        Q->p_last->p_next = p_hdr;
    else
        Q->p_first = p_hdr;

    Q->p_last      = p_hdr;
    p_hdr->p_next  = NULL;
    p_hdr->status  = BUF_STATUS_FREE;
    p_hdr->task_id = GKI_INVALID_TASK;
    if (Q->cur_cnt > 0)
        Q->cur_cnt--;

    phUwb_GKI_enable();

    return;
}

/*******************************************************************************
**
** Function         GKI_get_buf_size
**
** Description      Called by an application to get the size of a buffer.
**
** Parameters       p_buf - (input) address of the beginning of a buffer.
**
** Returns          the size of the buffer
**
*******************************************************************************/
UINT16 phUwb_GKI_get_buf_size(void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)p_buf - BUFFER_HDR_SIZE);

    if ((uintptr_t)p_hdr & 1)
        return (0);

    if (p_hdr->q_id < GKI_NUM_TOTAL_BUF_POOLS) {
        return (phUwb_gki_cb.com.freeq[p_hdr->q_id].size);
    }

    return (0);
}

/*******************************************************************************
**
** Function         phUwb_gki_chk_buf_damage
**
** Description      Called internally by OSS to check for buffer corruption.
**
** Returns          TRUE if there is a problem, else FALSE
**
*******************************************************************************/
bool phUwb_gki_chk_buf_damage(void *p_buf)
{
#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)

    UINT32 *magic;
    magic = (UINT32 *)((UINT8 *)p_buf + phUwb_GKI_get_buf_size(p_buf));

    if ((uintptr_t)magic & 1)
        return true;

    if (*magic == MAGIC_NO)
        return false;

    return true;

#else

    return false;

#endif
}

void phUwb_GKI_send_msg(UINT8 task_id, UINT16 mbox, void *pmsg)
{
    static phLibUwb_Message_t msg;
    intptr_t pMsgQ = 0;

    switch (mbox) {
    case TIMER_0_EVT_MASK:
    case TIMER_1_EVT_MASK:
    case TIMER_2_EVT_MASK:
    case TIMER_3_EVT_MASK:
    case UWB_TASK_EVT_TRANSPORT_READY:
    case GKI_SHUTDOWN_EVT_MASK:
        msg.eMsgType = (UINT16)mbox;
        break;
    default:
        msg.eMsgType = (UINT16)EVENT_MASK(mbox);
    }

    msg.pMsgData = pmsg;
    msg.Size     = 0;

    if (task_id == UWB_TASK) {
        pMsgQ = gp_uwbtask_ctrl->pMsgQHandle;
    }

    if (phDal4Uwb_msgsnd(pMsgQ, &msg, 0) != 0) {
        phUwb_GKI_exception(GKI_ERROR_SEND_MSG_BUF_LINKED, "phDal4Uwb_msgsnd() failed");
    }
}

/*******************************************************************************
**
** Function         GKI_enqueue
**
** Description      Enqueue a buffer at the tail of the queue
**
** Parameters:      p_q  -  (input) pointer to a queue.
**                  p_buf - (input) address of the buffer to enqueue
**
** Returns          void
**
*******************************************************************************/
void phUwb_GKI_enqueue(BUFFER_Q *p_q, void *p_buf)
{
    BUFFER_HDR_T *p_hdr;

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (phUwb_gki_chk_buf_damage(p_buf)) {
        phUwb_GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Enqueue - Buffer corrupted");
        return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)p_buf - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED) {
        phUwb_GKI_exception(GKI_ERROR_ENQUEUE_BUF_LINKED, "Eneueue - buf already linked");
        return;
    }

    phUwb_GKI_disable();

    /* Since the queue is exposed (C vs C++), keep the pointers in exposed format
   */
    if (p_q->p_first) {
        BUFFER_HDR_T *p_last_hdr = (BUFFER_HDR_T *)((UINT8 *)p_q->p_last - BUFFER_HDR_SIZE);
        p_last_hdr->p_next       = p_hdr;
    }
    else
        p_q->p_first = p_buf;

    p_q->p_last = p_buf;
    p_q->count++;

    p_hdr->p_next = NULL;
    p_hdr->status = BUF_STATUS_QUEUED;

    phUwb_GKI_enable();

    return;
}

/*******************************************************************************
**
** Function         GKI_dequeue
**
** Description      Dequeues a buffer from the head of a queue
**
** Parameters:      p_q  - (input) pointer to a queue.
**
** Returns          NULL if queue is empty, else buffer
**
*******************************************************************************/
void *phUwb_GKI_dequeue(BUFFER_Q *p_q)
{
    BUFFER_HDR_T *p_hdr;

    phUwb_GKI_disable();

    if (!p_q || !p_q->count) {
        phUwb_GKI_enable();
        return (NULL);
    }

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)p_q->p_first - BUFFER_HDR_SIZE);

    /* Keep buffers such that GKI header is invisible
   */
    if (p_hdr->p_next)
        p_q->p_first = ((UINT8 *)p_hdr->p_next + BUFFER_HDR_SIZE);
    else {
        p_q->p_first = NULL;
        p_q->p_last  = NULL;
    }

    p_q->count--;

    p_hdr->p_next = NULL;
    p_hdr->status = BUF_STATUS_UNLINKED;

    phUwb_GKI_enable();

    return ((UINT8 *)p_hdr + BUFFER_HDR_SIZE);
}

/*******************************************************************************
**
** Function         GKI_find_buf_start
**
** Description      This function is called with an address inside a buffer,
**                  and returns the start address ofthe buffer.
**
**                  The buffer should be one allocated from one of GKI's pools.
**
** Parameters:      p_user_area - (input) address of anywhere in a GKI buffer.
**
** Returns          void * - Address of the beginning of the specified buffer if
**                           successful, otherwise NULL if unsuccessful
**
*******************************************************************************/
void *phUwb_GKI_find_buf_start(void *p_user_area)
{
    UINT16 xx, size;
    UINT32 yy;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;
    UINT8 *p_ua       = (UINT8 *)p_user_area;

    for (xx = 0; xx < GKI_NUM_TOTAL_BUF_POOLS; xx++) {
        if ((p_ua > p_cb->pool_start[xx]) && (p_ua < p_cb->pool_end[xx])) {
            yy = (UINT32)(p_ua - p_cb->pool_start[xx]);

            size = p_cb->pool_size[xx];

            yy = (yy / size) * size;

            return ((void *)(p_cb->pool_start[xx] + yy + sizeof(BUFFER_HDR_T)));
        }
    }

    /* If here, invalid address - not in one of our buffers */
    phUwb_GKI_exception(GKI_ERROR_BUF_SIZE_ZERO, "GKI_get_buf_start:: bad addr");

    return (NULL);
}

/********************************************************
 * The following functions are not needed for light stack
 *********************************************************/
#ifndef BTU_STACK_LITE_ENABLED
#define BTU_STACK_LITE_ENABLED FALSE
#endif

#if (BTU_STACK_LITE_ENABLED == FALSE)

/*******************************************************************************
**
** Function         phUwb_GKI_set_pool_permission
**
** Description      This function is called to set or change the permissions for
**                  the specified pool ID.
**
** Parameters       pool_id - (input) pool ID to be set or changed
**                  permission - (input) GKI_PUBLIC_POOL or GKI_RESTRICTED_POOL
**
** Returns          GKI_SUCCESS if successful
**                  GKI_INVALID_POOL if unsuccessful
**
*******************************************************************************/
UINT8 phUwb_GKI_set_pool_permission(UINT8 pool_id, UINT8 permission)
{
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    if (pool_id < GKI_NUM_TOTAL_BUF_POOLS) {
        if (permission == GKI_RESTRICTED_POOL)
            p_cb->pool_access_mask = (UINT16)(p_cb->pool_access_mask | (1 << pool_id));

        else /* mark the pool as public */
            p_cb->pool_access_mask = (UINT16)(p_cb->pool_access_mask & ~(1 << pool_id));

        return (GKI_SUCCESS);
    }
    else
        return (GKI_INVALID_POOL);
}

/*******************************************************************************
**
** Function         phUwb_gki_add_to_pool_list
**
** Description      Adds pool to the pool list which is arranged in the
**                  order of size
**
** Returns          void
**
*******************************************************************************/
static void phUwb_gki_add_to_pool_list(UINT8 pool_id)
{
    INT32 i, j;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    /* Find the position where the specified pool should be inserted into the list
   */
    for (i = 0; i < p_cb->curr_total_no_of_pools; i++) {
        if (p_cb->freeq[pool_id].size <= p_cb->freeq[p_cb->pool_list[i]].size)
            break;
    }

    /* Insert the new buffer pool ID into the list of pools */
    for (j = p_cb->curr_total_no_of_pools; j > i; j--) {
        p_cb->pool_list[j] = p_cb->pool_list[j - 1];
    }

    p_cb->pool_list[i] = pool_id;

    return;
}

/*******************************************************************************
**
** Function         gki_remove_from_pool_list
**
** Description      Removes pool from the pool list. Called when a pool is
**                  deleted
**
** Returns          void
**
*******************************************************************************/
static void gki_remove_from_pool_list(UINT8 pool_id)
{
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;
    UINT8 i;

    for (i = 0; i < p_cb->curr_total_no_of_pools; i++) {
        if (pool_id == p_cb->pool_list[i])
            break;
    }

    while (i < (p_cb->curr_total_no_of_pools - 1)) {
        p_cb->pool_list[i] = p_cb->pool_list[i + 1];
        i++;
    }

    return;
}

#if (GKI_SEND_MSG_FROM_ISR == TRUE)
/*******************************************************************************
**
** Function         GKI_isend_msg
**
** Description      Called from interrupt context to send a buffer to a task
**
** Returns          Nothing
**
*******************************************************************************/
void GKI_isend_msg(UINT8 task_id, UINT8 mbox, void *msg)
{
    BUFFER_HDR_T *p_hdr;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    /* If task non-existant or not started, drop buffer */
    if ((task_id >= GKI_MAX_TASKS) || (mbox >= NUM_TASK_MBOX) || (p_cb->OSRdyTbl[task_id] == TASK_DEAD)) {
        phUwb_GKI_exception(GKI_ERROR_SEND_MSG_BAD_DEST, "Sending to unknown dest");
        phUwb_GKI_freebuf(msg);
        return;
    }

#if (GKI_ENABLE_BUF_CORRUPTION_CHECK == TRUE)
    if (phUwb_gki_chk_buf_damage(msg)) {
        phUwb_GKI_exception(GKI_ERROR_BUF_CORRUPTED, "Send - Buffer corrupted");
        return;
    }
#endif

#if (GKI_ENABLE_OWNER_CHECK == TRUE)
    if (phUwb_gki_chk_buf_owner(msg)) {
        phUwb_GKI_exception(GKI_ERROR_NOT_BUF_OWNER, "Send by non-owner");
        return;
    }
#endif

    p_hdr = (BUFFER_HDR_T *)((UINT8 *)msg - BUFFER_HDR_SIZE);

    if (p_hdr->status != BUF_STATUS_UNLINKED) {
        phUwb_GKI_exception(GKI_ERROR_SEND_MSG_BUF_LINKED, "Send - buffer linked");
        return;
    }

    if (p_cb->OSTaskQFirst[task_id][mbox])
        p_cb->OSTaskQLast[task_id][mbox]->p_next = p_hdr;
    else
        p_cb->OSTaskQFirst[task_id][mbox] = p_hdr;

    p_cb->OSTaskQLast[task_id][mbox] = p_hdr;

    p_hdr->p_next  = NULL;
    p_hdr->status  = BUF_STATUS_QUEUED;
    p_hdr->task_id = task_id;

    GKI_isend_event(task_id, (UINT16)EVENT_MASK(mbox));

    return;
}
#endif

/*******************************************************************************
**
** Function         GKI_create_pool
**
** Description      Called by applications to create a buffer pool.
**
** Parameters:      size - (input) length (in bytes) of each buffer in the pool
**                  count - (input) number of buffers to allocate for the pool
**                  permission - (input) restricted or public access?
**                                      (GKI_PUBLIC_POOL or GKI_RESTRICTED_POOL)
**                  p_mem_pool - (input) pointer to an OS memory pool, NULL if
**                                       not provided
**
** Returns          the buffer pool ID, which should be used in calls to
**                  GKI_getpoolbuf(). If a pool could not be created, this
**                  function returns 0xff.
**
*******************************************************************************/
UINT8 phUwb_GKI_create_pool(UINT16 size, UINT16 count, UINT8 permission, void *p_mem_pool)
{
    UINT8 xx;
    UINT32 mem_needed;
    INT32 tempsize    = size;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    /* First make sure the size of each pool has a valid size with room for the
   * header info */
    if (size > MAX_USER_BUF_SIZE)
        return (GKI_INVALID_POOL);

    /* First, look for an unused pool */
    for (xx = 0; xx < GKI_NUM_TOTAL_BUF_POOLS; xx++) {
        if (!p_cb->pool_start[xx])
            break;
    }

    if (xx == GKI_NUM_TOTAL_BUF_POOLS)
        return (GKI_INVALID_POOL);

    /* Ensure an even number of longwords */
    tempsize = (INT32)ALIGN_POOL(size);

    mem_needed = (tempsize + BUFFER_PADDING_SIZE) * count;

    if (!p_mem_pool)
        p_mem_pool = phUwb_GKI_os_malloc(mem_needed);

    if (p_mem_pool) {
        /* Initialize the new pool */
        phUwb_gki_init_free_queue(xx, size, count, p_mem_pool);
        phUwb_gki_add_to_pool_list(xx);
        (void)phUwb_GKI_set_pool_permission(xx, permission);
        p_cb->curr_total_no_of_pools++;

        return (xx);
    }
    else
        return (GKI_INVALID_POOL);
}

/*******************************************************************************
**
** Function         GKI_delete_pool
**
** Description      Called by applications to delete a buffer pool.  The
**                  function calls the operating specific function to free the
**                  actual memory. An exception is generated if an error is
**                  detected.
**
** Parameters:      pool_id - (input) Id of the poll being deleted.
**
** Returns          void
**
*******************************************************************************/
void phUwb_GKI_delete_pool(UINT8 pool_id)
{
    FREE_QUEUE_T *Q;
    tGKI_COM_CB *p_cb = &phUwb_gki_cb.com;

    if ((pool_id >= GKI_NUM_TOTAL_BUF_POOLS) || (!p_cb->pool_start[pool_id]))
        return;

    phUwb_GKI_disable();
    Q = &p_cb->freeq[pool_id];

    if (!Q->cur_cnt) {
        Q->size    = 0;
        Q->total   = 0;
        Q->cur_cnt = 0;
        Q->max_cnt = 0;
        Q->p_first = NULL;
        Q->p_last  = NULL;

        phUwb_GKI_os_free(p_cb->pool_start[pool_id]);

        p_cb->pool_start[pool_id] = NULL;
        p_cb->pool_end[pool_id]   = NULL;
        p_cb->pool_size[pool_id]  = 0;

        gki_remove_from_pool_list(pool_id);
        p_cb->curr_total_no_of_pools--;
    }
    else
        phUwb_GKI_exception(GKI_ERROR_DELETE_POOL_BAD_QID, "Deleting bad pool");

    phUwb_GKI_enable();

    return;
}

#endif /*  BTU_STACK_LITE_ENABLED == FALSE */
