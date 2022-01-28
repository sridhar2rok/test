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
#ifndef _PHNXPUCIHAL_EXT_H_
#define _PHNXPUCIHAL_EXT_H_

#include <phNxpUciHal.h>

UWBSTATUS phNxpUciHal_process_ext_rsp(uint8_t *p_ntf, uint16_t *p_len);
UWBSTATUS phNxpUciHal_send_ext_cmd(uint16_t cmd_len, uint8_t *p_cmd);
UWBSTATUS phNxpUciHal_send_ext_cmd_ntf(uint16_t cmd_len, uint8_t *p_cmd);
UWBSTATUS phNxpUciHal_dump_fw_crash_log();
UWBSTATUS phNxpUciHal_set_board_config();
#endif /* _PHNXPNICHAL_EXT_H_ */
