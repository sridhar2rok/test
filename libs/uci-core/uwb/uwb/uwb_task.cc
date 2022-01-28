/*
*
* Copyright 2018-2020 NXP.
*
* NXP Confidential. This software is owned or controlled by NXP and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading,installing, activating and/or otherwise
* using the software, you are agreeing that you have read,and that you agree to
* comply with and are bound by, such license terms. If you do not agree to be
* bound by the applicable license terms, then you may not retain, install, activate
* or otherwise use the software.
*
*/

/******************************************************************************
 *
 *  Entry point for UWB_TASK
 *
 ******************************************************************************/
#include "uwb_gki.h"
#include "uwb_target.h"

#include "uwb_api.h"
#include "uwb_hal_api.h"
#include "uwb_int.h"
#include "uci_hmsgs.h"

#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "uwb_osal_common.h"
#include "phNxpLogApis_UciCore.h"

phUwbtask_Control_t *gp_uwbtask_ctrl;

/*******************************************************************************
**
** Function         uwb_start_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution is in SECONDS! (Even
**                          though the timer structure field is ticks)
**
** Returns          void
**
*******************************************************************************/
void uwb_start_timer(TIMER_LIST_ENT* p_tle, uint16_t type, uint32_t timeout) {
  p_tle->id = phUwb_GKI_start_timer(UWB_TIMER_ID, GKI_SECS_TO_TICKS (timeout), type);
  p_tle->in_use = TRUE;
}

/*******************************************************************************
**
** Function         uwb_process_timer_evt
**
** Description      Process uwb GKI timer event
**
** Returns          void
**
*******************************************************************************/
void uwb_process_timer_evt(uint16_t event) {
 /*   switch (event) {
    case UWB_TTYPE_UCI_WAIT_RSP:
    uwb_ucif_cmd_timeout();
    break;

    default:
    UCI_TRACE_D("uwb_process_timer_evt: event (0x%04x)",
                event);
    }*/
}
/*******************************************************************************
**
** Function         uwb_stop_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void uwb_stop_timer(TIMER_LIST_ENT* p_tle) {
  if(p_tle->id)
  {
      phUwb_GKI_stop_timer (UWB_TIMER_ID, p_tle->id);
      p_tle->id = 0;
      p_tle->in_use = FALSE;
  }
}

/*******************************************************************************
**
** Function         uwb_start_quick_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution depends on including modules.
**                  QUICK_TIMER_TICKS_PER_SEC should be used to convert from
**                  time to ticks.
**
**
** Returns          void
**
*******************************************************************************/
void uwb_start_quick_timer(TIMER_LIST_ENT* p_tle, uint16_t type,
                           uint32_t timeout) {

UCI_TRACE_D("uwb_start_quick_timer enter: timeout: %d",timeout);

  p_tle->id = phUwb_GKI_start_timer(UWB_QUICK_TIMER_ID, (uint32_t)((GKI_SECS_TO_TICKS (timeout) / QUICK_TIMER_TICKS_PER_SEC)), type);
  p_tle->in_use = TRUE;

}

/*******************************************************************************
**
** Function         uwb_stop_quick_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void uwb_stop_quick_timer(TIMER_LIST_ENT* p_tle) {

UCI_TRACE_D("uwb_stop_quick_timer: enter");

  if(p_tle->id)
  {
      phUwb_GKI_stop_timer (UWB_QUICK_TIMER_ID, p_tle->id);
      p_tle->id = 0;
      p_tle->in_use = FALSE;
  }

}

/*******************************************************************************
**
** Function         uwb_process_quick_timer_evt
**
** Description      Process quick timer event
**
** Returns          void
**
*******************************************************************************/
void uwb_process_quick_timer_evt(uint16_t event) {
switch (event) {
    case UWB_TTYPE_UCI_WAIT_RSP:
    uwb_ucif_cmd_timeout();
    break;

    default:
    UCI_TRACE_D("uwb_process_quick_timer_evt: event (0x%04x)",
                event);
    }
}

