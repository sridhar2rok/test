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
#include "UWB_GpioIrq.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#include "board.h"

#include "FreeRTOS.h"

#include "driver_config.h"
#include "UWB_Assert.h"

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
    port_interrupt_t intType;
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
    /* Outputs */
    [LED_R] = {LEDR_PORT, LEDR_GPIO, LEDR_PIN, false, .outputInfo = {false}},
    [LED_G] = {LEDG_PORT, LEDG_GPIO, LEDG_PIN, false, .outputInfo = {false}},
    [LED_B] = {LEDB_PORT, LEDB_GPIO, LEDB_PIN, false, .outputInfo = {false}},

    [DIG_ENABLE_1V8] = {DIG_ENABLE_1V8_PORT, DIG_ENABLE_1V8_GPIO, DIG_ENABLE_1V8_PIN, false, .outputInfo = {true}},
    [UWB_ENABLE_1V8] = {UWB_ENABLE_1V8_PORT, UWB_ENABLE_1V8_GPIO, UWB_ENABLE_1V8_PIN, false, .outputInfo = {true}},
    [SD_ENABLE_3V3]  = {SD_ENABLE_3V3_PORT, SD_ENABLE_3V3_GPIO, SD_ENABLE_3V3_PIN, false, .outputInfo = {true}},

    [HELIOS_ENABLE]   = {UWB_HELIOS_CE_PORT, UWB_HELIOS_CE_GPIO, UWB_HELIOS_CE_PIN, false, .outputInfo = {true}},
    [HELIOS_SYNC]     = {UWB_HELIOS_SYNC_PORT, UWB_HELIOS_SYNC_GPIO, UWB_HELIOS_SYNC_PIN, false, .outputInfo = {true}},
    [HELIOS_RTC_SYNC] = {UWB_HELIOS_RTC_PORT, UWB_HELIOS_RTC_GPIO, UWB_HELIOS_RTC_PIN, false, .outputInfo = {true}},
    /* Inputs */
    [SW1]        = {SW1_PORT,
        SW1_GPIO,
        SW1_PIN,
        true,
        .inputInfo = {kPORT_InterruptOrDMADisabled, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},
    [SW3]        = {SW3_PORT,
        SW3_GPIO,
        SW3_PIN,
        true,
        .inputInfo = {kPORT_InterruptOrDMADisabled, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},
    [HELIOS_IRQ] = {UWB_HELIOS_IRQ_PORT,
        UWB_HELIOS_IRQ_GPIO,
        UWB_HELIOS_IRQ_PIN,
        true,
        .inputInfo = {kPORT_InterruptLogicOne, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},
    [NFC_IRQ]    = {NFC_PORT_IRQ,
        NFC_GPIO_IRQ,
        NFC_PIN_IRQ,
        true,
        .inputInfo = {kPORT_InterruptLogicOne, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY}},
};

static void UWB_DisableInterrupt(PORT_Type *port, uint8_t pin)
{
    PORT_SetPinInterruptConfig(port, pin, kPORT_InterruptOrDMADisabled);

    /* Look up all pins in the port and see if any has any interrupt associated & enabled */
    bool found = false;
    for (int i = 0; i < 32 && !found; i++) {
        if (port->PCR[i] & PORT_PCR_IRQC_MASK) {
            found = true;
        }
    }

    if (!found) {
        /* No other pins have any interrupts enabled, disable port interrupt */
        if (port == PORTA) {
            DisableIRQ(PORTA_IRQn);
        }
        else if (port == PORTB) {
            DisableIRQ(PORTB_IRQn);
        }
        else if (port == PORTC) {
            DisableIRQ(PORTC_IRQn);
        }
        else if (port == PORTD) {
            DisableIRQ(PORTD_IRQn);
        }
        else if (port == PORTE) {
            DisableIRQ(PORTE_IRQn);
        }
    }
}

static void UWB_EnableInterrupt(PORT_Type *port, uint8_t pin, port_interrupt_t type, uint32_t prio)
{
    PORT_SetPinInterruptConfig(port, pin, type);

    if (port == PORTA) {
        EnableIRQ(PORTA_IRQn);
        NVIC_SetPriority(PORTA_IRQn, prio);
    }
    else if (port == PORTB) {
        EnableIRQ(PORTB_IRQn);
        NVIC_SetPriority(PORTB_IRQn, prio);
    }
    else if (port == PORTC) {
        EnableIRQ(PORTC_IRQn);
        NVIC_SetPriority(PORTC_IRQn, prio);
    }
    else if (port == PORTD) {
        EnableIRQ(PORTD_IRQn);
        NVIC_SetPriority(PORTD_IRQn, prio);
    }
    else if (port == PORTE) {
        EnableIRQ(PORTE_IRQn);
        NVIC_SetPriority(PORTE_IRQn, prio);
    }
}

void UWB_GpioInit(void)
{
    UWB_GpioInfo_t *info;

    for (int i = 0; i < ARRAY_SIZE(mComponents); i++) {
        info = &mComponents[i];
        if (mComponents[i].isInput) {
            // UWB_EnableInterrupt(info->port, info->pin, info->inputInfo.intType, info->inputInfo.priority);
        }
        else {
            GPIO_WritePinOutput(info->gpio, info->pin, !info->outputInfo.activeHigh);
            info->gpio->PDDR |= (1U << info->pin);
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
    __disable_irq();
    UWB_DisableInterrupt(info->port, info->pin);
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

    info->inputInfo.cb.args = args;
    info->inputInfo.cb.fn   = cb;

    UWB_EnableInterrupt(info->port, info->pin, info->inputInfo.intType, info->inputInfo.priority);

    __enable_irq();
    __ISB();

end:
    return ok;
}

bool UWB_GpioRead(UWB_GpioComponent_t cp)
{
    ASSERT(cp < ARRAY_SIZE(mComponents));

    return GPIO_ReadPinInput(mComponents[cp].gpio, mComponents[cp].pin);
}

bool UWB_GpioSet(UWB_GpioComponent_t cp, bool on)
{
    ASSERT(cp < ARRAY_SIZE(mComponents));

    if (mComponents[cp].isInput) {
        return false;
    }

    GPIO_WritePinOutput(mComponents[cp].gpio, mComponents[cp].pin, mComponents[cp].outputInfo.activeHigh == on);
    return true;
}

void Generic_IRQHandler(GPIO_Type *gpio)
{
    bool done = false;
    for (int i = 0; !done && i < ARRAY_SIZE(mComponents); i++) {
        UWB_GpioInfo_t *info = &mComponents[i];

        if (!info->isInput) {
            continue;
        }

        if (gpio == info->gpio && GPIO_GetPinsInterruptFlags(gpio) & (1 << info->pin)) {
            if (info->inputInfo.cb.fn) {
                info->inputInfo.cb.fn(info->inputInfo.cb.args);
            }
            done = true;
            GPIO_ClearPinsInterruptFlags(gpio, 1 << info->pin);
        }
    }
}

void PORTA_IRQHandler(void)
{
    Generic_IRQHandler(GPIOA);
}

void PORTB_IRQHandler(void)
{
    Generic_IRQHandler(GPIOB);
}

void PORTC_IRQHandler(void)
{
    Generic_IRQHandler(GPIOC);
}

void PORTD_IRQHandler(void)
{
    Generic_IRQHandler(GPIOD);
}

void PORTE_IRQHandler(void)
{
    Generic_IRQHandler(GPIOE);
}
