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
#include <errno.h>

#include "phUwb_BuildConfig.h"

#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"
#include "phOsalUwb.h"
#include "phNxpLogApis_HalUtils.h"

/*********************** Link list functions **********************************/

/*******************************************************************************
**
** Function         phNxpUciHal_listInit
**
** Description      List initialization
**
** Returns          1, if list initialized, 0 otherwise
**
*******************************************************************************/
int phNxpUciHal_listInit(struct listHead *pList)
{
    UWBSTATUS status;

    pList->pFirst = NULL;
    if ((status = phOsalUwb_CreateMutex(&pList->mutex)) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIX_E("Mutex creation failed (status=0x%08x)", status);
        return 0;
    }

    return 1;
}

/*******************************************************************************
**
** Function         phNxpUciHal_listDestroy
**
** Description      List destruction
**
** Returns          1, if list destroyed, 0 if failed
**
*******************************************************************************/
int phNxpUciHal_listDestroy(struct listHead *pList)
{
    UWBSTATUS status;
    int bListNotEmpty = 1;
    while (bListNotEmpty) {
        bListNotEmpty = phNxpUciHal_listGetAndRemoveNext(pList, NULL);
    }
    if ((status = phOsalUwb_DeleteMutex(&pList->mutex)) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIX_E("Mutex destruction failed (status=0x%08x)", status);
        return 0;
    }

    return 1;
}

/*******************************************************************************
**
** Function         phNxpUciHal_listAdd
**
** Description      Add a node to the list
**
** Returns          1, if added, 0 if otherwise
**
*******************************************************************************/
int phNxpUciHal_listAdd(struct listHead *pList, void *pData)
{
    struct listNode *pNode;
    struct listNode *pLastNode;
    int result;

    /* Create node */
    pNode = (struct listNode *)phOsalUwb_GetMemory(sizeof(struct listNode));
    if (pNode == NULL) {
        result = 0;
        NXPLOG_UCIX_E("Failed to malloc");
        goto clean_and_return;
    }
    pNode->pData = pData;
    pNode->pNext = NULL;
    phOsalUwb_LockMutex(pList->mutex);
    /* Add the node to the list */
    if (pList->pFirst == NULL) {
        /* Set the node as the head */
        pList->pFirst = pNode;
    }
    else {
        /* Seek to the end of the list */
        pLastNode = pList->pFirst;
        while (pLastNode->pNext != NULL) {
            pLastNode = pLastNode->pNext;
        }

        /* Add the node to the current list */
        pLastNode->pNext = pNode;
    }

    result = 1;

clean_and_return:
    phOsalUwb_UnlockMutex(pList->mutex);
    return result;
}

/*******************************************************************************
**
** Function         phNxpUciHal_listRemove
**
** Description      Remove node from the list
**
** Returns          1, if removed, 0 if otherwise
**
*******************************************************************************/
int phNxpUciHal_listRemove(struct listHead *pList, void *pData)
{
    struct listNode *pNode;
    struct listNode *pRemovedNode;
    int result;
    phOsalUwb_LockMutex(pList->mutex);
    if (pList->pFirst == NULL) {
        /* Empty list */
        NXPLOG_UCIX_E("Failed to deallocate (list empty)");
        result = 0;
        goto clean_and_return;
    }

    pNode = pList->pFirst;
    if (pList->pFirst->pData == pData) {
        /* Get the removed node */
        pRemovedNode = pNode;

        /* Remove the first node */
        pList->pFirst = pList->pFirst->pNext;
    }
    else {
        while (pNode->pNext != NULL) {
            if (pNode->pNext->pData == pData) {
                /* Node found ! */
                break;
            }
            pNode = pNode->pNext;
        }

        if (pNode->pNext == NULL) {
            /* Node not found */
            result = 0;
            NXPLOG_UCIX_E("Failed to deallocate (not found %8p)", pData);
            goto clean_and_return;
        }

        /* Get the removed node */
        pRemovedNode = pNode->pNext;

        /* Remove the node from the list */
        pNode->pNext = pNode->pNext->pNext;
    }

    /* Deallocate the node */
    phOsalUwb_FreeMemory(pRemovedNode);

    result = 1;

clean_and_return:
    phOsalUwb_UnlockMutex(pList->mutex);
    return result;
}

