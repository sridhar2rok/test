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
#include "phOsalUwb_Timer.h"
#include "phUwb_BuildConfig.h"
#include "uwb_gki.h"

/* Make sure that this has been defined in target.h */
#ifndef GKI_NUM_TIMERS
#error NO TIMERS: Must define at least 1 timer in the system!
#endif

/* Largest signed positive timer count */
#define GKI_NO_NEW_TMRS_STARTED (0x7fffffffL)
/* Marks an unused timer list entry (initial value) */
#define GKI_UNUSED_LIST_ENTRY (0x80000000L)
#define GKI_MAX_INT32         (0x7fffffffL)

#include "uwb_gki.h"
#include "uwb_gki_int.h"
#include "uwb_logging.h"

typedef struct
{
    UINT32 timerId;
    UINT16 msgType;
} tGKI_TIMER_INFO;
#if (MAX_UCI_POOL_SIZE > 0)
static tGKI_TIMER_INFO uci_timerInfo[MAX_UCI_POOL_SIZE];
static UINT8 uciActiveCnt = 0;
#endif

#if (MAX_QUICK_POOL_SIZE > 0)
static tGKI_TIMER_INFO quick_timerInfo[MAX_QUICK_POOL_SIZE];
static UINT8 quickActiveCnt = 0;
#endif

#if (MAX_UFA_POOL_SIZE > 0)
static tGKI_TIMER_INFO ufa_timerInfo[MAX_UFA_POOL_SIZE];
static UINT8 ufaActiveCnt = 0;
#endif

/*******************************************************************************
**
** Function         phUwb_gki_get_msg_from_pool
**
** Description      Retrieve event associated with the given timer
**
** Parameters       pool   - (input) pool number to which the timer belongs to
**                                  (TIMER_0, TIMER_1, TIMER_2, or TIMER_3)
**                  id     - (input) id of the timer
**                  size   - (input) Active count of timers in the pool
**                  pEvent - (output) Event associated with the timer
**                            ONLY if the timer is found in the pool
**
** Returns          TRUE if the timer is found in the pool
**                  FALSE if the timer is not found in the pool
**
*******************************************************************************/
BOOLEAN phUwb_gki_get_msg_from_pool(tGKI_TIMER_INFO *pool, UINT32 id, UINT8 size, UINT16 *pEvent)
{
    UINT8 index;
    for (index = 0; index < size; index++) {
        if (pool[index].timerId == id) {
            *pEvent = pool[index].msgType;
            return TRUE;
        }
    }
    return FALSE;
}

/*******************************************************************************
**
** Function         phUwb_gki_dele_timer_from_pool
**
** Description      Remove a timer from the pool ONLY if found
**
** Parameters       pool   - (input) pool number to which the timer belongs to
**                                  (TIMER_0, TIMER_1, TIMER_2, or TIMER_3)
**                  id     - (input) id of the timer
**                  pSize  - (input/output) Active count of timers in the pool.
**                           This will be decremented by 1 if the timer is found
**                           in the pool
**
** Returns          void
**
*******************************************************************************/
void phUwb_gki_dele_timer_from_pool(tGKI_TIMER_INFO *pool, UINT32 id, UINT8 *pSize)
{
    UINT8 index;
    BOOLEAN found = FALSE;
    UINT8 pos     = 0;
    for (index = 0; index < *pSize; index++) {
        if (pool[index].timerId == id) {
            found = TRUE;
            pos   = index;
            break;
        }
    }
    if (found == TRUE) {
        if (index + 1 < *pSize) /* Move the timers post the deleted index to utilize
                               the freed entry */
        {
            for (index = pos; (index + 1) < *pSize; index++) {
                pool[index] = pool[index + 1];
            }
        }
        else /* Zero out the freed index so that a FALSE callback is ignored */
        {
            pool[pos].timerId = 0;
        }

        (*pSize)--;
    }
}

