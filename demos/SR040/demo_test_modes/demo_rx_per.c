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

#include "demo_rx_per.h"
#include "UwbApi_Types_Proprietary.h"
#include "phNxpLogApis_App.h"
#include "utlv.h"
#include "nxEnsure.h"
#include <inttypes.h>

#define PRINT_RX_PER_VALUE(var, var_name, desc)                       \
    do {                                                              \
        if (var != 0 && gRxPer.totalRxNtfs != 0) {                    \
            const float fMax = gRxPer.totalRxNtfs;                    \
            const float fVar = var;                                   \
            const float fPer = (fVar * 100.0) / fMax;                 \
            PRINTF("[%s]\n", desc);                                   \
            PRINTF("\t%10d : [%0.2f%%] : %s\n", var, fPer, var_name); \
        }                                                             \
    } while (0)
//
#define PRINT_RX_PER_VALUE_F(fVar, var_name, desc)     \
    do {                                               \
        if (fVar != 0 && gRxPer.totalRxNtfs != 0) {    \
            const float fMax = gRxPer.totalRxNtfs;     \
            const float fPer = (fVar) / fMax;          \
            PRINTF("[%s]\n", desc);                    \
            PRINTF("\t%10.2f : %s\n", fPer, var_name); \
        }                                              \
    } while (0)

#define PRINT_FIRSTPATH_PER_VALUE(total, var_name, desc)                      \
    do {                                                                      \
        if (total != 0 && gRxPer.fpInfo_count != 0) {                         \
            PRINTF("\t%10.2f : %s\n", total / gRxPer.fpInfo_count, var_name); \
        }                                                                     \
    } while (0)

/**
 * @brief Tags of parameters to be logged
 */
typedef enum
{
    /** @brief Tag of the PHY RX Status log */
    RX_PER_TAG_PHY_RX_STATUS_TAG = ((uint8_t)0x00u),
    /** @brief Tag of the PHY TX Status log */
    RX_PER_TAG_PHY_TX_STATUS_TAG = ((uint8_t)0x01u),
    /** @brief Tag of the RX CIR dump in the PHY log */
    RX_PER_TAG_CIR_LOG_TAG = ((uint8_t)0x02u),
    /** @brief Tag of the UWB SESSION ID log */
    RX_PER_TAG_SESSIONID_TAG = ((uint8_t)0x03u),
    /** @brief Tag of the Block Index log */
    RX_PER_TAG_BLOCK_INDEX_TAG = ((uint8_t)0x04u),
    /** @brief Tag of the PHY RX Timestamp log */
    RX_PER_TAG_PHY_RX_TS_TAG = ((uint8_t)0x05u),
    /** @brief Tag of the PHY RX Timestamp log */
    RX_PER_TAG_PHY_TX_TS_TAG = ((uint8_t)0x06u),
    /** @brief Tag of the PHY RX PSDU log */
    RX_PER_TAG_PHY_RX_PSDU_LOG_TAG = ((uint8_t)0x07u),
    /** @brief Tag of the PHY TX PSDU log */
    RX_PER_TAG_PHY_TX_PSDU_LOG_TAG = ((uint8_t)0x08u),
    /** @brief Tag of the STS Index log */
    RX_PER_TAG_STS_INDEX_TAG = ((uint8_t)0x09u),

    /** @brief Tag of the RX First Path Information */
    RX_PER_TAG_RXFIRSTPATHINFO_TAG = ((uint8_t)0x0Bu),
    /** @brief Tag of the RX Carrier Frequency offset
     *
     *  The device in Receive operation computes the frequency offset(CFO_INT) between transmitter and receiver carrier freq.
     *
     *  @param[in] freqOffset referred as CFO_INT will be provided after the Rx operation.
     *  To compute the ppm from freqOffset, the following are the steps
     *  1)Compute CFO in rad/pi per clock cycle: CFO = CFO_INT/2^17
     *  2)Compute CFO(Hz) = (124.8MHz/2)*CFO
     *  3)Compute CFO(ppm) = CFO(Hz) / (fc/1e6), where fc is the carrier frequency and for channel 9 it will be 7.9872GHz
     *
     */
    RX_PER_TAG_RXCARRIERFREQOFFSET_TAG = ((uint8_t)0x0Cu),
    /** @brief Tag of the MAC decoding status */
    RX_PER_TAG_MACSTATUS_TAG = ((uint8_t)0x0Du),
    /** @brief Tag of the MAC decoding status */
    RX_PER_TAG_MAC_DECODEFAILRSN_TAG = ((uint8_t)0x0Eu),
    /** @brief Tag of the MAC compliance */
    RX_PER_TAG_MAC_COMPLIANCE_TAG = ((uint8_t)0x0Fu),
    /** @brief Tag of the RX MAC header */
    RX_PER_TAG_RX_MAC_HEADER_TAG = ((uint8_t)0x10u),
    /** @brief Tag of the RX MAC payload */
    RX_PER_TAG_RX_MAC_PAYLOAD_TAG = ((uint8_t)0x11u),

} phscaUciMsgLogging_Tag_t;

/**
 * @brief MAC decoding status codes
 */
typedef enum
{
    /** @brief Decoding MAC header succeeded */
    PHSCA_MAC_PHY_DECODING_HEADER_SUCCESS = 0x00u,
    /** @brief Decoding MAC payload succeeded */
    PHSCA_MAC_PHY_DECODING_PAYLOAD_SUCCESS = 0x01u,
    /** @brief MAC decoding failed, reason of failure is saved in decodingFailRsn */
    PHSCA_MAC_PHY_DECODING_FAILURE = 0xFFu,
} phscaMacPhy_Status_t;

