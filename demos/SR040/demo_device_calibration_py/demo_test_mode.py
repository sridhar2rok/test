# Copyright 2020 NXP
#
# This software is owned or controlled by NXP and may only be used
# strictly in accordance with the applicable license terms.  By expressly
# accepting such terms or by downloading, installing, activating and/or
# otherwise using the software, you are agreeing that you have read, and
# that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you
# may not retain, install, activate or otherwise use the software.
#

import sys
import pnp_core as pnp
from test_mode_config import *

# RX Status config bits
uLogB0_RX_STATUS_AND_ERROR =  (1 << 0)
uLogB0_TX_STATUS_AND_ERROR =  (1 << 1)
uLogB0_CIR_LOG             =  (1 << 2)
uLogB0_UWB_SESSION_ID      =  (1 << 3)
uLogB0_BLOCK_INDEX         =  (1 << 4)
uLogB0_RX_TIMESTAMP        =  (1 << 5)
uLogB0_TX_TIMESTAMP        =  (1 << 6)
uLogB0_RX_PSDU             =  (1 << 7)

uLogB1_TX_PSDU                     =  (1 << (8 - 8))
uLogB1_STS_INDEX                   =  (1 << (9 - 8))
uLogB1_RX_FIRST_PATH_INFO          =  (1 << (10 - 8))
uLogB1_RX_CARRIER_FREQUENCY_OFFSET =  (1 << (11 - 8))
uLogB1_RX_MACSTATUS                =  (1 << (12 - 8))
uLogB1_RX_MAC_DECODEFAILRSN        =  (1 << (13 - 8))



# Param definitions
UCI_PARAM_ID_STS_INDEX = 0x0A
UCI_EXT_PARAM_ID_D_URSK = 0xED
UCI_EXT_PARAM_ID_D_URSK_LEN = 0x11
UCI_EXT_PARAM_ID_SALTED_HASH = 0xEC
UCI_EXT_PARAM_ID_SALTED_HASH_LEN = 0x11

# Test configs
ENABLE_HPRF = False
ENABLE_LOGS = True

# Other configs
RESET_CMD = 0x04
CHANNEL_ID = 0x09
TX_POWER_ID = 0

# Enable either of these
DO_TEST_MODE_CONTINUOUS_WAVE    = 0
DO_TEST_MODE_TX_ONLY            = 0
DO_TEST_MODE_RX_ONLY            = 1

if (DO_TEST_MODE_CONTINUOUS_WAVE + DO_TEST_MODE_TX_ONLY + DO_TEST_MODE_RX_ONLY ) > 1:
    print("Enable only one, exiting")
    sys.exit(1)

if (DO_TEST_MODE_CONTINUOUS_WAVE + DO_TEST_MODE_TX_ONLY + DO_TEST_MODE_RX_ONLY ) == 0:
    print("Enable only one, exiting")
    sys.exit(1)

def getUciParamIdStsIndexBuffer():
    STS_INDEX_ARRAY = [ 0x00, 0x00, 0x00, 0x00]
    UWB_SetAppConfig_STS_INDEX = [ UCI_PARAM_ID_STS_INDEX, 0x04,  ]
    UWB_SetAppConfig_STS_INDEX.extend(STS_INDEX_ARRAY)
    return UWB_SetAppConfig_STS_INDEX

def getUciExtParamIdDUrskBuffer():
    STS_KEY_D_URSK_ARRAY = [
        0, # Plain
        0x14, 0x14, 0x86, 0x74, 0xD1, 0xD3, 0x36, 0xAA, 0xF8, 0x60, 0x50, 0xA8, 0x14, 0xEB, 0x22, 0x0F
    ]
    UWB_SetAppConfig_URSK = [ UCI_EXT_PARAM_ID_D_URSK, UCI_EXT_PARAM_ID_D_URSK_LEN,  ]
    UWB_SetAppConfig_URSK.extend(STS_KEY_D_URSK_ARRAY)
    return UWB_SetAppConfig_URSK

def getUciExtParamIdSaltedHashBuffer():
    STS_SALTED_HASH_ARRAY = [
        0, #
        0x36, 0x2E, 0xEB, 0x34, 
        0xC4, 0x4F, 0xA8, 0xFB, 
        0xD3, 0x7E, 0xC3, 0xCA, 
        0x1F, 0x9A, 0x3D, 0xE4
    ]
    UWB_SetAppConfig_STS_SALTED_HASH_ARRAY = [ UCI_EXT_PARAM_ID_SALTED_HASH, UCI_EXT_PARAM_ID_SALTED_HASH_LEN,  ]
    UWB_SetAppConfig_STS_SALTED_HASH_ARRAY.extend(STS_SALTED_HASH_ARRAY)
    return UWB_SetAppConfig_STS_SALTED_HASH_ARRAY

