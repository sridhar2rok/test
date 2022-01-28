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

import pnp_core as pnp

RESET_CMD = 0x04

SR040_HardReset = [RESET_CMD, 0x00, 0x00]

UWB_CoreGetConfig = [0x20, 0x02, 0x00, 0x00]

UWB_SessionInit_Ranging =  [0x21, 0x00, 0x00, 0x05, 0x44, 0x33, 0x22, 0x11, 0x00]

UWB_SetAppConfig_RANGING_METHOD =   [0x21, 0x03, 0x00, 0x08, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0x01, 0x01, 0x02 # DS-TWR
    ]

UWB_SetAppConfig_RFRAME_CONFIG =    [0x21, 0x03, 0x00, 0x08, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0x12, 0x01, 0x03 # STS follows SFD, PPDU has no PHR or PSDU
    ]

UWB_SetAppConfig_SLOTS_PER_RR =     [0x21, 0x03, 0x00, 0x08, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0x1B, 0x01, 0x18 # 24 slots
    ]

UWB_SetAppConfig_RANGING_INTERVAL = [0x21, 0x03, 0x00, 0x09, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0x09, 0x02, 0xC0, 0x00 # 192 ms
    ]

UWB_SetAppConfig_SFD_ID =           [0x21, 0x03, 0x00, 0x08, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0x15, 0x01, 0x00 # BPRF
    ]

UWB_SetAppConfig_TX_POWER_ID =      [0x21, 0x03, 0x00, 0x08, 
    0x44, 0x33, 0x22, 0x11, 
    0x01, 
    0xF2, 0x01, 0x00 # +14dB
    ]

UWB_SetAppConfig =                  [0x21, 0x03, 0x00, 0x19, 
    0x44, 0x33, 0x22, 0x11, 
    0x06, 
    0x11, 0x01, 0x01,       # DEVICE_ROLE        => Initiator
    0x03, 0x01, 0x00,       # MULTI_NODE_MODE    => Unicast
    0x05, 0x01, 0x01,       # NUMBER_OF_ANCHORS  => 1
    0x06, 0x02, 0x11, 0x11, # DEVICE_MAC_ADDRESS => 0x1111
    0x07, 0x02, 0x22, 0x22, # DST_MAC_ADDRESS    => 0x2222
    0x00, 0x01, 0x01        # DEVICE_TYPE        => Controller
    ]

UWB_StartRanging = [0x22, 0x00, 0x00, 0x04, 0x44, 0x33, 0x22, 0x11]

SR040_Ranging_Command_List = [
    SR040_HardReset, 
    UWB_CoreGetConfig, 
    UWB_SessionInit_Ranging, 
    UWB_SetAppConfig_RANGING_METHOD, 
    UWB_SetAppConfig_RFRAME_CONFIG, 
    UWB_SetAppConfig_SLOTS_PER_RR, 
    UWB_SetAppConfig_RANGING_INTERVAL, 
    UWB_SetAppConfig_SFD_ID, 
    UWB_SetAppConfig_TX_POWER_ID, 
    UWB_SetAppConfig, 
    UWB_StartRanging,
    ]

def main():
    pnp.run(SR040_Ranging_Command_List)


if __name__ == '__main__':
    main()
