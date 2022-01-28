/*
 * Copyright (C) 2012-2019 NXP Semiconductors
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

#ifndef PHDAL4UWB_MESSAGEQUEUE_H
#define PHDAL4UWB_MESSAGEQUEUE_H

#include "phUwb_BuildConfig.h"
#include <phUwbTypes.h>

#define configTML_QUEUE_LENGTH 50
#define tmrNO_DELAY            (TickType_t)0U

#ifdef __cplusplus
extern "C" {
#endif

intptr_t phDal4Uwb_msgget(void);
void phDal4Uwb_msgrelease(intptr_t msqid);
int phDal4Uwb_msgctl(intptr_t msqid, int cmd, void *buf);
intptr_t phDal4Uwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, int msgflg);
int phDal4Uwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, long msgtyp, int msgflg);
#ifdef __cplusplus
}
#endif

#endif /*  PHDAL4UWB_MESSAGEQUEUE_H  */