/**
 * @brief Reasons of MAC decoding failure
 */
typedef enum
{
    /** @brief Reason of failure is unspecified */
    RX_PER_MAC_RSN_UNSPECIFIED = 0u,
    /** @brief Frame length is invalid */
    RX_PER_MAC_RSN_FRAME_INVALID_LEN = 1u,
    /** @brief Invalid CRC */
    RX_PER_MAC_RSN_INVALID_CRC = 2u,
    /** @brief Destination address is invalid */
    RX_PER_MAC_RSN_DSTADDR_INVALID = 3u,
    /** @brief Source address is invalid */
    RX_PER_MAC_RSN_SRCADDR_INVALID = 4u,

    /** @brief Frame control: invalid length */
    RX_PER_MAC_RSN_FCTRL_INVALID_LEN = 10u,
    /** @brief Frame control: invalid frame type */
    RX_PER_MAC_RSN_FCTRL_INVALID_FRAME_TYPE = 11u,
    /** @brief Frame control: invalid version number */
    RX_PER_MAC_RSN_FCTRL_INVALID_VERSION = 12u,
    /** @brief Frame control: not supported value of AR (acknowledgment request) */
    RX_PER_MAC_RSN_FCTRL_NOTSUPPORTED_AR = 13u,
    /** @brief Frame control: invalid addressing mode */
    RX_PER_MAC_RSN_FCTRL_INVALID_ADDR_MODE = 14u,
    /** @brief Frame control: IE not present */
    RX_PER_MAC_RSN_FCTRL_NOTSUPPORTED_IEPRESENT = 15u,

    /** @brief Auxiliary header is invalid */
    RX_PER_MAC_RSN_AUXHDR_INVALID = 20u,
    /** @brief Auxiliary header: invalid length */
    RX_PER_MAC_RSN_AUXHDR_INVALID_LEN = 21u,
    /** @brief Auxiliary header: not supported key identifier mode */
    RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_ID_MODE = 22u,
    /** @brief Auxiliary header: invalid key length */
    RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_LEN = 23u,
    /** @brief Auxiliary header: invalid security level  */
    RX_PER_MAC_RSN_AUXHDR_INVALID_SEC_LEVEL = 24u,
    /** @brief Auxiliary header: invalid Key index */
    RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_INDEX = 25u,

    /** @brief IE field is invalid */
    RX_PER_MAC_RSN_IE_INVALID = 30u,
    /** @brief IE field has invalid length */
    RX_PER_MAC_RSN_IE_INVALID_LEN = 31u,
    /** @brief IE Header: invalid length */
    RX_PER_MAC_RSN_IEHDR_INVALID_LEN = 32u,
    /** @brief IE Header: not supported type */
    RX_PER_MAC_RSN_IEHDR_NOTSUPPORTED_TYPE = 33u,
    /** @brief IE Header Termination: expected HT1 */
    RX_PER_MAC_RSN_IEHDR_HT1_EXPECTED = 34u,
    /** @brief IE Header Termination: expected HT2 */
    RX_PER_MAC_RSN_IEHDR_HT2_EXPECTED = 35u,
    /** @brief IE Header: header missing */
    RX_PER_MAC_RSN_IEHDR_MISSING = 36u,

    /** @brief FIRA IE: invalid length of IE field */
    RX_PER_MAC_RSN_IE_FIRA_INVALID_LEN = 50u,
    /** @brief FIRA IE Header: invalid length */
    RX_PER_MAC_RSN_IEHDR_FIRA_INVALID_LEN = 51u,
    /** @brief FIRA IE Header: invalid padding */
    RX_PER_MAC_RSN_IEHDR_FIRA_INVALID_PADDING = 52u,

    /** @brief MAC Payload: invalid length */
    RX_PER_MAC_RSN_MACPLD_INVALD_LEN = 60u,
    /** @brief MAC Payload: decoding failed */
    RX_PER_MAC_RSN_MACPLD_DECODE_FAILED = 61u,

    /** @brief FIRA MAC Payload: invalid length */
    RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_LEN = 80u,
    /** @brief FIRA MAC Payload: decription failed */
    RX_PER_MAC_RSN_MACPLD_FIRA_DECRYPT_FAILED = 81u,
    /** @brief FIRA MAC Payload: payload field is missing */
    RX_PER_MAC_RSN_MACPLD_FIRA_MISSING = 82u,
    /** @brief FIRA MAC Payload: invalid IE type */
    RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_IETYPE = 83u,
    /** @brief FIRA MAC Payload: invalid vendor specific IE type */
    RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE = 84u,
    /** @brief FIRA MAC Payload: invalid OUI */
    RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_OUI = 85u,

    /** @brief General error on invalid frame */
    RX_PER_MAC_RSN_INVALID_FRAME = 255u,
} phscaMacPhy_FailureReason_t;

/** @brief
 *  Additional information about the first path needed to calculate the time of arrival.
 *  @note This data structure is yet to be fully specified.
 */
