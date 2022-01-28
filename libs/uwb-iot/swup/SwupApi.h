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

#ifndef _SWUP_DEFS_H_
#define _SWUP_DEFS_H_

#include <stdint.h>

#define TRANSFER_MANIFEST_SIZE         512u
#define TRANSFER_CHUNK_SIZE            128u
#define TRANSFER_MANIFEST_OFFSET       0u
#define TRANSFER_MANIFEST_SEGMENT_SIZE 128u

#define TRANSFER_COMPONENT_0_OFFSET     512u
#define TRANSFER_COMPONENT_SEGMENT_SIZE 128u

#define SWUP_COMMAND    0x70
#define SWUP_FRAME_TYPE 0x3A

#define SWUP_STATUS_OFFSET       0x06
#define SWUP_RCI_RESPONSE_OFFSET 0x00
#define SWUP_RCI_DATA_OFFSET     0x04

#define SWUP_RCI_HEADER_LEN 0x04

#define SWUP_SUB_CMD_GET_DEVICE_INFO    0x21
#define SWUP_SUB_CMD_READ_DEVICE_ID     0x22
#define SWUP_SUB_CMD_CLEAR_RAM_MANIFEST 0x14
#define SWUP_SUB_CMD_TRANSFER_MANIFEST  0x02
#define SWUP_SUB_CMD_START_UPDATE       0x13
#define SWUP_SUB_CMD_TRANSFER_COMPONENT 0x01
#define SWUP_SUB_CMD_VERIFY_COMPONENT   0x10
#define SWUP_SUB_CMD_VERIFY_ALL         0x11
#define SWUP_SUB_CMD_FINISH_UPDATE      0x12

#define SWUP_LEN_TRANSFER_COMPONENT (TRANSFER_COMPONENT_SEGMENT_SIZE + 3)
#define SWUP_LEN_TRANSFER_MANIFEST  (TRANSFER_MANIFEST_SEGMENT_SIZE + 1)

#define SWUP_LEN_VERIFY_COMPONENT   0x01
#define SWUP_LEN_VERIFY_ALL         0x00
#define SWUP_LEN_FINISH_UPDATE      0x00
#define SWUP_LEN_START_UPDATE       0x00
#define SWUP_LEN_CLEAR_RAM_MANIFEST 0x00
#define SWUP_LEN_GET_DEVICE_INFO    0x00
#define SWUP_LEN_READ_DEVICE_ID     0x00

#define STREAM_TO_ARRAY(a, p, len)           \
    {                                        \
        register int ijk;                    \
        for (ijk = 0; ijk < (int)len; ijk++) \
            ((uint8_t *)a)[ijk] = *p++;      \
    }

#ifndef STREAM_TO_UINT32
#define STREAM_TO_UINT32(v32, p)      \
    do {                              \
        v32 = 0;                      \
        v32 |= (uint8_t)(*((p) + 3)); \
        v32 <<= 8;                    \
        v32 |= (uint8_t)(*((p) + 2)); \
        v32 <<= 8;                    \
        v32 |= (uint8_t)(*((p) + 1)); \
        v32 <<= 8;                    \
        v32 |= (uint8_t)(*((p) + 0)); \
        (p) += 4;                     \
    } while (0)
#endif

#ifndef STREAM_TO_UINT16
#define STREAM_TO_UINT16(v16, p)      \
    do {                              \
        uint32_t v32 = 0;             \
        v32 |= (uint8_t)(*((p) + 1)); \
        v32 <<= 8;                    \
        v32 |= (uint8_t)(*((p) + 0)); \
        v16 = (uint16_t)v32;          \
        (p) += 2;                     \
    } while (0)
#endif

/**
 * @addtogroup swup
 * @{
 */

/**
 * \brief Swup device Info parameters
 */
/* @{ */
typedef struct
{
    /** swup rci return status */
    uint32_t swupStatus;
    /** device product Id */
    uint8_t productId[8];
    /** device hardware Id */
    uint32_t hardwareId;
    /** device typecheck Id */
    uint8_t typeCheckId[8];
    /** rom Id */
    uint8_t romId[8];
    /** swup version */
    uint8_t swupVersion[8];
} SwupDeviceInfo_t;
/* @}*/

/**
 * \brief swup device ID parameters
 */
/* @{ */
typedef struct
{
    /** swup rci return status */
    uint32_t swupStatus;
    /** device wafer Id */
    uint8_t waferId[11];
    /** device wafer no. */
    uint8_t waferNumber;
    /** wafer x coordinate */
    uint16_t waferCoordinateX;
    /** wafer y coordinate */
    uint16_t waferCoordinateY;
    /** device serial no. */
    uint32_t serialNumber;
} SwupDeviceId_t;
/* @}*/

/**
 * \brief SWUP RCI response status codes
 */
