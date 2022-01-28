/*
 * Copyright 2012-2020 NXP.
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

/*
 * DAL independent message queue implementation for Android (can be used under
 * Linux too)
 */
#include "phDal4Uwb_messageQueueLib.h"

#include "phOsalUwb.h"

#include "FreeRTOS.h"
#include "phUwb_BuildConfig.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

typedef struct phDal4Uwb_Queue
{
    QueueHandle_t pQueue;
} phDal4Uwb_Queue_t;

/*******************************************************************************
**
** Function         phDal4Uwb_msgget
**
** Description      Allocates message queue
**
** Parameters       Ignored, included only for Linux queue API compatibility
**
** Returns          (int) value of pQueue if successful
**                  -1, if failed to allocate memory or to init mutex
**
*******************************************************************************/
intptr_t phDal4Uwb_msgget(void)
{
    phDal4Uwb_Queue_t *pMsgQ;
    pMsgQ = (phDal4Uwb_Queue_t *)phOsalUwb_GetMemory(sizeof(phDal4Uwb_Queue_t));
    configASSERT(pMsgQ);
    pMsgQ->pQueue = xQueueCreate((UBaseType_t)configTML_QUEUE_LENGTH, sizeof(phLibUwb_Message_t));
    configASSERT(pMsgQ->pQueue);
    if (pMsgQ->pQueue == NULL) {
        phOsalUwb_FreeMemory(pMsgQ);
        return -1;
    }

    return ((intptr_t)pMsgQ);
}

/*******************************************************************************
**
** Function         phDal4Uwb_msgrelease
**
** Description      Releases message queue
**
** Parameters       msqid - message queue handle
**
** Returns          None
**
*******************************************************************************/
void phDal4Uwb_msgrelease(intptr_t msqid)
{
    phDal4Uwb_Queue_t *pMsgQ = (phDal4Uwb_Queue_t *)msqid;

    if (pMsgQ != NULL) {
        if (pMsgQ->pQueue != NULL) {
            vQueueDelete(pMsgQ->pQueue);
            pMsgQ->pQueue = NULL;
        }
        phOsalUwb_FreeMemory(pMsgQ);
    }
    return;
}

/*******************************************************************************
**
** Function         phDal4Uwb_msgsnd
**
** Description      Sends a message to the queue. The message will be added at
**                  the end of the queue as appropriate for FIFO policy
**
** Parameters       msqid  - message queue handle
**                  msg    - message to be sent
**                  msgflg - ignored
**
** Returns          0,  if successful
**                  -1, if invalid parameter passed or failed to allocate memory
**
*******************************************************************************/
intptr_t phDal4Uwb_msgsnd(intptr_t msqid, phLibUwb_Message_t *msg, int msgflg)
{
    int returnVal            = errQUEUE_FULL;
    phDal4Uwb_Queue_t *pMsgQ = (phDal4Uwb_Queue_t *)msqid;
    PHUWB_UNUSED(msgflg);
    if ((pMsgQ == NULL) || (pMsgQ->pQueue == NULL) || (msg == NULL))
        return -1;
    returnVal = xQueueSendToBack(pMsgQ->pQueue, msg, tmrNO_DELAY);
    if (returnVal == pdTRUE)
        return 0;
    else
        return -1;
}

/*******************************************************************************
**
** Function         phDal4Uwb_msgrcv
**
** Description      Gets the oldest message from the queue.
**                  If the queue is empty the function waits (blocks on a mutex)
**                  until a message is posted to the queue with
**                  phDal4Uwb_msgsnd.
**
** Parameters       msqid  - message queue handle
**                  msg    - container for the message to be received
**                  msgtyp - ignored
**                  msgflg - ignored
**
** Returns          0,  if successful
**                  -1, if invalid parameter passed
**
*******************************************************************************/
int phDal4Uwb_msgrcv(intptr_t msqid, phLibUwb_Message_t *msg, long msgtyp, int msgflg)
{
    PHUWB_UNUSED(msgflg);
    PHUWB_UNUSED(msgtyp);
    int returnVal            = FALSE;
    phDal4Uwb_Queue_t *pMsgQ = (phDal4Uwb_Queue_t *)msqid;
    if ((pMsgQ == NULL) || (pMsgQ->pQueue == NULL) || (msg == NULL))
        return -1;

    returnVal = xQueueReceive(pMsgQ->pQueue, msg, portMAX_DELAY);
    if (returnVal)
        return 0;
    else
        return -1;
}

intptr_t phDal4Uwb_queuesize(intptr_t msqid)
{
    phDal4Uwb_Queue_t *pMsgQ = (phDal4Uwb_Queue_t *)msqid;
    if (pMsgQ == NULL)
        return -1;

    return uxQueueMessagesWaiting(pMsgQ->pQueue);
}
