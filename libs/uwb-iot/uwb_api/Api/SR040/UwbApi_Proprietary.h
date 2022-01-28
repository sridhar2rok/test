/*
 * Copyright 2019,2020 NXP
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

#ifndef UWBAPI_PROPRIETARY_H
#define UWBAPI_PROPRIETARY_H

#include "UwbCore_Types.h"
#include "UwbApi_Types_Proprietary.h"

/** \addtogroup uwb_apis_sr040
 *
 * @{ */

/**
  * brief returns UCI, FW and MW version
  *
  * \param pdevInfo - [out] Pointer to \ref phUwbDevInfo_t
  *
  * \retval UWBAPI_STATUS_OK              - if successful
  * \retval UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
  * \retval UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
  * \retval UWBAPI_STATUS_FAILED          - otherwise
  * \retval UWBAPI_STATUS_TIMEOUT         - if command is timeout
  */
EXTERNC tUWBAPI_STATUS UwbApi_GetStackCapabilities(phUwbDevInfo_t *pdevInfo);

/**
 * \brief APIs exposed to application to access UWB Functionality of SR040.
 */

/**
 * brief API to put the device in HPD mode
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SuspendDevice(void);

/**
 * @brief      Set core config DPD Timeout for SR040
 *
 * @param[in]  dpd_timeout  The dpd timeout
 *
 * Max value 5000.  Min value, determined by the host
 *
 * @return     Status of setting the value.
 */
EXTERNC tUWBAPI_STATUS UwbApi_PropSR040_SetDPDTimeout(uint16_t dpd_timeout);

/**
 * @brief      Set core config HPD Timeout for SR040
 *
 * @param[in]  dpd_timeout  The hpd timeout
 *
 * Max value 5000.  Min value, determined by the host
 *
 * @return     Status of setting the value.
 */
EXTERNC tUWBAPI_STATUS UwbApi_PropSR040_SetHPDTimeout(uint16_t hpd_timeout);

/**
 * brief API to mange the session related info in NVM
 *
 * \param sesNvmManageTag - [in] Session NVM manage tag field
 * \param sessionid       - [in] Session NVM manage session id field.
 * [Not present for the tag SESSION_NVM_MANAGE_DELETE_ALL. But pass sessionid = 0 [Any value] which is ignored ]
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if tag option is incorrect
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionNvmManage(esessionNvmManage sesNvmManageTag, UINT32 sessionid);

/**
 * brief API to start the device in Continuous Wave Mode
 *
 * \param testMode - [In] Test Mode to be started
 * \param testModeParams - [In] param used for the corresponding test mode
 * \param testModeOutputData - [Out] param used for test mode output values
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if any of the input parameter is NULL or invalid value
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartTestMode(eTestMode testMode, const phTestModeParams *testModeParams);

/**
 * brief API to stop Test Mode
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopTestMode();

/**
 * brief Set calibration parameters.
 *
 * \param paramId     - [In] Calibration parameter ID
 * \param pCalibData  -  [In] Calibration Data
 * \param calibResp   -  [Out] Calibration Status
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(
    eCalibParam paramId, phCalibrationData *pCalibData, phCalibrationStatus_t *calibResp);

/**
 * brief Radio Config Get CRC api.
 *
 * \param radioConfigIndex - [In] Radio Configuration index value. 0x00 - 0x0F for Rx and 0x10 - 0x1F for Tx
 * \param pCrc - [Out] Radio Config CRC
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW   - if response buffer is not sufficient to hold the response
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_RadioConfigGetCrc(UINT8 radioConfigIndex, UINT16 *pCrc);

/**
 * brief Radio Config Get Version api.
 *
 * \param radioConfigIndex - [In] Radio Configuration index value. 0x00 - 0x0F for Rx and 0x10 - 0x1F for Tx
 * \param pRadioConfigGetVersionResp - [Out] Radio Config Get Version Response
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW   - if response buffer is not sufficient to hold the response
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_RadioConfigGetVersion(
    UINT8 radioConfigIndex, phRadioConfigGetVersionResp_t *pRadioConfigGetVersionResp);

/** @}  */ /* @addtogroup uwb_apis_sr040 */

#endif // UWBAPI_PROPRIETARY_H