/* @{ */
typedef enum
{
    /** swup Rci cmd Seccess */
    STATUS_CMD_SUCCESS = 0x00000000u,
    /** swup Rci cmd parameter error */
    STATUS_CMD_PARAMETER_ERROR = 0x000000F0u,
    /** swup Rci cmd length error */
    STATUS_CMD_LENGTH_ERROR = 0x000000F1u,
    /** swup Rci cmd access denied */
    STATUS_CMD_ACCESS_DENIED = 0x000000F2u,
    /** swup Rci cmd incomplete transfer */
    STATUS_COMM_INCOMPLETE_TRANSFER = 0x000000F7u,
    /** swup Rci cmd crc error */
    STATUS_COMM_CRC_ERROR = 0x000000F8u,
    /** swup Rci cmd header error */
    STATUS_COMM_HEADER_ERROR = 0x000000F9u,
    /** swup Rci cmd unknown */
    STATUS_UNKNOWN_COMMAND = 0x000000FAu,
    /** swup Rci cmd not exist */
    STATUS_ERR_NOT_IMPLEMENTED = 0x000000FEu,
    /** swup Rci cmd verification failed */
    STATUS_ERR_VERIFICATION_FAILED = 0x0000007Du,
    /** swup Rci cmd generic error */
    STATUS_GENERIC_ERROR = 0xFFFFFFFFu,
} SwupResponseStatus_t;
/* @}*/

/**
 * \brief SWUP response status codes
 */
/* @{ */
typedef enum
{
    /** swup state error */
    SWUP_STATUS_ERROR = 0x00,
    /** swup in init state */
    SWUP_STATUS_INIT = 0x01,
    /** swup in active state */
    SWUP_STATUS_ACTIVE = 0x02,
    /** swup in transfer state */
    SWUP_STATUS_TRANSFER = 0x03
} SwupStatus_t;
/* @}*/

/**
* \brief Pacakge is signed with which version / variant of the key
*/

/* 0x0E02005 and 0x0A0206 are Magic numbers.
 * They have no paricular relevance. */

typedef enum
{
    /** Default to unknown value in case
     *  new package version is used.
     */
    kSWUP_KEY_VESRION_UNKNOWN = 0x00000000,
    /** NXP Internal Engineering Keys 20.05 */
    kSWUP_KEY_VESRION_ENGvZ20_05 = 0x0E02005,
    /** Proudction keys A 20.06 */
    kSWUP_KEY_VESRION_PRODvA20_06 = 0x0A0206,
} SwupKeyVersion_t;

/**
 * @brief      This API use to Activate SWUP interface on the device
 *
 * @return     STATUS_CMD_SUCCESS in case of activation or
 *             SwupResponseStatus_t error codes in case of failure
 */
SwupResponseStatus_t SwupIntfs_Activate(void);

/**
 * @brief      This API clears all manifest segments from the temporarly used RAM area.
 *
 * @param[out] pSwupStatus  The swup status
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_ClearRamManifest(SwupStatus_t *pSwupStatus);

/**
 * @brief      This API provides Product ID, Hardware ID, Typecheck ID, Rom ID and the SWUP
               version of the device.
 *
 * @param[out] swupDeviceInfo  The swup device information
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_GetDeviceInfo(SwupDeviceInfo_t *swupDeviceInfo);

/**
 * @brief      This API provides wafer details (ID, Number, X coordinate, Y coordinate) and
               Serial number of the device.
 *
 * @param[out] swupDeviceId  The swup device identifier
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_ReadDeviceId(SwupDeviceId_t *swupDeviceId);

/**
 * @brief      This API is use to temporarily store a manifest in RAM
 *
 * @param[out] pSwupStatus    The swup status
 * @param[in]  manifestIndex  The manifest index
 * @param[in]  manifestChunk  The manifest chunk
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_TransferManifest(
    SwupStatus_t *pSwupStatus, const uint8_t manifestIndex, const uint8_t manifestChunk[128]);

/**
 * @brief      This API is use for Verification of completeness and authenticity (signature) of the manifest
 *
 * @param      pSwupStatus  The swup status
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_StartUpdate(SwupStatus_t *pSwupStatus);

/**
 * @brief      This API is use to flash the manifest which is defined in valid flash area
 *
 * @param[out] pSwupStatus     The swup status
 * @param[in]  componentIndex  The component index
 * @param[in]  segmentNumber   The segment number
 * @param[in]  cmdData         The command data
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_TransferComponent(
    SwupStatus_t *pSwupStatus, const uint8_t componentIndex, const uint16_t segmentNumber, const uint8_t cmdData[128]);

/**
 * @brief      This API verifies the selected component described in the manifest
 *
 * @param[out] pSwupStatus     The swup status
 * @param[in]  componentIndex  The component index
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_VerifyComponent(SwupStatus_t *pSwupStatus, const uint8_t componentIndex);

/**
 * @brief      This API is use to verify all component in the manifest
 *
 * @param[out]      pSwupStatus  The swup status
 *
 * @return     The swup response status.
 *             STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_VerifyAll(SwupStatus_t *pSwupStatus);

/**
 * @brief      This API checks whether all components in manifest
                have been verified either by the Swup_VerifyComponent or
                the Swup_VerifyAll command
 *
 * @param[out] pSwupStatus  The swup status
 *
 * @return     The swup response status.
               STATUS_CMD_SUCCESS in case of success or
 *             SwupResponseStatus_t in case of failure
 */
SwupResponseStatus_t Swup_FinishUpdate(SwupStatus_t *pSwupStatus);

/**
 * @}
 */
#endif /*_SWUP_DEFS_H_*/
