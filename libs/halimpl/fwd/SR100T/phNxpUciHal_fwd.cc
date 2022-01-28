/*
*
* Copyright 2018-2020 NXP.
*
* NXP Confidential. This software is owned or controlled by NXP and may only be
* used strictly in accordance with the applicable license terms. By expressly
* accepting such terms or by downloading,installing, activating and/or otherwise
* using the software, you are agreeing that you have read,and that you agree to
* comply with and are bound by, such license terms. If you do not agree to be
* bound by the applicable license terms, then you may not retain, install, activate
* or otherwise use the software.
*
*/

/*************************************************************************************/
/*   INCLUDES */
/*************************************************************************************/

#include "phNxpUciHal_fwd.h"
#include "UWB_GpioIrq.h"
#include "phNxpUwb_SpiTransport.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_FwDnld.h"
#include <driver_config.h>
#include <phNxpUciHal_utils.h>
#include <phTmlUwb_spi.h>

#define ENABLE_FW_DOWNLOAD_LOG FALSE

static uint8_t *pfwImage    = NULL;
static uint32_t fwImageSize = 0;

/*************************************************************************************/
/*   LOCAL FUNCTIONS */
/*************************************************************************************/
static void setOpts(void)
{
    gOpts.link     = Link_Default;
    gOpts.mode     = Mode_Default;
    gOpts.capture  = Capture_Default;
    gOpts.imgFile  = NULL;
    gOpts.mosiFile = (char *)"Mosi.bin";
    gOpts.misoFile = (char *)"Miso.bin";
}

bool UWB_HeliosCE(bool set)
{
#if (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V2)
    GPIO_PinWrite(UWB_HELIOS_CE_GPIO, UWB_HELIOS_CE_PIN, set);
#elif (UWB_BOARD_RHODES_VERSION == UWB_BOARD_RHODES_V3)
    UWB_GpioSet(HELIOS_ENABLE, set);
    UWB_GpioSet(HELIOS_RTC_SYNC, set);
#endif
    return true;
}
static int init(void)
{
    UWB_HeliosCE(0);
    phOsalUwb_Delay(100); // Delay in Millisecond/
    UWB_HeliosCE(1);

    phOsalUwb_Delay(200);

    if (Capture_Off != gOpts.capture) {
        NXPLOG_UWB_FWDNLD_D("Not Capture_Off.....\n");
    }

    return 0;
}

void setFwImage(uint8_t *fwImgPtr, uint32_t fwSize)
{
    pfwImage    = fwImgPtr;
    fwImageSize = fwSize;
}

phHbci_Status_t phHbci_GetStatus(void)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_GetStatus Enter\n");
    gphHbci_MosiApdu.len = 0;
    gphHbci_MisoApdu.len = PHHBCI_LEN_HDR;
    if (phNxpUwb_HbciTransceive(
            (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &gphHbci_MisoApdu.len) == 0) {
        return phHbci_Success;
    }
    return phHbci_Failure;
}