typedef struct
{
    /** @brief Valid first path / signal detected */
    uint32_t firstPathdetected;
    /** @brief Reserved for future use */
    uint32_t RFU1[1u];
    /** @brief Index of first crossing of the detection threshold */
    uint32_t edgeIdx;
    /** @brief ToA estimate is time offset between strongest path and first path (negative values) */
    int32_t time;
    /*! @brief Index of strongest path */
    uint16_t maxTapIndex;
    /** @brief Power of the strongest path, the strongest-path power is measured at the sample
        with the strongest magnitude.Level[dBm] = (log10(2) * 10 * maxTapPower) >> 9 */
    int16_t maxTapPower;
    /*! @brief Index of first path */
    uint16_t firstPathIndex;
    /** @brief Power of the first path. the first-path power is measured at the first-path index.
        Level[dBm] = (log10(2) * 10 * firstPathPower) >> 9 */
    int16_t firstPathPower;
    /** @brief noise estimate power. Level[dBm] = (log10(2) * 10 * noisePower) >> 25 */
    int32_t noisePower;
    /** @brief Detection threshold power level. Level[dBm] = (log10(2) * 10 * detectThrPower) >> 25 */
    int32_t detectThrPower;
    /** @brief Reserved Fields */
    uint32_t RFU2[1u];
    /** @brief Overall received power. Level[dBm] = (log10(2)*10*overallRxPower) >> 25 */
    int32_t overallRxPower;
} phIscaBaseband_FirstPathInfo_t;

typedef struct
{
    /** Total RX Notifications */
    uint16_t totalRxNtfs;

    /** Success:indicate RX ready. */
    uint16_t rxSuccess_Ready;
    /** Success:indicate RX payload decoded. */
    uint16_t rxSuccess_DataAvailable;
    /** Failed: RX TOA_DETECT FAILED indicates failure in detection of valid first path/signal */
    uint16_t rxError_ToaDetectFailed;
    /** Failed: RX SIGNAL LOST indicates RX EOF event was caused because the signal disappeared */
    uint16_t rxError_SignalLost;
    /** Failed: RX PRMBL TIMEOUT indicates RX EOF event was caused because no signal was found within set timeout */
    uint16_t rxError_PreambleTimeout;
    /** Failed: RX SFD TIMEOUT indicates RX EOF event was caused because no SFD was found within set timeout  */
    uint16_t rxError_SFDTimeout;
    /** Failed: RX SECDED DECODE FAILURE indicates the IEEE Parity check performed on the PHR detected uncorrectable errors */
    uint16_t rxError_SecdedDecodeFailed;
    /** Failed: RX_RS_DECODE_FAILURE indicates the RS algorithm executed on the PSDU detected uncorrectable errors  */
    uint16_t rxError_DecodeFailure;
    /** Failed: RX DECODE CHAIN FAILURE indicates the PSDU was undecodable by RS algorithm */
    uint16_t rxError_DecodeChainFailure;
    /** Failed: RX DATA BUFFER OVERFLOW indicates RX data buffer overflow from the inner receiver */
    uint16_t rxError_DataBufferOverflow;
    /** Failed: RX STS MISMATCH indicates STS mismatch */
    uint16_t rxError_StsMismatch;

    uint16_t MacDecode_HeaderSuccess;
    uint16_t MacDecode_PayloadSuccess;

    /** Reason of failure is unspecified */
    uint16_t MacDecode_Fail_RSN_UNSPECIFIED;
    /** Frame length is invalid */
    uint16_t MacDecode_Fail_RSN_FRAME_INVALID_LEN;
    /** Invalid CRC */
    uint16_t MacDecode_Fail_RSN_INVALID_CRC;
    /** Destination address is invalid */
    uint16_t MacDecode_Fail_RSN_DSTADDR_INVALID;
    /** Source address is invalid */
    uint16_t MacDecode_Fail_RSN_SRCADDR_INVALID;
    /** Frame control: invalid length */
    uint16_t MacDecode_Fail_RSN_FCTRL_INVALID_LEN;
    /** Frame control: invalid frame type */
    uint16_t MacDecode_Fail_RSN_FCTRL_INVALID_FRAME_TYPE;
    /** Frame control: invalid version number */
    uint16_t MacDecode_Fail_RSN_FCTRL_INVALID_VERSION;
    /** Frame control: not supported value of AR (acknowledgment request) */
    uint16_t MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_AR;
    /** Frame control: invalid addressing mode */
    uint16_t MacDecode_Fail_RSN_FCTRL_INVALID_ADDR_MODE;
    /** Frame control: IE not present */
    uint16_t MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_IEPRESENT;
    /** Auxiliary header is invalid */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID;
    /** Auxiliary header: invalid length */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID_LEN;
    /** Auxiliary header: not supported key identifier mode */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_ID_MODE;
    /** Auxiliary header: invalid key length */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_LEN;
    /** Auxiliary header: invalid security level  */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID_SEC_LEVEL;
    /** Auxiliary header: invalid Key index */
    uint16_t MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_INDEX;
    /** IE field is invalid */
    uint16_t MacDecode_Fail_RSN_IE_INVALID;
    /** IE field has invalid length */
    uint16_t MacDecode_Fail_RSN_IE_INVALID_LEN;
    /** IE Header: invalid length */
    uint16_t MacDecode_Fail_RSN_IEHDR_INVALID_LEN;
    /** IE Header: not supported type */
    uint16_t MacDecode_Fail_RSN_IEHDR_NOTSUPPORTED_TYPE;
    /** IE Header Termination: expected HT1 */
    uint16_t MacDecode_Fail_RSN_IEHDR_HT1_EXPECTED;
    /** IE Header Termination: expected HT2 */
    uint16_t MacDecode_Fail_RSN_IEHDR_HT2_EXPECTED;
    /** IE Header: header missing */
    uint16_t MacDecode_Fail_RSN_IEHDR_MISSING;

    /** FIRA IE: invalid length of IE field */
    uint16_t MacDecode_Fail_RSN_IE_FIRA_INVALID_LEN;
    /** FIRA IE Header: invalid length */
    uint16_t MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_LEN;
    /** FIRA IE Header: invalid padding */
    uint16_t MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_PADDING;
    /** MAC Payload: invalid length */
    uint16_t MacDecode_Fail_RSN_MACPLD_INVALD_LEN;
    /** MAC Payload: decoding failed */
    uint16_t MacDecode_Fail_RSN_MACPLD_DECODE_FAILED;

    uint16_t MacDecode_Fail_RSN_MACPLD_CCC_INVALID_IENODES_NUM;
    /** FIRA MAC Payload: invalid length */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_LEN;
    /** FIRA MAC Payload: decription failed */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_DECRYPT_FAILED;
    /** FIRA MAC Payload: payload field is missing */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_MISSING;
    /** FIRA MAC Payload: invalid IE type */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_IETYPE;
    /** FIRA MAC Payload: invalid vendor specific IE type */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE;
    /** FIRA MAC Payload: invalid OUI */
    uint16_t MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_OUI;
    /** General error on invalid frame */
    uint16_t MacDecode_Fail_RSN_INVALID_FRAME;

    /* How many signals we got */
    uint16_t fpInfo_count;

    /** @brief Power of the strongest path, the strongest-path power is measured at the sample
        with the strongest magnitude. */
    float fpInfo_maxTapPower;

    /** @brief Power of the first path. the first-path power is measured at the first-path index. */
    float fpInfo_firstPathPower;

    /** @brief noise estimate power.  */
    float fpInfo_noisePower;

    /** @brief Detection threshold power level.  */
    float fpInfo_detectThrPower;

    /** @brief Overall received power.  */
    float fpInfo_overallRxPower;

    const uint8_t *rxCompare_expectedRxFrame;
    size_t rxCompare_expectedRxFrameLen;
    uint16_t rxCompare_success;
    uint16_t rxCompare_failed;

    /* See RX_PER_TAG_RXCARRIERFREQOFFSET_TAG */
    float rxCarrierFreqOffset_MHz;

    UWB_SR040_TxRxSlotType_t slotType;

} rxPerStatusCount_t;

