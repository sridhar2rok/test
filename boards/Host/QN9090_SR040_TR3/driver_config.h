/*
 * Copyright (c) 2013-2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019 NXP
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
#ifndef __GPIO_PINS_H__
#define __GPIO_PINS_H__

#include "UWBT_BuildConfig.h"
#include "GPIO_Adapter.h"
// #include "board.h"

/*! @file */
/*!*/
/*! This file contains gpio pin definitions used by gpio peripheral driver.*/
/*! The enums in _gpio_pins map to the real gpio pin numbers defined in*/
/*! gpioPinLookupTable. And this might be different in different board.*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* As per datasheet, taking to maximum 8Mhz of QN9090 */
#define UWB_SPI_BAUDRATE (8 * 1000 * 1000U) // 8 MHz

//#define HBCI_HEADER_SIZE              4
#define UWB_SPI_BASEADDR SPI1
#define UWB_SPI_SSEL     kSPI_Ssel0
#define ACCEL_SPI_SSEL   kSPI_Ssel1

#define UWB_HELIOS_CLK_GPIO GPIO
#define UWB_HELIOS_CLK_PORT 0
#define UWB_HELIOS_CLK_PIN  0
#define UWB_HELIOS_CLK_FUN  IOCON_PIO_FUNC5

#define UWB_HELIOS_MISO_GPIO GPIO
#define UWB_HELIOS_MISO_PORT 0
#define UWB_HELIOS_MISO_PIN  1
#define UWB_HELIOS_MISO_FUN  IOCON_PIO_FUNC5

#define UWB_HELIOS_MOSI_GPIO GPIO
#define UWB_HELIOS_MOSI_PORT 0
#define UWB_HELIOS_MOSI_PIN  2
#define UWB_HELIOS_MOSI_FUN  IOCON_PIO_FUNC5

#define UWB_HELIOS_CS_GPIO GPIO
#define UWB_HELIOS_CS_PORT 0
#define UWB_HELIOS_CS_PIN  3
#define UWB_HELIOS_CS_FUN  IOCON_PIO_FUNC0

#define UWB_ACCEL_CS_GPIO GPIO
#define UWB_ACCEL_CS_PORT 0
#define UWB_ACCEL_CS_PIN  4
#define UWB_ACCEL_CS_FUN  IOCON_PIO_FUNC5

#define UWB_PA_ENABLE_GPIO GPIO
#define UWB_PA_ENABLE_PORT 0
#define UWB_PA_ENABLE_PIN  5
#define UWB_PA_ENABLE_FUN  IOCON_PIO_FUNC0

#define UWB_UART_TX_GPIO GPIO
#define UWB_UART_TX_PORT 0
#define UWB_UART_TX_PIN  8
#define UWB_UART_TX_FUN  IOCON_PIO_FUNC2

#define UWB_UART_RX_GPIO GPIO
#define UWB_UART_RX_PORT 0
#define UWB_UART_RX_PIN  9
#define UWB_UART_RX_FUN  IOCON_PIO_FUNC2

#define UWB_RDY_N_GPIO  GPIO
#define UWB_RDY_N_PORT  0
#define UWB_RDY_N_PIN   14
#define UWB_RDY_N_CONN  kINPUTMUX_GpioPort0Pin14ToPintsel
#define UWB_RDY_N_INDEX 0
#define UWB_RDY_N_FUN   IOCON_PIO_FUNC0

#define UWB_HELIOS_IRQ_GPIO  GPIO
#define UWB_HELIOS_IRQ_PORT  0
#define UWB_HELIOS_IRQ_PIN   15
#define UWB_HELIOS_IRQ_CONN  kINPUTMUX_GpioPort0Pin15ToPintsel
#define UWB_HELIOS_IRQ_INDEX 0
#define UWB_HELIOS_IRQ_FUN   IOCON_PIO_FUNC0

#define UWB_RST_N_GPIO GPIO
#define UWB_RST_N_PORT 0
#define UWB_RST_N_PIN  16
#define UWB_RST_N_FUN  IOCON_PIO_FUNC0

#define BUZZER_PORT            0
#define BUZZER_PIN             17
#define BUZZER_FUN             IOCON_PIO_FUNC4
#define BUZZER_PWM             PWM
#define BUZZER_PWM_CH          kPWM_Pwm6
#define BUZZER_PWM_IRQ_NAME    PWM6_IRQn
#define BUZZER_PWM_IRQ_HANDLER PWM6_IRQHandler

#define UWB_VIB_ALERT_GPIO GPIO
#define UWB_VIB_ALERT_PORT 0
#define UWB_VIB_ALERT_PIN  18
#define UWB_VIB_ALERT_FUN  IOCON_PIO_FUNC0

#define BOARD_LED_BLUE_GPIO      GPIO
#define BOARD_LED_BLUE_GPIO_PORT 0U
#define BOARD_LED_BLUE_GPIO_PIN  19U
#define IOCON_LED_BLUE_PIN       BOARD_LED_BLUE_GPIO_PIN

#define BOARD_LED_GREEN_GPIO      GPIO
#define BOARD_LED_GREEN_GPIO_PORT 0U
#define BOARD_LED_GREEN_GPIO_PIN  20U
#define IOCON_LED_GREEN_PIN       BOARD_LED_GREEN_GPIO_PIN

#define BOARD_LED_RED_GPIO      GPIO
#define BOARD_LED_RED_GPIO_PORT 0U
#define BOARD_LED_RED_GPIO_PIN  21U
#define IOCON_LED_RED_PIN       BOARD_LED_RED_GPIO_PIN

#define SWO_GPIO GPIO
#define SWO_PORT 0
#define SWO_PIN  21
#define SWO_FUN  IOCON_PIO_FUNC6

/* We have 2 switch push-buttons on DK6 board */
#define BOARD_USER_BUTTON1_GPIO      GPIO
#define BOARD_USER_BUTTON1_GPIO_PORT 0U
#define BOARD_USER_BUTTON1_GPIO_PIN  1U
#define IOCON_USER_BUTTON1_PIN       BOARD_USER_BUTTON1_GPIO_PIN

#define BOARD_USER_BUTTON2_GPIO      GPIO
#define BOARD_USER_BUTTON2_GPIO_PORT 0U
#define BOARD_USER_BUTTON2_GPIO_PIN  5U /* shared with ISP entry */
#define IOCON_USER_BUTTON2_PIN       BOARD_USER_BUTTON2_GPIO_PIN

#define IOCON_LED_MODE_FUNC         (0U)
#define IOCON_USER_BUTTON_MODE_FUNC (0U)

extern const gpioInputPinConfig_t dk6_button_io_pins[];
extern const gpioOutputPinConfig_t dk6_leds_io_pins[];

#define ledPins    dk6_leds_io_pins
#define switchPins dk6_button_io_pins

#endif /* __GPIO_PINS_H__ */
