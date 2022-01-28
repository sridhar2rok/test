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

#include "phNxpLogApis_Swup.h"
#include <Swup_update.h>
#include <SwupApi.h>

/* Prints the Swup Execution status */
void Swup_PrintExecStatus(SwupResponseStatus_t message, SwupStatus_t swupStatus)
{
    if (message != STATUS_CMD_SUCCESS) {
        /* Print the Swup status during failure.*/
        switch (swupStatus) {
        case SWUP_STATUS_INIT:
            LOG_W("Swup Status INIT during failure");
            break;
        case SWUP_STATUS_ACTIVE:
            LOG_W("Swup Status ACTIVE during failure");
            break;
        case SWUP_STATUS_TRANSFER:
            LOG_W("Swup Status TRANSFER during failure");
            break;
        default:
            LOG_W("Swup Status ERROR during failure");
            break;
        }
    }

    switch (message) {
    case STATUS_CMD_SUCCESS:
        LOG_I("Swup RCI Command Execution Successful");
        break;
    case STATUS_CMD_PARAMETER_ERROR:
        LOG_E("Swup RCI Command Parameter Error");
        LOG_W("Failure due to an error in the DATA field of RCI Frame Format.");
        break;
    case STATUS_CMD_LENGTH_ERROR:
        LOG_E("Swup RCI Command Length Error");
        LOG_W("Failure due to an error in the HEADER (length byte for the payload).");
        break;
    case STATUS_CMD_ACCESS_DENIED:
        LOG_E("Swup RCI Command Access Denied Error");
        LOG_W("Failure due to Package Mismatch or Keys Mismtach for the corresponding package.");
        break;
    case STATUS_COMM_INCOMPLETE_TRANSFER:
        LOG_E("Swup RCI Command Incomplete Transfer Error");
        LOG_W(
            "Failure Due to incomplete execution of the transfer manifest/component. Reset the [Power Off-on] Device "
            "and try.");
        break;
    case STATUS_COMM_CRC_ERROR:
        LOG_E("Swup RCI Command CRC Error");
        LOG_W("Failure Due to incorrect CRC16 field in the RCI frame format.");
        break;
    case STATUS_COMM_HEADER_ERROR:
        LOG_E("Swup RCI Command Header Error");
        LOG_W("Failure Due to incorrect Header field in the RCI frame format.");
        break;
    case STATUS_UNKNOWN_COMMAND:
        LOG_E("Swup RCI Command Unknown Error");
        LOG_W("Failure Due to incorrect Cmd/SubCmd field in the RCI frame format.");
        break;
    case STATUS_ERR_NOT_IMPLEMENTED:
        LOG_E("Swup RCI Command Not Implemented Error");
        LOG_W("Failure Due to incorrect Frame Type field in the RCI frame format.");
        break;
    case STATUS_ERR_VERIFICATION_FAILED:
        LOG_E("Swup RCI Command Verification Failed Error");
        LOG_W("Failure Due to mismatch in the signature of the manifest/component.");
        break;
    case STATUS_GENERIC_ERROR:
        LOG_E("Swup RCI Command Generic Error");
        LOG_W("Possible Reasons:");
        LOG_W("1. SWUP Status is not Active. Please Activate SWUP and try.");
        LOG_W("2. Mismatch in the Key Version of the Package.");
        LOG_W("3. Mismatch in the Device Info type check id.");
        LOG_W("4. Any incomplete transfer of the manifest. Reset the [Power Off-on] Device and try.");
        LOG_W("5. UWB Device Init failure. Reset the [Power Off-on] Device and try.");
        LOG_W("6. Any other error then Reset the [Power Off-on] Device and try.");
        break;
    default:
        break;
    }
}