static rxPerStatusCount_t gRxPer;
#if defined(_MSC_VER)
static FILE *gPerLogFile;
extern void board_SerialGetCOMPort(char *comPortName, size_t *pInOutcomPortNameLen);
#endif

extern void AppCallback(eNotificationType opType, void *pData);

static void demo_rx_per_AppCallback_PhyLogNtf(phPhyLogNtfnData_t *pNtf);

#if defined(_MSC_VER)
void demo_open_per_log_file(void)
{
    char comPortName[32]  = {0};
    size_t comPortNameLen = sizeof(comPortName) - 1;
    char logFileName[128] = {0};
    board_SerialGetCOMPort(comPortName, &comPortNameLen);
    snprintf(logFileName, sizeof(logFileName) - 1, "uwb_per_log_%s.log", comPortName);
    gPerLogFile = fopen(logFileName, "a+");
}

void demo_close_per_log_file(void)
{
    char comPortName[32]  = {0};
    size_t comPortNameLen = sizeof(comPortName) - 1;
    char logFileName[128] = {0};
    board_SerialGetCOMPort(comPortName, &comPortNameLen);
    snprintf(logFileName, sizeof(logFileName) - 1, "uwb_per_log_%s.log", comPortName);
    fclose(gPerLogFile);
}
#endif

void demo_rx_per_AppCallback(eNotificationType opType, void *pData)
{
    switch (opType) {
    case UWB_TEST_PHY_LOG_NTF: {
        phPhyLogNtfnData_t *pNtf = (phPhyLogNtfnData_t *)pData;
        demo_rx_per_AppCallback_PhyLogNtf(pNtf);
#if defined(_MSC_VER)
        if (gPerLogFile == NULL) {
            demo_open_per_log_file();
        }
        UINT16 len = pNtf->size;
        if (gPerLogFile != NULL) {
            for (UINT16 i = 0; i < len + 2; i++) {
                fprintf(gPerLogFile, "%02X", *((((UINT8 *)pData) + i)));
            }
            fprintf(gPerLogFile, "\n");
        }
#endif
        break;
    }
    default:
        AppCallback(opType, pData);
    }
}

static void rxper_ParseRxPer(phPhyLogNtfnData_t *pNtf);
static void rxper_ParseMacFailRSN(phPhyLogNtfnData_t *pNtf);
static void rxper_ParseFirstPathInfo(phPhyLogNtfnData_t *pNtf);
static void rxper_ParseRxFrame(phPhyLogNtfnData_t *pNtf);
static void rxper_ParseRxCarrierFreqOffset_MHz(phPhyLogNtfnData_t *pNtf);
static void demo_rx_per_AppCallback_PhyLogNtf(phPhyLogNtfnData_t *pNtf)
{
    if (pNtf->size > 1) {
        // LOG_MAU8_I("NTF", pNtf->data, pNtf->size);
        rxper_ParseRxPer(pNtf);
        rxper_ParseMacFailRSN(pNtf);
        rxper_ParseFirstPathInfo(pNtf);
        rxper_ParseRxFrame(pNtf);
        rxper_ParseRxCarrierFreqOffset_MHz(pNtf);
    }
}

void demo_rx_per_Init(UWB_SR040_TxRxSlotType_t slotType,
    uint8_t const *const rxCompare_expectedRxFrame,
    size_t rxCompare_expectedRxFrameLen)
{
    memset(&gRxPer, 0, sizeof(gRxPer));
    gRxPer.rxCompare_expectedRxFrame    = rxCompare_expectedRxFrame;
    gRxPer.rxCompare_expectedRxFrameLen = rxCompare_expectedRxFrameLen;
    gRxPer.slotType                     = slotType;
}