/*******************************************************************************
**
** Function         uwb_task_shutdown_uwbc
**
** Description      Handle UWB shutdown
**
** Returns          nothing
**
*******************************************************************************/
void uwb_task_shutdown_uwbc(void) {
  uwb_gen_cleanup();

  uwb_set_state(UWB_STATE_W4_HAL_CLOSE);
  uwb_cb.p_hal->close();

  /* Stop the timers */
  phUwb_GKI_stop_timer(UWB_TIMER_ID, 0);
  phUwb_GKI_stop_timer(UWB_QUICK_TIMER_ID, 0);
  phUwb_GKI_stop_timer(UWA_TIMER_ID, 0);
}


#define UWB_TASK_ARGS    void *args

/*******************************************************************************
**
** Function         uwb_task
**
** Description      UWB event processing task
**
** Returns          nothing
**
*******************************************************************************/
OSAL_TASK_RETURN_TYPE uwb_task(UWB_TASK_ARGS) {
  uint32_t event;
  bool free_buf = false;
  UWB_HDR* p_msg = NULL;
  phLibUwb_Message msg;
  gp_uwbtask_ctrl = (phUwbtask_Control_t *)args;

  /* Initialize the uwb control block */
  memset(&uwb_cb, 0, sizeof(tUWB_CB));

  /* Initialize the message */
  memset(&msg, 0, sizeof(msg));

  UCI_TRACE_D("UWB_TASK started.");

  /* main loop */
  while (1) {
    if (phDal4Uwb_msgrcv(gp_uwbtask_ctrl->pMsgQHandle, &msg, 0, 0) == -1) {
      continue;
    }
    event = msg.eMsgType;
    /* Handle UWB_TASK_EVT_TRANSPORT_READY from UWB HAL */
    if (event & UWB_TASK_EVT_TRANSPORT_READY) {
      UCI_TRACE_D("UWB_TASK got UWB_TASK_EVT_TRANSPORT_READY.");

      /* Reset the UWB controller. */
      uwb_set_state(UWB_STATE_IDLE);
      uwb_enabled(UWB_STATUS_OK, NULL);
    }
    if (event & GKI_SHUTDOWN_EVT_MASK) {
        break;
    }

    if (event & UWB_MBOX_EVT_MASK) {
      /* Process all incoming UCI messages */
        p_msg = (UWB_HDR *)msg.pMsgData;
        free_buf = true;

        /* Determine the input message type. */
        if(p_msg != NULL) {
          switch (p_msg->event & UWB_EVT_MASK) {
            case BT_EVT_TO_UWB_UCI:
              free_buf = uwb_ucif_process_event(p_msg);
              break;

            case BT_EVT_TO_UWB_MSGS:
              uwb_main_handle_hal_evt((tUWB_HAL_EVT_MSG*)p_msg);
              break;

            default:
              UCI_TRACE_E(
                  "uwb_task: unhandle mbox message, event=%04x", p_msg->event);
            break;
          }
        }

        if (free_buf) {
          phUwb_GKI_freebuf(p_msg);
        }
    }

    /* Process gki timer tick */
    if (event & UWB_TIMER_EVT_MASK) {
      uwb_process_timer_evt(*((UINT16*)msg.pMsgData));
    }

    /* Process quick timer tick */
    if (event & UWB_QUICK_TIMER_EVT_MASK) {
      uwb_process_quick_timer_evt(*((UINT16*)msg.pMsgData));
    }

    if (event & UWA_MBOX_EVT_MASK) {
      uwa_sys_event(&(((tUWA_DM_API_ENABLE*) msg.pMsgData)->hdr));
    }

    if (event & UWA_TIMER_EVT_MASK) {
      uwa_sys_timer_update(*((uint16_t *)msg.pMsgData));
    }
  }

  UCI_TRACE_D("uwb_task terminated");
  phUwb_GKI_binary_sem_post(gp_uwbtask_ctrl->uwb_task_sem);

#if UWBIOT_OS_FREERTOS //FIX THIS
  phUwb_GKI_exit_task(NULL);
#endif
}
