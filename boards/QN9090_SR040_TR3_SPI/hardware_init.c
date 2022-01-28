/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2019 NXP.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*${header:start}*/
#include "pin_mux.h"
#include "board.h"
#include <stdbool.h>
#include "fsl_power.h"
#include "fsl_wtimer.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitClocks(void)
{
    /*!< Set up the clock sources */
    CLOCK_EnableClock(kCLOCK_Fro32M);     /*!< Ensure FRO 32MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro48M);     /*!< Ensure FRO 48MHz is on */
    CLOCK_EnableClock(kCLOCK_Fro12M);     /*!< Ensure FRO 12MHz is on */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK); /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    CLOCK_EnableAPBBridge();           /* The Async_APB clock is enabled. */
    CLOCK_EnableClock(kCLOCK_Xtal32M); /*!< Enable XTAL 32 MHz output */
    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);     /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 1U, false); /*!< Set SYSTICKCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivTraceClk, 1U, false);   /*!< Set TRACECLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kXTAL32M_to_MAIN_CLK);   /*!< Switch MAIN_CLK to XTAL32M */
    CLOCK_AttachClk(kMAIN_CLK_to_ASYNC_APB); /*!< Switch ASYNC_APB to MAIN_CLK */
    CLOCK_AttachClk(kXTAL32M_to_OSC32M_CLK); /*!< Switch OSC32M_CLK to XTAL32M */
    CLOCK_AttachClk(kOSC32M_to_PWM_CLK);     /*!< Switch PWM_CLK to OSC32M */
    CLOCK_AttachClk(kOSC32M_to_USART_CLK);
    /*!< Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}

void hardware_init(void)
{
    BOARD_common_hw_init();
}

/*!
 * @brief Configure the wake-up timer for run time stats.
 */
void RTOS_AppConfigureTimerForRuntimeStats(void)
{
    WTIMER_Init();
    WTIMER_StartTimer(WTIMER_TIMER0_ID, ~0UL);
}

/*!
 * @brief Get run counter from wake-up timer.
 */
uint32_t RTOS_AppGetRuntimeCounterValue(void)
{
    return (~0UL - WTIMER_ReadTimer(WTIMER_TIMER0_ID));
}

/*${function:end}*/
