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

#include "UWB_ConfigHandler.h"
#include "UWBT_Config.h"
#include "gap_types.h"
#include "UWBT_BuildConfig.h"

/* Flash framework */
#include "Flash_Adapter.h"
#include "QN9090.h"

/* Log */
#include "fsl_debug_console.h"

#if TAG_BUILD_CFG == VALIDATION_V3_BUILD
extern bool pingBle;
#endif

// BLE_NAME
#define BLE_NAME_SIZE 24
typedef struct
{
    UWBT_ConfigId_t configId;
    bool setEnabled;
    bool getEnabled;
    uint8_t expLenLowerBound; // Lower bound for the configuration length
    uint8_t expLenUpperBound; // Upper bound for the configuration length
} UWBT_Config_t;

static UWBT_Config_t configTable[] = {
    {BLE_NAME, true, true, 1, BLE_NAME_SIZE},
    {BLE_INTERVAL, true, true, 2, 2},
};

UWBT_Config_t *findConfigById(uint32_t configId)
{
    for (int i = 0; i < sizeof(configTable) / sizeof(configTable[0]); i++) {
        if (configId == configTable[i].configId) {
            return &configTable[i];
        }
    }
    return NULL;
}

static bool checkLength(UWBT_Config_t *cfg, uint32_t actualLen)
{
    assert(cfg);

    uint8_t lenLB = cfg->expLenLowerBound;
    uint8_t lenUB = cfg->expLenUpperBound;

    if (lenLB <= actualLen && actualLen <= lenUB) {
        return true;
    }
    else {
        return false;
    }
}

static bool handleWriteCfgRequest(UWBT_ConfigRequest_t *request)
{
    bool ok = false;
    UWBT_Config_t *cfg;

    if ((cfg = findConfigById(request->configId)) == NULL) {
        PRINTF("%s(): ERROR, wrong config Id (%02x) \n", __FUNCTION__, request->configId);
        goto end;
    }

    if (!checkLength(cfg, request->configLen)) {
        PRINTF("%s(): ERROR, wrong cfg request length (%d)\n", request->configLen);
        goto end;
    }

    if (!cfg->setEnabled) {
        PRINTF("Permission denied for cfg id %02x\n", request->configId);
        goto end;
    }

    switch (cfg->configId) {
    case BLE_NAME: {
        ok = UWBT_CfgWriteBleName(request->configValue, request->configLen);
    } break;

    case BLE_INTERVAL: {
        uint16_t interval = request->configValue[0] << 8 | request->configValue[1];
        ok                = UWBT_CfgWriteBleInterval(interval);
    } break;
    }

end:
    return ok;
}

static void prepareReadRsp(UWBT_Config_t *cfg)
{
    switch (cfg->configId) {
    case BLE_NAME: {
        const char *name = UWBT_CfgReadBleName();
        tlvAdd_PTR((uint8_t *)name, strlen(name));
    } break;

    case BLE_INTERVAL: {
        tlvAdd_UINT32(UWBT_CfgReadBleInterval());
    } break;
    }
}

void handleConfigCmd(tlv_t *tlv, bool *error)
{
    bool justRspStatus = true;
    uint8_t subtype    = tlv->value[0];
    bool respStatus    = true;
    *error             = false;

#if TAG_BUILD_CFG == VALIDATION_V3_BUILD
    pingBle = false;
#endif

    switch (subtype) {
    case SET_CONFIG: {
        if (tlv->size >= 5) {
            UWBT_ConfigRequest_t req = {
                .configId    = (tlv->value[1] << 24) | (tlv->value[2] << 16) | (tlv->value[3] << 8) | tlv->value[4],
                .configValue = &tlv->value[5],
                .configLen   = tlv->size - 5,
            };

            respStatus = handleWriteCfgRequest(&req);
        }
        else {
            respStatus = false;
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 5);
        }

        break;
    }

    case GET_CONFIG: {
        if (tlv->size == 5) {
            UWBT_ConfigRequest_t req = {
                .configId = (tlv->value[1] << 24) | (tlv->value[2] << 16) | (tlv->value[3] << 8) | tlv->value[4],
            };

            UWBT_Config_t *cfg = findConfigById(req.configId);

            if (cfg != NULL && cfg->getEnabled) {
                respStatus    = true;
                justRspStatus = false;

                tlvAdd_UINT8(respStatus);
                prepareReadRsp(cfg);
            }
            else {
                PRINTF("%s(): ERROR, invalid config ID %02x or read not enabled)\n", __FUNCTION__, req.configId);
                respStatus = false;
            }
        }
        else {
            respStatus = false;
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 5);
        }
        break;
    }

    /*TODO: Temporarily, to be removed*/
    case DEVICE_PING: {
        if (tlv->size == 1) {
            tlvAdd_UINT8(tlv->value[0]); // Add subtype
            tlvAdd_UINT8(CHIP_VERSION);
            tlvAdd_UINT8(MW_VERSION);

            justRspStatus = false;
#if TAG_BUILD_CFG == VALIDATION_V3_BUILD && BOARD_VERSION == TAG_V3
            pingBle = true;
#endif
        }
        else {
            respStatus = false;
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 1);
        }
        break;
    }

    default:
        respStatus = false;
        break;
    }
    // Prepare response tlv
    if (justRspStatus) {
        tlvAdd_UINT8(respStatus);
    }
    *error = (respStatus != true);
}