uint16_t demo_rx_per_GetNtfsCount()
{
    return gRxPer.totalRxNtfs;
}

void demo_rx_per_PrintSummary(UWB_SR040_TxRxSlotType_t slotType)
{
    PRINTF("###################################################\n");
    PRINTF("## RX PER STATISTICS \n");
    PRINTF("###################################################\n");

    /* clang-format off */

    PRINT_RX_PER_VALUE(gRxPer.rxSuccess_Ready, "rxSuccess_Ready", "Success:indicate RX ready.");
    PRINT_RX_PER_VALUE(gRxPer.rxSuccess_DataAvailable, "rxSuccess_DataAvailable", "Success:indicate RX payload decoded.");
    PRINT_RX_PER_VALUE(gRxPer.rxError_ToaDetectFailed, "rxError_ToaDetectFailed", "Failed: RX TOA_DETECT FAILED indicates failure in detection of valid first path/signal");
    PRINT_RX_PER_VALUE(gRxPer.rxError_SignalLost, "rxError_SignalLost", "Failed: RX SIGNAL LOST indicates RX EOF event was caused because the signal disappeared");
    PRINT_RX_PER_VALUE(gRxPer.rxError_PreambleTimeout, "rxError_PreambleTimeout", "Failed: RX PRMBL TIMEOUT indicates RX EOF event was caused because no signal was found within set timeout");
    PRINT_RX_PER_VALUE(gRxPer.rxError_SFDTimeout, "rxError_SFDTimeout", "Failed: RX SFD TIMEOUT indicates RX EOF event was caused because no SFD was found within set timeout ");
    PRINT_RX_PER_VALUE(gRxPer.rxError_SecdedDecodeFailed, "rxError_SecdedDecodeFailed", "Failed: RX SECDED DECODE FAILURE indicates the IEEE Parity check performed on the PHR detected uncorrectable errors");
    PRINT_RX_PER_VALUE(gRxPer.rxError_DecodeFailure, "rxError_DecodeFailure", "Failed: RX_RS_DECODE_FAILURE indicates the RS algorithm executed on the PSDU detected uncorrectable errors ");
    PRINT_RX_PER_VALUE(gRxPer.rxError_DecodeChainFailure, "rxError_DecodeChainFailure", "Failed: RX DECODE CHAIN FAILURE indicates the PSDU was undecodable by RS algorithm");
    PRINT_RX_PER_VALUE(gRxPer.rxError_DataBufferOverflow, "rxError_DataBufferOverflow", "Failed: RX DATA BUFFER OVERFLOW indicates RX data buffer overflow from the inner receiver");
    PRINT_RX_PER_VALUE(gRxPer.rxError_StsMismatch, "rxError_StsMismatch", "Failed: RX STS MISMATCH indicates STS mismatch");

    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_UNSPECIFIED, "MacDecode_Fail:RSN_UNSPECIFIED", "Reason of failure is unspecified");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FRAME_INVALID_LEN, "MacDecode_Fail:RSN_FRAME_INVALID_LEN", "Frame length is invalid");
    //PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_INVALID_CRC, "MacDecode_Fail:RSN_INVALID_CRC", "Invalid CRC");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_DSTADDR_INVALID, "MacDecode_Fail:RSN_DSTADDR_INVALID", "Destination address is invalid");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_SRCADDR_INVALID, "MacDecode_Fail:RSN_SRCADDR_INVALID", "Source address is invalid");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_LEN, "MacDecode_Fail:RSN_FCTRL_INVALID_LEN", "Frame control: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_FRAME_TYPE, "MacDecode_Fail:RSN_FCTRL_INVALID_FRAME_TYPE", "Frame control: invalid frame type");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_VERSION, "MacDecode_Fail:RSN_FCTRL_INVALID_VERSION", "Frame control: invalid version number");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_AR, "MacDecode_Fail:RSN_FCTRL_NOTSUPPORTED_AR", "Frame control: not supported value of AR (acknowledgment request)");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_ADDR_MODE, "MacDecode_Fail:RSN_FCTRL_INVALID_ADDR_MODE", "Frame control: invalid addressing mode");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_IEPRESENT, "MacDecode_Fail:RSN_FCTRL_NOTSUPPORTED_IEPRESENT", "Frame control: IE not present");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID, "MacDecode_Fail:RSN_AUXHDR_INVALID", "Auxiliary header is invalid");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_LEN, "MacDecode_Fail:RSN_AUXHDR_INVALID_LEN", "Auxiliary header: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_ID_MODE, "MacDecode_Fail:RSN_AUXHDR_INVALID_KEY_ID_MODE", "Auxiliary header: not supported key identifier mode");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_LEN, "MacDecode_Fail:RSN_AUXHDR_INVALID_KEY_LEN", "Auxiliary header: invalid key length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_SEC_LEVEL, "MacDecode_Fail:RSN_AUXHDR_INVALID_SEC_LEVEL", "Auxiliary header: invalid security level ");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_INDEX, "MacDecode_Fail:RSN_AUXHDR_INVALID_KEY_INDEX", "Auxiliary header: invalid Key index");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IE_INVALID, "MacDecode_Fail:RSN_IE_INVALID", "IE field is invalid");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IE_INVALID_LEN, "MacDecode_Fail:RSN_IE_INVALID_LEN", "IE field has invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_INVALID_LEN, "MacDecode_Fail:RSN_IEHDR_INVALID_LEN", "IE Header: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_NOTSUPPORTED_TYPE, "MacDecode_Fail:RSN_IEHDR_NOTSUPPORTED_TYPE", "IE Header: not supported type");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_HT1_EXPECTED, "MacDecode_Fail:RSN_IEHDR_HT1_EXPECTED", "IE Header Termination: expected HT1");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_HT2_EXPECTED, "MacDecode_Fail:RSN_IEHDR_HT2_EXPECTED", "IE Header Termination: expected HT2");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_MISSING, "MacDecode_Fail:RSN_IEHDR_MISSING", "IE Header: header missing");

    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IE_FIRA_INVALID_LEN, "MacDecode_Fail:RSN_IE_FIRA_INVALID_LEN", "FIRA IE: invalid length of IE field");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_LEN, "MacDecode_Fail:RSN_IEHDR_FIRA_INVALID_LEN", "FIRA IE Header: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_PADDING, "MacDecode_Fail:RSN_IEHDR_FIRA_INVALID_PADDING", "FIRA IE Header: invalid padding");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_INVALD_LEN, "MacDecode_Fail:RSN_MACPLD_INVALD_LEN", "MAC Payload: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_DECODE_FAILED, "MacDecode_Fail:RSN_MACPLD_DECODE_FAILED", "MAC Payload: decoding failed");

    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_LEN, "MacDecode_Fail:RSN_MACPLD_FIRA_INVALID_LEN", "FIRA MAC Payload: invalid length");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_DECRYPT_FAILED, "MacDecode_Fail:RSN_MACPLD_FIRA_DECRYPT_FAILED", "FIRA MAC Payload: decription failed");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_MISSING, "MacDecode_Fail:RSN_MACPLD_FIRA_MISSING", "FIRA MAC Payload: payload field is missing");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_IETYPE, "MacDecode_Fail:RSN_MACPLD_FIRA_INVALID_IETYPE", "FIRA MAC Payload: invalid IE type");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE, "MacDecode_Fail:RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE", "FIRA MAC Payload: invalid vendor specific IE type");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_OUI, "MacDecode_Fail:RSN_MACPLD_FIRA_INVALID_OUI", "FIRA MAC Payload: invalid OUI");
    PRINT_RX_PER_VALUE(gRxPer.MacDecode_Fail_RSN_INVALID_FRAME, "MacDecode_Fail:RSN_INVALID_FRAME", "General error on invalid frame");

    /* clang-format on */

    PRINTF("###################################################\n");
    PRINT_RX_PER_VALUE_F(gRxPer.rxCarrierFreqOffset_MHz, "FreqOffset MHz", "RX Carrier FREQ Offset");
    PRINTF("###################################################\n");
    PRINT_FIRSTPATH_PER_VALUE(gRxPer.fpInfo_firstPathPower, "firstPathPower", "FirstPath firstPathPower");
    PRINT_FIRSTPATH_PER_VALUE(
        (gRxPer.fpInfo_firstPathPower - gRxPer.fpInfo_noisePower), "SNR", "Signal To Noise Ratio");
    PRINT_FIRSTPATH_PER_VALUE(gRxPer.fpInfo_maxTapPower, "MaxTapPower", "FirstPath MaxTapPower");
    PRINT_FIRSTPATH_PER_VALUE(gRxPer.fpInfo_detectThrPower, "detectThrPower", "FirstPath detectThrPower");
    PRINT_FIRSTPATH_PER_VALUE(gRxPer.fpInfo_overallRxPower, "overallRxPower", "FirstPath overallRxPower");
    PRINT_FIRSTPATH_PER_VALUE(gRxPer.fpInfo_noisePower, "noisePower", "FirstPath noisePower");

    if ((gRxPer.rxCompare_success + gRxPer.rxCompare_failed) > 1) {
        // float total_failed_frames = gRxPer.totalRxNtfs -
        float rxPsduSuccessRate =
            (gRxPer.rxCompare_success * 100.0) / (gRxPer.rxCompare_failed + gRxPer.rxCompare_success);
        PRINTF("RX PSDU Success Rate = %0.2f%%\n", rxPsduSuccessRate);
    }

    PRINTF("###################################################\n");