#if (MAX_UFA_POOL_SIZE > 0)
void ufa_timer_callback(UINT32 timerid, void *pContext)
{
    phUwb_GKI_disable();
    static UINT16 nfaEvent = 0xFFFF;
    if (TRUE == phUwb_gki_get_msg_from_pool(ufa_timerInfo, timerid, ufaActiveCnt, &nfaEvent)) {
        // BT_ERROR_TRACE_2(TRACE_LAYER_GKI, "**********NFA Timer elapsed %d, event
        // %d",timerid, nfaEvent);
        phOsalUwb_Timer_Stop(timerid);
        phUwb_GKI_send_msg(UWB_TASK, TIMER_2_EVT_MASK, &nfaEvent);
        phOsalUwb_Timer_Delete(timerid);
        phUwb_gki_dele_timer_from_pool(ufa_timerInfo, timerid, &ufaActiveCnt);
    }
    phUwb_GKI_enable();
}
#endif

#if (MAX_QUICK_POOL_SIZE > 0)
static void quick_timer_callback(UINT32 timerid, void *pContext)
{
    phUwb_GKI_disable();
    static UINT16 quickEvent = 0xFFFF;
    if (TRUE == phUwb_gki_get_msg_from_pool(quick_timerInfo, timerid, quickActiveCnt, &quickEvent)) {
        // BT_ERROR_TRACE_2(TRACE_LAYER_GKI, "**********QUICK Timer elapsed %d,
        // event %d",timerid, quickEvent);
        phOsalUwb_Timer_Stop(timerid);
        phUwb_GKI_send_msg(UWB_TASK, TIMER_1_EVT_MASK, &quickEvent);
        phOsalUwb_Timer_Delete(timerid);
        phUwb_gki_dele_timer_from_pool(quick_timerInfo, timerid, &quickActiveCnt);
    }
    phUwb_GKI_enable();
}
#endif

#if (MAX_UCI_POOL_SIZE > 0)
static void uci_timer_callback(UINT32 timerid, void *pContext)
{
    phUwb_GKI_disable();
    static UINT16 nciEvent = 0xFFFF;

    if (TRUE == phUwb_gki_get_msg_from_pool(uci_timerInfo, timerid, uciActiveCnt, &nciEvent)) {
        // BT_ERROR_TRACE_2(TRACE_LAYER_GKI, "**********NCI Timer elapsed %d, event
        // %d",timerid, nciEvent);
        phOsalUwb_Timer_Stop(timerid);
        phUwb_GKI_send_msg(UWB_TASK, TIMER_0_EVT_MASK, &nciEvent);
        phOsalUwb_Timer_Delete(timerid);
        phUwb_gki_dele_timer_from_pool(uci_timerInfo, timerid, &uciActiveCnt);
    }
    phUwb_GKI_enable();
}
#endif

UINT32 phUwb_GKI_start_timer(UINT8 tnum, UINT32 ticks, UINT16 event)
{
    UINT32 gkiTimerHandle;
    pphOsalUwb_TimerCallbck_t pApplication_callback = NULL;
    phUwb_GKI_disable();
    switch (tnum) {
#if (MAX_UCI_POOL_SIZE > 0)
    case TIMER_0: // nci
        pApplication_callback = (pphOsalUwb_TimerCallbck_t)uci_timer_callback;
        break;
#endif
#if (MAX_QUICK_POOL_SIZE > 0)
    case TIMER_1: // quick
        pApplication_callback = (pphOsalUwb_TimerCallbck_t)quick_timer_callback;
        break;
#endif
#if (MAX_UFA_POOL_SIZE > 0)
    case TIMER_2: // nfa
        pApplication_callback = (pphOsalUwb_TimerCallbck_t)ufa_timer_callback;
        break;
#endif
    }
    gkiTimerHandle = phOsalUwb_Timer_Create(FALSE);
    if (gkiTimerHandle != PH_OSALUWB_TIMER_ID_INVALID) {
        if (phOsalUwb_Timer_Start(gkiTimerHandle, ticks, pApplication_callback, NULL) != 0) {
            // BT_ERROR_TRACE_0(TRACE_LAYER_GKI, "Timer Start failed");
        }
        switch (tnum) {
#if (MAX_UCI_POOL_SIZE > 0)
        case TIMER_0:
            uci_timerInfo[uciActiveCnt].timerId = gkiTimerHandle;
            uci_timerInfo[uciActiveCnt].msgType = event;
            uciActiveCnt++;
            break;
#endif
#if (MAX_QUICK_POOL_SIZE > 0)
        case TIMER_1:
            quick_timerInfo[quickActiveCnt].timerId = gkiTimerHandle;
            quick_timerInfo[quickActiveCnt].msgType = event;
            quickActiveCnt++;
            break;
#endif
#if (MAX_UFA_POOL_SIZE > 0)
        case TIMER_2:
            ufa_timerInfo[ufaActiveCnt].timerId = gkiTimerHandle;
            ufa_timerInfo[ufaActiveCnt].msgType = event;
            ufaActiveCnt++;
            // BT_ERROR_TRACE_3(TRACE_LAYER_GKI, "**********Timer started %d handle
            // %d, event %d",tnum,gkiTimerHandle, event);
            break;
#endif
        }
    }
    else {
        ALOGE("Timer creation failed");
    }
    phUwb_GKI_enable();
    return gkiTimerHandle;
}

