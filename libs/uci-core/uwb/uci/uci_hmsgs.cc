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
 *  This file contains function of the UCI unit to format and send UCI
 *  commands (for DH).
 *
 ******************************************************************************/
#include "uwb_target.h"

#include "uci_defs.h"
#include "uci_test_defs.h"
#include "uci_hmsgs.h"
#include "uwb_api.h"
#include "uwb_int.h"
#include "uwb_osal_common.h"


/*******************************************************************************
**
** Function         uci_snd_get_device_info_cmd
**
** Description      compose and send CORE_GET_DEVICE_INFO_CMD command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_get_device_info_cmd(void) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_CORE_DEVICE_INFO_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_CORE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_CORE_DEVICE_INFO);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_CORE_DEVICE_INFO_CMD_SIZE);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_device_reset_cmd
**
** Description      compose and send DEVICE_RESET command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_device_reset_cmd(uint8_t resetConfig) {
  UWB_HDR* p;
  uint8_t* pp;
  p = UCI_GET_CMD_BUF(UCI_MSG_CORE_DEVICE_RESET_CMD_SIZE);
  if (p == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_CORE_DEVICE_RESET_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_CORE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_CORE_DEVICE_RESET);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_CORE_DEVICE_RESET_CMD_SIZE);
  UWB_UINT8_TO_STREAM(pp, resetConfig);
  uwb_ucif_send_cmd(p);

  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_core_set_config_cmd
**
** Description      compose and send CORE SET_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_core_set_config_cmd(uint8_t num_ids, uint8_t length, uint8_t* data) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_CORE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_CORE_SET_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(num_ids) + length);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, data, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_core_get_config_cmd
**
** Description      compose and send CORE GET_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_core_get_config_cmd(uint8_t num_ids, uint8_t length, uint8_t* param_ids) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_CORE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_CORE_GET_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(num_ids) + length);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, param_ids, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_session_init_cmd
**
** Description      compose and send session init command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_session_init_cmd(uint32_t session_id,uint8_t sessionType){

  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_SESSION_INIT_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_SESSION_INIT_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_INIT);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_SESSION_INIT_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, sessionType);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_session_deinit_cmd
**
** Description      compose and session de-init command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_session_deinit_cmd(uint32_t session_id){

  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_SESSION_DEINIT_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_SESSION_DEINIT_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_DEINIT);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_SESSION_DEINIT_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_app_set_config_cmd
**
** Description      compose and send SET_APP_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_app_set_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* data) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(session_id) + sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(session_id) + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_SET_APP_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(session_id) + sizeof(num_ids) + length);

  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, data, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_app_get_config_cmd
**
** Description      compose and send GET_APP_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_app_get_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* param_ids) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(session_id) + sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(session_id) + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_GET_APP_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(session_id) + sizeof(num_ids) + length);

  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, param_ids, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_get_session_count_cmd
**
** Description      compose and send SESSION_GET_COUNT command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_get_session_count_cmd(void) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_SESSION_GET_COUNT_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_GET_COUNT);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_SESSION_GET_COUNT_CMD_SIZE);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_get_session_status_cmd
**
** Description      compose and send SESSION_GET_STATUS command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_get_session_status_cmd(uint32_t session_id) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_SESSION_GET_STATE_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_SESSION_GET_STATE_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_GET_STATE);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_SESSION_GET_STATE_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_get_range_count_cmd
**
** Description      compose and send RANGE_GET_COUNT command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_get_range_count_cmd(uint32_t session_id) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_RANGE_GET_COUNT_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_RANGE_GET_COUNT_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_RANGE_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_RANGE_GET_RANGING_COUNT);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_RANGE_GET_COUNT_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}
/*******************************************************************************
**
** Function         uci_snd_range_start_cmd
**
** Description      compose and send RANGE_START_CMD command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_range_start_cmd(uint32_t session_id) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_RANGE_START_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_RANGE_START_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_RANGE_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_RANGE_START);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_RANGE_START_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_range_stop_cmd
**
** Description      compose and send RANGE_STOP_CMD command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_range_stop_cmd(uint32_t session_id) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_RANGE_STOP_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_RANGE_STOP_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_RANGE_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_RANGE_STOP);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_RANGE_STOP_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);
  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_range_interval_update_request_cmd
**
** Description      compose and send range interval update request command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_range_interval_update_request_cmd(uint32_t session_id, uint16_t interval){

  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_RANGE_INTERVAL_UPDATE_REQ_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_RANGE_INTERVAL_UPDATE_REQ_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_RANGE_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_RANGE_CTRL_REQ);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_RANGE_INTERVAL_UPDATE_REQ_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT16_TO_STREAM(pp, interval);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_app_data_send_cmd
**
** Description      compose and send application specific data
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_app_data_send_cmd(uint32_t session_id,uint16_t destAddr, uint16_t dataLen, uint8_t* appData) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_APP_DATA_TX_CMD_SIZE + dataLen)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + UCI_MSG_APP_DATA_TX_CMD_SIZE + dataLen);
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_APP_DATA_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_APP_DATA_TX);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_APP_DATA_TX_CMD_SIZE + dataLen);
  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT16_TO_STREAM(pp, destAddr);
  UWB_ARRAY_TO_STREAM(pp, appData,dataLen);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_app_data_rcve_cmd
**
** Description      compose and send application specific data
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_app_data_rcve_cmd(uint32_t session_id,uint16_t srcAddr) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_APP_DATA_RX_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_APP_DATA_RX_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_APP_DATA_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_APP_DATA_RX);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_APP_DATA_RX_CMD_SIZE);
  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT16_TO_STREAM(pp, srcAddr);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_core_get_device_capability
**
** Description      compose and send get device capability command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_core_get_device_capability(void) {
  UWB_HDR* p;
  uint8_t* pp;
  p = UCI_GET_CMD_BUF(UCI_MSG_CORE_GET_CAPS_INFO_CMD_SIZE);
  if (p == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE + UCI_MSG_CORE_GET_CAPS_INFO_CMD_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_CORE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_CORE_GET_CAPS_INFO);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_CORE_GET_CAPS_INFO_CMD_SIZE);
  uwb_ucif_send_cmd(p);

  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_multicast_list_update_cmd
**
** Description      compose and send SESSION_UPDATE_CONTROLLER _MULTICAST_LIST_CMD
**                  command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_multicast_list_update_cmd(uint32_t session_id, uint8_t action, uint8_t noOfControlees, uint16_t* shortAddressList, uint32_t* subSessionIdList) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(session_id) + sizeof(action) + sizeof(noOfControlees) + (noOfControlees*SHORT_ADDRESS_LEN) + (noOfControlees*SESSION_ID_LEN))) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(session_id) + sizeof(action) + sizeof(noOfControlees) + (noOfControlees*SHORT_ADDRESS_LEN) + (noOfControlees*SESSION_ID_LEN));

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_SESSION_MANAGE);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_SESSION_UPDATE_CONTROLLER_MULTICAST_LIST);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(session_id) + sizeof(action) + sizeof(noOfControlees) + (noOfControlees*SHORT_ADDRESS_LEN) + (noOfControlees*SESSION_ID_LEN));

  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, action);
  UWB_UINT8_TO_STREAM(pp, noOfControlees);
  for (uint8_t i = 0; i < noOfControlees; i++) {
    UWB_UINT16_TO_STREAM(pp, shortAddressList[i]);
    UWB_UINT32_TO_STREAM(pp, subSessionIdList[i]);
  }

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*  APIs for UWB RF test functionality */
/*******************************************************************************
**
** Function         uci_snd_test_set_config_cmd
**
** Description      compose and send SET_TEST_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_set_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* data) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(session_id) + sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(session_id) + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_SET_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(session_id) + sizeof(num_ids) + length);

  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, data, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_get_config_cmd
**
** Description      compose and send GET_TEST_CONFIG command to command queue
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_get_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* param_ids) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(sizeof(session_id) + sizeof(num_ids) + length)) == NULL) return (UCI_STATUS_FAILED);

  p->event = BT_EVT_TO_UWB_UCI;

  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + sizeof(session_id) + sizeof(num_ids) + length);

  p->offset = UCI_MSG_OFFSET_SIZE;
  p->layer_specific = 0;
  pp = (uint8_t*)(p + 1) + p->offset;

  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_GET_CONFIG);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, sizeof(session_id) + sizeof(num_ids) + length);

  UWB_UINT32_TO_STREAM(pp, session_id);
  UWB_UINT8_TO_STREAM(pp, num_ids);
  UWB_ARRAY_TO_STREAM(pp, param_ids, length);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_periodic_tx_cmd
**
** Description      compose and send periodic tx command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_periodic_tx_cmd(uint16_t psduLen, uint8_t* psduData) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(psduLen)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + psduLen);
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  if(psduLen <= UCI_MAX_PAYLOAD_SIZE) {
        UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_PERIODIC_TX);
        UWB_UINT8_TO_STREAM(pp, 0x00);
        UWB_UINT8_TO_STREAM(pp, psduLen);
    } else { /* extended len apdu */
        UCI_MSG_BLD_HDR1(pp,(1 << 7) | UCI_MSG_TEST_PERIODIC_TX);
        UWB_UINT8_TO_STREAM(pp, psduLen & 0x00FF);
        UWB_UINT8_TO_STREAM(pp, (psduLen & 0xFF00) >> 8);
    }
  if((psduLen > 0) && (psduData != NULL)){
    UWB_ARRAY_TO_STREAM(pp, psduData, psduLen);
  }

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_per_rx_cmd
**
** Description      compose and send per rx command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_per_rx_cmd(uint16_t psduLen, uint8_t* psduData) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(psduLen)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + psduLen);
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  if(psduLen <= UCI_MAX_PAYLOAD_SIZE) {
        UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_PER_RX);
        UWB_UINT8_TO_STREAM(pp, 0x00);
        UWB_UINT8_TO_STREAM(pp, psduLen);
    } else { /* extended len apdu */
        UCI_MSG_BLD_HDR1(pp,(1 << 7) | UCI_MSG_TEST_PER_RX);
        UWB_UINT8_TO_STREAM(pp, psduLen & 0x00FF);
        UWB_UINT8_TO_STREAM(pp, (psduLen & 0xFF00) >> 8);
    }
  if((psduLen > 0) && (psduData != NULL)){
    UWB_ARRAY_TO_STREAM(pp, psduData, psduLen);
  }

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_uwb_loopback_cmd
**
** Description      compose and send rf loop back command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_uwb_loopback_cmd(uint16_t psduLen, uint8_t* psduData) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(psduLen)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = (uint16_t) (UCI_MSG_HDR_SIZE + psduLen);
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  if(psduLen <= UCI_MAX_PAYLOAD_SIZE) {
        UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_LOOPBACK);
        UWB_UINT8_TO_STREAM(pp, 0x00);
        UWB_UINT8_TO_STREAM(pp, psduLen);
    } else { /* extended len apdu */
        UCI_MSG_BLD_HDR1(pp,(1 << 7) | UCI_MSG_TEST_LOOPBACK);
        UWB_UINT8_TO_STREAM(pp, psduLen & 0x00FF);
        UWB_UINT8_TO_STREAM(pp, (psduLen & 0xFF00) >> 8);
    }
  if((psduLen > 0) && (psduData != NULL)){
      UWB_ARRAY_TO_STREAM(pp, psduData, psduLen);
  }

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_rx_cmd
**
** Description      compose and send test rx command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_rx_cmd(void)
{
    UWB_HDR *p;
    uint8_t *pp;

    if ((p = UCI_GET_CMD_BUF(UCI_MSG_TEST_RX_CMD_SIZE)) == NULL)
        return (UCI_STATUS_FAILED);
    p->event  = BT_EVT_TO_UWB_UCI;
    p->len    = UCI_MSG_HDR_SIZE;
    p->offset = UCI_MSG_OFFSET_SIZE;
    pp        = (uint8_t *)(p + 1) + p->offset;
    UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
    UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_RX);
    UWB_UINT8_TO_STREAM(pp, 0x00);
    UWB_UINT8_TO_STREAM(pp, UCI_MSG_TEST_RX_CMD_SIZE);

    uwb_ucif_send_cmd(p);
    return (UCI_STATUS_OK);
}

/*******************************************************************************
**
** Function         uci_snd_test_stop_session_cmd
**
** Description      compose and send test stop session command
**
** Returns          status
**
*******************************************************************************/
uint8_t uci_snd_test_stop_session_cmd(void) {
  UWB_HDR* p;
  uint8_t* pp;

  if ((p = UCI_GET_CMD_BUF(UCI_MSG_TEST_STOP_SESSION_CMD_SIZE)) == NULL)
    return (UCI_STATUS_FAILED);
  p->event = BT_EVT_TO_UWB_UCI;
  p->len = UCI_MSG_HDR_SIZE;
  p->offset = UCI_MSG_OFFSET_SIZE;
  pp = (uint8_t*)(p + 1) + p->offset;
  UCI_MSG_BLD_HDR0(pp, UCI_MT_CMD, UCI_GID_TEST);
  UCI_MSG_BLD_HDR1(pp, UCI_MSG_TEST_STOP_SESSION);
  UWB_UINT8_TO_STREAM(pp, 0x00);
  UWB_UINT8_TO_STREAM(pp, UCI_MSG_TEST_STOP_SESSION_CMD_SIZE);

  uwb_ucif_send_cmd(p);
  return (UCI_STATUS_OK);
}
