/*
 * Copyright (C) 2012-2020 NXP Semiconductors
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

#ifndef _PHNXPUCIHAL_H_
#define _PHNXPUCIHAL_H_

#include "FreeRTOS.h"
#include "phUwb_BuildConfig.h"
#include "task.h"
#include <phNxpUciHal_Adaptation.h>
#include <phNxpUciHal_utils.h>
#include <phOsalUwb.h>
#include <uwb_hal_int.h>

/********************* Definitions and structures *****************************/
#define MAX_RETRY_COUNT    5
#define UCI_MAX_DATA_LEN   2060 // maximum CIR log packet size
#define UCI_MAX_PACKET_LEN (UCI_MSG_HDR_SIZE + UCI_MAX_PAYLOAD_SIZE)

#define UCI_MT_MASK  0xE0
#define UCI_OID_MASK 0x3F

/* UCI Data */
#define NXP_MAX_CONFIG_STRING_LEN 260
typedef struct uci_data
{
    uint16_t len;
    uint8_t p_data[UCI_MAX_DATA_LEN];
} uci_data_t;

typedef enum
{
    HAL_STATUS_CLOSE = 0,
    HAL_STATUS_OPEN,
    HAL_STATUS_MIN_OPEN
} phNxpUci_HalStatus;

/* Macros to enable and disable extensions */
#define HAL_ENABLE_EXT()  (nxpucihal_ctrl.hal_ext_enabled = 1)
#define HAL_DISABLE_EXT() (nxpucihal_ctrl.hal_ext_enabled = 0)

/* UCI Control structure */
typedef struct phNxpUciHal_Control
{
    phNxpUci_HalStatus halStatus; /* Indicate if hal is open or closed */
    UWBOSAL_TASK_HANDLE client_thread;
    uint8_t thread_running;     /* Thread running if set to 1, else set to 0 */
    phLibUwb_sConfig_t gDrvCfg; /* Driver config data */

    /* Rx data */
    uint8_t *p_rx_data;
    uint16_t rx_data_len;

    /* libuwb-uci callbacks */
    uwb_stack_callback_t *p_uwb_stack_cback;
    uwb_stack_data_callback_t *p_uwb_stack_data_cback;

    /* HAL open status */
    bool_t hal_open_status;

    /* HAL extensions */
    uint8_t hal_ext_enabled;

    /* Waiting semaphore */
    phNxpUciHal_Sem_t ext_cb_data;

    uint16_t cmd_len;
    uint8_t p_cmd_data[UCI_MAX_PACKET_LEN];
    uint16_t rsp_len;
    uint8_t p_rsp_data[UCI_MAX_PACKET_LEN];

    /* retry count used to force download */
    uint8_t read_retry_cnt;
    bool_t IsDev_suspend_enabled;
    bool_t IsFwDebugDump_enabled;
    bool_t IsCIRDebugDump_enabled;
    /* To skip sending packets to upper layer from HAL*/
    uint8_t isSkipPacket;
    uint16_t cir_dump_len;
    bool_t fw_dwnld_mode;
    void *halClientSemaphore;
} phNxpUciHal_Control_t;

/* Internal messages to handle callbacks */
#define UCI_HAL_OPEN_CPLT_MSG  0x411
#define UCI_HAL_CLOSE_CPLT_MSG 0x412
#define UCI_HAL_ERROR_MSG      0x415

#define UCIHAL_CMD_CODE_LEN_BYTE_OFFSET (2U)
#define UCIHAL_CMD_CODE_BYTE_LEN        (3U)

/* Pointer to Firmware Image & its Size */
void setFwImage(uint8_t *fwImgPtr, uint32_t fwSize);

/******************** UCI HAL exposed functions *******************************/

uint16_t phNxpUciHal_write_unlocked(uint16_t data_len, const uint8_t *p_data);

#endif /* _PHNXPUCIHAL_H_ */
