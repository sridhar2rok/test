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
/******************************************************************************
 *
 *  Protocol timer services (taken from bta ptim)
 *
 ******************************************************************************/
#include "uwb_target.h"
#include "uwb_gki.h"
#include "uwa_sys_ptim.h"
#include "uwa_sys.h"
#include "uwa_sys_int.h"
#include "uwb_osal_common.h"

#include "phNxpLogApis_UciCore.h"

/*******************************************************************************
**
** Function         uwa_sys_ptim_init
**
** Description      Initialize a protocol timer control block.  Parameter
**                  period is the GKI timer period in milliseconds.  Parameter
**                  timer_id is the GKI timer id.
**
** Returns          void
**
*******************************************************************************/
void uwa_sys_ptim_init(tPTIM_CB* p_cb, uint16_t period, uint8_t timer_id) {
  phUwb_GKI_init_timer_list(&p_cb->timer_queue);
  p_cb->period = period;
  p_cb->timer_id = timer_id;
}

/*******************************************************************************
**
** Function         uwa_sys_ptim_start_timer
**
** Description      Start a protocol timer for the specified amount
**                  of time in seconds.
**
** Returns          void
**
*******************************************************************************/
void uwa_sys_ptim_start_timer(tPTIM_CB* p_cb, TIMER_LIST_ENT* p_tle,
                              uint16_t type, uint32_t timeout) {
  p_tle->id = phUwb_GKI_start_timer(p_cb->timer_id, GKI_MS_TO_TICKS (timeout), type);
  p_tle->in_use = TRUE;
}

/*******************************************************************************
**
** Function         uwa_sys_ptim_stop_timer
**
** Description      Stop a protocol timer.
**
** Returns          void
**
*******************************************************************************/
void uwa_sys_ptim_stop_timer(tPTIM_CB* p_cb, TIMER_LIST_ENT* p_tle) {
  if(p_tle->id)
  {
      phUwb_GKI_stop_timer (p_cb->timer_id, p_tle->id);
      p_tle->id = 0;
      p_tle->in_use = FALSE;
  }
}