phHbci_Status_t phHbci_GeneralStatus(phHbci_General_Command_t mode)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_GeneralStatus Enter\n");
    switch (gphHbci_MisoApdu.cls) {
    case phHbci_Class_General | phHbci_SubClass_Answer:
        switch (gphHbci_MisoApdu.ins) {
        case phHbci_General_Ans_HBCI_Ready:
            if (!mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x\n", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_Mode_Patch_ROM_Ready:
            if (phHbci_General_Cmd_Mode_Patch_ROM == mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x\n", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_Mode_HIF_Image_Ready:
            if (phHbci_General_Cmd_Mode_HIF_Image == mode) {
                return phHbci_Success;
            }

            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected General Status 0x%02x In Mode 0x%02x\n", gphHbci_MisoApdu.ins, mode);
            break;

        case phHbci_General_Ans_HBCI_Fail:
        case phHbci_General_Ans_Boot_Autoload_Fail:
        case phHbci_General_Ans_Boot_GPIOConf_CRC_Fail:
        case phHbci_General_Ans_Boot_TRIM_CRC_Fail:
        case phHbci_General_Ans_Boot_GPIOTRIM_CRC_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: HBCI Interface Failed With 0x%02x\n", gphHbci_MisoApdu.ins);
            break;

        case phHbci_General_Ans_Mode_Patch_ROM_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: Patch ROM Mode Failed!\n");
            break;

        case phHbci_General_Ans_Mode_HIF_Image_Fail:
            NXPLOG_UWB_FWDNLD_E("ERROR: HIF Image Mode Failed!\n");
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown General Status 0x%02x\n", gphHbci_MisoApdu.ins);
            break;
        }
        break;

    case phHbci_Class_General | phHbci_SubClass_Ack:
        switch (gphHbci_MisoApdu.ins) {
        case phHbci_Invlaid_Class:
            NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Class Error From Slave!\n");
            break;

        case phHbci_Invalid_Instruction:
            NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Instruction Error From Slave!\n");
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unexpected Instruction From Slave 0x%02x\n", gphHbci_MisoApdu.ins);
            break;
        }
        break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Unknown General Class 0x%02x\n", gphHbci_MisoApdu.cls);
        break;
    }

    return phHbci_Failure;
}

phHbci_Status_t phHbci_QueryInfo(uint8_t *pInfo, uint32_t *pInfoSz, uint32_t maxSz, bool matchMaxSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_QueryInfo Enter\n");
    uint8_t expCls, expIns;
    uint16_t lrc, dataSz = 0, payloadSz, segment;

    if (maxSz > PHHBCI_MAX_LEN_DATA_MISO) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Info Size Cannot Be Greater Than %u Bytes!\n", PHHBCI_MAX_LEN_DATA_MISO);
        return phHbci_Failure;
    }

    expCls = (gphHbci_MosiApdu.cls & (uint8_t)PHHBCI_CLASS_MASK) | phHbci_SubClass_Answer;
    expIns = gphHbci_MosiApdu.ins;

    gphHbci_MosiApdu.len = 0;
    phNxpUwb_HbciTransceive(
        (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &gphHbci_MisoApdu.len);

    payloadSz = gphHbci_MisoApdu.len;
    segment   = payloadSz & PHHBCI_APDU_SEG_FLAG;

    if (!segment) {
        lrc    = payloadSz ? PHHBCI_LEN_LRC : 0;
        dataSz = (uint16_t)(payloadSz - lrc);

        if (!dataSz) {
            NXPLOG_UWB_FWDNLD_E("ERROR: No Info From Slave!\n");
            return phHbci_Failure;
        }
    }

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Ack);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_Valid_APDU;

    if (gphHbci_MisoApdu.cls != expCls) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Class - Exp 0x%02x, Got 0x%02x\n", expCls, gphHbci_MisoApdu.cls);
        gphHbci_MosiApdu.ins = phHbci_Invlaid_Class;
    }
    else if (gphHbci_MisoApdu.ins != expIns) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Instruction - Exp 0x%02x, Got 0x%02x\n", expIns, gphHbci_MisoApdu.ins);
        gphHbci_MosiApdu.ins = phHbci_Invalid_Instruction;
    }
    else if (segment) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Invalid Payload Length!\n");
        gphHbci_MosiApdu.ins = phHbci_Invalid_Segment_Length;
    }
    else if (dataSz > maxSz) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Total Size (%u) Greater Than Max. Size (%u)!\n", dataSz, maxSz);
        gphHbci_MosiApdu.ins = phHbci_Invalid_Segment_Length;
    }
    else if (matchMaxSz && (dataSz != maxSz)) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Total Size (%u) Not Equal To Expected Size (%u)!\n", dataSz, maxSz);
        gphHbci_MosiApdu.ins = phHbci_Invalid_Segment_Length;
    }

    phNxpUwb_HbciTransceive(
        (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &gphHbci_MisoApdu.len);
    if (gphHbci_MosiApdu.ins & PHHBCI_ERROR_STATUS_MASK) {
        return phHbci_Failure;
    }

    if (gphHbci_MisoApdu.payload[dataSz] !=
        phHbci_CalcLrc((uint8_t *)&gphHbci_MisoApdu, (uint16_t)(PHHBCI_LEN_HDR + dataSz))) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Invalid LRC!\n");
        return phHbci_Failure;
    }

    phOsalUwb_MemCopy(&pInfo[*pInfoSz], gphHbci_MisoApdu.payload, dataSz);
    *pInfoSz += dataSz;

    return phHbci_Success;
}

