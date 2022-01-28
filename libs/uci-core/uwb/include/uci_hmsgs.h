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
 *  defines UCI interface messages (for DH)
 *
 ******************************************************************************/
#ifndef UWB_UCI_HMSGS_H
#define UWB_UCI_HMSGS_H

#include "uwb_types.h"
#include "uci_defs.h"

#include <stdbool.h>

uint8_t uci_snd_get_device_info_cmd(void);
uint8_t uci_snd_device_reset_cmd(uint8_t resetConfig);
uint8_t uci_snd_core_set_config_cmd(uint8_t num_ids, uint8_t length, uint8_t* data);
uint8_t uci_snd_core_get_config_cmd(uint8_t num_ids, uint8_t length, uint8_t* data);
uint8_t uci_snd_core_get_device_capability(void);
uint8_t uci_snd_session_init_cmd(uint32_t session_id,uint8_t session_type);
uint8_t uci_snd_session_deinit_cmd(uint32_t session_id);
uint8_t uci_snd_get_session_count_cmd(void);
uint8_t uci_snd_get_range_count_cmd(uint32_t session_id);
uint8_t uci_snd_get_session_status_cmd(uint32_t session_id);
uint8_t uci_snd_multicast_list_update_cmd(uint32_t session_id, uint8_t action, uint8_t noOfControlees, uint16_t* shortAddressList, uint32_t* subSessionIdList);
uint8_t uci_snd_app_get_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* param_ids);
uint8_t uci_snd_app_set_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* data);
uint8_t uci_snd_range_start_cmd(uint32_t session_id);
uint8_t uci_snd_range_stop_cmd(uint32_t session_id);
uint8_t uci_snd_range_interval_update_request_cmd(uint32_t session_id, uint16_t interval);
uint8_t uci_snd_app_data_send_cmd(uint32_t session_id,uint16_t dest_addr, uint16_t data_len, uint8_t* app_data);
uint8_t uci_snd_app_data_rcve_cmd(uint32_t session_id,uint16_t src_addr);

/*  APIs for UWB RF test functionality */
uint8_t uci_snd_test_get_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* param_ids);
uint8_t uci_snd_test_set_config_cmd(uint32_t session_id, uint8_t num_ids, uint8_t length, uint8_t* data);
uint8_t uci_snd_test_periodic_tx_cmd(uint16_t length, uint8_t* data);
uint8_t uci_snd_test_per_rx_cmd(uint16_t length, uint8_t* data);
uint8_t uci_snd_test_uwb_loopback_cmd(uint16_t length, uint8_t* data);
uint8_t uci_snd_test_rx_cmd(void);
uint8_t uci_snd_test_stop_session_cmd(void);

extern void uci_proc_session_management_rsp(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_session_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_device_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_core_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_rang_management_rsp(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_data_management_rsp(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_rang_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_app_data_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_test_management_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_test_management_rsp(uint8_t op_code, uint8_t* p_buf, uint16_t len);
extern void uci_proc_raw_cmd_rsp(uint8_t* p_buf, uint16_t len);
extern void uci_proc_proprietary_ntf(uint8_t op_code, uint8_t* p_buf, uint16_t len);
#endif /* UWB_UCI_MSGS_H */