SwupResponseStatus_t Swup_Execute(
    const uint8_t swup_pkg[], const uint32_t size_swup_pkg, const SwupKeyVersion_t key_version)
{
    SwupResponseStatus_t error      = STATUS_GENERIC_ERROR;
    SwupDeviceInfo_t swupDeviceInfo = {0};
    SwupStatus_t swupStatus         = SWUP_STATUS_ERROR;
    uint16_t starting_segment_index;

    error      = Swup_GetDeviceInfo(&swupDeviceInfo);
    swupStatus = swupDeviceInfo.swupStatus;
    if (STATUS_CMD_SUCCESS != error) {
        goto cleanup;
    }
    if ((swupDeviceInfo.swupVersion[4] < 40) && swupDeviceInfo.swupVersion[6] == 0x04) {
        /* Old, Nxp Internal IC */
        starting_segment_index = 1;
    }
    else {
        /* Production samples */
        starting_segment_index = 0;
    }
    if (swupStatus == SWUP_STATUS_TRANSFER) {
        goto doTransfer;
    }
    if (swupStatus != SWUP_STATUS_ACTIVE) {
        goto cleanup;
    }

    switch (key_version) {
    case kSWUP_KEY_VESRION_ENGvZ20_05:
        if (swupDeviceInfo.typeCheckId[6] == 0x0B) {
            /* OK */
            break;
        }
        else if (swupDeviceInfo.typeCheckId[6] == 0x0C) {
            /* OK */
            break;
        }
        else {
            LOG_E("Not Updating. Only allowed for 0x0B and 0xC typeCheckId[6].");
            /* Mismatch typecheck ID */
            goto cleanup;
        }
    case kSWUP_KEY_VESRION_PRODvA20_06:
        if (swupDeviceInfo.typeCheckId[6] == 'S') { /* == 0x53 */
            /* OK */
            break;
        }
        else {
            LOG_E("Not Updating. Only allowed for 'S' == typeCheckId[6].");
            /* Mismatch typecheck ID */
            goto cleanup;
        }
    default:
        break;
    }

    error = Swup_ClearRamManifest(&swupStatus);
    GO_CLENAUP_IF_NOT_ACTIVE;

    for (uint8_t iChunk = 0; iChunk < (TRANSFER_MANIFEST_SIZE / TRANSFER_CHUNK_SIZE); iChunk++) {
        if (swupStatus == SWUP_STATUS_ACTIVE) {
            error = Swup_TransferManifest(
                &swupStatus, iChunk, &swup_pkg[TRANSFER_MANIFEST_OFFSET + (iChunk * TRANSFER_MANIFEST_SEGMENT_SIZE)]);
            GO_CLENAUP_IF_NOT_ACTIVE;
        }
    }
    LOG_I("Swup Transfer Manifest Completed");

    error = Swup_StartUpdate(&swupStatus);
    GO_CLENAUP_IF_NOT_TRANSFER;
    LOG_I("Swup Start Update Completed");

doTransfer:
    /* Changes Component Index as required */
    {
        uint8_t componentIndex  = 0;
        uint32_t noOfComponents = (size_swup_pkg - TRANSFER_MANIFEST_SIZE) / TRANSFER_CHUNK_SIZE;
        for (uint16_t segment = starting_segment_index; segment < (noOfComponents + starting_segment_index);
             segment++) {
            if (swupStatus == SWUP_STATUS_TRANSFER) {
                uint32_t index = TRANSFER_COMPONENT_0_OFFSET +
                                 ((segment - starting_segment_index) * TRANSFER_COMPONENT_SEGMENT_SIZE);
                error = Swup_TransferComponent(&swupStatus, componentIndex, segment, &swup_pkg[index]);
                GO_CLENAUP_IF_NOT_TRANSFER;
            }
        }
    }
    LOG_I("Swup Transfer Component Completed");

    error = Swup_VerifyAll(&swupStatus);
    if (STATUS_CMD_SUCCESS != error) {
        goto cleanup;
    }
    LOG_I("Swup Verify All Completed");

    if (swupStatus == SWUP_STATUS_TRANSFER) {
        /*As SR040 self resets after successfull Swup_FinishUpdate execution
        so we don't expect return as such*/
        error = Swup_FinishUpdate(&swupStatus);
        if (STATUS_CMD_SUCCESS != error) {
            if (error == 0xFFFFFFFF) {
                error = STATUS_CMD_SUCCESS;
            }
            else {
                LOG_W("WARN: Swup_FinishUpdate returned 0x%08X", error);
                goto cleanup;
            }
        }
    }
    LOG_I("Swup Finish Update Completed");

cleanup:
    Swup_PrintExecStatus(error, swupStatus);
    return error;
}

SwupResponseStatus_t SwupUpdate(
    const uint8_t swup_pkg[], const uint32_t size_swup_pkg, const SwupKeyVersion_t key_version)
{
    SwupResponseStatus_t status = STATUS_GENERIC_ERROR;
    SwupDeviceInfo_t swupDeviceInfo;
    SwupDeviceId_t swupDeviceId;

    /** The device should be in SWUP for this to run.
      * If this command fails, return a failure. The
      * device might not be in SWUP mode.
      */
    status = Swup_GetDeviceInfo(&swupDeviceInfo);
    if (STATUS_CMD_SUCCESS != status) {
        LOG_E("Swup_GetDeviceInfo failed! Exiting");
        goto cleanup;
    }
    else {
#if (SWUP_LOG_LEVEL >= UWB_LOG_INFO_LEVEL)
        uint32_t swupStatus = swupDeviceInfo.swupStatus;
        uint32_t hardwareId = swupDeviceInfo.hardwareId;
#endif
        LOG_D("Swup_GetDeviceInfo passed");
        LOG_X32_I(swupStatus);
        LOG_MAU8_I("productId", swupDeviceInfo.productId, sizeof(swupDeviceInfo.productId));
        LOG_X32_I(hardwareId);
        LOG_MAU8_I("typeCheckId", swupDeviceInfo.typeCheckId, sizeof(swupDeviceInfo.typeCheckId));
        LOG_MAU8_I("romId", swupDeviceInfo.romId, sizeof(swupDeviceInfo.romId));
        LOG_MAU8_I("swupVersion", swupDeviceInfo.swupVersion, sizeof(swupDeviceInfo.swupVersion));
    }
    status = Swup_ReadDeviceId(&swupDeviceId);
    if (STATUS_CMD_SUCCESS == status) {
        LOG_D("Swup_ReadDeviceId passed");
    }
    else {
        LOG_W("Swup_ReadDeviceId failed! Exiting");
        goto cleanup;
    }
    // printf("swupDeviceId waferCoordinateX= 0x%04X\n", swupDeviceId.waferCoordinateX);
    // printf("swupDeviceId waferCoordinateY= 0x%04X\n", swupDeviceId.waferCoordinateY);
    // printf("swupDeviceId serialNumber= 0x%08X\n", swupDeviceId.serialNumber);

    /** The device is in SWUP state.
      * Perform SoftWare UPdate
      */
    status = Swup_Execute(swup_pkg, size_swup_pkg, key_version);

cleanup:
    return status;
}
