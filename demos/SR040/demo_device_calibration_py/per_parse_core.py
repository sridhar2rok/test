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
from test_mode_config import *
import csv
import math

LOG_TAG_RX_STATUS           = 0x00
LOG_TAG_STS_INDEX           = 0x09
LOG_TAG_RX_FIRST_PATH_INFO  = 0x0B
LOG_TAG_RX_PSDU             = 0x07

LOG_TAG_RX_STATUS_LENGTH = 2
LOG_TAG_STS_INDEX_LENGTH = 4
LOG_TAG_RX_FIRST_PATH_INFO_LENGTH = 40

RX_ERROR_MASK_TOA_DETECT_FAILED = 0x1
RX_ERROR_MASK_SIGNAL_LOST = 0x2
RX_ERROR_MASK_PRMBL_TIMEOUT = 0x4
RX_ERROR_MASK_SFD_TIMEOUT = 0x8
RX_ERROR_MASK_SECDED_DECODE_FAILURE = 0x10
RX_ERROR_MASK_RS_DECODE_FAILURE = 0x20
RX_ERROR_MASK_DECODE_CHAIN_FAILURE = 0x40
RX_ERROR_MASK_DATA_BUFFER_OVERFLOW = 0x80
RX_ERROR_MASK_STS_MISMATCH = 0x100

def DEBUG(*args):
    # print(args)
    pass

