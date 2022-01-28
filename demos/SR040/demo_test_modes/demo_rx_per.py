"""
/* Copyright 2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */
"""

MAC_PHY_FAILURES = [

    ("Reason of failure is unspecified",
        "RSN_UNSPECIFIED", 0),
    ("Frame length is invalid",
        "RSN_FRAME_INVALID_LEN", 1),
    ("Invalid CRC",
        "RSN_INVALID_CRC", 2),
    ("Destination address is invalid",
        "RSN_DSTADDR_INVALID", 3),
    ("Source address is invalid",
        "RSN_SRCADDR_INVALID", 4),

    ("Frame control: invalid length",
        "RSN_FCTRL_INVALID_LEN", 10),
    ("Frame control: invalid frame type",
        "RSN_FCTRL_INVALID_FRAME_TYPE", 11),
    ("Frame control: invalid version number",
        "RSN_FCTRL_INVALID_VERSION", 12),
    ("Frame control: not supported value of AR (acknowledgment request)",
        "RSN_FCTRL_NOTSUPPORTED_AR", 13),
    ("Frame control: invalid addressing mode",
        "RSN_FCTRL_INVALID_ADDR_MODE", 14),
    ("Frame control: IE not present",
        "RSN_FCTRL_NOTSUPPORTED_IEPRESENT", 15),

    ("Auxiliary header is invalid",
        "RSN_AUXHDR_INVALID", 20),
    ("Auxiliary header: invalid length",
        "RSN_AUXHDR_INVALID_LEN", 21),
    ("Auxiliary header: not supported key identifier mode",
        "RSN_AUXHDR_INVALID_KEY_ID_MODE", 22),
    ("Auxiliary header: invalid key length",
        "RSN_AUXHDR_INVALID_KEY_LEN", 23),
    ("Auxiliary header: invalid security level ",
        "RSN_AUXHDR_INVALID_SEC_LEVEL", 24),
    ("Auxiliary header: invalid Key index",
        "RSN_AUXHDR_INVALID_KEY_INDEX", 25),

    ("IE field is invalid",
        "RSN_IE_INVALID", 30),
    ("IE field has invalid length",
        "RSN_IE_INVALID_LEN", 31),
    ("IE Header: invalid length",
        "RSN_IEHDR_INVALID_LEN", 32),
    ("IE Header: not supported type",
        "RSN_IEHDR_NOTSUPPORTED_TYPE", 33),
    ("IE Header Termination: expected HT1",
        "RSN_IEHDR_HT1_EXPECTED", 34),
    ("IE Header Termination: expected HT2",
        "RSN_IEHDR_HT2_EXPECTED", 35),
    ("IE Header: header missing",
        "RSN_IEHDR_MISSING", 36),

    ("CCC IE: invalid length of IE field",
        "RSN_IE_CCC_INVALID_LEN", 40),
    ("CCC IE Header: invalid length",
        "RSN_IEHDR_CCC_INVALID_LEN", 41),
    ("CCC IE Header: invalid message ID",
        "RSN_IEHDR_CCC_INVALID_MSG_ID", 42),
    ("CCC IE Header: invalid message len",
        "RSN_IEHDR_CCC_INVALID_MSG_LEN", 43),

    ("FIRA IE: invalid length of IE field",
        "RSN_IE_FIRA_INVALID_LEN", 50),
    ("FIRA IE Header: invalid length",
        "RSN_IEHDR_FIRA_INVALID_LEN", 51),
    ("FIRA IE Header: invalid padding",
        "RSN_IEHDR_FIRA_INVALID_PADDING", 52),

    ("MAC Payload: invalid length",
        "RSN_MACPLD_INVALD_LEN", 60),
    ("MAC Payload: decoding failed",
        "RSN_MACPLD_DECODE_FAILED", 61),

    ("CCC MAC Payload: invalid length",
        "RSN_MACPLD_CCC_INVALID_LEN", 70),
    ("CCC MAC Payload: decription failed",
        "RSN_MACPLD_CCC_DECRYPT_FAILED", 71),
    ("CCC MAC Payload: invalid message ID",
        "RSN_MACPLD_CCC_INVALID_MSG_ID", 72),
    ("CCC MAC Payload: invalid length of a RCM",
        "RSN_MACPLD_CCC_RCM_INVALID_LEN", 73),
    ("CCC MAC Payload: invalid length of a measurement report",
        "RSN_MACPLD_CCC_MEASREPORT_INVALID_LEN", 74),
    ("CCC MAC Payload: number of IE nodes is invalid",
        "RSN_MACPLD_CCC_INVALID_IENODES_NUM", 75),

    ("FIRA MAC Payload: invalid length",
        "RSN_MACPLD_FIRA_INVALID_LEN", 80),
    ("FIRA MAC Payload: decription failed",
        "RSN_MACPLD_FIRA_DECRYPT_FAILED", 81),
    ("FIRA MAC Payload: payload field is missing",
        "RSN_MACPLD_FIRA_MISSING", 82),
    ("FIRA MAC Payload: invalid IE type",
        "RSN_MACPLD_FIRA_INVALID_IETYPE", 83),
    ("FIRA MAC Payload: invalid vendor specific IE type",
        "RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE", 84),
    ("FIRA MAC Payload: invalid OUI",
        "RSN_MACPLD_FIRA_INVALID_OUI", 85),

    ("General error on invalid frame",
        "RSN_INVALID_FRAME", 255),
]