/*******************************************************************************
**
** Function         phUwb_GKI_stop_timer
**
** Description      An application can call this function to stop one of
**                  it's four general purpose timers. There is no harm in
**                  stopping a timer that is already stopped.
**
** Parameters       tnum            - (input) timer number to be started
*(TIMER_0,
**                                              TIMER_1, TIMER_2, or TIMER_3)
**                  timer_id        - (input) id of the timer to stopped.
**                                     if the id is 0, all the timers in 'tnum'
**                                     pool will be stopped.
** Returns          void
**
*******************************************************************************/
void phUwb_GKI_stop_timer(UINT8 tnum, UINT32 timer_id)
{
    UINT32 index                = 0;
    UINT8 tempCnt               = 0;
    UINT8 *pActiveCnt           = NULL;
    UINT8 maxPoolSize           = 0;
    tGKI_TIMER_INFO *pTimerInfo = NULL;

    phUwb_GKI_disable();

#if ((MAX_UFA_POOL_SIZE > 0) || (MAX_UCI_POOL_SIZE > 0) || (MAX_QUICK_POOL_SIZE > 0))
    {
#if (MAX_UCI_POOL_SIZE > 0)
        if (tnum == TIMER_0) {
            pActiveCnt  = &uciActiveCnt;
            maxPoolSize = MAX_UCI_POOL_SIZE;
            pTimerInfo  = uci_timerInfo;
        }
#endif
#if (MAX_QUICK_POOL_SIZE > 0)
        if (tnum == TIMER_1) {
            pActiveCnt  = &quickActiveCnt;
            maxPoolSize = MAX_QUICK_POOL_SIZE;
            pTimerInfo  = quick_timerInfo;
        }
#endif
#if (MAX_UFA_POOL_SIZE > 0)
        if (tnum == TIMER_2) {
            pActiveCnt  = &ufaActiveCnt;
            maxPoolSize = MAX_UFA_POOL_SIZE;
            pTimerInfo  = ufa_timerInfo;
        }
#endif
        if ((pActiveCnt == NULL) || (pTimerInfo == NULL)) {
            phUwb_GKI_enable();
            return;
        }

        if (timer_id != 0) { /* Delete only the timer specified by the id */
            if (*pActiveCnt > 0 && *pActiveCnt <= maxPoolSize) {
                phOsalUwb_Timer_Stop(timer_id);
                phOsalUwb_Timer_Delete(timer_id);
                phUwb_gki_dele_timer_from_pool(pTimerInfo, timer_id, pActiveCnt);
            }
        }
        else { /* Delete all timers in the pool */
            tempCnt = *pActiveCnt;
            for (index = 0; index < tempCnt; index++) {
                phOsalUwb_Timer_Stop(pTimerInfo[index].timerId);
                phOsalUwb_Timer_Delete(pTimerInfo[index].timerId);
                phUwb_gki_dele_timer_from_pool(pTimerInfo, pTimerInfo[index].timerId, pActiveCnt);
            }
        }
    }
#endif
    phUwb_GKI_enable();
}

/*******************************************************************************
**
** Function         GKI_init_timer_list
**
** Description      This function is called by applications when they
**                  want to initialize a timer list.
**
** Parameters       p_timer_listq - (input) pointer to the timer list queue
**                                          object
**
** Returns          void
**
*******************************************************************************/
void phUwb_GKI_init_timer_list(TIMER_LIST_Q *p_timer_listq)
{
    p_timer_listq->p_first    = NULL;
    p_timer_listq->p_last     = NULL;
    p_timer_listq->last_ticks = 0;

    return;
}
