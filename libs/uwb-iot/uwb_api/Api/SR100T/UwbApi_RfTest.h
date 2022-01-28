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

#ifndef UWBAPI_RF_TEST_H
#define UWBAPI_RF_TEST_H

#include "UwbApi_Types.h"
#include "UwbApi_Types_RfTest.h"
#include "uwa_api.h"

/**
 *  \brief APIs exposed to access Uwb Rf Test related Functionalities
 */

/**
 *  \name RF start test supported in UWB API layer.
 */
/* @{ */
typedef enum startRfParam
{
    /**  PER Tx start */
    RF_START_PER_TX = 0x0,
    /** PER Rx start */
    RF_START_PER_RX,
    /**  RF Loopback test */
    RF_LOOPBACK_TEST,
    /**  Single Rx RF TEST */
    RF_TEST_RX,
} eStartRfParam;
/* @} */

/**
 * \brief START PER TX Data
 */
/* @{ */
typedef struct
{
    /** tx data */
    UINT8 txData[MAX_UCI_PACKET_SIZE];

    /** tx data length */
    UINT16 txDataLength;

} phStartPerTxData_t;
/* @} */

/**
 * \brief START PER RX Data
 */
/* @{ */
typedef struct
{
    /** rx data */
    UINT8 rxData[MAX_UCI_PACKET_SIZE];

    /** rx data length */
    UINT16 rxDataLength;

} phStartPerRxData_t;
/* @} */

/**
 * \brief LOOPBACK Data
 */
/* @{ */
typedef struct
{
    /** loopback data */
    UINT8 loopbackData[MAX_UCI_PACKET_SIZE];

    /** loopback data length */
    UINT16 loopbackDataLength;

} phLoopbackTestData_t;
/* @} */

/**
 * \brief Rf Start Data
 */
/* @{ */
typedef union {
    /** START PER TX Data */
    phStartPerTxData_t startPerTxData;
    /** START PER RX Data */
    phStartPerRxData_t startPerRxData;
    /** LOOPBACK TEST Data */
    phLoopbackTestData_t loopbackTestData;
} phRfStartData_t;
/* @} */

/** \addtogroup Uwb_RfTest_Apis
 *
 * @{ */

/**
 * \brief Set session specific PER parameters

 * \param sessionId - [In] Initialized Session ID
 * \param perParams - [In] Pointer to \ref phRfTestParams_t
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetRfTestParams(UINT32 sessionId, const phRfTestParams_t *pRfTestParams);

/**
 * \brief Get session specific PER parameters
 *
 * \param sessionId  - [in] Initialized Session ID
 * \param pPerParams - [out] Pointer to \ref phRfTestParams_t . Valid only if API
 * status is success.
 *
 * \retval #UWBAPI_STATUS_OK              - if successful
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UCI stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetRfTestParams(UINT32 sessionId, phRfTestParams_t *pRfTestParams);

/**
 * \brief Set Test specific config parameters.
 *
 * \param param_id - [In] Test Config Parameter Id
 * \param param_value - [In] Param value for Test config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_SetTestConfig(eTestConfig param_id, UINT32 param_value);

/**
 * \brief Get Test specific config parameters.
 *
 * \param param_id      - [In]  Test Config Parameter Id
 * \param param_value   - [In]  Param value for Test config param id
 * \param param_value   - [Out] Param value for Test config param id
 *
 * \retval #UWBAPI_STATUS_OK                - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED   - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM     - if invalid parameters are passed
 * \retval #UWBAPI_STATUS_SESSION_NOT_EXIST - if session is not initialized with
 *                                            sessionId
 * \retval #UWBAPI_STATUS_TIMEOUT           - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED            - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_GetTestConfig(eTestConfig param_id, UINT32 *param_value);

/**
 * \brief RF Start test
 *
 * \param paramId      - [in] Param ID
 * \param pStartData   - [in] Start data
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_INVALID_PARAM   - if input data in null
 * \retval #UWBAPI_STATUS_REJECTED        - if rf parameters are not set
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_StartRfTest(eStartRfParam paramId, phRfStartData_t *pStartData);

/**
 * \brief RF Test stop
 *
 * \retval #UWBAPI_STATUS_OK              - on success
 * \retval #UWBAPI_STATUS_NOT_INITIALIZED - if UWB stack is not initialized
 * \retval #UWBAPI_STATUS_REJECTED        - if per parameters are not set
 * \retval #UWBAPI_STATUS_TIMEOUT         - if command is timeout
 * \retval #UWBAPI_STATUS_FAILED          - otherwise
 */
EXTERNC tUWBAPI_STATUS UwbApi_Stop_RfTest(void);
/** @} */ /* @addtogroup Uwb_RfTest_Apis */
EXTERNC void ufaTestDeviceManagementCallback(eResponse_Test_Event dmEvent, tUWA_DM_TEST_CBACK_DATA *eventData);
#endif // UWBAPI_H