#if defined(_MSC_VER)
    if (NULL != gPerLogFile) {
        demo_close_per_log_file();
    }
#endif
}

#define UPDATE_RX_ERROR_COUNT(value, index, field) \
    do {                                           \
        if (0 != (value & (0x01u << (index)))) {   \
            (field) += 1;                          \
        }                                          \
    } while (0)

static void rxper_ParseRxPer(phPhyLogNtfnData_t *pNtf)
{
    utlv_status_t status;
    utlv_entry_t entryRxStatus = {
        .tag      = RX_PER_TAG_PHY_RX_STATUS_TAG,
        .tag_type = kUTLV_Type_u16,
    };

    status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryRxStatus);

    ENSURE_OR_GO_CLEANUP(kUTLV_Status_Found == status);
    /* We found a valid RX Notification */
    gRxPer.totalRxNtfs++;

    const uint16_t rxStatus = entryRxStatus.tag_value.vu16;
    /*  indicate RX ready. */
    UPDATE_RX_ERROR_COUNT(rxStatus, 0, gRxPer.rxSuccess_Ready);
    /*  indicate RX payload decoded. */
    UPDATE_RX_ERROR_COUNT(rxStatus, 1, gRxPer.rxSuccess_DataAvailable);
    /*  RX TOA_DETECT FAILED indicates failure in detection of valid first path/signal.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 2, gRxPer.rxError_ToaDetectFailed);
    /*  RX SIGNAL LOST indicates RX EOF event was caused because the signal disappeared.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 3, gRxPer.rxError_SignalLost);
    /*  RX PRMBL TIMEOUT indicates RX EOF event was caused because no signal was found within set timeout.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 4, gRxPer.rxError_PreambleTimeout);
    /*  RX SFD TIMEOUT indicates RX EOF event was caused because no SFD was found within set timeout  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 5, gRxPer.rxError_SFDTimeout);
    /*  RX SECDED DECODE FAILURE indicates the IEEE Parity check performed on the PHR detected uncorrectable errors.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 6, gRxPer.rxError_SecdedDecodeFailed);
    /*  RX_RS_DECODE_FAILURE indicates the RS algorithm executed on the PSDU detected uncorrectable errors  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 7, gRxPer.rxError_DecodeFailure);
    /*  RX DECODE CHAIN FAILURE indicates the PSDU was undecodable by RS algorithm.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 8, gRxPer.rxError_DecodeChainFailure);
    /*  RX DATA BUFFER OVERFLOW indicates RX data buffer overflow from the inner receiver.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 9, gRxPer.rxError_DataBufferOverflow);
    /*  RX STS MISMATCH indicates STS mismatch.  */
    UPDATE_RX_ERROR_COUNT(rxStatus, 10, gRxPer.rxError_StsMismatch);