RX_ERROR_BITS = [
    ("Success:indicate RX ready.",
        "rxSuccess_Ready"),

    ("Success:indicate RX payload decoded.",
        "rxSuccess_DataAvailable"),

    ("Failed: RX TOA_DETECT FAILED indicates failure in detection of valid first path/signal",
        "rxError_ToaDetectFailed"),
    ("Failed: RX SIGNAL LOST indicates RX EOF event was caused because the signal disappeared",
        "rxError_SignalLost"),
    ("Failed: RX PRMBL TIMEOUT indicates RX EOF event was caused because no signal was found within set timeout",
        "rxError_PreambleTimeout"),
    ("Failed: RX SFD TIMEOUT indicates RX EOF event was caused because no SFD was found within set timeout ",
        "rxError_SFDTimeout"),
    ("Failed: RX SECDED DECODE FAILURE indicates the IEEE Parity check performed on the PHR detected uncorrectable errors",
        "rxError_SecdedDecodeFailed"),
    ("Failed: RX_RS_DECODE_FAILURE indicates the RS algorithm executed on the PSDU detected uncorrectable errors ",
        "rxError_DecodeFailure"),
    ("Failed: RX DECODE CHAIN FAILURE indicates the PSDU was undecodable by RS algorithm",
        "rxError_DecodeChainFailure"),
    ("Failed: RX DATA BUFFER OVERFLOW indicates RX data buffer overflow from the inner receiver",
        "rxError_DataBufferOverflow"),
    ("Failed: RX STS MISMATCH indicates STS mismatch",
        "rxError_StsMismatch"),
]

print("\n\n\n")
for (desc, hdr, val) in MAC_PHY_FAILURES:
    if True:
        print("/** %s */"%(desc,))
        print("uint16_t MacDecode_Fail_%s;"%(hdr,))


print("\n\n\n")
for (desc, hdr, val) in MAC_PHY_FAILURES:
    if True:
        print ("CASE_MAC_DECODE_FAIL_COUNT(PHSCA_MAC_PHY_%s, gRxPer.MacDecode_Fail_%s);"%(hdr, hdr))

print("\n\n\n")
for desc, variable in RX_ERROR_BITS:
    if True:
        print("/** %s */"%(desc,))
        print("uint16_t %s;"%(variable,))

print("\n\n\n")
for (count, (desc, variable)) in enumerate(RX_ERROR_BITS):
    if True:
        print ("/* %s */"%(desc,))
        print ("UPDATE_RX_ERROR_COUNT(rxStatus, %d, gRxPer.%s);"%(count, variable))

print("\n\n\n")
for desc, variable in RX_ERROR_BITS:
    if True:
        print ('PRINT_RX_PER_VALUE(gRxPer.%s, "%s", "%s");'%(variable, variable, desc))

print("\n\n\n")
for (desc, hdr, val) in MAC_PHY_FAILURES:
    if True:
        print ('PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_%s, "MacDecode_Fail:%s", "%s");'%(hdr, hdr, desc))

