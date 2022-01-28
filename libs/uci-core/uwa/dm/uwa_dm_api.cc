/******************************************************************************
*
* Copyright (C) 2010-2014 Broadcom Corporation
* Copyright 2018-2020 NXP.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/

/******************************************************************************
*
*  UWA interface for device management
*
******************************************************************************/
#include "phNxpLogApis_UciCore.h"

#include "uwa_api.h"
#include "uwa_sys.h"
#include "uwa_dm_int.h"
#include "uwa_sys_int.h"
#include "uwb_osal_common.h"

tHAL_UWB_CONTEXT hal_Initcntxt;

/*****************************************************************************
**  APIs
*****************************************************************************/
/*******************************************************************************
**
** Function         UWA_Init
**
** Description      This function initializes control blocks for UWA
**
**                  p_hal_entry_tbl points to a table of HAL entry points
**
**                  NOTE: the buffer that p_hal_entry_tbl points must be
**                  persistent until UWA is disabled.
**
** Returns          none
**
*******************************************************************************/
void UWA_Init(tHAL_UWB_ENTRY* p_hal_entry_tbl) {
  UCI_TRACE_D(__func__);
  hal_Initcntxt.hal_entry_func = p_hal_entry_tbl;
  uwa_sys_init();
  uwa_dm_init();
  UWB_Init(&hal_Initcntxt);
}

