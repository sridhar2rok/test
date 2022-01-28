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

#ifndef UWBAPI_H
#define UWBAPI_H

#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"
#include "UwbApi_Types.h"
#include "UwbApi_Types_Proprietary.h"
#include "UwbApi_Internal.h"
#include "UwbApi_Proprietary.h"

/**
 * \brief APIs exposed to application to access UWB Functionality.
 */

/** \addtogroup Uwb_Apis
 * @{ */

/**
 * \brief Initialize the UWB Middleware stack
 *
 * \param pCallback - [in] Pointer to \ref tUwbApi_AppCallback
 *                         (Callback function to receive notifications (Ranging
 * data/App Data/Per Tx & Rx) at application layer.)
 *
 * \retval #UWBAPI_STATUS_OK            - on success
 * \retval #UWBAPI_STATUS_TIMEOUT       - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED        - otherwise
 *
 */
EXTERNC tUWBAPI_STATUS UwbApi_Init(tUwbApi_AppCallback *pCallback);

/**
 * \brief De-initializes the UWB Middleware stack.
 *
 * \retval #UWBAPI_STATUS_OK      - on success
 * \retval #UWBAPI_STATUS_FAILED  - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_ShutDown();

/**
 * \brief API to recover from crash, cmd timeout.
 *
 * \retval #UWBAPI_STATUS_OK      - on success
 * \retval #UWBAPI_STATUS_FAILED  - otherwise
 * \retval #UWBAPI_STATUS_TIMEOUT - if command is timeout
 */
EXTERNC tUWBAPI_STATUS UwbApi_RecoverUWBS();

/**
 * \brief Resets UWBD device to Ready State
 *
 * \param resetConfig - [in] Supported Value: UWBD_RESET
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_UwbdReset(UINT8 resetConfig);

/**
 * \brief Gets UWB Device State
 *
 * \param pDeviceState - [out] pointer to UINT8 to get Device State. Valid only
 * if API status is success.
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if parameter is invalid
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetUwbDevState(UINT8 *pDeviceState);

/**
 * \brief Initializes session for a Type(Ranging/Data/Per)
 *
 * \param sessionId   - [In] Unique Session ID
 * \param sessionType - [In] Type of Session(Ranging/Data/Per)
 *
 * \retval #UWBAPI_STATUS_OK                    - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED       - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_DUPLICATE     - if session with the session ID
 *                                                passed already exists
 * \retval #UWBAPI_STATUS_MAX_SESSIONS_EXCEEDED - if more than 5 sessions are
 *                                                exceeded
 * \retval #UWBAPI_STATUS_TIMEOUT               - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED                - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionInit(UINT32 sessionId, eSessionType sessionType);

/**
 * \brief De-initialize based on Session ID
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SessionDeinit(UINT32 sessionId);

/**
 * \brief Set session specific ranging parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param pRangingParam - [In] Pointer to \ref phRangingParams_t
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRangingParams(UINT32 sessionId, const phRangingParams_t *pRangingParam);

/**
 * \brief Get session specific ranging parameters

 * \param sessionId     - [In] Initialized Session ID
 * \param pRangingParam - [Out] Pointer to \ref phRangingParams_t .Valid only if
 API status is success
 *
 * \retval #UWBAPI_STATUS_OK on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetRangingParams(UINT32 sessionId, phRangingParams_t *pRangingParams);

/**
 * \brief Set session specific app config parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param param_id - [In] App Config Parameter Id
 * \param param_value - [In] Param value for App config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 **/

EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfig(UINT32 sessionId, eAppConfig param_id, UINT32 param_value);

/**
 * \brief Host shall use this API to set multiple application confifuration parameters.
 * Number of Parameters also needs to be indicated.
 *
 * \param sessionId      - [In] Initialized Session ID
 * \param noOfparams     - [In] Number of App Config Parameters
 * \param AppParams_List - [In] Application parameters values in tlv format
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetAppConfigMultipleParams(
    UINT32 sessionId, UINT8 noOfparams, const SetAppParams_List_t *AppParams_List);

/**
 * \brief Get session specific app config parameters.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param param_id - [In] App Config Parameter Id
 * \param param_value - [Out] Param value for App config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetAppConfig(UINT32 sessionId, eAppConfig param_id, UINT32 *param_value);

/**
 * \brief Sets session specific app config parameters Vendor ID and Static STS
 * IV.
 *
 * \param sessionId     - [In] Initialized Session ID
 * \param vendorId      - [In] App Config Parameter Vendor Id
 * \param staticStsIv   - [In] Param value for App config param static Sts Iv
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetStaticSts(UINT32 sessionId, UINT16 vendorId, UINT8 *staticStsIv);

/**
 * \brief Start Ranging for a session. Before Invoking Start ranging its
 * mandatory to set all the ranging configurations.
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                           sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRangingSession(UINT32 sessionId);

/**
 * \brief Stop Ranging for a session
 *
 * \param sessionId - [In] Initialized Session ID
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StopRangingSession(UINT32 sessionId);

/**
 * \brief Enable Ranging Data Notifications different options.

 * \param sessionId            - [In] Initialized Session ID
 * \param enableRangingDataNtf - [In] Enable Ranging data notification - 0/1/2.
 option 2 is not allowed when ranging is ongoing.
 * \param proximityNear        - [In] Proximity Near value valid if
 enableRangingDataNtf sets to 2
 * \param proximityFar         - [In] Proximity far value valid if
 enableRangingDataNtf sets to 2
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_EnableRangingDataNtf(
    UINT32 sessionId, UINT8 enableRangingDataNtf, UINT16 proximityNear, UINT16 proximityFar);

/**
 * \brief Get Session State
 *
 * \param sessionID    - [in] Initialized Session ID
 * \param sessionState - [out] Session Status
 *
 * \retval #UWBAPI_STATUS_OK               - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED  - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM    - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT          - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED           - otherwise
 *
 * if API returns #UWBAPI_STATUS_OK, Session State would be one of the below
 * values
 * \n #UWBAPI_SESSION_INIT_SUCCESS     - Session is Initialized
 * \n #UWBAPI_SESSION_DEINIT_SUCCESS   - Session is De-initialized
 * \n #UWBAPI_SESSION_ACTIVATED        - Session is Busy
 * \n #UWBAPI_SESSION_IDLE             - Session is Idle
 * \n #UWBAPI_SESSION_ERROR            - Session Not Found
 *
 * if API returns not #UWBAPI_STATUS_OK, Session State is set to
 * #UWBAPI_SESSION_ERROR
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetSessionState(UINT32 sessionId, UINT8 *sessionState);

/**
 * \brief Send UCI RAW command.
 *
 * \param data     - [in] UCI Command to be sent
 * \param data_len - [in] Length of the UCI Command
 * \param resp     - [out] Response Received
 * \param respLen  - [out] Response length
 *
 * \retval #UWBAPI_STATUS_OK              - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if wrong parameter is passed
 * \retval #UWBAPI_STATUS_BUFFER_OVERFLOW - if response buffer is not sufficient
 *                                          to hold the response
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SendRawCommand(UINT8 data[], UINT16 data_len, UINT8 *pResp, UINT16 *pRespLen);

/**
 * \brief Update Controller Multicast List.
 *
 * \param pControleeContext - [In] Controlee Context
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_UpdateControllerMulticastList(phMulticastControleeListContext_t *pControleeContext);
/** @}  */ /* @addtogroup Uwb_Apis */

#endif // UWBAPI_H