/*******************************************************************************
**
** Function         phNxpUciHal_listGetAndRemoveNext
**
** Description      Get next node on the list and remove it
**
** Returns          1, if successful, 0 if otherwise
**
*******************************************************************************/
int phNxpUciHal_listGetAndRemoveNext(struct listHead *pList, void **ppData)
{
    struct listNode *pNode;
    int result;

    phOsalUwb_LockMutex(pList->mutex);
    if (pList->pFirst == NULL) {
        /* Empty list */
        NXPLOG_UCIX_D("Failed to deallocate (list empty)");
        result = 0;
        goto clean_and_return;
    }

    /* Work on the first node */
    pNode = pList->pFirst;

    /* Return the data */
    if (ppData != NULL) {
        *ppData = pNode->pData;
    }

    /* Remove and deallocate the node */
    pList->pFirst = pNode->pNext;
    phOsalUwb_FreeMemory(pNode);

    result = 1;

clean_and_return:
    phNxpUciHal_listDump(pList);
    phOsalUwb_UnlockMutex(pList->mutex);
    return result;
}

/*******************************************************************************
**
** Function         phNxpUciHal_listDump
**
** Description      Dump list information
**
** Returns          None
**
*******************************************************************************/
void phNxpUciHal_listDump(struct listHead *pList)
{
    struct listNode *pNode = pList->pFirst;

    NXPLOG_UCIX_D("Node dump:");
    while (pNode != NULL) {
        NXPLOG_UCIX_D("- %8p (%8p)", pNode, pNode->pData);
        pNode = pNode->pNext;
    }

    return;
}

/* END Linked list source code */

/****************** Semaphore and mutex helper functions **********************/

static phNxpUciHal_Monitor_t *nxpucihal_monitor = NULL;

/*******************************************************************************
**
** Function         phNxpUciHal_init_monitor
**
** Description      Initialize the semaphore monitor
**
** Returns          Pointer to monitor, otherwise NULL if failed
**
*******************************************************************************/
phNxpUciHal_Monitor_t *phNxpUciHal_init_monitor(void)
{
    NXPLOG_UCIX_D("Entering phNxpUciHal_init_monitor");
    UWBSTATUS wStatus;

    if (nxpucihal_monitor == NULL) {
        nxpucihal_monitor = (phNxpUciHal_Monitor_t *)phOsalUwb_GetMemory(sizeof(phNxpUciHal_Monitor_t));
    }

    if (nxpucihal_monitor != NULL) {
        phOsalUwb_SetMemory(nxpucihal_monitor, 0x00, sizeof(phNxpUciHal_Monitor_t));
        wStatus = phOsalUwb_CreateBinSem(&nxpucihal_monitor->reentrance_binSem);
        if (wStatus != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIX_E("reentrance_binSem creation returned error: %u", wStatus);
            goto clean_and_return;
        }

        wStatus = phOsalUwb_CreateBinSem(&nxpucihal_monitor->concurrency_binSem);
        if (wStatus != UWBSTATUS_SUCCESS) {
            NXPLOG_UCIX_E("concurrency_binSem creation returned error: %u", wStatus);
            phOsalUwb_DeleteSemaphore(&nxpucihal_monitor->reentrance_binSem);
            goto clean_and_return;
        }

        if (phNxpUciHal_listInit(&nxpucihal_monitor->sem_list) != 1) {
            NXPLOG_UCIX_E("Semaphore List creation failed");
            phOsalUwb_DeleteSemaphore(&nxpucihal_monitor->concurrency_binSem);
            phOsalUwb_DeleteSemaphore(&nxpucihal_monitor->reentrance_binSem);
            goto clean_and_return;
        }
    }
    else {
        NXPLOG_UCIX_E("nxphal_monitor creation failed");
        goto clean_and_return;
    }

    phOsalUwb_ProduceSemaphore(nxpucihal_monitor->concurrency_binSem);
    phOsalUwb_ProduceSemaphore(nxpucihal_monitor->reentrance_binSem);
    NXPLOG_UCIX_D("Returning with SUCCESS");

    return nxpucihal_monitor;

clean_and_return:
    NXPLOG_UCIX_D("Returning with FAILURE");

    if (nxpucihal_monitor != NULL) {
        phOsalUwb_FreeMemory(nxpucihal_monitor);
        nxpucihal_monitor = NULL;
    }

    return NULL;
}