def getUciLoggingBuffer():
    UwbApi_SetCoreConfigs_LOGGING = [ 0x20, 0x04, 0x00, 16,
        # No of parameters
        0x04,
        # DEBUG_LOG_LEVEL
        0xF3, 0x01, 0x01,
        # RX_PHY_LOGGING_ENBL
        0xF4, 0x01, 0x01,
        # TX_PHY_LOGGING_ENBL
        0xF5, 0x01, 0x01,
        # LOG_PARAMS_CONF
        0xF6, 0x04,
    ]
    UWB_SetAppConfig_LOGGING_CONFIG = [ 0x00, 0x00, 0x00, 0x00, ]
    UWB_SetAppConfig_LOGGING_CONFIG[0] = UWB_SetAppConfig_LOGGING_CONFIG[0] | uLogB0_RX_STATUS_AND_ERROR | uLogB0_RX_TIMESTAMP
    if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP0:
        UWB_SetAppConfig_LOGGING_CONFIG[0] = UWB_SetAppConfig_LOGGING_CONFIG[0] | uLogB0_RX_PSDU
    
    UWB_SetAppConfig_LOGGING_CONFIG[1] = UWB_SetAppConfig_LOGGING_CONFIG[1] | uLogB1_RX_CARRIER_FREQUENCY_OFFSET
    if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP3:
        UWB_SetAppConfig_LOGGING_CONFIG[1] = UWB_SetAppConfig_LOGGING_CONFIG[1] | uLogB1_RX_FIRST_PATH_INFO
    if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP0:
        UWB_SetAppConfig_LOGGING_CONFIG[1] = UWB_SetAppConfig_LOGGING_CONFIG[1] | uLogB1_RX_MACSTATUS
    if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP0:
        UWB_SetAppConfig_LOGGING_CONFIG[1] = UWB_SetAppConfig_LOGGING_CONFIG[1] | uLogB1_RX_MAC_DECODEFAILRSN
    
    UwbApi_SetCoreConfigs_LOGGING.extend(UWB_SetAppConfig_LOGGING_CONFIG)
    return UwbApi_SetCoreConfigs_LOGGING

# Define all commands
SR040_HardReset = [RESET_CMD, 0x00, 0x00]

# UCI Commands
UWB_CoreGetConfig            = [ 0x20, 0x02, 0x00, 0x00 ]

# doc:start:test-mode-configs
UWB_SessionInit_TestMode     = [ 0x21, 0x00, 0x00, 0x05,
    0x00, 0x00, 0x00, 0x00, # Session ID for Test session
    0xD0                    # Session Type 0xD0 for Test session
    ]

UWB_SetAppConfig_CHANNEL_ID  = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, # Session ID for Test session
    0x01,
    0x04, 0x01, CHANNEL_ID  # Channel 9
    ]

UWB_SetAppConfig_TX_POWER_ID = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, # Session ID for Test session
    0x01,
    0xF2, 0x01, TX_POWER_ID # TX_POWER 0x00 (max)
    ]

# SFD ID config for BPRF
UWB_SetAppConfig_SFD_ID_BPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x15, 0x01, 0x02 
]

# PREAMBLE ID config for BPRF
UWB_SetAppConfig_PREAMBLE_CODE_INDEX_BPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x14, 0x01, 10 # PREAMBLE ID = 10  
]
# doc:end:test-mode-configs

# SFD ID config for HPRF
UWB_SetAppConfig_SFD_ID_HPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x15, 0x01, 0x02 
]

# PREAMBLE ID config for HPRF
UWB_SetAppConfig_PREAMBLE_CODE_INDEX_HPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x14, 0x01, 25 # PREAMBLE ID = 25
]

# PREAMBLE DURATION config for HPRF
UWB_SetAppConfig_PREAMBLE_DURATION_HPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x17, 0x01, 1 # 1 = 64 Symbols
]

# PRF MODE config for HPRF
UWB_SetAppConfig_PRF_MODE_HPRF = [ 0x21, 0x03, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00,
    0x01,
    0x1F, 0x01, 1 # 1 = HPRF mode
]


# All commands that will be sent one by one
SR040_TestMode_Command_List = [
    SR040_HardReset, # Reset SR040 to apply Trim values
    UWB_CoreGetConfig,
    ]

if ENABLE_LOGS == True:
    UwbApi_SetCoreConfigs_LOGGING = getUciLoggingBuffer()
    SR040_TestMode_Command_List.append(UwbApi_SetCoreConfigs_LOGGING)