phHbci_Status_t phHbci_GetGeneralInfo(uint8_t *pInfo, uint32_t *pInfoSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_GetGeneralInfo\n");
    if (gphHbci_MosiApdu.cls != (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query)) {
        NXPLOG_UWB_FWDNLD_E("ERROR: Invalid General Info Class = 0x%02x\n", gphHbci_MosiApdu.cls);
        return phHbci_Failure;
    }

    switch (gphHbci_MosiApdu.ins) {
    case phHbci_General_Qry_Chip_ID:
        return phHbci_QueryInfo(pInfo, pInfoSz, PHHBCI_HELIOS_CHIP_ID_SZ, TRUE);

    case phHbci_General_Qry_Helios_ID:
        return phHbci_QueryInfo(pInfo, pInfoSz, PHHBCI_HELIOS_ID_SZ, TRUE);

    case phHbci_General_Qry_CA_Root_Pub_Key:
        return phHbci_QueryInfo(pInfo, pInfoSz, PHHBCI_HELIOS_CA_ROOT_PUB_KEY_SZ, TRUE);

    case phHbci_General_Qry_NXP_Pub_Key:
        return phHbci_QueryInfo(pInfo, pInfoSz, PHHBCI_HELIOS_NXP_PUB_KEY_SZ, TRUE);

    case phHbci_General_Qry_ROM_Version:
        return phHbci_QueryInfo(pInfo, pInfoSz, PHHBCI_HELIOS_ROM_VERSION_SZ, TRUE);

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Undefined General Query = 0x%02x\n", gphHbci_MosiApdu.ins);
        return phHbci_Failure;
    }
}

phHbci_Status_t phHbci_GetInfo(uint8_t *pInfo, uint32_t *pInfoSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_GetInfo Enter\n");
    switch (gphHbci_MosiApdu.cls) {
    case phHbci_Class_General | phHbci_SubClass_Query:
        return phHbci_GetGeneralInfo(pInfo, pInfoSz);
        break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: No Info Defined For Class = 0x%02x\n", gphHbci_MosiApdu.cls);
        return phHbci_Failure;
    }
}

phHbci_Status_t phHbci_PutCommand(const uint8_t *pImg, uint32_t imgSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_PutCommand Enter\n");
    uint8_t ackCls, ackIns;
    uint16_t lrc;
    uint32_t dataSz, payloadSz;

    ackCls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Ack);
    ackIns = (uint8_t)phHbci_Valid_APDU;

    do {
        if (imgSz > PHHBCI_MAX_LEN_DATA_MOSI) {
            dataSz    = PHHBCI_MAX_LEN_DATA_MOSI;
            payloadSz = PHHBCI_APDU_SEG_FLAG;
        }
        else {
            lrc       = imgSz ? PHHBCI_LEN_LRC : 0;
            dataSz    = imgSz;
            payloadSz = (uint32_t)(dataSz + lrc);
        }
        gphHbci_MosiApdu.len = (uint16_t)payloadSz;

        phNxpUwb_HbciTransceive(
            (uint8_t *)&gphHbci_MosiApdu, PHHBCI_LEN_HDR, (uint8_t *)&gphHbci_MisoApdu, &gphHbci_MisoApdu.len);

        if ((gphHbci_MisoApdu.cls != ackCls) || (gphHbci_MisoApdu.ins != ackIns)) {
            NXPLOG_UWB_FWDNLD_E(
                "ERROR: NACK (CLS = 0x%02x, INS = 0x%02x)\n", gphHbci_MisoApdu.cls, gphHbci_MisoApdu.ins);
            return phHbci_Failure;
        }

        if (dataSz) {
            phOsalUwb_MemCopy(gphHbci_MosiApdu.payload, pImg, dataSz);
            gphHbci_MosiApdu.payload[dataSz] =
                phHbci_CalcLrc((uint8_t *)&gphHbci_MosiApdu, (uint16_t)(PHHBCI_LEN_HDR + dataSz));

            pImg += dataSz;
            imgSz -= dataSz;
            payloadSz = (uint16_t)(dataSz + PHHBCI_LEN_LRC);

            phNxpUwb_HbciTransceive((uint8_t *)&gphHbci_MosiApdu.payload,
                (uint16_t)payloadSz,
                (uint8_t *)&gphHbci_MisoApdu,
                &gphHbci_MisoApdu.len);

            if ((gphHbci_MisoApdu.cls != ackCls) || (gphHbci_MisoApdu.ins != ackIns)) {
                NXPLOG_UWB_FWDNLD_E(
                    "ERROR: NACK (CLS = 0x%02x, INS = 0x%02x)\n", gphHbci_MisoApdu.cls, gphHbci_MisoApdu.ins);
                return phHbci_Failure;
            }
        }
    } while (imgSz);

    return phHbci_Success;
}

