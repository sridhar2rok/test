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

#ifndef PHUWB_CONFIGURATION_H
#define PHUWB_CONFIGURATION_H

#include "UwbCore_Types.h"

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include <uwbiot_ver.h>

/* Rhodes version */

/**  Major Version */
#define RHODES_MW_MAJOR_VERSION (UWBIOTVER_STR_VER_MAJOR)
/**  Minor Version */
#define RHODES_MW_MINOR_VERSION (UWBIOTVER_STR_VER_MINOR)

/* R4 version */
/**  Major Version */
#define R4_MW_MAJOR_VERSION (UWBIOTVER_STR_VER_MAJOR)
/**  Minor Version */
#define R4_MW_MINOR_VERSION (UWBIOTVER_STR_VER_MINOR)

/* Build UWB Middle-ware for a specific Mode */
#define UWB_BUILD_STANDALONE_DEFAULT  1
#define UWB_BUILD_STANDALONE_TEST     2
#define UWB_BUILD_PLUG_AND_PLAY_MODE  3
#define UWB_BUILD_STANDALONE_CDC_MODE 4
#define UWB_BUILD_STANDALONE_WITH_BLE 5
#define UWBCORE_SDK_BUILDCONFIG       UWB_BUILD_STANDALONE_DEFAULT

/* Select Board version here */
#define UWB_BOARD_RHODES_V3      3
#define UWB_BOARD_RHODES_V2      2
#define UWB_BOARD_RHODES_VERSION UWB_BOARD_RHODES_V3

/* Select Platform */
#define NXP_UWB_EXTNS TRUE

// Board Variants are defined below:
#define BOARD_VARIANT_NXPREF   0x01
#define BOARD_VARIANT_CUSTREF1 0x2A
#define BOARD_VARIANT_CUSTREF2 0x2B
#define BOARD_VARIANT_RHODES   0x73

/* Select Board Variant here */
#define BOARD_VARIANT BOARD_VARIANT_RHODES

#define ENABLED  1
#define DISABLED 0

#if (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE)
/* Internal Firmware Download is DISABLED by Default */
/* Enable Firmware if required by setting INTERNAL_FIRMWARE_DOWNLOAD to ENABLED */
#define INTERNAL_FIRMWARE_DOWNLOAD DISABLED

/* By Default UCI and HBCI Command and Response logging is disabled */
/* Enable Debugging if needed by Setting to ENABLED */
#define ENABLE_UCI_CMD_LOGGING  DISABLED
#define ENABLE_HBCI_CMD_LOGGING DISABLED
#define ENABLE_UWB_RESPONSE     ENABLED

#define USB_TASK_SIZE        512
#define HELIOS_TASK_SIZE     512
#define UCI_READER_TASK_SIZE 512
#define USB_WRITER_TASK_SIZE 512

#define WRITER_QUEUE_SIZE 100

#define TRACE_UCI           PRINTF
#define TRACE_HBCI          PRINTF
#define MAX_RSP_PACKET_SIZE 2176
#else
#if ((UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_DEFAULT) || \
     (UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_TEST))
/* disable SE related features */
#define ENABLE_NFC FALSE
#elif UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_CDC_MODE
/* enable SE related features */
#define ENABLE_NFC TRUE
#elif UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_STANDALONE_WITH_BLE
/* Set Rhodes BLE Mode Here */
#define TAG                     1 /* BLE Always on SCAN Mode */
#define ANCHOR                  2 /* BLE Always on Advertising Mode */
#define RHODES_OPERATIONAL_MODE ANCHOR
/* To enable/disable SE related features */
#define ENABLE_NFC              TRUE
#endif

/* Enable the below option is Factory Mode is needed in Rhodes SDK */
#define FACTORY_MODE                DISABLED

//#if defined (DEBUG)
#define ENABLE_PRINT_UTILITY_TRACES FALSE
//#endif //(DEBUG)
#endif

#endif