/*******************************************************************************
**
** Function         UWA_Enable
**
** Description      This function enables UWB. Prior to calling UWA_Enable,
**                  the UWBC must be powered up, and ready to receive commands.
**                  This function enables the tasks needed by UWB, opens the UCI
**                  transport, resets the UWB controller, downloads patches to
**                  the UWBC (if necessary), and initializes the UWB subsystems.
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_Enable(tUWA_DM_CBACK *p_dm_cback, tUWA_DM_TEST_CBACK *p_dm_test_cback)
{
  tUWA_DM_API_ENABLE* p_msg;

  UCI_TRACE_D(__func__);

  /* Validate parameters */
  if (!p_dm_cback) {
    UCI_TRACE_E("error null callback");
    return (UWA_STATUS_FAILED);
  }

  if ((p_msg = (tUWA_DM_API_ENABLE*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_ENABLE))) !=
      NULL) {
    p_msg->hdr.event = UWA_DM_API_ENABLE_EVT;
    p_msg->p_dm_cback = p_dm_cback;
    p_msg->p_dm_test_cback = p_dm_test_cback;
    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_Disable
**
** Description      This function is called to shutdown UWB. The tasks for UWB
**                  are terminated, and clean up routines are performed. This
**                  function is typically called during platform shut-down, or
**                  when UWB is disabled from a settings UI. When the UWB
**                  shutdown procedure is completed, an uwa_dm_disable_EVT is
**                  returned to the application using the tUWA_DM_CBACK.
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_Disable(bool graceful) {
  tUWA_DM_API_DISABLE* p_msg;

  UCI_TRACE_D("UWA_Disable (graceful=%i)", graceful);

  if ((p_msg = (tUWA_DM_API_DISABLE*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_DISABLE))) !=
      NULL) {
    p_msg->hdr.event = UWA_DM_API_DISABLE_EVT;
    p_msg->graceful = graceful;

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_RegisterExtCallback
**
** Description      This function enables proprietary callbacks.
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_RegisterExtCallback(tUWA_RAW_CMD_CBACK* p_dm_ext_cback) {
  tUWA_DM_API_REGISTER_EXT_CB* p_msg;

  UCI_TRACE_D(__func__);

  /* Validate parameters */
  if (!p_dm_ext_cback) {
    UCI_TRACE_E("error null callback");
    return (UWA_STATUS_FAILED);
  }

  if ((p_msg = (tUWA_DM_API_REGISTER_EXT_CB*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_REGISTER_EXT_CB))) !=
      NULL) {
    p_msg->hdr.event = UWA_DM_API_REGISTER_EXT_CB_EVT;
    p_msg->p_dm_ext_cback = p_dm_ext_cback;
    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function:        UWA_GetDeviceInfo
**
** Description:     This function gets the UWB device Information
**
** Returns:         UCI version and manuwacturer specific information
**
*******************************************************************************/
tUWA_STATUS UWA_GetDeviceInfo() {
  tUWA_DM_API_GET_DEVICE_INFO * p_msg;

  UCI_TRACE_D("UWA_GetDeviceInfo ()");

  p_msg = (tUWA_DM_API_GET_DEVICE_INFO*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_GET_DEVICE_INFO));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_GET_DEVICE_INFO_EVT;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SetCoreConfig
**
** Description      Set the configuration parameters to UWBC. The result is
**                  reported with an uwa_dm_set_core_config_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_SetCoreConfig(uint8_t noOfParams, uint8_t paramLen, tUWA_PMID* p_param_ids) {
  tUWA_DM_API_CORE_SET_CONFIG* p_msg;

  if ((p_msg = (tUWA_DM_API_CORE_SET_CONFIG*)phUwb_GKI_getbuf(
         (uint16_t)(sizeof(tUWA_DM_API_CORE_SET_CONFIG) + paramLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_SET_CORE_CONFIG_EVT;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);

    /* Copy parameter data */
    memcpy(p_msg->p_data, p_param_ids, paramLen);

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_GetCoreConfig
**
** Description      Get the configuration parameters from UWBC. The result is
**                  reported with an uwa_dm_get_core_config_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetCoreConfig(uint8_t noOfParams, uint8_t paramLen, tUWA_PMID* p_param_ids) {
  tUWA_DM_API_CORE_GET_CONFIG* p_msg;

  UCI_TRACE_D("UWA_GetCoreConfig (): num_ids: %i", noOfParams);
  if ((p_msg = (tUWA_DM_API_CORE_GET_CONFIG*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_CORE_GET_CONFIG) + paramLen))) != NULL)
  {
    p_msg->hdr.event = UWA_DM_API_GET_CORE_CONFIG_EVT;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_pmids = (tUWA_PMID*)(p_msg + 1);

    /* Copy the param IDs */
    memcpy(p_msg->p_pmids, p_param_ids, paramLen);

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendDeviceReset
**
** Description      Send Device Reset Command to UWBC. The result is
**                  reported with an UWA_DM_DEVICE_RESET_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_SendDeviceReset(uint8_t resetConfig) {
  tUWA_DM_API_DEVICE_RESET* p_msg;

  UCI_TRACE_D("UWA_SendDeviceReset(): resetConfig:0x%X", resetConfig);

  if ((p_msg = (tUWA_DM_API_DEVICE_RESET*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_DEVICE_RESET)))) != NULL) {
    p_msg->hdr.event = UWA_DM_API_DEVICE_RESET_EVT;
    p_msg->resetConfig = resetConfig;

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendSessionInit
**
** Description      This function is called to send session init command
**
**                  session id - value of particular session ID
**                  session type - type of session to start ex: ranging,app etc
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_SendSessionInit(uint32_t session_id, uint8_t sessionType){
  tUWA_DM_API_SESSION_INIT* p_msg;
  p_msg = (tUWA_DM_API_SESSION_INIT*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_SESSION_INIT));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_SESSION_INIT_EVT;
    p_msg->session_id = session_id;
    p_msg->sessionType = sessionType;
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }

  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function         UWA_SendSessionDeInit
**
** Description      This function is called to send session deinit command
**
**                  session id - value of particular session ID
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_SendSessionDeInit(uint32_t session_id){
  tUWA_DM_API_SESSION_DEINIT* p_msg;
  p_msg = (tUWA_DM_API_SESSION_DEINIT*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_SESSION_DEINIT));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_SESSION_DEINIT_EVT;
    p_msg->session_id = session_id;
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }

  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function         UWA_GetSessionCount
**
** Description      Get session count.The result is
**                  reported in the tUWA_DM_CBACK callback.
**
** Returns          UWA_STATUS_OK if command is successfully sent
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetSessionCount() {
  tUWA_DM_API_GET_SESSION_COUNT* p_msg;

  UCI_TRACE_D("UWA_GetSessionCount ()");

  p_msg = (tUWA_DM_API_GET_SESSION_COUNT*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_GET_SESSION_COUNT));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_SESSION_GET_COUNT_EVT;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SetAppConfig
**
** Description      Set the configuration parameters to UWBC. The result is
**                  reported with an UWA_DM_API_SET_APP_CONFIG_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_SetAppConfig(uint32_t session_id, uint8_t noOfParams, uint8_t paramLen , uint8_t appConfigParams[]) {
  tUWA_DM_API_SET_APP_CONFIG* p_msg;

  if ((p_msg = (tUWA_DM_API_SET_APP_CONFIG*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_SET_APP_CONFIG) + paramLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_SET_APP_CONFIG_EVT;

    p_msg->session_id = session_id;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);

    /* Copy parameter data */
    memcpy(p_msg->p_data, appConfigParams, paramLen);

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_GetAppConfig
**
** Description      Get the configuration parameters from UWBC. The result is
**                  reported with an UWA_DM_GET_APP_CONFIG_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetAppConfig(uint32_t session_id, uint8_t noOfParams, uint8_t paramLen, tUWA_PMID* p_param_ids) {
  tUWA_DM_API_GET_APP_CONFIG* p_msg;

  UCI_TRACE_D("UWA_GetAppConfig (): num_ids: %i", noOfParams);
  if ((p_msg = (tUWA_DM_API_GET_APP_CONFIG*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_GET_APP_CONFIG) + paramLen))) != NULL)
  {
    p_msg->hdr.event = UWA_DM_API_GET_APP_CONFIG_EVT;
    p_msg->session_id = session_id;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_pmids = (tUWA_PMID*)(p_msg + 1);

    /* Copy the param IDs */
    if(p_param_ids != NULL){
    memcpy(p_msg->p_pmids, p_param_ids, paramLen);
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
    }
    else{
        UCI_TRACE_E("param_value is NULL");
    }
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_StartRangingSession
**
** Description      start the ranging session.The result is
**                  reported in the tUWA_DM_CBACK callback
**
** Returns          UWA_STATUS_OK if ranging started successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_StartRangingSession(uint32_t session_id) {
  tUWA_DM_API_RANGING_START* p_msg;

  UCI_TRACE_D("UWA_StartRangingSession ():");

  p_msg = (tUWA_DM_API_RANGING_START*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_RANGING_START));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_START_RANGE_EVT;
    p_msg->session_id = session_id;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_StopRangingSession
**
** Description      stop the ranging session.The result is
**                  reported in the tUWA_DM_CBACK callback.
**
** Returns          UWA_STATUS_OK if ranging is stopped successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_StopRangingSession(uint32_t session_id) {
  tUWA_DM_API_RANGING_STOP* p_msg;

  UCI_TRACE_D("UWA_StopRangingSession ()");

  p_msg = (tUWA_DM_API_RANGING_STOP*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_RANGING_STOP));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_STOP_RANGE_EVT;
    p_msg->session_id = session_id;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendRangingIntervalUpdateRequest
**
** Description      This function is called to send ranging interval update request command
**                  to change the ranging interval during ongoing ranging session
**
**                  interval  - interval value in ms
**                  session id - value of particular session ID
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_SendRangingIntervalUpdateRequest(uint32_t session_id, uint16_t interval){
  tUWA_DM_API_RANGE_INTERVAL_UPDATE_REQ* p_msg;
  p_msg = (tUWA_DM_API_RANGE_INTERVAL_UPDATE_REQ*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_RANGE_INTERVAL_UPDATE_REQ));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_RANGE_INTERVAL_UPDATE_REQ_EVT;
    p_msg->session_id = session_id;
    p_msg->interval = interval;
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }
  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function         UWA_SendAppData
**
** Description      This function is called to send raw data over UWB.
**
**
**                  appData - application raw data
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_SendAppData(uint32_t session_id,uint16_t destAddress,uint16_t dataLen, uint8_t* appData) {
  tUWA_DM_API_SEND_APP_DATA* p_msg;
  p_msg = (tUWA_DM_API_SEND_APP_DATA*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_SEND_APP_DATA));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_APP_DATA_SEND_EVT;
    p_msg->session_id = session_id;
    p_msg->dest_addr = destAddress;
    p_msg->data_len = dataLen;
    p_msg->app_data = appData;
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }
  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function         UWA_ReceiveAppData
**
** Description      This function is called to receive app data over UWB.
**
**
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_ReceiveAppData(uint32_t session_id,uint16_t srcAddress) {
  tUWA_DM_API_RCVE_APP_DATA* p_msg;
  p_msg = (tUWA_DM_API_RCVE_APP_DATA*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_RCVE_APP_DATA));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_APP_DATA_RCVE_EVT;
    p_msg->session_id = session_id;
    p_msg->src_addr = srcAddress;
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }
  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function         UWA_GetRangingCount
**
** Description      Get ranging count.The result is
**                  reported in the tUWA_DM_CBACK callback.
**
** Returns          ranging count if successfull
**                  0 otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetRangingCount(uint32_t session_id) {
  tUWA_DM_API_GET_RANGING_COUNT* p_msg;

  UCI_TRACE_D("UWA_GetRangeCount ()");

  p_msg = (tUWA_DM_API_GET_RANGING_COUNT*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_GET_RANGING_COUNT));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_GET_RANGE_COUNT_EVT;
    p_msg->session_id = session_id;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_GetSessionStatus
**
** Description      Get session status.The result is
**                  reported in the tUWA_DM_CBACK callback.
**
** Returns          session status if successfull
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetSessionStatus(uint32_t session_id) {
  tUWA_DM_API_GET_SESSION_STATUS* p_msg;

  UCI_TRACE_D("UWA_GetSessionStatus ()");

  p_msg = (tUWA_DM_API_GET_SESSION_STATUS*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_GET_SESSION_STATUS));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_GET_SESSION_STATUS_EVT;
    p_msg->session_id = session_id;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_GetCoreGetDeviceCapability
**
** Description      Get core capability info command.The result is
**                  reported in the tUWA_DM_CBACK callback.
**
** Returns          UWA_STATUS_OK if successfully sent
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_GetCoreGetDeviceCapability() {
  tUWA_DM_API_CORE_GET_DEVICE_CAPABILITY* p_msg;

  UCI_TRACE_D("UWA_GetCoreGetDeviceCapability()");

  p_msg = (tUWA_DM_API_CORE_GET_DEVICE_CAPABILITY*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_CORE_GET_DEVICE_CAPABILITY));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_CORE_GET_DEVICE_CAPABILITY_EVT;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_ControllerMulticastListUpdate
**
** Description      This function is called to send Controller Multicast List Update.
**
**
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
extern tUWA_STATUS UWA_ControllerMulticastListUpdate(uint32_t session_id, uint8_t action, uint8_t noOfControlees, uint16_t* shortAddressList, uint32_t* subSessionIdList) {
  tUWA_DM_API_SESSION_UPDATE_MULTICAST_LIST* p_msg;
  p_msg = (tUWA_DM_API_SESSION_UPDATE_MULTICAST_LIST*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_SESSION_UPDATE_MULTICAST_LIST));

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_SESSION_UPDATE_MULTICAST_LIST_EVT;
    p_msg->session_id = session_id;
    p_msg->action = action;
    p_msg->no_of_controlee = noOfControlees;
    if((noOfControlees > 0) && (shortAddressList != NULL)) {
      memcpy(p_msg->short_address_list, shortAddressList, (noOfControlees*SHORT_ADDRESS_LEN));
    }
    if((noOfControlees > 0) && (subSessionIdList != NULL)) {
      memcpy(p_msg->subsession_id_list, subSessionIdList, (noOfControlees*SESSION_ID_LEN));
    }
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }
  return UWA_STATUS_FAILED;
}

/*                      UWA APIs for RF Test functionality                                               */

/*******************************************************************************
**
** Function         UWA_TestSetConfig
**
** Description      Set the configuration parameters to UWBC. The result is
**                  reported with an UWA_DM_API_TEST_SET_CONFIG_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_TestSetConfig(uint32_t session_id, uint8_t noOfParams, uint8_t paramLen , uint8_t testConfigParams[]) {
  tUWA_DM_API_TEST_SET_CONFIG* p_msg;

  if ((p_msg = (tUWA_DM_API_TEST_SET_CONFIG*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_TEST_SET_CONFIG) + paramLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_TEST_SET_CONFIG_EVT;

    p_msg->session_id = session_id;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);

    /* Copy parameter data */
    memcpy(p_msg->p_data, testConfigParams, paramLen);

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_TestGetConfig
**
** Description      Get the configuration parameters from UWBC. The result is
**                  reported with an UWA_DM_TEST_GET_CONFIG_EVT in the tUWA_DM_CBACK
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_TestGetConfig(uint32_t session_id, uint8_t noOfParams, uint8_t paramLen, tUWA_PMID* p_param_ids) {
  tUWA_DM_API_TEST_GET_CONFIG* p_msg;

  UCI_TRACE_D("UWA_TestGetConfig (): num_ids: %i", noOfParams);
  if ((p_msg = (tUWA_DM_API_TEST_GET_CONFIG*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_TEST_GET_CONFIG) + paramLen))) != NULL)
  {
    p_msg->hdr.event = UWA_DM_API_TEST_GET_CONFIG_EVT;
    p_msg->session_id = session_id;
    p_msg->num_ids = noOfParams;
    p_msg->length = paramLen;
    p_msg->p_pmids = (tUWA_PMID*)(p_msg + 1);

    /* Copy the param IDs */
    memcpy(p_msg->p_pmids, p_param_ids, paramLen);

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_PerRxTest
**
** Description      send perRX test command to UWBC. The result is
**                  reported with an UWA_DM_API_TEST_PER_RX_EVT in the tUWA_DM_API_TEST_PER_RX
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_PerRxTest(uint16_t psduLen , uint8_t psduData[]) {
  tUWA_DM_API_TEST_PER_RX* p_msg;

  if ((p_msg = (tUWA_DM_API_TEST_PER_RX*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_TEST_PER_RX) + psduLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_TEST_PER_RX_EVT;
    p_msg->length = psduLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);
    if((psduLen > 0) && (psduData != NULL)){
      memcpy(p_msg->p_data, psduData, psduLen);
    }

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_PeriodicTxTest
**
** Description      Set the configuration parameters to UWBC. The result is
**                  reported with an UWA_DM_API_TEST_PERIODIC_TX_EVT in the tUWA_DM_API_TEST_PERIODIC_TX
**                  callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_PeriodicTxTest(uint16_t psduLen , uint8_t psduData[]) {
  tUWA_DM_API_TEST_PERIODIC_TX* p_msg;

  if ((p_msg = (tUWA_DM_API_TEST_PERIODIC_TX*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_TEST_PERIODIC_TX) + psduLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_TEST_PERIODIC_TX_EVT;
    p_msg->length = psduLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);
    if((psduLen > 0) && (psduData != NULL)){
      memcpy(p_msg->p_data, psduData, psduLen);
    }

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_UwbLoopBackTest
**
** Description      Set the configuration parameters to UWBC. The result is
**                  reported with an UWA_DM_API_TEST_UWB_LOOPBACK_EVT in the
**                  tUWA_DM_API_TEST_UWB_LOOPBACK_EVT callback.
**
** Returns          UWA_STATUS_OK if command is sent successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_UwbLoopBackTest(uint16_t psduLen , uint8_t psduData[]) {
  tUWA_DM_API_TEST_UWB_LOOPBACK* p_msg;

  if ((p_msg = (tUWA_DM_API_TEST_UWB_LOOPBACK*)phUwb_GKI_getbuf(
           (uint16_t)(sizeof(tUWA_DM_API_TEST_UWB_LOOPBACK) + psduLen))) != NULL) {

    p_msg->hdr.event = UWA_DM_API_TEST_UWB_LOOPBACK_EVT;
    p_msg->length = psduLen;
    p_msg->p_data = (uint8_t*)(p_msg + 1);
    if((psduLen > 0) && (psduData != NULL)){
      memcpy(p_msg->p_data, psduData, psduLen);
    }

    uwa_sys_sendmsg(p_msg);

    return (UWA_STATUS_OK);
  }

  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_RxTest
**
** Description      send RX test command to UWBC. The response from UWBC is reported by
**                  UWA_DM_TEST_RX_EVT callback.
**
** Returns          UWA_STATUS_OK if Per Session stopped successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_RxTest()
{
    tUWA_DM_API_TEST_RX *p_msg;

    UCI_TRACE_D("UWA_RxTest()");

    p_msg = (tUWA_DM_API_TEST_RX *)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_TEST_RX));
    if (p_msg != NULL) {
        p_msg->hdr.event = UWA_DM_API_TEST_RX_EVT;
        uwa_sys_sendmsg(p_msg);
        return (UWA_STATUS_OK);
    }
    return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_TestStopSession
**
** Description      Send Test Stop session. The response from UWBC is reported by
**                  tUWA_DM_API_TEST_STOP_SESSION as UWA_DM_API_TEST_STOP_SESSION_EVT
**
** Returns          UWA_STATUS_OK if Per Session stopped successfully
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_TestStopSession() {
  tUWA_DM_API_TEST_STOP_SESSION* p_msg;

  UCI_TRACE_D("UWA_TestStopSession()");

  p_msg = (tUWA_DM_API_TEST_STOP_SESSION*)phUwb_GKI_getbuf(sizeof(tUWA_DM_API_TEST_STOP_SESSION));
  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_TEST_STOP_SESSION_EVT;
    uwa_sys_sendmsg(p_msg);
    return (UWA_STATUS_OK);
  }
  return (UWA_STATUS_FAILED);
}

/*******************************************************************************
**
** Function         UWA_SendRawVsCommand
**
** Description      This function is called to send raw Vendor Specific
**                  command to Helios.
**
**                  cmd_params_len  - The command parameter len
**                  p_cmd_params    - The command parameter
**                  p_cback         - The callback function to receive the
**                                    command
**
** Returns          UWA_STATUS_OK if successfully initiated
**                  UWA_STATUS_FAILED otherwise
**
*******************************************************************************/
tUWA_STATUS UWA_SendRawCommand(uint16_t cmd_params_len, uint8_t* p_cmd_params,
                               tUWA_RAW_CMD_CBACK* p_cback) {
  if (cmd_params_len == 0x00 || p_cmd_params == NULL || p_cback == NULL) {
    return UWA_STATUS_INVALID_PARAM;
  }
  uint16_t size = (uint16_t) (sizeof(tUWA_DM_API_SEND_RAW) + cmd_params_len);
  tUWA_DM_API_SEND_RAW* p_msg = (tUWA_DM_API_SEND_RAW*)phUwb_GKI_getbuf(size);

  if (p_msg != NULL) {
    p_msg->hdr.event = UWA_DM_API_SEND_RAW_EVT;
    p_msg->p_cback = p_cback;
    if (cmd_params_len && p_cmd_params) {
      p_msg->cmd_params_len = cmd_params_len;
      p_msg->p_cmd_params = (uint8_t*)(p_msg + 1);
      memcpy(p_msg->p_cmd_params, p_cmd_params, cmd_params_len);
    }
    uwa_sys_sendmsg(p_msg);

    return UWA_STATUS_OK;
  }

  return UWA_STATUS_FAILED;
}

/*******************************************************************************
**
** Function:        UWA_GetMwVersion
**
** Description:     This function gets the Middleware Version
**
** Returns:         First 8 bit Major Version
**                  Last 8 bit Minor Version
**
*******************************************************************************/
tUWA_MW_VERSION UWA_GetMwVersion() {
  tUWA_MW_VERSION mwVer;
  mwVer.validation = NXP_CHIP_SR100;
  mwVer.android_version = NXP_ANDROID_VERSION;
  UCI_TRACE_D("0x%x:UWB MW Major Version:", UWB_NXP_MW_VERSION_MAJ);
  UCI_TRACE_D("0x%x:UWB MW Minor Version:", UWB_NXP_MW_VERSION_MIN);
  mwVer.major_version = UWB_NXP_MW_VERSION_MAJ;
  mwVer.minor_version = UWB_NXP_MW_VERSION_MIN;
  return mwVer;
}
