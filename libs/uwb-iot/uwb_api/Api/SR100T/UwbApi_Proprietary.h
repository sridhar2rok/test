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

#ifndef UWBAPI_PROPRIETARY_H
#define UWBAPI_PROPRIETARY_H

#include "UwbCore_Types.h"
#include "UwbApi_Types_Proprietary.h"

/**
 *  \brief APIs exposed to application to access UWB Baord Specific Functionality
 */

/** \addtogroup Uwb_Board_Specific_APIs
 * @{ */
/**
  * \brief returns UCI, FW and MW version
  *
  * \param pdevInfo - [out] Pointer to \ref phUwbDevInfo_t
  *
  * \retval #UWBAPI_STATUS_OK              - if successful
  * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
  * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
  * \retval #UWBAPI_STATUS_FAILED          - otherwise
  * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
  */
EXTERNC tUWBAPI_STATUS UwbApi_GetStackCapabilities(phUwbDevInfo_t *pdevInfo);

/**
 * \brief Initialize the UWB Middleware stack with Factory Firmware
 *
 * \param pCallback - [in] Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications at
 * application layer.)
 *
 * \retval #UWBAPI_STATUS_OK            - on success
 * \retval #UWBAPI_STATUS_FAILED        - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT       - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_FactoryInit(tUwbApi_AppCallback *pCallback);

/**
 * \brief API to recover from Factory Firmware crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverFactoryUWBS();

/**
 * \brief Do calibration parameters.
 *
 * \param paramId     - [In] Channel
 * \param paramValue  - [In] DoCalibration param Id
 * \param paramcalibResp - [Out] Pointer to \ref phCalibRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoCalibration(UINT8 channel, eDoCalibParam paramId, phCalibRespStatus_t *calibResp);

/**
 * \brief Set calibration parameters.
 *
 * \param channel     - [In] channel
 * \param paramId     - [In] Calibration parameter ID
 * \param paramValue  -  [In] Calibration value
 * \param length      -  [In] Calibration value array length
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetCalibration(UINT8 channel, eCalibParam paramId, UINT8 *calibrationValue, UINT8 length);

/**
 * \brief Get calibration parameters.
 *
 * \param paramId     - [In] Channel
 * \param paramValue  - [In] Calibration param Id
 * \param paramcalibResp - [Out] Pointer to \ref phCalibRespStatus_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetCalibration(UINT8 channel, eCalibParam paramId, phCalibRespStatus_t *calibResp);

/**
 * \brief Set Uwb Debug Configuration Parameters
 *
 * \param sessionId    - [in] Initialized Session ID
 * \param pDebugParams - [in] Pointer to \ref phDebugParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetDebugParams(UINT32 sessionId, const phDebugParams_t *pDebugParams);

/**
 * \brief Get Uwb Debug Configuration Parameters
 *
 * \param sessionId    - [in] Initialized Session ID
 * \param pDebugParams - [out] Pointer to \ref phDebugParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetDebugParams(UINT32 sessionId, phDebugParams_t *pDebugParams);

/**
 * \brief DoBind Perform Factory Binding using this API
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_DoBind(phSeDoBindStatus_t *doBindStatus);

/**
 * \brief Get the binding count using this API
 *
 * \param getBindingCount    - [out] getBindingCount data. valid only if API
 * status is success
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingCount(phSeGetBindingCount_t *getBindingCount);

/**
 * \brief TestConnectivity Perform SE connectivity test using this API
 *
 * \param wtxCount                       - [out] WTX count
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_TestConnectivity(UINT8 *wtxCount);

/**
 * \brief TestLoop Perform SE loop test using this API
 *
 * \param loopCnt       - [In] No of times test to be run
 * \param timeInterval  - [In] time interval in ms
 * \param testLoopData  - [out] Test loop notification data
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_SeTestLoop(UINT16 loopCnt, UINT16 timeInterval, phTestLoopData_t *testLoopData);

/**
 * \brief API to get current binding status
 * Use of this API will lock binding status if UWBD is in unlock state
 * \param  getBindingStatus  - [In] Binding status notification
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetBindingStatus(phSeGetBindingStatus_t *getBindingStatus);

/**
 * \brief API to query states of all the existing sessions.
 *
 * \param  pUwbSessionsContext - [Out] Session data. Valid only if API status
 * success
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAllUwbSessions(phUwbSessionsContext_t *pUwbSessionsContext);

/**
 * \brief API to get the current temperature
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_QueryTemperature(UINT8 *pTemperatureValue);
/* @}  */

#endif // UWBAPI_PROPRIETARY_H
