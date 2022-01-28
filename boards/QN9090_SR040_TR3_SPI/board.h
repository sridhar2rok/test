/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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

#ifndef _BOARD_H_
#define _BOARD_H_

#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "psector_api.h"
#include "core_cm4.h"
#include "UWBT_BuildConfig.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#ifndef BOARD_NAME
#define BOARD_NAME "DK6"
#endif
/*! @brief The manufacturer name */
#define MANUFACTURER_NAME "NXP"
#ifndef gUartDebugConsole_d
#define gUartDebugConsole_d 1
#endif
#ifndef gUartAppConsole_d
#define gUartAppConsole_d 0
#endif

#if defined gLoggingActive_d && (gLoggingActive_d > 0)

#ifndef DBG_ADC
#define DBG_ADC 0
#endif

#ifndef DBG_INIT
#define DBG_INIT 0
#endif

#define ADC_DBG_LOG(fmt, ...)                                                      \
    if (DBG_ADC)                                                                   \
        do {                                                                       \
            DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); \
        } while (0);
#define INIT_DBG_LOG(fmt, ...)                                                     \
    if (DBG_INIT)                                                                  \
        do {                                                                       \
            DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__); \
        } while (0);

#else
#define ADC_DBG_LOG(...)
#define INIT_DBG_LOG(...)
#endif

/* Connectivity */
#ifndef APP_SERIAL_INTERFACE_TYPE
#define APP_SERIAL_INTERFACE_TYPE (gSerialMgrUsart_c)
#endif

#ifndef APP_SERIAL_INTERFACE_INSTANCE
#define APP_SERIAL_INTERFACE_INSTANCE 1
#endif

#ifndef DEBUG_SERIAL_INTERFACE_INSTANCE
#define DEBUG_SERIAL_INTERFACE_INSTANCE 0
#endif

#ifndef APP_SERIAL_INTERFACE_SPEED
#define APP_SERIAL_INTERFACE_SPEED (115200U)
#endif

#define IOCON_QSPI_MODE_FUNC (7U)

#define IOCON_USART0_TX_PIN      (8U)
#define IOCON_USART0_RX_PIN      (9U)
#define IOCON_DBG_UART_MODE_FUNC (2U)

#define IOCON_USART1_TX_PIN         (10U)
#define IOCON_USART1_RX_PIN         (11U)
#define IOCON_HOSTIF_UART_MODE_FUNC (2U)

#define IOCON_SWCLK_PIN     (12U)
#define IOCON_SWDIO_PIN     (13U)
#define IOCON_SWD_MODE_FUNC (2U) /* no choice for SWD */

#define IOCON_SPIFI_CS_PIN    (16U)
#define IOCON_SPIFI_CLK_PIN   (18U)
#define IOCON_SPIFI_IO0_PIN   (19U)
#define IOCON_SPIFI_IO1_PIN   (21U)
#define IOCON_SPIFI_IO2_PIN   (20U)
#define IOCON_SPIFI_IO3_PIN   (17U)
#define IOCON_SPIFI_MODE_FUNC (7U)

#define BOARD_SPIFI_CLK_RATE (8000000UL)

/* Select flash to use */
#if gOTA_externalFlash_d == 1
#define gEepromType_d gEepromDevice_MX25R8035F_c
#else
#define gEepromType_d gEepromDevice_InternalFlash_c
#endif

#define BOARD_USART_IRQ(x)         USART##x_IRQn
#define BOARD_USART_IRQ_HANDLER(x) USART##x_IRQHandler
#define BOARD_UART_BASEADDR(x)     (uint32_t) USART##x
#define BOARD_UART_RESET_PERIPH(x) kUSART##x_RST_SHIFT_RSTn
#define BOARD_UART_RX_PIN(x)       IOCON_USART##x_RX_PIN
#define BOARD_UART_TX_PIN(x)       IOCON_USART##x_TX_PIN

#if gUartAppConsole_d
#if APP_SERIAL_INTERFACE_INSTANCE == 1
#define BOARD_APP_UART_IRQ         BOARD_USART_IRQ(1)
#define BOARD_APP_UART_IRQ_HANDLER BOARD_USART_IRQ_HANDLER(1)
#define BOARD_APP_UART_BASEADDR    BOARD_UART_BASEADDR(1)
#define BOARD_APP_UART_RESET       kUSART1_RST_SHIFT_RSTn
#define BOARD_APP_UART_RX_PIN      IOCON_USART1_RX_PIN
#define BOARD_APP_UART_TX_PIN      IOCON_USART1_TX_PIN
#else
#define BOARD_APP_UART_IRQ         LPUART0_IRQn
#define BOARD_APP_UART_IRQ_HANDLER LPUART0_IRQHandler
#define BOARD_APP_UART_BASEADDR    BOARD_UART_BASEADDR(0)
#define BOARD_APP_UART_RESET       kUSART0_RST_SHIFT_RSTn
#define BOARD_APP_UART_RX_PIN      IOCON_USART0_RX_PIN
#define BOARD_APP_UART_TX_PIN      IOCON_USART0_TX_PIN
#endif
#define BOARD_APP_UART_CLK_ATTACH kOSC32M_to_USART_CLK
#define BOARD_APP_UART_CLK_FREQ   CLOCK_GetFreq(kCLOCK_Fro32M)
#endif

