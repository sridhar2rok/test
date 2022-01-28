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
#include "UwbAdaptation.h"
#include "UwbCoreSDK_Internal.h"
#include "phDal4Uwb_messageQueueLib.h"
#include "phNxpUciHal_Adaptation.h"
#include "phNxpUwbConfig.h"
#include "phOsalUwb_Thread.h"
#include "uwa_api.h"
#include "uwb_hal_int.h"
#include "uwb_int.h"
#include "uwb_logging.h"
#include "uwb_target.h"

#include "phUwb_BuildConfig.h"
static phUwbtask_Control_t uwb_ctrl;

static void HalInitialize(void);
static void HalTerminate(void);
static void HalOpen(tHAL_UWB_CBACK *p_hal_cback, tHAL_UWB_DATA_CBACK *p_data_cback);
static void HalClose(void);
static void HalWrite(UINT16 data_len, UINT8 *p_data);
static tUWB_STATUS HalIoctl(long arg, void *p_data);
static void InitializeHalDeviceContext();
static tHAL_UWB_ENTRY mHalEntryFuncs;

UINT32 StartUwbTask()
{
    phOsalUwb_ThreadCreationParams_t threadparams;

    threadparams.stackdepth = UWBTASK_STACK_SIZE;
    strcpy((char *)threadparams.taskname, "UWB_TASK");
    threadparams.pContext = &uwb_ctrl;
    threadparams.priority = GKI_TASK_PRIORITY;
    if (phOsalUwb_Thread_Create((void **)&uwb_ctrl.task_handle, uwb_task, &threadparams) != 0) {
        return UWA_STATUS_FAILED;
    }

    return UWA_STATUS_OK;
}

/*******************************************************************************
**
** Function:    Initialize()
**
**
** Returns:     none
**
*******************************************************************************/
void Initialize()
{
    uint8_t mConfig;
    phOsalUwb_SetMemory(&mHalEntryFuncs, 0, sizeof(mHalEntryFuncs));
    phUwb_GKI_init();
    phUwb_GKI_enable();

    uwb_ctrl.pMsgQHandle  = phDal4Uwb_msgget();
    uwb_ctrl.uwb_task_sem = phUwb_GKI_binary_sem_init();
    StartUwbTask();

    phNxpUciHal_GetNxpNumValue(UWB_FW_LOG_THREAD_ID, &mConfig, sizeof(mConfig));

    InitializeHalDeviceContext();
}

/*******************************************************************************
**
** Function:    Finalize()
**
** Returns:     none
**
*******************************************************************************/
void Finalize()
{
    phUwb_GKI_shutdown();
    phUwb_GKI_binary_sem_wait(uwb_ctrl.uwb_task_sem);
    phOsalUwb_SetMemory(&mHalEntryFuncs, 0, sizeof(mHalEntryFuncs));
    phUwb_GKI_binary_sem_destroy(uwb_ctrl.uwb_task_sem);
    phDal4Uwb_msgrelease(uwb_ctrl.pMsgQHandle);
}

/*******************************************************************************
**
** Function:    GetHalEntryFuncs()
**
** Description: Get the set of HAL entry points.
**
** Returns:     Functions pointers for HAL entry points.
**
*******************************************************************************/
tHAL_UWB_ENTRY *GetHalEntryFuncs()
{
    return &mHalEntryFuncs;
}

/*******************************************************************************
**
** Function:    InitializeHalDeviceContext
**
** Description: Ask the generic Android HAL to find the Broadcom-specific HAL.
**
** Returns:     None.
**
*******************************************************************************/
void InitializeHalDeviceContext()
{
    phOsalUwb_SetMemory(&mHalEntryFuncs, 0, sizeof(mHalEntryFuncs));

    mHalEntryFuncs.initialize = HalInitialize;
    mHalEntryFuncs.terminate  = HalTerminate;
    mHalEntryFuncs.open       = HalOpen;
    mHalEntryFuncs.close      = HalClose;
    mHalEntryFuncs.write      = HalWrite;
    mHalEntryFuncs.ioctl      = HalIoctl;
}

