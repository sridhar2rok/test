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

CHANNEL_ID = 0x09

SR040_HardReset = [RESET_CMD, 0x00, 0x00]

# doc:start:trim-values
UWB_Set_Trim_TX_POWER      = [0x2E, 0x26, 0x00, 0x06, 
    0x01, 
    0x00, 0x03, 
    CHANNEL_ID, # Channel 9
    0x00, 0x00, # +0 dB
    ]

UWB_Set_Trim_FREQ_DIFF     = [0x2E, 0x26, 0x00, 0x09, 
    0x01, 
    0x01, 0x06, 
    CHANNEL_ID,                     # Channel 9
    0x00, 0x00, 0x00, 0x00, 0x00    # +0 kHz
    ]

UWB_Set_Trim_ANTENNA_DELAY = [0x2E, 0x26, 0x00, 0x06, 
    0x01, 
    0x02, 0x03, 
    CHANNEL_ID, # Channel 9
    0x00, 0x00  # 0 ps
    ]

UWB_Set_Trim_TX_ADAPTIVE_POWER_CALC = [0x2E, 0x26, 0x00, 0x06, 
    0x01, 
    0x06, 0x03, 
    CHANNEL_ID, # Channel 9
    0x00,       # powerIdRms
    0x00,       # peakDelta
    ]
# doc:end:trim-values

SR040_Calib_Command_List = [
    UWB_Set_Trim_TX_POWER,
    UWB_Set_Trim_FREQ_DIFF,
    UWB_Set_Trim_ANTENNA_DELAY,
    UWB_Set_Trim_TX_ADAPTIVE_POWER_CALC,
    SR040_HardReset, # Reset SR040 to apply Trim values
    ]

def main():
    pnp.run(SR040_Calib_Command_List)


if __name__ == '__main__':
    main()