static phHbci_Status_t phHbci_MasterPatchROM(const uint8_t *pImg, uint32_t imgSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_MasterPatchROM enter");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_General_Qry_Status;

    while (1) {
        if (phHbci_Success != (ret = phHbci_GetStatus())) {
            return ret;
        }

        switch (gphHbci_MisoApdu.cls) {
        case phHbci_Class_General | phHbci_SubClass_Answer:
        case phHbci_Class_General | phHbci_SubClass_Ack:
            if (phHbci_Success != (ret = phHbci_GeneralStatus(phHbci_General_Cmd_Mode_Patch_ROM))) {
                return ret;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_Patch_ROM | phHbci_SubClass_Command);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_Patch_ROM_Cmd_Download_Patch;

            if (phHbci_Success != (ret = phHbci_PutCommand(pImg, imgSz))) {
                return ret;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_Patch_ROM | phHbci_SubClass_Query);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_Patch_ROM_Qry_Patch_Status;
            break;

        case phHbci_Class_Patch_ROM | phHbci_SubClass_Answer:
            switch (gphHbci_MisoApdu.ins) {
            case phHbci_Patch_ROM_Ans_Patch_Success:
                NXPLOG_UWB_FWDNLD_D("Patch ROM Transfer Complete.\n");
                ret = phHbci_Success;
                break;

            case phHbci_Patch_ROM_Ans_File_Too_Large:
            case phHbci_Patch_ROM_Ans_Invalid_Patch_File_Marker:
            case phHbci_Patch_ROM_Ans_Too_Many_Patch_Table_Entries:
            case phHbci_Patch_ROM_Ans_Invalid_Patch_Code_Size:
            case phHbci_Patch_ROM_Ans_Invalid_Global_Patch_Marker:
            case phHbci_Patch_ROM_Ans_Invalid_Signature_Size:
            case phHbci_Patch_ROM_Ans_Invalid_Signature:
                NXPLOG_UWB_FWDNLD_E("EROOR: Patch ROM Transfer Failed With 0x%02x!\n", gphHbci_MisoApdu.ins);
                ret = phHbci_Failure;
                break;

            default:
                NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Patch ROM Status 0x%02x\n", gphHbci_MisoApdu.ins);
                ret = phHbci_Failure;
                break;
            }
            return ret;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Class 0x%02x\n", gphHbci_MisoApdu.cls);
            return phHbci_Failure;
        }
    }

    return phHbci_Success;
}

static phHbci_Status_t phHbci_MasterHIFImage(const uint8_t *pImg, uint32_t imgSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_MasterHIFImage enter");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_General_Qry_Status;

    while (1) {
        if (phHbci_Success != (ret = phHbci_GetStatus())) {
            return ret;
        }

        switch (gphHbci_MisoApdu.cls) {
        case phHbci_Class_General | phHbci_SubClass_Answer:
        case phHbci_Class_General | phHbci_SubClass_Ack:
            if (phHbci_Success != (ret = phHbci_GeneralStatus(phHbci_General_Cmd_Mode_HIF_Image))) {
                return ret;
            }

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_HIF_Image | phHbci_SubClass_Command);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_HIF_Image_Cmd_Download_Image;

            if (phHbci_Success != (ret = phHbci_PutCommand(pImg, imgSz))) {
                return ret;
            }

            phOsalUwb_Delay(100);

            gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_HIF_Image | phHbci_SubClass_Query);
            gphHbci_MosiApdu.ins = (uint8_t)phHbci_HIF_Image_Qry_Image_Status;
            break;

        case phHbci_Class_HIF_Image | phHbci_SubClass_Answer:
            switch (gphHbci_MisoApdu.ins) {
            case phHbci_HIF_Image_Ans_Image_Success:
                NXPLOG_UWB_FWDNLD_D("HIF Image Transfer Complete.\n");
                /*Check FW download throughput measurement*/
                return phHbci_Success;

            case phHbci_HIF_Image_Ans_Header_Too_Large:
            case phHbci_HIF_Image_Ans_Header_Parse_Error:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Crypto:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Curve:
            case phHbci_HIF_Image_Ans_Invalid_ECC_Key_Length:
            case phHbci_HIF_Image_Ans_Invalid_Payload_Description:
            case phHbci_HIF_Image_Ans_Invalid_Firmware_Version:
            case phHbci_HIF_Image_Ans_Invalid_ECID_Mask:
            case phHbci_HIF_Image_Ans_Invalid_ECID_Value:
            case phHbci_HIF_Image_Ans_Invalid_Encrypted_Payload_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Header_Signature:
            case phHbci_HIF_Image_Ans_Install_Settings_Too_Large:
            case phHbci_HIF_Image_Ans_Install_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Payload_Too_Large:
            case phHbci_HIF_Image_Ans_Quickboot_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Invalid_Static_Hash:
            case phHbci_HIF_Image_Ans_Invalid_Dynamic_Hash:
            case phHbci_HIF_Image_Ans_Execution_Settings_Parse_Error:
            case phHbci_HIF_Image_Ans_Key_Read_Error:
                NXPLOG_UWB_FWDNLD_E("EROOR: HIF Image Transfer Failed With 0x%02x!\n", gphHbci_MisoApdu.ins);
                return phHbci_Failure;
            default:
                NXPLOG_UWB_FWDNLD_E("ERROR: Unknown HIF Status 0x%02x\n", gphHbci_MisoApdu.ins);
                return phHbci_Failure;
            }
            break;

        default:
            NXPLOG_UWB_FWDNLD_E("ERROR: Unknown Class 0x%02x\n", gphHbci_MisoApdu.cls);
            return phHbci_Failure;
        }
    }

    return phHbci_Success;
}

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS */
/*********************************************************************************************************************/
phHbci_Status_t phHbci_Master(phHbci_General_Command_t mode, const uint8_t *pImg, uint32_t imgSz)
{
    NXPLOG_UWB_FWDNLD_D("phHbci_Master Enter\n");
    phHbci_Status_t ret = phHbci_Failure;

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Query);
    gphHbci_MosiApdu.ins = (uint8_t)phHbci_General_Qry_Status;

    if (phHbci_Success != (ret = phHbci_GetStatus())) {
        return ret;
    }
    if (phHbci_Success != (ret = phHbci_GeneralStatus((phHbci_General_Command_t)0))) {
        return ret;
    }

    gphHbci_MosiApdu.cls = (uint8_t)(phHbci_Class_General | phHbci_SubClass_Command);
    gphHbci_MosiApdu.ins = (uint8_t)mode;
    NXPLOG_UWB_FWDNLD_D("STARTING FW DOWNLOAD.....\n");
    if (phHbci_Success != (ret = phHbci_PutCommand(pImg, 0))) {
        return ret;
    }

    switch (mode) {
    case phHbci_General_Cmd_Mode_Patch_ROM:
        return phHbci_MasterPatchROM(pImg, imgSz);

    case phHbci_General_Cmd_Mode_HIF_Image:
        return phHbci_MasterHIFImage(pImg, imgSz);

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Undefined mode 0x%02x\n", mode);
        break;
    }

    return phHbci_Failure;
}