class UCILogger(object):
    def __init__(self):
        self.ntfCount = 0
        self.rxntfCount = 0
        self.psduErrors = 0
        self.RXERROR = {
            "OK" : 0,
            "TIMEOUT" : 0,
            "STS_FAILED" : 0,
            "TOA_FAILED" : 0,
            "PHY_DECODING_FAILED" : 0,
        }
        self.RSSI = list()
    def _resetLogStats(self):
        self.ntfCount = 0
        self.rxntfCount = 0
        self.psduErrors = 0
        self.RXERROR["OK"] = 0,
        self.RXERROR["TIMEOUT"] = 0,
        self.RXERROR["STS_FAILED"] = 0,
        self.RXERROR["TOA_FAILED"] = 0,
        self.RXERROR["PHY_DECODING_FAILED"] = 0,
        self.RSSI = list()
    def printTestStats(self):
        record = {
            "NUM_FRAMES" : self.rxntfCount,
            "ATTENUATION" : 0,
            "RX_TIMEOUTS" : self.RXERROR["TIMEOUT"],
            "RX_DECODING_FAILS" : self.RXERROR["PHY_DECODING_FAILED"],
            "RX_TOA_FAILS" : self.RXERROR["TOA_FAILED"],
            "RX_STS_FAILS" : self.RXERROR["STS_FAILED"],
            "RX_PSDU_FAILS" : self.psduErrors,
            "RX_ERRORS_TOTAL" : self.rxntfCount - self.RXERROR["OK"] + self.psduErrors,
            "PER_PERCENT" : 0,
        }
        if self.rxntfCount == 0:
            record["PER_PERCENT"] = 0
            RSSI = 0
        else:
            record["PER_PERCENT"] = (record["RX_ERRORS_TOTAL"] / self.rxntfCount) * 100
            RSSI = 0
            if len(self.RSSI):
                for i in self.RSSI:
                    RSSI += i
                RSSI = RSSI / len(self.RSSI)
        print("\n")
        print("#"*40)
        print(" RX Stats Summary")
        print("#"*40)
        print("ntfCount = %d" %(self.ntfCount))
        print("RxNtfCount = %d" %(self.rxntfCount))
        print("Average RSSI = %f" %(RSSI))
        
        header_row = [ \
            "ATTENUATION", \
            "NUM_FRAMES", \
            "RX_TIMEOUTS", \
            "RX_DECODING_FAILS", \
            "RX_TOA_FAILS", \
            "RX_STS_FAILS", \
            "RX_PSDU_FAILS", \
            "RX_ERRORS_TOTAL", \
            "PER_PERCENT",
        ]

        csv_log_file_data = list()
        try:
            with open(os.path.dirname(os.path.abspath(__file__)) + os.sep + CSV_LOG_FILE, "r+", newline = '') as log_file:
                csv_writer = csv.reader(log_file)
                for row in csv_writer:
                    csv_log_file_data.append(row)
        except(FileNotFoundError):
            pass

        with open(os.path.dirname(os.path.abspath(__file__)) + os.sep + CSV_LOG_FILE, "w+", newline = '') as log_file:
            csv_writer = csv.writer(log_file)
            csv_writer.writerow(header_row)
            if len(csv_log_file_data) != 0:
                if csv_log_file_data[0][0] != "ATTENUATION":
                    csv_writer.writerow(csv_log_file_data[0])
                for log_len in range(1, len(csv_log_file_data)):
                    csv_writer.writerow(csv_log_file_data[log_len])
            csv_writer.writerow([
                    record["ATTENUATION"], \
                    record["NUM_FRAMES"], \
                    record["RX_TIMEOUTS"], \
                    record["RX_DECODING_FAILS"], \
                    record["RX_TOA_FAILS"], \
                    record["RX_STS_FAILS"], \
                    record["RX_PSDU_FAILS"], \
                    record["RX_ERRORS_TOTAL"], \
                    record["PER_PERCENT"], \
                ]
            )

        for k,v in self.RXERROR.items():
            print("%-40s = %3d"%(k,v))
        print("*"*40)
        for k,v in record.items():
            if k == "PER_PERCENT":
                print("%-40s = %2f"%(k,v))
            else:
                print("%-40s = %3d"%(k,v))
        print("#"*40)
        self._resetLogStats()
    def _handleTagRxStatus(self, log_payload, index, length):
        # 2-byte Rx status
        self.rxntfCount += 1
        value = log_payload[index:(index + length)]
        rx_ready          = (value[0] & 0x1)
        rx_data_available = (value[0] & 0x2) >> 1
        value.reverse()
        rx_ok = False
        rx_error          = ((value[0] << (8 * 1)) | \
                             (value[1] << (8 * 0))) & 0xFFFF
        rx_error = rx_error >> 2;
        DEBUG("rx_error = 0x%X" %(rx_error))
        if rx_error == 0:
            self.RXERROR["OK"] += 1
            rx_ok = True
        else:
            if rx_error & RX_ERROR_MASK_TOA_DETECT_FAILED:
                DEBUG("RX_TOA_DETECT_FAILED")
                self.RXERROR["TOA_FAILED"] += 1
            if (rx_error & RX_ERROR_MASK_SIGNAL_LOST) or \
                (rx_error & RX_ERROR_MASK_PRMBL_TIMEOUT) or \
                (rx_error & RX_ERROR_MASK_SFD_TIMEOUT):
                DEBUG("RX_SIGNAL_LOST")
                self.RXERROR["TIMEOUT"] += 1
            if (rx_error & RX_ERROR_MASK_SECDED_DECODE_FAILURE) or \
                (rx_error & RX_ERROR_MASK_RS_DECODE_FAILURE) or \
                (rx_error & RX_ERROR_MASK_DECODE_CHAIN_FAILURE) or \
                (rx_error & RX_ERROR_MASK_DATA_BUFFER_OVERFLOW):
                DEBUG("RX_SECDED_DECODE_FAILURE")
                self.RXERROR["PHY_DECODING_FAILED"] += 1
            if rx_error & RX_ERROR_MASK_STS_MISMATCH:
                DEBUG("RX_STS_MISMATCH")
                self.RXERROR["STS_FAILED"] += 1

        index += length
        return (index, rx_ok)
    def _handleTagRxFirstPathInfo(self, log_payload, index, length):
        value = log_payload[index:(index + length)]
        FIRST_PATH_DETECTED     = value[:4]
        RFU1                    = value[4:8]
        EDGE_INDEX              = value[8:12]
        TIME                    = value[12:16]
        
        MAX_TAP_INDEX           = value[16:18]
        MAX_TAP_POWER           = value[18:20]
        FIRST_PATH_INDEX        = value[20:22]
        FIRST_PATH_POWER        = value[22:24]
        
        NOISE_POWER             = value[24:28]
        DETECT_THRESHOLD_POWER  = value[28:32]
        RFU2                    = value[32:36]
        OVERALL_RX_POWER        = value[36:]

        FIRST_PATH_POWER.reverse()
        FIRST_PATH_POWER = FIRST_PATH_POWER[0] << (8 * 1) | \
                            FIRST_PATH_POWER[1] << (8 * 0)

        FIRST_PATH_POWER = 0xFFFF << (8 * 2) | FIRST_PATH_POWER # FFFFCDDC
        FIRST_PATH_POWER = FIRST_PATH_POWER - 0xFFFFFFFF

        RSSI = (math.log10(2) * 10 * FIRST_PATH_POWER) / (2 ** 9)
        self.RSSI.append(RSSI)

        index += length
        return index

    def handleLogNtf(self, log_payload):
        log_payload = list(log_payload)
        self.ntfCount = self.ntfCount + 1;
        DEBUG("Notification number: %d" %(self.ntfCount))
        index = 0
        num_params = log_payload[index]
        index += 1
        rx_ok = False
        rx_psdu = []
        while index < len(log_payload):
            tag = log_payload[index]
            index += 1
            length = log_payload[index]
            index += 1
            # TODO: Add more tags as required
            if tag == LOG_TAG_RX_STATUS:
                if length != LOG_TAG_RX_STATUS_LENGTH:
                    DEBUG("LOG_TAG_RX_STATUS length mismatch")
                    index += length
                    continue
                (index, rx_ok) = self._handleTagRxStatus(log_payload, index, length)
            elif tag == LOG_TAG_STS_INDEX:
                if length != LOG_TAG_STS_INDEX_LENGTH:
                    DEBUG("LOG_TAG_STS_INDEX length mismatch")
                    index += length
                    continue
                # 4-byte STS_INDEX
                value = log_payload[index:(index + length)]
                value.reverse()
                sts_index = ((value[0] & 0xFF) << (8 * 3) | \
                             (value[1] & 0xFF) << (8 * 2) | \
                             (value[2] & 0xFF) << (8 * 1) | \
                             (value[3] & 0xFF) << (8 * 0))
                DEBUG("STS Index : 0x%X" %(sts_index))
                index += length
            elif tag == LOG_TAG_RX_FIRST_PATH_INFO:
                if length != LOG_TAG_RX_FIRST_PATH_INFO_LENGTH:
                    DEBUG("LOG_TAG_RX_FIRST_PATH_INFO length = 0")
                    index += length
                    continue
                index = self._handleTagRxFirstPathInfo(log_payload, index, length)
            elif tag == LOG_TAG_RX_PSDU:
                if length == 0:
                    DEBUG("LOG_TAG_RX_PSDU length = 0")
                    index += length
                    continue
                rx_psdu = log_payload[index:(index + length)]
                DEBUG(rx_psdu)
                index += length
            else:
                print("Unkown LOG TAG 0x%X" %(tag))
                index += length

        if TEST_MODE_FRAME_TYPE == FRAME_TYPE_SP0 and rx_ok and TX_PSDU != rx_psdu:
            self.psduErrors += 1
