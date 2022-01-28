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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "SwupApi.h"
#include "phUwb_BuildConfig.h"
#include "phNxpLogApis_Swup.h"
#include "phNxpUwb_SpiTransport.h"
#include "phOsalUwb.h"
#include "phUwbErrorCodes.h"

#define MAX_RETRY_COUNT 0x09

static int read_swup_response(uint8_t *rsp, uint16_t *rsp_len)
{
    int retry_count  = 0;
    uint16_t readLen = 0;

retry:
    if (retry_count > MAX_RETRY_COUNT) {
        return RETURNED_FAILURE;
    }
    readLen = phNxpUwb_RciRead(rsp);
    if (readLen < 4) {
        phOsalUwb_Delay(100);
        retry_count++;
        goto retry;
    }
    else {
        *rsp_len = readLen;
        LOG_MAU8_I("SwupRx< ", rsp, *rsp_len);
        return RETURNED_SUCCESS;
    }
}

SwupResponseStatus_t Swup_TransferComponent(
    SwupStatus_t *swupStatus, const uint8_t componentIndex, const uint16_t segmentNumber, const uint8_t cmdData[])
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    LOG_I("%s #%d", __FUNCTION__ + 5, segmentNumber);
    /*send SWUP_SUB_CMD_TRANSFER_COMPONENT Command*/
    if (cmdData == NULL)
        return error;
    uint8_t App_RciTxData[SWUP_RCI_HEADER_LEN + SWUP_LEN_TRANSFER_COMPONENT] = {
        SWUP_COMMAND, SWUP_SUB_CMD_TRANSFER_COMPONENT, SWUP_FRAME_TYPE, SWUP_LEN_TRANSFER_COMPONENT};
    App_RciTxData[SWUP_RCI_DATA_OFFSET]     = componentIndex;
    App_RciTxData[SWUP_RCI_DATA_OFFSET + 1] = (uint8_t)(segmentNumber >> 8 * 0);
    App_RciTxData[SWUP_RCI_DATA_OFFSET + 2] = (uint8_t)(segmentNumber >> 8 * 1);
    memcpy(&App_RciTxData[SWUP_RCI_DATA_OFFSET + 3], &cmdData[0], TRANSFER_COMPONENT_SEGMENT_SIZE);

    LOG_MAU8_D("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_TransferManifest(
    SwupStatus_t *swupStatus, const uint8_t manifestIndex, const uint8_t cmdData[])
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    LOG_I("%s #%d", __FUNCTION__ + 5, manifestIndex);
    uint16_t resp_len = 0;
    uint8_t resp[256] = {0};

    if (cmdData == NULL)
        return error;
    /*send SWUP_SUB_CMD_TRANSFER_MANIFEST Command*/
    uint8_t App_RciTxData[SWUP_RCI_HEADER_LEN + SWUP_LEN_TRANSFER_MANIFEST] = {
        SWUP_COMMAND, SWUP_SUB_CMD_TRANSFER_MANIFEST, SWUP_FRAME_TYPE, SWUP_LEN_TRANSFER_MANIFEST};
    App_RciTxData[SWUP_RCI_DATA_OFFSET] = manifestIndex;
    memcpy(&App_RciTxData[SWUP_RCI_DATA_OFFSET + 1], &cmdData[0], TRANSFER_MANIFEST_SEGMENT_SIZE);

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_VerifyComponent(SwupStatus_t *swupStatus, const uint8_t componentIndex)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    /*send SWUP_SUB_CMD_VERIFY_COMPONENT Command*/
    uint8_t App_RciTxData[SWUP_RCI_HEADER_LEN + 1] = {
        SWUP_COMMAND, SWUP_SUB_CMD_VERIFY_COMPONENT, SWUP_FRAME_TYPE, SWUP_LEN_VERIFY_COMPONENT};
    App_RciTxData[SWUP_RCI_HEADER_LEN] = componentIndex;

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_VerifyAll(SwupStatus_t *swupStatus)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    LOG_I("%s", __FUNCTION__ + 5);
    /*send SWUP_SUB_CMD_VERIFY_ALL Command*/
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_VERIFY_ALL,
        SWUP_FRAME_TYPE,
        SWUP_LEN_VERIFY_ALL,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_FinishUpdate(SwupStatus_t *swupStatus)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    LOG_I("%s", __FUNCTION__ + 5);
    /*send SWUP_SUB_CMD_FINISH_UPDATE Command*/
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_FINISH_UPDATE,
        SWUP_FRAME_TYPE,
        SWUP_LEN_FINISH_UPDATE,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_StartUpdate(SwupStatus_t *swupStatus)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    LOG_I("%s", __FUNCTION__ + 5);
    /*send SWUP_SUB_CMD_START_UPDATE Command*/
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_START_UPDATE,
        SWUP_FRAME_TYPE,
        SWUP_LEN_START_UPDATE,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_ClearRamManifest(SwupStatus_t *swupStatus)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    /*send SWUP_SUB_CMD_CLEAR_RAM_MANIFEST Command*/
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_CLEAR_RAM_MANIFEST,
        SWUP_FRAME_TYPE,
        SWUP_LEN_CLEAR_RAM_MANIFEST,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1])) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32((*swupStatus), ptr);
    return error;
}

