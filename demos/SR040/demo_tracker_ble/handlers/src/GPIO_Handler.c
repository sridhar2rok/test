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

/* GPIO framework */
#include "driver_config.h"
#include "fsl_gpio.h"
#include "GPIO.h"
/* Drivers */
#include "fsl_debug_console.h"
/* TLV */
#include "TLV_Defs.h"
#include "TLV_Builder.h"
//#include "UWBT_Buzzer.h"

static bool setGPIOState(uint32_t mask, uint32_t maskValues)
{
    for (uint8_t maskIndex = 0; maskIndex < 22; maskIndex++) { // Byte pin register [0..21]
        uint32_t currentMask = 1 << maskIndex;
        if (currentMask & mask) {
            uint8_t gpioState = (currentMask & maskValues) >> maskIndex;
            GPIO_Set(maskIndex, gpioState);
        }
    }

    return true;
}

static bool getGPIOState(uint32_t mask, uint32_t *responseMask)
{
    *responseMask = 0x0;
    bool error    = false;

    for (uint8_t maskIndex = 0; maskIndex < 22; maskIndex++) { // Byte pin register [0..21]
        uint32_t currentMask = 1 << maskIndex;
        if (currentMask & mask) {
            uint32_t readMask = GPIO_Get(maskIndex) << maskIndex;
            *responseMask     = *responseMask | readMask;
        }
    }

    return !error;
}

static bool setGPIOPeriod(uint32_t mask, uint32_t period)
{
    for (uint8_t maskIndex = 0; maskIndex < 22; maskIndex++) { // Byte pin register [0..21]
        uint32_t currentMask = 1 << maskIndex;
        if (currentMask & mask) {
            GPIO_StartFlashWithPeriod(maskIndex, period);
        }
    }

    return true;
}

void handleUiCmd(tlv_t *tlv, bool *error)
{
    bool justRspStatus = true;
    uint8_t subtype    = tlv->value[0];
    uint8_t respStatus = true;
    *error             = false;

    //PRINTF("%s(): handling subtype  %02x\n", __FUNCTION__, subtype);

    switch (subtype) {
    case SET_GPIO_STATE:
        if (tlv->size == 9) {
            uint32_t gpioMask   = (tlv->value[1] << 24) | (tlv->value[2] << 16) | (tlv->value[3] << 8) | tlv->value[4];
            uint32_t gpioValues = (tlv->value[5] << 24) | (tlv->value[6] << 16) | (tlv->value[7] << 8) | tlv->value[8];
            respStatus          = setGPIOState(gpioMask, gpioValues);
        }
        else {
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 8);
            respStatus = false;
        }
        break;

    case GET_GPIO_STATE:
        if (tlv->size == 5) {
            uint32_t gpioMask = (tlv->value[1] << 24) | (tlv->value[2] << 16) | (tlv->value[3] << 8) | tlv->value[4];
            uint32_t rspMask;
            respStatus = getGPIOState(gpioMask, &rspMask);

            if (respStatus) {
                tlvAdd_UINT8(respStatus);
                tlvAdd_UINT32(rspMask);
                justRspStatus = false;
            }
        }
        else {
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 5);
            respStatus = false;
        }
        break;
#if 0
        case UI_BUZZER_TONE: {
            uint16_t freq;
            uint16_t ms;

            if (tlv->size == 5) {
                freq = (tlv->value[1] << 8) | (tlv->value[2]);
                ms = (tlv->value[3] << 8) | (tlv->value[4]);
                respStatus = UWBT_BuzzerPlayTone(freq, ms) ? false : true;
            } else {
                respStatus = false;
            }

        } break;

        case UI_BUZZER_MELODY: {
            if (tlv->size == 2) {
                respStatus = UWBT_BuzzerPlayMelody(tlv->value[1]) ? false : true;
            } else {
                respStatus = false;
            }

        } break;
#endif
    case SET_GPIO_PERIOD: {
        if (tlv->size == 9) {
            uint32_t gpioMask = (tlv->value[1] << 24) | (tlv->value[2] << 16) | (tlv->value[3] << 8) | tlv->value[4];
            uint32_t period   = (tlv->value[5] << 24) | (tlv->value[6] << 16) | (tlv->value[7] << 8) | tlv->value[8];
            respStatus        = setGPIOPeriod(gpioMask, period);
        }
        else {
            respStatus = false;
            PRINTF("%s(): ERROR, value length (%d) does not match minimum size (%d)\n", __FUNCTION__, tlv->size, 9);
        }
    } break;

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
