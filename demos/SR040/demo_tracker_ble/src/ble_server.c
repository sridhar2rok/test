/*! *********************************************************************************
* \addtogroup Private Profile Server
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2018 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the QPP Server application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* #undef CR_INTEGER_PRINTF to force the usage of the sprintf() function provided
 * by the compiler in this file. The sprintf() function is #included from
 * the <stdio.h> file. */
#ifdef CR_INTEGER_PRINTF
#undef CR_INTEGER_PRINTF
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "PWR_Configuration.h"

/* Framework / Drivers */
#include "stdio.h"
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "MemManager.h"
#include "Panic.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "private_profile_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "ble_server.h"
#include "fsl_debug_console.h"

/* TLV includes */
#include "TLV_Builder.h"
#include "UWB_ConfigHandler.h"

#include "UWBT_BuildConfig.h"
#include "UWBT_PowerMode.h"
#include "UWBT_Config.h"

/* UWB Handler */
#include "UWB_Handler.h"

#if (gAppNtagSupported_d)
#include "app_ntag.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mAttNotifHeaderSize_c (3) /* ATT op code + ATT handle  */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
#if gAppUseBonding_d
    fastWhiteListAdvState_c,
#endif
    fastAdvState_c,
    slowAdvState_c,
    defaultAdvState_c
} advType_t;

typedef struct advState_tag
{
    bool_t advOn;
    advType_t advType;
} advState_t;

typedef struct appPeerInfo_tag
{
    uint8_t deviceId;
    uint8_t ntf_cfg;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
/* Adv State */
static advState_t mAdvState;
//static bool_t      mRestartAdv;
static uint32_t mAdvTimeout;
/* Service Data*/
static qppsConfig_t qppServiceConfig = {service_qpps};

static uint16_t cpHandles[1] = {value_qpps_rx};

/* Application specific data*/
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
extern bool bleConnection;
#endif

/* UWB operation status */
extern QueueHandle_t mTlvMutex;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent);
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);
static void BleApp_Config(void);
static void BleApp_Advertise(void);
static void BleApp_ReceivedDataHandler(deviceId_t deviceId, uint8_t *aValue, uint16_t valueLength);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
#if defined(CPU_JN518X) && (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(UWBT_ExitPowerDownCb);
    PWR_RegisterLowPowerEnterCallback(UWBT_EnterLowPowerCb);
#endif

    // Nothing to do here at the moment
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
#if gAppUseBonding_d
    if (gcBondedDevices > 0) {
        mAdvState.advType = fastWhiteListAdvState_c;
    }
    else {
#endif
        mAdvState.advType = defaultAdvState_c;
#if gAppUseBonding_d
    }
#endif

#if (gAppNtagSupported_d)
    NtagApp_NdefPairingWr(PERIPHERAL_AND_CENTRAL_ROLE, NTAG_LOCAL_DEV_NAME, strlen(NTAG_LOCAL_DEV_NAME));
