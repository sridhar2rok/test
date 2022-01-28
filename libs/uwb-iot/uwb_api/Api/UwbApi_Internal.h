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

#ifndef UWBAPI_INTERNAL_H
#define UWBAPI_INTERNAL_H

#include "phUwb_BuildConfig.h"
#include "UwbCore_Types.h"
#include "UwbApi_Types.h"
#include "UwbApi_Types_RfTest.h"
#include "UwbApi_Types_Proprietary.h"
#include "uwa_api.h"

/**
 * \brief Structure for storing  UWB API Context.
 */
typedef struct phUwbApiContext
{
    /** Semaphore used for making UWB APIs synchronous, it will *
     * be signaled when UCI response/notification  is received */
    void *devMgmtSem;
    /** Data to be send to device */
    UINT8 snd_data[MAX_UCI_PACKET_SIZE];
    /** Common response buffer to store *
     * response/ntfn received */
    UINT8 rsp_data[MAX_UCI_PACKET_SIZE];
    /** Session data */
    phUwbSessionInfo_t sessionInfo;
    /** Generic callback registered by Application */
    tUwbApi_AppCallback *pAppCallback;
    /** device Info */
    phUwbDevInfo_t devInfo;
    /** Current Event on which UWB API  is waiting for
     * response/ntfn */
    UINT16 currentEventId;
    UINT16 receivedEventId;
    /** Session Count */
    INT8 sessionCnt;
    /** Session State */
    UINT8 sessionState;
    /** Ranging Count */
    INT32 rangingCnt;
    /** Length of the response received */
    UINT16 rsp_len;
    /** Current device state *
     * UCI_DEV_IDLE/UCI_DEV_READY/UCI_DEV_BUSY/UCI_DEV_ERROR */
    UINT8 dev_state;
    /** Current per state */
    UINT8 perState;
    /** Holds status of the current ongoing operations */
    tUWBAPI_STATUS wstatus;
    /** This is set after UWB stack is initialized */
    BOOLEAN isUfaEnabled;
#if (UWBIOT_UWBD_SR100T)
    /** Loop Test notification data */
    phTestLoopData_t testLoopData;
    /** Connectivity test notification data */
    phTestConnectivityData_t testConnectivityData;
    /** Get Binding status notification data */
    phSeGetBindingStatus_t getBindingStatus;
    /** Do Bind status notification data */
    phSeDoBindStatus_t doBindStatus;
    /** Do Calibration notification data */
    phCalibRespStatus_t doCalibrationStatus;
#else
    /** Calibration status notification data */
    phCalibrationStatus_t calibrationStatus;
    /** Test Loopback status data */
    phTestLoopbackData_t testLoopbackStatus;
    /** Test Initiator Range data */
    phTestInitiatorRangekData_t testInitiatorRangeStatus;
    phPhyLogNtfnData_t testPhyLogNtfnData;

#endif

} phUwbApiContext_t;

extern phUwbApiContext_t uwbContext;

void cleanUp();
tUWBAPI_STATUS uwbInit(tUwbApi_AppCallback *pCallback);
tUWBAPI_STATUS recoverUWBS();
void sep_SetWaitEvent(UINT16 eventID);
tUWBAPI_STATUS getDeviceInfo(void);
tUWBAPI_STATUS sendRawUci(UINT8 *p_cmd_params, UINT16 cmd_params_len);
tUWBAPI_STATUS waitforNotification(UINT16 waitEventId, UINT32 waitEventNtftimeout);
UINT8 getAppConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer);
UINT8 getTestConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer);
UINT8 getCoreDeviceConfigTLVBuffer(UINT8 paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer);
void parseRangingParams(UINT8 *rspPtr, UINT8 noOfParams, phRangingParams_t *pRangingParams);
void ufaDeviceManagementCallback(eResponse_Event dmEvent, tUWA_DM_CBACK_DATA *eventData);
tUWBAPI_STATUS AppConfig_TlvParser(
    const SetAppParams_List_t *pAppParams_List, SetAppParams_value_au8_t *pOutput_param_value);
#if (UWBIOT_UWBD_SR040)
void ufaTestDeviceManagementCallback(eResponse_Test_Event dmEvent, tUWA_DM_TEST_CBACK_DATA *eventData);
#endif
/** @brief API to add TLV to tlvBuffer
 *
 * @param[paramId]    Parameter ID (tag) to be configured
 * @param[paramLen]   Length of paramValue
 * @param[paramValue] Pointer to value to be set
 * @param[tlvBuffer]  Output buffer to contain TLV (MUST be allocated by application)
 */
EXTERNC UINT8 tlvSet_appConfigBuf(eAppConfig paramId, UINT8 paramLen, void *paramValue, UINT8 *tlvBuffer);

#endif
