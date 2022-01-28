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

#include "UWB_GpioIrq.h"
#include "UWB_Assert.h"

#include "pins_driver.h"
#include "pin_config.h"
#include "pin_control.h"
#include "interrupt_manager.h"

#include "FreeRTOS.h"

#include "driver_config.h"
// #include "UWB_Assert.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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
    port_interrupt_config_t intType;
    uint32_t priority;

    // Maybe the array of callbacks can be allocated dynamically?
    UWB_GpioCallback_t cb;
} UWB_GpioInputPinInfo_t;

typedef struct
{
    bool activeHigh;
} UWB_GpioOutputPinInfo_t;

typedef struct
{
    PORT_Type *port;
    GPIO_Type *gpio;
    uint8_t pin;
    bool isInput;
    union {
        UWB_GpioInputPinInfo_t inputInfo;
        UWB_GpioOutputPinInfo_t outputInfo;
    };

} UWB_GpioInfo_t;

static UWB_GpioInfo_t mComponents[] = {

    /* Inputs */

    [HELIOS_IRQ] = {UWB_HELIOS_IRQ_PORT,
        UWB_HELIOS_IRQ_GPIO,
        UWB_HELIOS_IRQ_PIN,
        true,
        .inputInfo = {PORT_INT_LOGIC_ZERO, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},

    [RDY_N_IRQ] = {UWB_RDY_N_PORT,
        UWB_RDY_N_GPIO,
        UWB_RDY_N_PIN,
        true,
        .inputInfo = {PORT_INT_LOGIC_ZERO, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},
};

void UWB_GpioDeinit(void)
{
}

static void UWB_DisableInterrupt(PORT_Type *port, uint8_t pin)
{
    PINS_DRV_SetPinIntSel(port, pin, PORT_DMA_INT_DISABLED);
    if (port == UWB_HELIOS_IRQ_PORT) {
        INT_SYS_DisableIRQ(UWB_HELIOS_IRQ_NAME);
    }
    else if (port == UWB_RDY_N_PORT) {
        INT_SYS_DisableIRQ(UWB_RDY_N_IRQ_NAME);
    }
    else {
        /*Do nothing */
    }
}

static void UWB_EnableInterrupt(PORT_Type *port, uint8_t pin, port_interrupt_config_t type, uint32_t prio)
{
    /*Set INT_N pin as IRQ*/
    PINS_DRV_SetPinIntSel(port, pin, type);
    if (port == UWB_HELIOS_IRQ_PORT) {
        INT_SYS_SetPriority(UWB_HELIOS_IRQ_NAME, prio);
        INT_SYS_EnableIRQ(UWB_HELIOS_IRQ_NAME);
    }
    else if (port == UWB_RDY_N_PORT) {
        INT_SYS_SetPriority(UWB_RDY_N_IRQ_NAME, prio);
        INT_SYS_EnableIRQ(UWB_RDY_N_IRQ_NAME);
    }
    else {
        /*Do nothing */
    }
}

void UWB_GpioInit(void)
{
    UWB_GpioInfo_t *info;

    for (int i = 0; i < ARRAY_SIZE(mComponents); i++) {
        info = &mComponents[i];
        if (mComponents[i].isInput) {
            //UWB_EnableInterrupt(info->port, info->pin, info->inputInfo.intType, info->inputInfo.priority);
        }
        else {
            PINS_DRV_WritePin(info->gpio, info->pin, 1u);
        }
    }
}

bool UWB_GpioIrqDisable(UWB_GpioComponent_t cp)
{
    UWB_GpioInfo_t *info = &mComponents[cp];
    bool ok              = true;

    ASSERT(cp < ARRAY_SIZE(mComponents));

    if (!info->isInput) {
        ok = false;
        goto end;
    }

    /*
     * Note: __disable_irq()/_enable_irq()+__ISB() apparently cause no
     * trouble, but don't match FreeRTOS's portDISABLE_INTERRUPTS/portENABLE_INTERRUPTS
     * definition. Consider using FreeRTOS's macros in case of trouble
     */
    DISABLE_INTERRUPTS()

    UWB_DisableInterrupt(info->port, info->pin);

    ENABLE_INTERRUPTS()
    //__ISB();

end:
    return ok;
}

bool UWB_GpioIrqEnable(UWB_GpioComponent_t cp, void (*cb)(void *args), void *args)
{
    UWB_GpioInfo_t *info = &mComponents[cp];
    bool ok              = true;

    ASSERT(cp < ARRAY_SIZE(mComponents));

    if (!info->isInput) {
        ok = false;
        goto end;
    }

    /*
     * Note: __disable_irq()/_enable_irq()+__ISB() apparently cause no
     * trouble, but don't match FreeRTOS's portDISABLE_INTERRUPTS/portENABLE_INTERRUPTS
     * definition. Consider using FreeRTOS's macros in case of trouble
     */
    DISABLE_INTERRUPTS()
    if (info->gpio == UWB_HELIOS_IRQ_GPIO && info->pin == UWB_HELIOS_IRQ_PIN) {
        INT_SYS_InstallHandler(UWB_HELIOS_IRQ_NAME, (isr_t)cb, (isr_t *)0u);
    }
    else if (info->gpio == UWB_RDY_N_GPIO && info->pin == UWB_RDY_N_PIN) {
        INT_SYS_InstallHandler(UWB_RDY_N_IRQ_NAME, (isr_t)cb, (isr_t *)0u);
    }

    UWB_EnableInterrupt(info->port, info->pin, info->inputInfo.intType, info->inputInfo.priority);

    ENABLE_INTERRUPTS()
    //__ISB();

end:
    return ok;
}

bool UWB_GpioRead(UWB_GpioComponent_t cp)
{
    pin_level_t retVal = PIN_LEVEL_HIGH;
    pins_channel_type_t portVal;
    switch (cp) {
    case RST_N:
        portVal = PINS_DRV_ReadPins(phscaS32DriverUCIHw_Pin_RSTn.GpioBase);
        retVal = (0u == (portVal & (pins_channel_type_t)1u << (phscaS32DriverUCIHw_Pin_RSTn.PinsIdx))) ? PIN_LEVEL_LOW :
                                                                                                         PIN_LEVEL_HIGH;
        break;
    case RDY_N_IRQ:
        portVal = PINS_DRV_ReadPins(phscaS32DriverUCIHw_Pin_RDYn.GpioBase);
        retVal = (0u == (portVal & (pins_channel_type_t)1u << (phscaS32DriverUCIHw_Pin_RDYn.PinsIdx))) ? PIN_LEVEL_LOW :
                                                                                                         PIN_LEVEL_HIGH;
        break;
    case HELIOS_CS:
        portVal = PINS_DRV_ReadPins(phscaS32DriverUCIHw_Pin_CSn.GpioBase);
        retVal  = (0u == (portVal & (pins_channel_type_t)1u << (phscaS32DriverUCIHw_Pin_CSn.PinsIdx))) ? PIN_LEVEL_LOW :
                                                                                                        PIN_LEVEL_HIGH;
        break;
    case HELIOS_IRQ:
        portVal = PINS_DRV_ReadPins(phscaS32DriverUCIHw_Pin_IRQn.GpioBase);
        retVal = (0u == (portVal & (pins_channel_type_t)1u << (phscaS32DriverUCIHw_Pin_IRQn.PinsIdx))) ? PIN_LEVEL_LOW :
                                                                                                         PIN_LEVEL_HIGH;
        break;
    default:
        break;
    }
    return retVal;
}

bool UWB_GpioSet(UWB_GpioComponent_t cp, bool on)
{
    phscaS32DriverUCIHw_Portpin_t gpioPortpin = {0};

    switch (cp) {
    case RST_N:
        gpioPortpin = phscaS32DriverUCIHw_Pin_RSTn;
        break;
    case RDY_N_IRQ:
        gpioPortpin = phscaS32DriverUCIHw_Pin_RSTn;
        break;
    case HELIOS_CS:
        gpioPortpin = phscaS32DriverUCIHw_Pin_RSTn;
        break;
    case HELIOS_IRQ:
        gpioPortpin = phscaS32DriverUCIHw_Pin_RSTn;
        break;
    default:
        break;
    }

    if (PIN_LEVEL_HIGH == on) {
        PINS_DRV_SetPins(gpioPortpin.GpioBase, (pins_channel_type_t)1u << (gpioPortpin.PinsIdx));
    }
    else {
        PINS_DRV_ClearPins(gpioPortpin.GpioBase, (pins_channel_type_t)1u << (gpioPortpin.PinsIdx));
    }
    return true;
}