cleanup:
    return;
}

#define CASE_MAC_DECODE_FAIL_COUNT(VAL, VAR) \
    case (VAL):                              \
        ((VAR)++);                           \
        break;

static void rxper_ParseMacFailRSN(phPhyLogNtfnData_t *pNtf)
{
    utlv_status_t status;
    utlv_entry_t entryMAC_DECODSTATUS = {
        .tag      = RX_PER_TAG_MACSTATUS_TAG,
        .tag_type = kUTLV_Type_u8,
    };
    utlv_entry_t entryMAC_DECODEFAILRSN = {
        .tag      = RX_PER_TAG_MAC_DECODEFAILRSN_TAG,
        .tag_type = kUTLV_Type_u8,
    };

    status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryMAC_DECODSTATUS);
    if (kUTLV_Status_Found != status) {
        goto cleanup;
    }
    if (entryMAC_DECODSTATUS.tag_value.vu8 == PHSCA_MAC_PHY_DECODING_FAILURE) {
        status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryMAC_DECODEFAILRSN);
        ENSURE_OR_GO_CLEANUP(kUTLV_Status_Found == status);

        switch (entryMAC_DECODEFAILRSN.tag_value.vu16) {
            /* clang-format off */

        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_UNSPECIFIED, gRxPer.MacDecode_Fail_RSN_UNSPECIFIED);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FRAME_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_FRAME_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_INVALID_CRC, gRxPer.MacDecode_Fail_RSN_INVALID_CRC);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_DSTADDR_INVALID, gRxPer.MacDecode_Fail_RSN_DSTADDR_INVALID);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_SRCADDR_INVALID, gRxPer.MacDecode_Fail_RSN_SRCADDR_INVALID);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_INVALID_FRAME_TYPE, gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_FRAME_TYPE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_INVALID_VERSION, gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_VERSION);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_NOTSUPPORTED_AR, gRxPer.MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_AR);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_INVALID_ADDR_MODE, gRxPer.MacDecode_Fail_RSN_FCTRL_INVALID_ADDR_MODE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_FCTRL_NOTSUPPORTED_IEPRESENT, gRxPer.MacDecode_Fail_RSN_FCTRL_NOTSUPPORTED_IEPRESENT);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_ID_MODE, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_ID_MODE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_LEN, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID_SEC_LEVEL, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_SEC_LEVEL);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_AUXHDR_INVALID_KEY_INDEX, gRxPer.MacDecode_Fail_RSN_AUXHDR_INVALID_KEY_INDEX);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IE_INVALID, gRxPer.MacDecode_Fail_RSN_IE_INVALID);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IE_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_IE_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_IEHDR_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_NOTSUPPORTED_TYPE, gRxPer.MacDecode_Fail_RSN_IEHDR_NOTSUPPORTED_TYPE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_HT1_EXPECTED, gRxPer.MacDecode_Fail_RSN_IEHDR_HT1_EXPECTED);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_HT2_EXPECTED, gRxPer.MacDecode_Fail_RSN_IEHDR_HT2_EXPECTED);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_MISSING, gRxPer.MacDecode_Fail_RSN_IEHDR_MISSING);

        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IE_FIRA_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_IE_FIRA_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_FIRA_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_IEHDR_FIRA_INVALID_PADDING, gRxPer.MacDecode_Fail_RSN_IEHDR_FIRA_INVALID_PADDING);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_INVALD_LEN, gRxPer.MacDecode_Fail_RSN_MACPLD_INVALD_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_DECODE_FAILED, gRxPer.MacDecode_Fail_RSN_MACPLD_DECODE_FAILED);

        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_LEN, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_LEN);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_DECRYPT_FAILED, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_DECRYPT_FAILED);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_MISSING, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_MISSING);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_IETYPE, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_IETYPE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_VENDOR_IETYPE);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_MACPLD_FIRA_INVALID_OUI, gRxPer.MacDecode_Fail_RSN_MACPLD_FIRA_INVALID_OUI);
        CASE_MAC_DECODE_FAIL_COUNT(RX_PER_MAC_RSN_INVALID_FRAME, gRxPer.MacDecode_Fail_RSN_INVALID_FRAME);

        /* clang-format on */
        default:
            gRxPer.MacDecode_Fail_RSN_UNSPECIFIED++;
            break;
        }
    }
    else if (entryMAC_DECODSTATUS.tag_value.vu8 == PHSCA_MAC_PHY_DECODING_HEADER_SUCCESS) {
        gRxPer.MacDecode_HeaderSuccess++;
    }
    else if (entryMAC_DECODSTATUS.tag_value.vu8 == PHSCA_MAC_PHY_DECODING_PAYLOAD_SUCCESS) {
        gRxPer.MacDecode_PayloadSuccess++;
    }
    else {
        LOG_E("UNKNOWN entryMAC_DECODSTATUS=0x%X", entryMAC_DECODSTATUS.tag_value.vu8);
    }