#define BOARD_BOOTCLOCKRUN_CORE_CLOCK 48000000U

#if gUartDebugConsole_d

#if DEBUG_SERIAL_INTERFACE_INSTANCE == 1
#define BOARD_DEBUG_UART_IRQ         BOARD_USART_IRQ(1)
#define BOARD_DEBUG_UART_IRQ_HANDLER BOARD_USART_IRQ_HANDLER(1)
#define BOARD_DEBUG_UART_BASEADDR    BOARD_UART_BASEADDR(1)
#define BOARD_DEBUG_UART_RESET       kUSART1_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_RX_PIN      IOCON_USART1_RX_PIN
#define BOARD_DEBUG_UART_TX_PIN      IOCON_USART1_TX_PIN
#else
#define BOARD_DEBUG_UART_IRQ         LPUART0_IRQn
#define BOARD_DEBUG_UART_IRQ_HANDLER LPUART0_IRQHandler
#define BOARD_DEBUG_UART_BASEADDR    BOARD_UART_BASEADDR(0)
#define BOARD_DEBUG_UART_RESET       kUSART0_RST_SHIFT_RSTn
#define BOARD_DEBUG_UART_RX_PIN      IOCON_USART0_RX_PIN
#define BOARD_DEBUG_UART_TX_PIN      IOCON_USART0_TX_PIN
#endif

#define BOARD_DEBUG_UART_CLK_ATTACH kOSC32M_to_USART_CLK
#define BOARD_DEBUG_UART_CLK_FREQ   CLOCK_GetFreq(kCLOCK_Fro32M)

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BAUDRATE 115200U

#endif

//#define BOARD_DIAG_PORT_MODE           (0x00840083)
//#define BOARD_DIAG_PORT_MODE           (0x9F8B8783)
//#define BOARD_DIAG_PORT_MODE           (0x00000083)
#ifndef BOARD_DIAG_PORT_MODE
#define BOARD_DIAG_PORT_MODE 0
#endif

#define gDbgIoCfg_c 0 /* Dbg Io forbidden */
//#define gDbgIoCfg_c                     1 /* For Low Power Dbg Io use */
//#define gDbgIoCfg_c                     2 /* For General Purpose Dbg Io use */

#if gDbgUseSramIssueDetector
#undef gDbgIoCfg_c
#define gDbgIoCfg_c 3
#endif

/* Battery voltage level pin for ADC0 */
#define gADC0BatLevelInputPin (14)

/* when Diag Port is enabled (gDbgUseLLDiagPort set to 1), define the mode to use */
/* Use IOs API for Debugging BOARD_DbgSetIoUp() and BOARD_DbgSetIoUp() in board.h */
#define gDbgUseDbgIos (gDbgIoCfg_c != 0)

/* Enable Link Layer Diag Port - enable BOARD_DbgDiagIoConf() and BOARD_DbgDiagEnable() API in board.h */
#ifndef BIT
#define BIT(x) (1 << (x))
#endif
#define gDbgLLDiagPort0Msk (BIT(7) | BIT(23))
#define gDbgLLDiagPort1Msk (BIT(15) | BIT(31))
#define gDbgUseLLDiagPort  (BOARD_DIAG_PORT_MODE & (gDbgLLDiagPort0Msk | gDbgLLDiagPort1Msk))

/* Bluetooth MAC adress size */
#define BD_ADDR_SIZE 6

#define BOARD_MAINCLK_FRO12M  0U
#define BOARD_MAINCLK_XTAL32M 2U
#define BOARD_MAINCLK_FRO32M  3U
#define BOARD_MAINCLK_FRO48M  4U
#define gPWR_CpuClk_48MHz     1
#if gPWR_CpuClk_48MHz
#define BOARD_TARGET_CPU_FREQ BOARD_MAINCLK_FRO48M
#else
#define BOARD_TARGET_CPU_FREQ BOARD_MAINCLK_XTAL32M
//#define BOARD_TARGET_CPU_FREQ           BOARD_MAINCLK_FRO32M

