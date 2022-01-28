/******************************************************************************
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *  Copyright 2018-2019 NXP.
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

#include <stdio.h>
#include <inttypes.h>
#include <phNxpUwbConfig.h>

#include "phOsalUwb.h"
#include "phNxpLogApis_HalUci.h"

/*
  * 0x01 ID: LOW_POWER_MODE : default value 0x00
  * 0xE4 0x00 : Delay Calibration : default value : 0x3AC0 for all channels
  * 0xE4 0x01 : Antenna spacing : default value : 0x28000000 total of 128 bytes
  * 0xE4 0x02 : DPD wakeup source : default value : 0x00
  * 0xE4 0x03 : WTX count config : default value : 20 (0x14)
  * */

/* clang-format off */

const uint8_t phNxpUciHal_core_configs[] =
{
    0x9E, 0x20, 0x04, 0x00, 0x9A, 0x05,
    0x01, 0x01, 0x00,
    0xE4, 0x00, 0x08, 0xC0, 0x3A, 0xC0, 0x3A, 0xC0, 0x3A, 0xC0, 0x3A,
    0xE4, 0x01, 0x80, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
                      0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28,
    0xE4, 0x02, 0x01, 0x00,
    0xE4, 0x03, 0x01, 0x14
};
/* clang-format on */

const NxpParam_t phNxpUciHal_NXPConfig[] = {
    /*
     *  Ranging session period: which means, it is the duration of one 1:N
     session and the interval before start of next 1:N session. (1:N ranging
     period + interval between two consecutive  1:N cycles)
        UWB_RANG_SESSION_INTERVAL= (UWB_RANG_CYCLE_INTERVAL * No_of_anchors) +
     IDLE_BEFORE_START_OF_NEXT_1_N Value is in milliseconds This config is valid
     only in 1:N ranging session
     * */
    {UWB_RANG_SESSION_INTERVAL, TYPE_VAL, CONFIG_VAL 2000},
    /*
     *  Application session timeout: How log ranging shall continue.
        value is in milliseconds
        0 value: MW shall configure the value which is passed from application
        Non zero value: MW shall configure timeout with this config value
     provided here
     * */
    {UWB_APP_SESSION_TIMEOUT, TYPE_VAL, CONFIG_VAL 3600000},
    /*
     *  Ranging cycle interval: intreval between two consecutive SS/DS-TWR
     ranging cycle value is in milliseconds 0 value: MW shall configure the
     value which is passed from application Non zero value: MW shall configure
     interval with this config value provided here
     * */
    {UWB_RANG_CYCLE_INTERVAL, TYPE_VAL, CONFIG_VAL 200},
    /*
     *
     *  Timeout value in milliseconds for UWB standby mode.
        The range is between 5000 msec to 20000 msec and zero is to disable
     * */
    {UWB_STANDBY_TIMEOUT_VALUE, TYPE_VAL, CONFIG_VAL 0x00},
    /*
    *FW log level for each above module
     Logging Level Error            0x0001
     Logging Level Warning          0x0002
     Logging Level Timestamp        0x0004
     Logging Level Sequence Number  0x0008
     Logging Level Info-1           0x0010
     Logging Level Info-2           0x0020
     Logging Level Info-3           0x0040
     * */
    {UWB_SET_FW_LOG_LEVEL, TYPE_VAL, CONFIG_VAL 0x003},
    /*
     * Enable/disable to dump FW binary log for different Modules as below
       0x00 for disable the binary log
       Secure Thread      0x01
       Secure ISR         0x02
       Non-Secure ISR     0x04
       Shell Thread       0x08
       PHY Thread         0x10
       Ranging Thread     0x20
     * */
    {UWB_FW_LOG_THREAD_ID, TYPE_VAL, CONFIG_VAL 0x00},
    /*
     * Ranging feature:  Single Sided Two Way Ranging or Double Sided Two Way
     Ranging SS-TWR =0x00 DS-TWR =0x01
     */
    {UWB_MW_RANGING_FEATURE, TYPE_VAL, CONFIG_VAL 0x01},
    /*Board Varaints are defined below:
    BOARD_VARIANT_NXPREF    0x01
    BOARD_VARIANT_CUSTREF1  0x2A
    BOARD_VARIANT_CUSTREF2  0x2B
    BOARD_VARIANT_RHODES    0x73*/
    {UWB_BOARD_VARIANT_CONFIG, TYPE_VAL, CONFIG_VAL 0x73},
    /*
     * # Minor versions
        VERSION_V1   0x01
        VERSION_V2   0x02
     * */
    {UWB_BOARD_VARIANT_VERSION, TYPE_VAL, CONFIG_VAL UWB_BOARD_RHODES_VERSION},
    {UWB_CORE_CONFIG_PARAM, TYPE_DATA, phNxpUciHal_core_configs},
    /* Timeout for Firmware to enter DPD mode
     * Note: value set for UWB_DPD_ENTRY_TIMEOUT shall be in MilliSeconds.
     * Min : 100ms
     * Max : 2000ms */
    {UWB_DPD_ENTRY_TIMEOUT, TYPE_VAL, CONFIG_VAL 500},
    /* Firmware Low Power Mode
     * if UWB_LOW_POWER_MODE is 0, Firmware is Configured in non Low Power Mode
     * if UWB_LOW_POWER_MODE in 1, Firmware is Configured with Low Power Mode */
    {UWB_LOW_POWER_MODE, TYPE_VAL, CONFIG_VAL 0x01},
};