#endif
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    switch (events) {
    case gKBD_EventPressPB1_c: {
        break;
    }
    case gKBD_EventLongPB1_c: {
        break;
    }
    case gKBD_EventLongPB2_c: {
        break;
    }
    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType) {
    case gInitializationComplete_c: {
        BleApp_Config();
    } break;

    case gAdvertisingParametersSetupComplete_c: {
        (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
    } break;

    case gAdvertisingDataSetupComplete_c: {
        (void)Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    } break;

    case gTxPowerLevelSetComplete_c: {
        (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
    }

    case gAdvertisingSetupFailed_c: {
        //Serial_Print(gAppSerMgrIf, "\r\ngAdvertisingSetupFailed_c\r\n", gNoBlock_d);
        //panic(0,0,0,0);
    } break;

    default:
        break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config()
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    App_RegisterGattServerCallback(BleApp_GattServerCallback);

    /* TODO: Load required BLE configurations from flash*/

    mAdvState.advOn = FALSE;
    for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
        mPeerInformation[i].deviceId = gInvalidDeviceId_c;
    }

    Qpp_Start(&qppServiceConfig);
    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);

    mAdvState.advType = defaultAdvState_c;
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    switch (mAdvState.advType) {
#if gAppUseBonding_d
    case fastWhiteListAdvState_c: {
        gAdvParams.minInterval  = gFastConnMinAdvInterval_c;
        gAdvParams.maxInterval  = gFastConnMaxAdvInterval_c;
        gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
        mAdvTimeout             = gFastConnWhiteListAdvTime_c;
    } break;
#endif
    case fastAdvState_c: {
        gAdvParams.minInterval  = gFastConnMinAdvInterval_c;
        gAdvParams.maxInterval  = gFastConnMaxAdvInterval_c;
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
    } break;

    case slowAdvState_c: {
        gAdvParams.minInterval  = gReducedPowerMinAdvInterval_c;
        gAdvParams.maxInterval  = gReducedPowerMinAdvInterval_c;
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gReducedPowerAdvTime_c;
    } break;
    case defaultAdvState_c: {
        gAdvParams.minInterval  = UWBT_CfgReadBleInterval();
        gAdvParams.maxInterval  = UWBT_CfgReadBleInterval();
        gAdvParams.filterPolicy = gProcessAll_c;
        mAdvTimeout             = gReducedPowerAdvTime_c;
    } break;
    }

    /* Set advertising parameters*/
    Gap_SetAdvertisingParameters(&gAdvParams);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType) {
    case gAdvertisingStateChanged_c: {
        mAdvState.advOn = !mAdvState.advOn;
        if (mAdvState.advOn) {
            PWR_AllowDeviceToSleep();
            UWBT_PowerModeEnter(UWBT_POWER_DOWN_MODE);
        }
    } break;

    case gAdvertisingCommandFailed_c: {
        //Serial_Print(gAppSerMgrIf, "\r\ngAdvertisingCommandFailed_c\r\n", gNoBlock_d);
        //panic(0,0,0,0);
    } break;

    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType) {
    case gConnEvtConnected_c: {
        mAdvState.advOn = FALSE;
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        UWBT_PowerModeEnter(UWBT_RUN_MODE);
#endif /* cPWR_UsePowerDownMode */

        mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
        //Serial_Print(gAppSerMgrIf,"Connected with peerDeviceId = 0x",gNoBlock_d);
        //Serial_PrintHex(gAppSerMgrIf, &peerDeviceId, 1, gNoBlock_d);
        //Serial_Print(gAppSerMgrIf, "\r\n", gNoBlock_d);

        /* Subscribe client*/
        Qpp_Subscribe(peerDeviceId);
        PRINTF("Connected\n");

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
        bleConnection = true;
#endif

    } break;

    case gConnEvtDisconnected_c: {
        /* qpps Unsubscribe client */
        Qpp_Unsubscribe();
        mPeerInformation[peerDeviceId].ntf_cfg  = QPPS_VALUE_NTF_OFF;
        mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
        PRINTF("Disconnected\n");
        for (uint8_t i = 0; i < gAppMaxConnections_c; i++) {
            if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                break;
        }

        xSemaphoreTake(mTlvMutex, portMAX_DELAY);
        handleShutDown();
        UWBT_PowerModeEnter(UWBT_POWER_DOWN_MODE);
        BleApp_Start();
        xSemaphoreGive(mTlvMutex);

#if TAG_BUILD_CFG == MANUF_TEST_BUILD || TAG_BUILD_CFG == VALIDATION_V3_BUILD
        bleConnection = false;
#endif
    } break;
    default:
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
    uint16_t handle;
    uint8_t status;

    switch (pServerEvent->eventType) {
    case gEvtMtuChanged_c: {
        //uint8_t notifMaxPayload = 0;
        //notifMaxPayload = pServerEvent->eventData.mtuChangedEvent.newMtu - mAttNotifHeaderSize_c;
    } break;

    case gEvtAttributeWritten_c: {
        handle = pServerEvent->eventData.attributeWrittenEvent.handle;
        status = gAttErrCodeNoError_c;
        GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
    } break;

    case gEvtAttributeWrittenWithoutResponse_c: {
        handle = pServerEvent->eventData.attributeWrittenEvent.handle;

        if (handle == value_qpps_rx) {
            BleApp_ReceivedDataHandler(deviceId,
                pServerEvent->eventData.attributeWrittenEvent.aValue,
                pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        }
    } break;

    case gEvtCharacteristicCccdWritten_c: {
        handle = pServerEvent->eventData.charCccdWrittenEvent.handle;
        if (handle == cccd_qpps_tx) {
            mPeerInformation[deviceId].ntf_cfg = pServerEvent->eventData.charCccdWrittenEvent.newCccd;
        }
    } break;

    default:
        break;
    }
}

static void BleApp_ReceivedDataHandler(deviceId_t deviceId, uint8_t *aValue, uint16_t valueLength)
{
    tlvRecv(UWB_HIF_BLE, aValue, valueLength);
}

/*! *********************************************************************************
* @}
********************************************************************************** */