#endif

#if gClkUseFro32K
#define CLOCK_32k_source kCLOCK_Fro32k
#else
#define CLOCK_32k_source kCLOCK_Xtal32k
#endif

#if gLoggingActive_d
#define RAM2_BASE 0x04020000
#define RAM2_SIZE (64 * 1024)
#endif

/* gAdvertisingPowerLeveldBm_c and gConnectPowerLeveldBm_c defualt values if not defiend otherwise
 * Valid values are in the range [-30, 10] nonetheless [-4, 3] is a reasonable range
 * e.g.  DBM(-3) for -3dBm
 * */
#define DBM(x) ((uint8_t)(x))

#ifndef gAdvertisingPowerLeveldBm_c
#define gAdvertisingPowerLeveldBm_c DBM(0)
#endif
#ifndef gConnectPowerLeveldBm_c
#define gConnectPowerLeveldBm_c DBM(0)
#endif

#define IS_DEBUG_SESSION (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* Capacitance values for 32MHz and 32kHz crystals; board-specific. Value is
   pF x 100. For example, 6pF becomes 600, 1.2pF becomes 120 */
#define CLOCK_32MfXtalIecLoadpF_x100    (600) /* 6.0pF */
#define CLOCK_32MfXtalPPcbParCappF_x100 (20)  /* 0.2pF */
#define CLOCK_32MfXtalNPcbParCappF_x100 (40)  /* 0.4pF */
#define CLOCK_32kfXtalIecLoadpF_x100    (600) /* 6.0pF */
#define CLOCK_32kfXtalPPcbParCappF_x100 (40)  /* 0.4pF */
#define CLOCK_32kfXtalNPcbParCappF_x100 (40)  /* 0.4pF */

/*******************************************************************************
 * API
 ******************************************************************************/
//extern flash_config_t gFlashConfig;

status_t BOARD_InitDebugConsole(void);
status_t BOARD_DeinitDebugConsole(void);

/* Function to initialize/deinitialize ADC on board configuration. */
void BOARD_InitAdc(void);
void BOARD_EnableAdc(void);
void BOARD_DeInitAdc(void);

/* Function to read battery level on board configuration. */
uint8_t BOARD_GetBatteryLevel(void);

int32_t BOARD_GetTemperature(void);

/* Function called by the BLE connection manager to generate PER MCU keys */
extern void BOARD_GetMCUUid(uint8_t *aOutUid16B, uint8_t *pOutLen);

/* Function called to get the USART Clock in Hz */
extern uint32_t BOARD_GetUsartClock(int8_t instance);

/* Function called to get the CTIMER clock in Hz */
extern uint32_t BOARD_GetCtimerClock(CTIMER_Type *timer);

extern void BOARD_UnInitButtons(void);

extern void BOARD_InitFlash(void);

extern uint16_t BOARD_GetPotentiometerLevel(void);

extern void BOARD_SetFaultBehaviour(void);

extern uint32_t BOARD_GetSpiClock(uint32_t instance);
extern void BOARD_InitPMod_SPI_I2C(void);
extern void BOARD_InitSPI(void);

extern void BOARD_InitSPIFI(void);

/* For debug only */
extern void BOARD_DbgDiagIoConf(void);
extern void BOARD_DbgDiagEnable();
extern void BOARD_InitDbgIo(void);

extern void BOARD_DbgLpIoSet(int pinid, int val);
extern void BOARD_DbgIoSet(int pinid, int val);
extern void BOARD_DbgSetIoUp(int pinid);   /* Should be deprecated and replaced by BOARD_DbgLpIoSet */
extern void BOARD_DbgSetIoDown(int pinid); /* Should be deprecated and replaced by BOARD_DbgLpIoSet */
/* Passivate outputs used for driving LEDs */
extern void BOARD_SetLEDs_LowPower(void);

/* Perform preparation to let inputs acquire wake capability */
extern void BOARD_SetButtons_LowPowerEnter(void);

/* Switch to target frequency */
extern void BOARD_CpuClockUpdate(void);

extern void BOARD_common_hw_init(void);

/* Function called to initial the pins */
extern void BOARD_InitDEBUG_UART(void);
extern void BOARD_InitHostInterface(void);
extern void BOARD_InitPins(void);
extern void BOARD_SetPinsForPowerDown(void);
extern void BOARD_SetPinsForPowerMode(void);
extern void BOARD_SetPinsForRunMode(void);
extern bool BOARD_CheckSwdAttached(void);
//void BOARD_SetPinsForRunMode(void);
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