/*******************************************************************************
**
** Function:    phNxpUciHal_NxpParamFind
**
** Description: search if a setting exist in the setting array
**
** Returns:     pointer to the setting object
**
*******************************************************************************/
const NxpParam_t *phNxpUciHal_NxpParamFind(const unsigned char key)
{
    int i;
    int listSize;

    listSize = (sizeof(phNxpUciHal_NXPConfig) / sizeof(NxpParam_t));

    if (listSize == 0)
        return NULL;

    for (i = 0; i < listSize; ++i) {
        if (phNxpUciHal_NXPConfig[i].key == key) {
            if (phNxpUciHal_NXPConfig[i].type == TYPE_DATA) {
                NXPLOG_UCIHAL_D("%s found key %d, data len = %d\n",
                    __func__,
                    key,
                    *((unsigned char *)(phNxpUciHal_NXPConfig[i].val)));
            }
            else {
                NXPLOG_UCIHAL_D(
                    "%s found key %d = (0x% " PRIxPTR ")\n", __func__, key, (uintptr_t)phNxpUciHal_NXPConfig[i].val);
            }
            return &(phNxpUciHal_NXPConfig[i]);
        }
    }
    return NULL;
}

/*******************************************************************************
**
** Function:    GetStrValue()
**
** Description: API function for getting a string value of a setting
**
** Returns:     True if found, otherwise False.
**
*******************************************************************************/
extern int phNxpUciHal_GetNxpStrValue(unsigned char key, char *pValue, unsigned long len)
{
    if (!pValue)
        return FALSE;

    const NxpParam_t *pParam = phNxpUciHal_NxpParamFind(key);

    if (pParam == NULL)
        return FALSE;

    if ((pParam->type == TYPE_STR) && (pParam->val != NULL) && (strlen((const char *)pParam->val) <= len)) {
        phOsalUwb_SetMemory(pValue, 0, len);
        phOsalUwb_MemCopy(pValue, pParam->val, (int32_t)strlen((const char *)pParam->val));
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
**
** Function:    GetByteArrayValue()
**
** Description: Read byte array value from the config file.
**
** Parameters:
**              name    - name of the config param to read.
**              pValue  - pointer to input buffer.
**              len     - input buffer length.
**              readlen - out parameter to return the number of bytes read from
**                        config file
**                        return -1 in case bufflen is not enough.
**
** Returns:     TRUE[1] if config param name is found in the config file, else
**              FALSE[0]
**
*******************************************************************************/
extern int phNxpUciHal_GetNxpByteArrayValue(unsigned char key, void **pValue, long *readlen)
{
    if (!pValue)
        return FALSE;

    const NxpParam_t *pParam = phNxpUciHal_NxpParamFind(key);

    if (pParam == NULL)
        return FALSE;

    if ((pParam->type == TYPE_DATA) && (pParam->val != 0)) {
        *pValue  = &(((unsigned char *)pParam->val)[1]);
        *readlen = (long)((unsigned char *)(pParam->val))[0];
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
**
** Function:    GetNumValue
**
** Description: API function for getting a numerical value of a setting
**
** Returns:     true, if successful
**
*******************************************************************************/
extern int phNxpUciHal_GetNxpNumValue(unsigned char key, void *pValue, unsigned long len)
{
    if (!pValue)
        return FALSE;

    const NxpParam_t *pParam = phNxpUciHal_NxpParamFind(key);

    if (pParam == NULL)
        return FALSE;

    size_t v = (size_t)pParam->val;

    switch (len) {
    case sizeof(unsigned long):
        *((unsigned long *)(pValue)) = (unsigned long)v;
        break;
    case sizeof(unsigned short):
        *((unsigned short *)(pValue)) = (unsigned short)v;
        break;
    case sizeof(unsigned char):
        *((unsigned char *)(pValue)) = (unsigned char)v;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
