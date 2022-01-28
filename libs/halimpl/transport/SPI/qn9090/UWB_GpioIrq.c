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
#include <stdbool.h>

#include "UWBT_BuildConfig.h"
#include "QN9090.h"

#include "UWB_GpioIrq.h"
#include "UWB_Assert.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"

#include "board.h"

#include "FreeRTOS.h"

#include "driver_config.h"

#define MAIN_INPUT_CB     0
#define ONE_SHOT_INPUT_CB 1
#define NUM_INPUT_CB      2

typedef struct
{
    void (*fn)(void *args);
    void *args;
} UWB_GpioCallback_t;

typedef struct
{
    pint_pin_enable_t intType;
    uint32_t priority;
    uint32_t interruptIndex;
    inputmux_connection_t interruptConn;
} UWB_GpioInputPinInfo_t;

typedef struct
{
    bool activeHigh;
} UWB_GpioOutputPinInfo_t;

typedef struct
{
    GPIO_Type *gpio;
    uint32_t port;
    uint32_t pin;
    bool isInput;

    union {
        UWB_GpioInputPinInfo_t inputInfo;
        UWB_GpioOutputPinInfo_t outputInfo;
    };

} UWB_GpioInfo_t;

static void UWB_GpioIrqGeneric(pint_pin_int_t pintr, uint32_t pmatch_status);
static UWB_GpioCallback_t mCallbacks[FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS];

static UWB_GpioInfo_t mComponents[] = {
    /* Outputs */

    /* Helios sensorint */
    [HELIOS_IRQ] = {UWB_HELIOS_IRQ_GPIO,
        UWB_HELIOS_IRQ_PORT,
        UWB_HELIOS_IRQ_PIN,
        true,
        .inputInfo = {kPINT_PinIntEnableLowLevel,
            configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
            UWB_HELIOS_IRQ_INDEX,
            UWB_HELIOS_IRQ_CONN}},

    [RST_N] = {UWB_RST_N_GPIO, UWB_RST_N_PORT, UWB_RST_N_PIN, false, .outputInfo = {true}},

    [RDY_N_IRQ] = {UWB_RDY_N_GPIO,
        UWB_RDY_N_PORT,
        UWB_RDY_N_PIN,
        true,
        .inputInfo = {kPINT_PinIntEnableLowLevel,
            configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,
            UWB_RDY_N_INDEX,
            UWB_RDY_N_CONN}},

};

static void UWB_EnableInterrupt(uint32_t index, inputmux_connection_t connection, pint_pin_enable_t type, uint32_t prio)
{
    INPUTMUX_AttachSignal(INPUTMUX, index, connection);
    PINT_PinInterruptConfig(PINT, index, type, UWB_GpioIrqGeneric);
    PINT_EnableCallbackByIndex(PINT, index, prio);
}

static void UWB_DisableInterrupt(uint32_t index, inputmux_connection_t connection)
{
    PINT_DisableCallbackByIndex(PINT, index);
}

void UWB_GpioDeinit(void)
{
    for (int i = 0; i < sizeof(mCallbacks) / sizeof(mCallbacks[0]); i++) {
        mCallbacks[i].fn = 0;
    }

    for (int i = 0; i < sizeof(mComponents) / sizeof(mComponents[0]); i++) {
        if (!mComponents[i].isInput && mComponents[i].inputInfo.interruptIndex &&
            mCallbacks[mComponents[i].inputInfo.interruptIndex].fn) {
            UWB_GpioIrqDisable(i);
        }
    }
}

void UWB_GpioInit(void)
{
    UWB_GpioInfo_t *info;
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t pinGpioCfg = {
        kGPIO_DigitalOutput,
        0,
    };

    memset(mCallbacks, 0, sizeof(UWB_GpioCallback_t) * FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS);

    for (int i = 0; i < ARRAY_SIZE(mComponents); i++) {
        info = &mComponents[i];

        pinGpioCfg.pinDirection = (info->isInput) ? kGPIO_DigitalInput : kGPIO_DigitalOutput;
        pinGpioCfg.outputLogic  = 1;

        GPIO_PinInit(info->gpio, info->port, info->pin, &pinGpioCfg);

        if (!mComponents[i].isInput) {
            GPIO_PinWrite(info->gpio, info->port, info->pin, true);
        }
    }
}

bool UWB_GpioIrqDisable(UWB_GpioComponent_t cp)
{
    UWB_GpioInfo_t *info = &mComponents[cp];
    bool ok              = true;

    ASSERT(cp < ARRAY_SIZE(mComponents));
    ASSERT(mComponents[cp].isInput);
    ASSERT(mComponents[cp].inputInfo.interruptIndex < FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS);
    // ASSERT(mCallbacks[mComponents[cp].inputInfo.interruptIndex].fn);

    if (!info->isInput) {
        ok = false;
        goto end;
    }

    /*
     * Note: __disable_irq()/_enable_irq()+__ISB() apparently cause no
     * trouble, but don't match FreeRTOS's portDISABLE_INTERRUPTS/portENABLE_INTERRUPTS
     * definition. Consider using FreeRTOS's macros in case of trouble
     */
    __disable_irq();
    UWB_DisableInterrupt(info->port, info->pin);
    mCallbacks[mComponents[cp].inputInfo.interruptIndex].fn = 0;

    __enable_irq();
    __ISB();

end:
    return ok;
}

bool UWB_GpioIrqEnable(UWB_GpioComponent_t cp, void (*cb)(void *args), void *args)
{
    UWB_GpioInfo_t *info = &mComponents[cp];
    bool ok              = true;

    ASSERT(cp < ARRAY_SIZE(mComponents));
    ASSERT(mComponents[cp].isInput);
    ASSERT(mComponents[cp].inputInfo.interruptIndex < FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS);
    // ASSERT(!mCallbacks[mComponents[cp].inputInfo.interruptIndex].fn);

    if (!info->isInput) {
        ok = false;
        goto end;
    }

    /*
     * Note: __disable_irq()/_enable_irq()+__ISB() apparently cause no
     * trouble, but don't match FreeRTOS's portDISABLE_INTERRUPTS/portENABLE_INTERRUPTS
     * definition. Consider using FreeRTOS's macros in case of trouble
     */
    __disable_irq();

    mCallbacks[info->inputInfo.interruptIndex].fn   = cb;
    mCallbacks[info->inputInfo.interruptIndex].args = args;

    UWB_EnableInterrupt(info->inputInfo.interruptIndex,
        info->inputInfo.interruptConn,
        info->inputInfo.intType,
        info->inputInfo.priority);

    __enable_irq();
    __ISB();

end:
    return ok;
}

bool UWB_GpioRead(UWB_GpioComponent_t cp)
{
    ASSERT(cp < ARRAY_SIZE(mComponents));
    return GPIO_PinRead(mComponents[cp].gpio, mComponents[cp].port, mComponents[cp].pin);
}

bool UWB_GpioSet(UWB_GpioComponent_t cp, bool on)
{
    ASSERT(cp < ARRAY_SIZE(mComponents));

    if (mComponents[cp].isInput) {
        return false;
    }

    GPIO_PinWrite(
        mComponents[cp].gpio, mComponents[cp].port, mComponents[cp].pin, mComponents[cp].outputInfo.activeHigh == on);

    return true;
}

static void UWB_GpioIrqGeneric(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    if (mCallbacks[pintr].fn) {
        mCallbacks[pintr].fn(mCallbacks[pintr].args);
    }
}
