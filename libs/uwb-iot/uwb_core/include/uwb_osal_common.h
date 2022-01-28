/*
*
* Copyright 2018-2019 NXP.
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

#include "phUwb_BuildConfig.h"
#include "phDal4Uwb_messageQueueLib.h"
#include "uwb_gki.h"
#include "uwb_logging.h"
// extern phUwbtask_Control_t* gp_uwbtask_ctrl;
// extern uint8_t rtos_GKI_create_task (TASKPTR, UINT8, INT8*, UINT16*, UINT16,
// void*);
void rtos_GKI_shutdown(void);

/*Function call context*/
#define ADAPTATION_INIT 1