/*******************************************************************************
**
** Function         phNxpUciHal_cleanup_monitor
**
** Description      Clean up semaphore monitor
**
** Returns          None
**
*******************************************************************************/
void phNxpUciHal_cleanup_monitor(void)
{
    if (nxpucihal_monitor != NULL) {
        phOsalUwb_DeleteSemaphore(&nxpucihal_monitor->concurrency_binSem);
        REENTRANCE_UNLOCK();
        phOsalUwb_DeleteSemaphore(&nxpucihal_monitor->reentrance_binSem);
        phNxpUciHal_releaseall_cb_data();
        phNxpUciHal_listDestroy(&nxpucihal_monitor->sem_list);
    }
    phOsalUwb_FreeMemory(nxpucihal_monitor);
    nxpucihal_monitor = NULL;
    return;
}

/*******************************************************************************
**
** Function         phNxpUciHal_get_monitor
**
** Description      Get monitor
**
** Returns          Pointer to monitor
**
*******************************************************************************/
phNxpUciHal_Monitor_t *phNxpUciHal_get_monitor(void)
{
    if (nxpucihal_monitor == NULL) {
        NXPLOG_UCIX_E("nxpucihal_monitor is null");
    }
    return nxpucihal_monitor;
}

/* Initialize the callback data */
UWBSTATUS phNxpUciHal_init_cb_data(phNxpUciHal_Sem_t *pCallbackData, void *pContext)
{
    /* Create semaphore */
    if (phOsalUwb_CreateSemaphore(&pCallbackData->sem, 0) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIX_E("Semaphore creation failed");
        return UWBSTATUS_FAILED;
    }

    /* Set default status value */
    pCallbackData->status = UWBSTATUS_FAILED;

    /* Copy the context */
    pCallbackData->pContext = pContext;

    /* Add to active semaphore list */
    if (phNxpUciHal_listAdd(&phNxpUciHal_get_monitor()->sem_list, pCallbackData) != 1) {
        NXPLOG_UCIX_E("Failed to add the semaphore to the list");
    }

    return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phNxpUciHal_cleanup_cb_data
**
** Description      Clean up callback data
**
** Returns          None
**
*******************************************************************************/
void phNxpUciHal_cleanup_cb_data(phNxpUciHal_Sem_t *pCallbackData)
{
    /* Destroy semaphore */
    UWBSTATUS status;
    /* Destroy semaphore */
    if ((status = phOsalUwb_DeleteSemaphore(&pCallbackData->sem)) != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIX_E(
            "phNxpUciHal_cleanup_cb_data: Failed to destroy semaphore "
            "(status=0x%08x)",
            status);
    }
    /* Remove from active semaphore list */
    if (phNxpUciHal_listRemove(&phNxpUciHal_get_monitor()->sem_list, pCallbackData) != 1) {
        NXPLOG_UCIX_E(
            "phNxpUciHal_cleanup_cb_data: Failed to remove semaphore from the "
            "list");
    }
    return;
}

/*******************************************************************************
**
** Function         phNxpUciHal_releaseall_cb_data
**
** Description      Release all callback data
**
** Returns          None
**
*******************************************************************************/
void phNxpUciHal_releaseall_cb_data(void)
{
    phNxpUciHal_Sem_t *pCallbackData;

    while (phNxpUciHal_listGetAndRemoveNext(&phNxpUciHal_get_monitor()->sem_list, (void **)&pCallbackData)) {
        pCallbackData->status = UWBSTATUS_FAILED;
        phOsalUwb_ProduceSemaphore(pCallbackData->sem);
    }
    return;
}

/* END Semaphore and mutex helper functions */

/**************************** Other functions *********************************/

/*******************************************************************************
**
** Function         phNxpUciHal_emergency_recovery
**
** Description      Emergency recovery in case of no other way out
**
** Returns          None
**
*******************************************************************************/

void phNxpUciHal_emergency_recovery(void)
{
    NXPLOG_UCIX_E("%s: abort()", __func__);
    abort();
}