SR040_TestMode_Command_List.append(UWB_SessionInit_TestMode)
SR040_TestMode_Command_List.append(UWB_SetAppConfig_CHANNEL_ID)
SR040_TestMode_Command_List.append(UWB_SetAppConfig_SFD_ID_BPRF)
SR040_TestMode_Command_List.append(UWB_SetAppConfig_PREAMBLE_CODE_INDEX_BPRF)
SR040_TestMode_Command_List.append(UWB_SetAppConfig_TX_POWER_ID)

if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP3:
    UWB_StoreSTSKeys = [ 0x21, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, # Session ID
        0x03, # Number of parameters
    ]
    UWB_SetAppConfig_STS_INDEX = getUciParamIdStsIndexBuffer()
    UWB_SetAppConfig_URSK = getUciExtParamIdDUrskBuffer()
    UWB_SetAppConfig_STS_SALTED_HASH_ARRAY = getUciExtParamIdSaltedHashBuffer()
    UWB_StoreSTSKeys.extend(UWB_SetAppConfig_STS_INDEX)
    UWB_StoreSTSKeys.extend(UWB_SetAppConfig_URSK)
    UWB_StoreSTSKeys.extend(UWB_SetAppConfig_STS_SALTED_HASH_ARRAY)
    UWB_StoreSTSKeys[3] = len(UWB_StoreSTSKeys) - 4
    SR040_TestMode_Command_List.append(UWB_StoreSTSKeys)

if ENABLE_HPRF == True:
    SR040_TestMode_Command_List.append(UWB_SetAppConfig_SFD_ID_HPRF)
    SR040_TestMode_Command_List.append(UWB_SetAppConfig_PREAMBLE_CODE_INDEX_HPRF)
    SR040_TestMode_Command_List.append(UWB_SetAppConfig_PRF_MODE_HPRF)
    SR040_TestMode_Command_List.append(UWB_SetAppConfig_PREAMBLE_DURATION_HPRF)

if DO_TEST_MODE_TX_ONLY == 1:
    UwbApi_SetAppConfig_TX_PHY_LOGGING = [ 0x21, 0x03, 0x00, 0x08,
        0x00, 0x00, 0x00, 0x00,
        0x01,
        0xF5, 0x01, 0x01 # TX Logging enabled
    ]

    UWB_StartTestMode_TX = [ 0x2E, 0x20, 0x00, 0x59,
        0x06,
        0x00, 0x01, 0x01,                   # Test mode type 0x01 (TX Mode)
        0x02, 0x01, TEST_MODE_FRAME_TYPE,   # Slot type: SP3 type which has only STS
        0x01, 0x02, 0x00, 0x00,             # Delay: 0
        0x05, 0x04, 0x00, 0x00, 0x00, 0x00, # EVENT_COUNTER_MAX: 0x00 => Transmit continuously
        0x06, 0x04, 0xA0, 0x86, 0x01, 0x00, # TX_CYCLE_TIME: 100 * 1000us
        0x03, len(TX_PSDU),                 # Payload
    ]

    UWB_StartTestMode_TX.extend(TX_PSDU)
    SR040_TestMode_Command_List.append(UwbApi_SetAppConfig_TX_PHY_LOGGING)
    SR040_TestMode_Command_List.append(UWB_StartTestMode_TX)

elif DO_TEST_MODE_RX_ONLY == 1:
    UwbApi_SetAppConfig_RX_PHY_LOGGING = [ 0x21, 0x03, 0x00, 0x08,
        0x00, 0x00, 0x00, 0x00,
        0x01,
        0xF4, 0x01, 0x01 # RX Logging enabled
    ]

    UWB_StartTestMode_RX = [ 0x2E, 0x20, 0x00, 0x15,
        0x05,
        0x00, 0x01, 0x00,                   # Test mode type 0x00 (RX Mode)
        0x02, 0x01, # Slot Type
            TEST_MODE_FRAME_TYPE,   # Slot type Value: SP0
        0x01, 0x02, 0x00, 0x00,             # Delay: 0
        0x05, 0x04, # EVENT_COUNTER_MAX
            20, 0x00, 0x00, 0x00,   # EVENT_COUNTER_MAX:Value
        0x04, 0x02, # TIME_OUT
            0x60, 0xEA,              # TIME_OUT:Value
    ]

    SR040_TestMode_Command_List.append(UwbApi_SetAppConfig_RX_PHY_LOGGING)
    SR040_TestMode_Command_List.append(UWB_StartTestMode_RX)

elif DO_TEST_MODE_CONTINUOUS_WAVE == 1:
    UWB_StartTestMode_CW         = [ 0x2E, 0x20, 0x00, 0x04,
        0x01,
        0x00, 0x01, 0x02        # Test mode type 0x02 (CW Mode)
    ]
    SR040_TestMode_Command_List.append(UWB_StartTestMode_CW)

def main():
    pnp.run(SR040_TestMode_Command_List)

if __name__ == '__main__':
    main()


