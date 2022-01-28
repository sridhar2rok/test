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

import os
import sys
from test_mode_config import *
from per_parse_core import *

# PHYLOG_FILE_NAME = "uwb_phy_log.log"

def usage():
	print("%s <path-to-log-file>" %(os.path.basename(__file__)))


if __name__ == '__main__':
	per_uci_logger = UCILogger()
	if (len(sys.argv) == 1):
		usage()
		sys.exit(1)

	phylog_file_name = sys.argv[1]
	with open(phylog_file_name, "r") as log_file:
		for line in log_file:
			stripped_line = line.strip()
			# First 2 bytes are length
			payload_length = stripped_line[:4]
			payload_length = list(bytes.fromhex(payload_length))
			payload_length.reverse()
			payload_length = payload_length[0] << (8 * 1) | payload_length[1]
			stripped_line = stripped_line[4:]
			if (payload_length != len(stripped_line) / 2):
				print("Payload and length mismatch : %d and %d" %(payload_length, len(stripped_line)))
			per_uci_logger.handleLogNtf(bytes.fromhex(stripped_line))
			# print(bytes.fromhex(stripped_line))
	per_uci_logger.printTestStats()
	pass

