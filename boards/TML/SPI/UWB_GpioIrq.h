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

#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <stdint.h>

#if defined(QN9090DK6) || defined(CPU_S32K144)
#include "UWBT_BuildConfig.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Shared file acrros vaious TML/BUS/Bridge implementation.
 *
 * Eventually */

#if !defined(QN9090DK6) && !defined(CPU_S32K144)

typedef enum
{
    LED_R           = 0,
    LED_G           = 1,
    LED_B           = 2,
    SW1             = 3,
    SW3             = 4,
    HELIOS_IRQ      = 5,
    HELIOS_ENABLE   = 6,
    HELIOS_SYNC     = 7,
    NFC_IRQ         = 8,
    DIG_ENABLE_1V8  = 9,
    UWB_ENABLE_1V8  = 10,
    SD_ENABLE_3V3   = 11,
    HELIOS_RTC_SYNC = 12,

} UWB_GpioComponent_t;

#endif

void UWB_GpioInit(void);
void UWB_GpioDeinit(void);
bool UWB_GpioSet(UWB_GpioComponent_t cp, bool on);
bool UWB_GpioRead(UWB_GpioComponent_t cp);
bool UWB_GpioWaitOnce(UWB_GpioComponent_t cp, void (*cb)(void *args), void *args);
bool UWB_GpioIrqEnable(UWB_GpioComponent_t cp, void (*cb)(void *args), void *args);
bool UWB_GpioIrqDisable(UWB_GpioComponent_t cp);

#ifdef __cplusplus
}
#endif

#endif
