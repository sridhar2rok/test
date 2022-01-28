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

FRAME_TYPE_SP0 = 0x00
FRAME_TYPE_SP3 = 0x03

# Test mode configs
TEST_MODE_FRAME_TYPE = FRAME_TYPE_SP3

TX_PSDU = list()
for i in range(1, 65):
	TX_PSDU.append(i)

CSV_LOG_FILE = "test_mode_log.csv"