cleanup:
    return;
}

#define LOG_10_2 (0.3010299956639810)

#include <math.h>

#if 0
#define LOG_10_2_25(X) ((int)((LOG_10_2 * 10 * (X))) >> 25)

#define LOG_10_2_9(X) ((long int)(LOG_10_2 * 10 * (X)) >> 9)

#else
#define POW_2_25       33554432.0
#define POW_2_9        512.0
#define LOG_10_2_25(X) (((LOG_10_2 * 10.0 * (X))) / (POW_2_25))

#define LOG_10_2_9(X) ((LOG_10_2 * 10.0 * (X)) / (POW_2_9))

#endif

static void rxper_ParseFirstPathInfo(phPhyLogNtfnData_t *pNtf)
{
    utlv_status_t status;
    phIscaBaseband_FirstPathInfo_t firstPathInfo = {0};
    utlv_entry_t entryFirstPathInfo              = {
        .tag      = RX_PER_TAG_RXFIRSTPATHINFO_TAG,
        .tag_type = kUTLV_Type_au8,
    };
    entryFirstPathInfo.tag_value.au8.ptr      = (uint8_t *)&firstPathInfo;
    entryFirstPathInfo.tag_value.au8.inMaxLen = sizeof(firstPathInfo);

    status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryFirstPathInfo);
    if (kUTLV_Status_Found != status) {
        goto cleanup;
    }
    if (entryFirstPathInfo.tag_value.au8.outLen != sizeof(firstPathInfo)) {
        goto cleanup;
    }
    if (firstPathInfo.firstPathdetected != 0) {
        //      PRINTF("overallRxPower=%ld %lX\n", firstPathInfo.overallRxPower, firstPathInfo.overallRxPower);
        //      PRINTF("firstPathIndex=%ld %lX\n", firstPathInfo.firstPathIndex, firstPathInfo.firstPathIndex);
        //      PRINTF("firstPathPower=%ld %lX\n", firstPathInfo.firstPathPower, firstPathInfo.firstPathPower);
        gRxPer.fpInfo_count += 1;
        gRxPer.fpInfo_maxTapPower += LOG_10_2_9(firstPathInfo.maxTapPower);
        gRxPer.fpInfo_firstPathPower += LOG_10_2_9(firstPathInfo.firstPathPower);
        gRxPer.fpInfo_noisePower += LOG_10_2_25(firstPathInfo.noisePower);
        gRxPer.fpInfo_detectThrPower += LOG_10_2_25(firstPathInfo.detectThrPower);
        gRxPer.fpInfo_overallRxPower += LOG_10_2_25(firstPathInfo.overallRxPower);
    }

cleanup:
    return;
}

static void rxper_ParseRxFrame(phPhyLogNtfnData_t *pNtf)
{
    utlv_status_t status;
    utlv_entry_t entryRXPSDU = {
        .tag      = RX_PER_TAG_PHY_RX_PSDU_LOG_TAG,
        .tag_type = kUTLV_Type_au8,
    };
    status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryRXPSDU);

    if (kUTLV_Status_Found != status) {
        if (gRxPer.slotType == kUWB_SR040_TxRxSlotType_SP0) {
            gRxPer.rxCompare_failed++;
        }
        goto cleanup;
    }

    if (
        /* same length */
        (entryRXPSDU.tag_value.au8.outLen == gRxPer.rxCompare_expectedRxFrameLen) && /* and */
        /* same contents */
        (0 == memcmp(gRxPer.rxCompare_expectedRxFrame,
                  entryRXPSDU.tag_value.au8.ptr,
                  gRxPer.rxCompare_expectedRxFrameLen))) {
        gRxPer.rxCompare_success++;
    }
    else {
        gRxPer.rxCompare_failed++;
    }

cleanup:
    return;
}

static void rxper_ParseRxCarrierFreqOffset_MHz(phPhyLogNtfnData_t *pNtf)
{
    utlv_status_t status;
    utlv_entry_t entryRxCarrierFreqOffset = {
        .tag      = RX_PER_TAG_RXCARRIERFREQOFFSET_TAG,
        .tag_type = kUTLV_Type_u16,
    };
    status = utlv_parse_entry(&pNtf->data[1], pNtf->size - 1u, &entryRxCarrierFreqOffset);

    if (kUTLV_Status_Found != status) {
        goto cleanup;
    }

    int16_t i16val = (int16_t)entryRxCarrierFreqOffset.tag_value.vu16;
    float CFO_INT  = i16val;
    // 1) Compute CFO in rad/pi per clock cycle: CFO = CFO_INT/2^17
    float CFO = CFO_INT / 131072.0;
    // 2)Compute CFO(Hz) = (124.8MHz/2)*CFO
    float CFO_MHz = (124.8 / 2.0) * CFO;

    gRxPer.rxCarrierFreqOffset_MHz += CFO_MHz;

cleanup:
    return;
}