SwupResponseStatus_t Swup_GetDeviceInfo(SwupDeviceInfo_t *swupDeviceInfo)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    if (swupDeviceInfo == NULL) {
        return error;
    }

    LOG_I("%s", __FUNCTION__ + 5);
    /*send SWUP_SUB_CMD_GET_DEVICE_INFO Command*/
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_GET_DEVICE_INFO,
        SWUP_FRAME_TYPE,
        SWUP_LEN_GET_DEVICE_INFO,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1]) || (*(resp + 3) != 0x2C)) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32(swupDeviceInfo->swupStatus, ptr);
    if (error == STATUS_CMD_SUCCESS) {
        /* Ptr will be automatically incremented based on the size */
        STREAM_TO_ARRAY(&swupDeviceInfo->productId[0], ptr, sizeof(swupDeviceInfo->productId));
        STREAM_TO_UINT32(swupDeviceInfo->hardwareId, ptr);
        STREAM_TO_ARRAY(&swupDeviceInfo->typeCheckId[0], ptr, sizeof(swupDeviceInfo->typeCheckId));
        STREAM_TO_ARRAY(&swupDeviceInfo->romId[0], ptr, sizeof(swupDeviceInfo->romId));
        STREAM_TO_ARRAY(&swupDeviceInfo->swupVersion[0], ptr, sizeof(swupDeviceInfo->swupVersion));
    }
    return error;
}

SwupResponseStatus_t Swup_ReadDeviceId(SwupDeviceId_t *swupDeviceId)
{
    SwupResponseStatus_t error = STATUS_GENERIC_ERROR;
    uint16_t resp_len          = 0;
    uint8_t resp[256]          = {0};

    if (swupDeviceId == NULL) {
        return error;
    }
    /*send SWUP_SUB_CMD_READ_DEVICE_ID Command*/
    LOG_I("%s", __FUNCTION__ + 5);
    uint8_t App_RciTxData[] = {
        SWUP_COMMAND,
        SWUP_SUB_CMD_READ_DEVICE_ID,
        SWUP_FRAME_TYPE,
        SWUP_LEN_READ_DEVICE_ID,
    };

    LOG_MAU8_I("SwupTx> ", App_RciTxData, sizeof(App_RciTxData));
    if (phNxpUwb_RciWrite(App_RciTxData, sizeof(App_RciTxData)) != 0) {
        return error;
    }

    if (read_swup_response(resp, &resp_len) != RETURNED_SUCCESS) {
        return error;
    }
    if ((*resp != App_RciTxData[0]) || (*(resp + 1) != App_RciTxData[1]) || (*(resp + 3) != 0x1C)) {
        return error;
    }

    uint8_t *ptr = &resp[SWUP_RCI_DATA_OFFSET];
    /* Ptr will be automatically incremented based on the size */
    STREAM_TO_UINT32(error, ptr);
    STREAM_TO_UINT32(swupDeviceId->swupStatus, ptr);
    if (error == STATUS_CMD_SUCCESS) {
        /* Ptr will be automatically incremented based on the size */
        STREAM_TO_ARRAY(&swupDeviceId->waferId[0], ptr, sizeof(swupDeviceId->waferId));
        STREAM_TO_ARRAY(&swupDeviceId->waferNumber, ptr, sizeof(swupDeviceId->waferNumber));
        STREAM_TO_UINT16(swupDeviceId->waferCoordinateX, ptr);
        STREAM_TO_UINT16(swupDeviceId->waferCoordinateY, ptr);
        STREAM_TO_UINT32(swupDeviceId->serialNumber, ptr);
    }
    return error;
}