/*********************************************************************************************************************/
/*   GLOBAL FUNCTIONS */
/*********************************************************************************************************************/
uint8_t phHbci_CalcLrc(uint8_t *pBuf, uint16_t bufSz)
{
    uint8_t lrc = 0;
    uint16_t i;

    if (!pBuf || !bufSz)
        return lrc;

    /* ISO 1155:1978 Information processing -- Use of longitudinal parity to
   * detect errors in information messages */
    for (i = 0; i < bufSz; i++) {
        lrc = (uint8_t)(lrc + *pBuf++);
    }

    lrc ^= 0xFF;
    lrc = (uint8_t)(lrc + 1);

    return lrc;
}

/******************************************************************************
 * Function         phNxpUciHal_fw_download
 *
 * Description      This function is called by jni when wired mode is
 *                  performed.First SR100 driver will give the access
 *                  permission whether wired mode is allowed or not
 *                  arg (0):
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
int phNxpUciHal_fw_download()
{
    const uint8_t *pImg;
    uint32_t imgSz = 0, err = 0;
    phHbci_General_Command_t cmd;

    NXPLOG_UWB_FWDNLD_D("phNxpUciHal_fw_download enter and FW download started.....\n");
    setOpts();

    if (init()) {
        NXPLOG_UWB_FWDNLD_E("INIT Failed.....\n");
        return 1;
    }

    switch (gOpts.mode) {
    case Mode_Patch_ROM:
        cmd = phHbci_General_Cmd_Mode_Patch_ROM;
        break;

    case Mode_HIF_Image:
        cmd = phHbci_General_Cmd_Mode_HIF_Image;
        break;

    default:
        NXPLOG_UWB_FWDNLD_E("ERROR: Undefined Master Mode = %u\n", gOpts.mode);
        return 1;
    }

    pImg  = pfwImage;
    imgSz = fwImageSize;

    NXPLOG_UWB_FWDNLD_D("FWD file size: %d\n", imgSz);

    if (cmd == phHbci_General_Cmd_Mode_HIF_Image) {
        NXPLOG_UWB_FWDNLD_D("HIF Image mode.\n");
    }

    if (phHbci_Success != phHbci_Master(cmd, pImg, imgSz)) {
        NXPLOG_UWB_FWDNLD_E("Failure!\n");
        err = 1;
    }

    return err;
}