/*******************************************************************************
**
** Function:    HalInitialize
**
** Description: Not implemented because this function is only needed
**              within the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void HalInitialize()
{
}

/*******************************************************************************
**
** Function:    HalTerminate
**
** Description: Not implemented because this function is only needed
**              within the HAL.
**
** Returns:     None.
**
*******************************************************************************/
void HalTerminate()
{
}

/*******************************************************************************
**
** Function:    HalOpen
**
** Description: Turn on controller, download firmware.
**
** Returns:     None.
**
*******************************************************************************/
void HalOpen(ATTRIBUTE_UNUSED tHAL_UWB_CBACK *p_hal_cback, ATTRIBUTE_UNUSED tHAL_UWB_DATA_CBACK *p_data_cback)
{
    phNxpUciHal_open(p_hal_cback, p_data_cback);
}

/*******************************************************************************
**
** Function:    HalClose
**
** Description: Turn off controller.
**
** Returns:     None.
**
*******************************************************************************/
void HalClose()
{
    phNxpUciHal_close();
}

/*******************************************************************************
**
** Function:    HalWrite
**
** Description: Write UCI message to the controller.
**
** Returns:     None.
**
*******************************************************************************/
void HalWrite(ATTRIBUTE_UNUSED uint16_t data_len, ATTRIBUTE_UNUSED uint8_t *p_data)
{
    phNxpUciHal_write(data_len, p_data);
}

/*******************************************************************************
**
** Function:    HalIoctl
**
** Description: Calls ioctl to the Uwb driver.
**              If called with a arg value of 0x01 than wired access requested,
**              status of the requst would be updated to p_data.
**              If called with a arg value of 0x00 than wired access will be
**              released, status of the requst would be updated to p_data.
**              If called with a arg value of 0x02 than current p61 state would
*be
**              updated to p_data.
**
** Returns:     -1 or 0.
**
*******************************************************************************/
tUWB_STATUS HalIoctl(ATTRIBUTE_UNUSED long arg, ATTRIBUTE_UNUSED void *p_data)
{
    tUWB_STATUS status = 0;
    status             = (tUWB_STATUS)phNxpUciHal_ioctl(arg, p_data);
    return status;
}

/*******************************************************************************
**
** Function:    DownloadFirmware
**
** Description: Download firmware patch files.
**
** Returns:     None.
**
*******************************************************************************/
tUWB_STATUS DownloadFirmware()
{
    tUWB_STATUS status = 0;
    uwb_uci_IoctlInOutData_t inpOutData;
    inpOutData.recovery = false;
    status              = (tUWB_STATUS)phNxpUciHal_ioctl(HAL_UWB_IOCTL_FW_DWNLD, &inpOutData);
    return status;
}

/*******************************************************************************
**
** Function:    DownloadFirmwareRecovery
**
** Description: DownloadFirmware for crash/timeout recovery
**
** Returns:     fw download status.
**
*******************************************************************************/
tUWB_STATUS DownloadFirmwareRecovery()
{
    tUWB_STATUS status = 0;
    uwb_uci_IoctlInOutData_t inpOutData;
    inpOutData.recovery = true;
    status              = (tUWB_STATUS)phNxpUciHal_ioctl(HAL_UWB_IOCTL_FW_DWNLD, &inpOutData);
    return status;
}
#if UWBIOT_UWBD_SR100T
/*******************************************************************************
**
** Function:    SetFirmwareImage
**
** Description: This function is used to select firmware image(Factory/Mainline)
**
** Returns:     true if parameters are fine, false otherwise.
**
*******************************************************************************/
bool SetFirmwareImage(uint8_t *fwImgPtr, uint32_t fwSize)
{
    if (fwImgPtr != NULL && fwSize > 0) {
        setFwImage(fwImgPtr, fwSize);
        return true;
    }
    else {
        return false;
    }
}
#endif
/*******************************************************************************
**
** Function:    isCmdRespPending
**
** Description: This function is get the Response status for the current command sent to fw
**
** Returns:     true if response is pending, false otherwise.
**
*******************************************************************************/
bool isCmdRespPending()
{
    return uwb_cb.is_resp_pending;
}
