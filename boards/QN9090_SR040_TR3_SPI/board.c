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

#include "fsl_debug_console.h"
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_flash.h"
#include "fsl_xcvr.h"

#include <driver_config.h>
#include "board.h"
#include "FunctionLib.h"

#include "fsl_adc.h"
#include "RNG_Interface.h"
#include "Panic.h"
#include "rom_api.h"
#include "rom_psector.h"
#include "GPIO_Adapter.h"
#include "Keyboard.h"
#include "pin_mux.h"
#include "radio.h"
#include "LED.h"
#include "UWB_GpioIrq.h"

#if gOtaEepromPostedOperations_d
#include "OtaSupport.h"
#endif

#define gAdcUsed_d           1
#define gRtcLockupWorkAround 1

#if gLoggingActive_d || gAdcUsed_d || gRtcLockupWorkAround
#include "fsl_wtimer.h"
#endif

#if gUartDebugConsole_d
extern void BOARD_InitDEBUG_UART(void);
#endif
extern void BOARD_InitClocks(void);

#if gUartAppConsole_d
extern void BOARD_InitHostInterface(void);
#endif

/* Set gTcxo32M_ModeEn_c if you wish to activate the 32M Xtal trimming vs temperature.
 * Set gTcxo32k_ModeEn_c if you wish to activate the 32k Xtal trimming vs temperature.
 * dk6 boards are equipped with a CMOS040LP 32 MHz ultra low power DCXO (Digital Controlled Xtal Oscillator).
 * A temperature sweep has been performed from -40°C to +125°C in 5°C step and the frequency accuracy
 * has been recorded using a frequency meter for a fixed capbank code corresponding to 6 pF IEC load,
 * and the optimum IEC load giving the lowest frequency error obtained thanks to a dichotomy-based algorithm.
 * The PCB parasitic capacitors have been taken into account and specified in the C header file of the SW API
 * XTAL Reference: NDK NX2016SA 32MHz EXS00A-CS11213-6(IEC)
 *                 NDK NX2012SA 32.768kHz EXS00A-MU01089-6(IEC)
 * As a result CLOCK_ai32MXtalIecLoadFfVsTemp is setup to compensate IEC Load vs temperature for Xtal 32MHz.
 *
 * Similarly CLOCK_ai32kXtalIecLoadFfVsTemp has to be put together for the 32k Xtal.
 * The 32k compensation turns out inefficient outside the -40°C to +80°C range.
 *
 *
 *
 **/
#define gTcxo32M_ModeEn_c (1)
#define gXo32M_Trim_c     (1 || gTcxo32M_ModeEn_c)

/* Xtal 32kHz temperature compensation is disabled because table is *not* correct: values populating the temperature
 * compensation array below are just for example TODO */
#define gTcxo32k_ModeEn_c (0)
/* 32k not temperature compensated but ATE trimming used */
#if gTcxo32k_ModeEn_c
#define gXo32k_Trim_c (1)
#else
#define gXo32k_Trim_c (0)
#endif

#define TEMP_ZERO_K           -273
#define TEMP_ZERO_K_128th_DEG (TEMP_ZERO_K * 128)

/* Need to remember the latest temperature value */
int last_temperature_report_in_128th_of_degree = TEMP_ZERO_K_128th_DEG; /* absolute zero : no measurement yet */

#define ABSOLUTE_VALUE(x) (((x) < 0) ? -(x) : (x))

/*******************************************************************************
 * Code
 ******************************************************************************/

#if !defined CPU_JN518X_REV || (CPU_JN518X_REV > 1)
#define BLE_MACID_SZ                  6
#define MANUFACTURER_BLE_MACID_ADRESS (const uint8_t *)(0x9fc00 + 0x100)
#define gBD_ADDR_NXP_OUI_c            0x00, 0x60, 0x37
#endif

#ifndef gLogRingPlacementOffset_c
#define gLogRingPlacementOffset_c 0
#endif

#if gAdcUsed_d

/* Full scale voltage of ADC0 is 3600 mV */
#define gAdc0FullRangeVoltage (3600)

/* Full voltage value of battery is 3300 mV */
#define gBatteryFullVoltage (3300)

/* Resolution of ADC0 */
#define gAdc0MaxResolution (12)

/* Convert ADC0 output to voltage in mV */
#define ADC_TO_MV(x) (((x)*gAdc0FullRangeVoltage) >> gAdc0MaxResolution)

/* Compute voltage percentage */
#define ADC_MV_TO_PERCENT(x) (((x)*100) / gBatteryFullVoltage)

/* Temperature sensor channel of ADC */
#define ADC_TEMPERATURE_SENSOR_CHANNEL 7U

/* Battery level input channel of ADC */
#define ADC_BAT_LEVEL_CHANNEL 0x06

/* Temperature sensor driver code enable */
#define ADC_TEMP_SENSOR_DRIVER_EN 1

/* ADC initiate time */
#define ADC_WAIT_TIME_US 300
#else
#define ADC_TEMP_SENSOR_DRIVER_EN 0
#endif

#if (((defined gTcxo32k_ModeEn_c) && (gTcxo32k_ModeEn_c != 0)) || \
     ((defined gTcxo32M_ModeEn_c) && (gTcxo32M_ModeEn_c != 0)))

#define GET_ATE_TEMP() (*(uint32_t *)0x9FDC8)

static int16_t i16GetAteTemp(void)
{
    uint32_t u32AteTempReg;
    int16_t i16AteTempValue;

    /* ATE temperature should be stored in flash in a 32-bit word:
         bit 0     1 if value is valid, 0 if not
         bit 16:1  Value, degrees C x 128 */
    u32AteTempReg = GET_ATE_TEMP();
    if (u32AteTempReg & 1U) {
        /* Stored value is valid, so read it, taking care of sign bits */
        i16AteTempValue = (int16_t)(u32AteTempReg >> 1U);
        i16AteTempValue /= 128;
    }
    else {
        /* Assume 23 degrees C */
        i16AteTempValue = 23;
    }

    return i16AteTempValue;
}

#endif

#if gXo32M_Trim_c
/* Capacitance values for 32MHz  crystals; board-specific. Value is
   pF x 100. For example, 6pF becomes 600, 1.2pF becomes 120 */
//#define CLOCK_32MfXtalIecLoadpF_x100    (600) /* 6.0pF IEC load capacitance */
//#define CLOCK_32MfXtalPPcbParCappF_x100 (10)  /* 0.1pF P PCB Parasitic capacitance */
//#define CLOCK_32MfXtalNPcbParCappF_x100 (5)   /* 0.05pF N PCB Parasitic capacitance */

/* Capacitance values for 32MHz. Actual values for all of
 * these should be defined in board.h for systems where crystal accuracy is
 * vital. Adding default values here for builds that do not need this. Value
 * is pF x 100. For example, 6pF becomes 600, 1.2pF becomes 120 */
#ifndef CLOCK_32MfXtalIecLoadpF_x100
#define CLOCK_32MfXtalIecLoadpF_x100 (600)
#endif
#ifndef CLOCK_32MfXtalPPcbParCappF_x100
#define CLOCK_32MfXtalPPcbParCappF_x100 (10)
#endif
#ifndef CLOCK_32MfXtalNPcbParCappF_x100
#define CLOCK_32MfXtalNPcbParCappF_x100 (5)
#endif

#if ((defined gTcxo32M_ModeEn_c) && (gTcxo32M_ModeEn_c != 0))

/* Values below are for NDK NX2016SA 32MHz EXS00A-CS11213-6(IEC) */

/* Temperature related to element 0 of CLOCK_ai32MXtalIecLoadFfVsTemp */
#define HW_32M_LOAD_VS_TEMP_MIN (-40)

/* Temperature related to final element of CLOCK_ai32MXtalIecLoadFfVsTemp */
#define HW_32M_LOAD_VS_TEMP_MAX (130)

/* Temperature step between elements of CLOCK_ai32MXtalIecLoadFfVsTemp */
#define HW_32M_LOAD_VS_TEMP_STEP (5)

#define HW_32M_LOAD_VS_TEMP_SIZE ((HW_32M_LOAD_VS_TEMP_MAX - HW_32M_LOAD_VS_TEMP_MIN) / HW_32M_LOAD_VS_TEMP_STEP + 1U)

/* Table of load capacitance versus temperature for 32MHz crystal. Values are
   for temperatures from -40 to +130 in steps of 5, expressed in femto Farads */
#if OLD_XTAL32M_CAL
static const int32_t CLOCK_ai32MXtalIecLoadFfVsTemp[HW_32M_LOAD_VS_TEMP_SIZE] = {
    537,
    714,
    837,
    909,
    935,
    922,
    873,
    796, /* -40, -35, ... -5 */
    694,
    574,
    440,
    297,
    149,
    0,
    -147,
    -290, /* 0, 5, ... 35 */
    -425,
    -551,
    -667,
    -770,
    -860,
    -936,
    -998,
    -1042, /* 40, 45, ... 75 */
    -1069,
    -1076,
    -1061,
    -1019,
    -947,
    -837,
    -682,
    -472, /* 80, 85, ... 115 */
    -196,
    161,
    618 /* 120, 125, 130 */
};
#else
static const int32_t CLOCK_ai32MXtalIecLoadFfVsTemp[HW_32M_LOAD_VS_TEMP_SIZE] = {506,
    687,
    818,
    900,
    938,
    933,
    892,
    819, /* -40, -35, ... -5 */
    720,
    599,
    461,
    313,
    158,
    0,
    -156,
    -307, /* 0, 5, ... 35 */
    -451,
    -584,
    -705,
    -812,
    -903,
    -977,
    -1032,
    -1067, /* 40, 45, ... 75 */
    -1079,
    -1067,
    -1025,
    -950,
    -835,
    -673,
    -453,
    -163, /* 80, 85, ... 115 */
    212,
    691,
    1296}; /* 120, 125, 130 */
#endif

static int iCapDeltaCalculation_MHz_x1000(int32_t iTemp)
{
    uint32_t Ind_A_32M;
    uint32_t Ind_B_32M;
    int32_t a_Temp_32M_x1000;
    int32_t b_Temp_32M_x1000;

    /* Get index into array for temperature value at or below the ATE
    * temperature, but not above. For case where ATE temperature is at
    * highest index in array, drop back by 1 because we want to get the
    * next index later */
    if (iTemp >= HW_32M_LOAD_VS_TEMP_MAX) {
        Ind_A_32M = ((HW_32M_LOAD_VS_TEMP_MAX - HW_32M_LOAD_VS_TEMP_MIN) / HW_32M_LOAD_VS_TEMP_STEP) - 1;
    }
    else if (iTemp < HW_32M_LOAD_VS_TEMP_MIN) {
        Ind_A_32M = 0;
    }
    else {
        Ind_A_32M = (iTemp - HW_32M_LOAD_VS_TEMP_MIN) / HW_32M_LOAD_VS_TEMP_STEP;
    }

    /* Get index after the selected one */
    Ind_B_32M = Ind_A_32M + 1;

    /* Linear fit coefficients calculation */
    a_Temp_32M_x1000 = (CLOCK_ai32MXtalIecLoadFfVsTemp[Ind_B_32M] - CLOCK_ai32MXtalIecLoadFfVsTemp[Ind_A_32M]) /
                       HW_32M_LOAD_VS_TEMP_STEP;
    b_Temp_32M_x1000 = CLOCK_ai32MXtalIecLoadFfVsTemp[Ind_A_32M] -
                       a_Temp_32M_x1000 * (HW_32M_LOAD_VS_TEMP_MIN + HW_32M_LOAD_VS_TEMP_STEP * (int32_t)Ind_A_32M);

    return iTemp * a_Temp_32M_x1000 + b_Temp_32M_x1000;
}

static int32_t Calculate_32MOscCapCompensation(int32_t iTemperature)
{
    int32_t x0_32M_osc_cap_delta_fF = 0;
#if ((defined gTcxo32M_ModeEn_c) && (gTcxo32M_ModeEn_c != 0))

    int16_t i16AteTemp;

    /* LUT Normalization to ATE temperature */
    i16AteTemp = i16GetAteTemp();

    if (i16AteTemp != 23) {
        x0_32M_osc_cap_delta_fF = iCapDeltaCalculation_MHz_x1000(i16AteTemp);
    }
    else {
        x0_32M_osc_cap_delta_fF = 0;
    }

    x0_32M_osc_cap_delta_fF = iCapDeltaCalculation_MHz_x1000(iTemperature) - x0_32M_osc_cap_delta_fF;
#endif
    return x0_32M_osc_cap_delta_fF;
}
#endif
#endif

#if defined gXo32k_Trim_c && (gXo32k_Trim_c == 1)

/* Capacitance values for 32kHz crystals; board-specific. Value is
   pF x 100. For example, 6pF becomes 600, 1.2pF becomes 120 */
#define CLOCK_32kfXtalIecLoadpF_x100    (600) /* 6.0pF IEC load capacitance */
#define CLOCK_32kfXtalPPcbParCappF_x100 (300) /* 3.0pF P PCB Parasitic capacitance*/
#define CLOCK_32kfXtalNPcbParCappF_x100 (250) /* 2.5pF N PCB Parasitic capacitance*/

#ifndef CLOCK_32kfXtalIecLoadpF_x100
#define CLOCK_32kfXtalIecLoadpF_x100 (600)
#endif
#ifndef CLOCK_32kfXtalPPcbParCappF_x100
#define CLOCK_32kfXtalPPcbParCappF_x100 (300)
#endif
#ifndef CLOCK_32kfXtalNPcbParCappF_x100
#define CLOCK_32kfXtalNPcbParCappF_x100 (250)
#endif

#if (defined gTcxo32k_ModeEn_c) && (gTcxo32k_ModeEn_c != 0)

/* Capacitance variation for 32kHz crystal across temperature
   ----------------------------------------------------------

   gTcxo32k_ModeEn_c should be 1 to indicate that temperature-compensated 32kHz
   XO is supported and required. If so, HW_32k_LOAD_VS_TEMP_MIN,
   _MAX, _STEP must be defined here and CLOCK_ai32kXtalIecLoadFfVsTemp
   must be defined in board.c.

   Values are used as follows:
   CLOCK_ai32kXtalIecLoadFfVsTemp is an array of crystal load capacitance
   values across temp, with each value being at a specific temp. First value is
   for temp given by HW_32k_LOAD_VS_TEMP_MIN, next value is for
   temp given by HW_32k_LOAD_VS_TEMP_MIN + _STEP, next value is
   for temp given by HW_32k_LOAD_VS_TEMP_MIN + _ STEP x 2, etc.
   Final value is for temp given by HW_32k_LOAD_VS_TEMP_MAX. It is
   important for HW_32k_LOAD_VS_TEMP_x defines and the table to be
   matched to one another */

/* Temperature related to element 0 of CLOCK_ai32kXtalIecLoadFfVsTemp */
#define HW_32k_LOAD_VS_TEMP_MIN (-20)

/* Temperature related to final element of CLOCK_ai32kXtalIecLoadFfVsTemp */
#define HW_32k_LOAD_VS_TEMP_MAX (100)

/* Temperature step between elements of CLOCK_ai32kXtalIecLoadFfVsTemp */
#define HW_32k_LOAD_VS_TEMP_STEP (5)

#define HW_32k_LOAD_VS_TEMP_SIZE ((HW_32k_LOAD_VS_TEMP_MAX - HW_32k_LOAD_VS_TEMP_MIN) / HW_32k_LOAD_VS_TEMP_STEP + 1U)

/* Table of load capacitance versus temperature for 32kHz crystal. Values are
   for temperatures from -20 to +100 in steps of 20. *Note* values below are
   just for example */
#ifdef OLD_XTAL32K_CAL
static const int32_t CLOCK_ai32kXtalIecLoadFfVsTemp[HW_32k_LOAD_VS_TEMP_SIZE] = {-1843,
    -1583,
    -1351,
    -1138,
    -939,
    -752,
    -579,
    -422, /* -40, -35, ... -5 */
    -285,
    -170,
    -81,
    -22,
    5,
    0,
    -38,
    -107, /* 0, 5, ... 35 */
    -205,
    -330,
    -478,
    -645,
    -830,
    -1031,
    -1247,
    -1482, /* 40, 45, ... 75 */
    -1742,
    -2078,
    -2409,
    -2766,
    -3147,
    -3554,
    -3995,
    -4442, /* 80, 85, ... 115 */
    -4923,
    -5430,
    -5961}; /* 120, 125, 130 */
#else
static const int32_t CLOCK_ai32kXtalIecLoadFfVsTemp[HW_32k_LOAD_VS_TEMP_SIZE] = {-1984,
    -1728,
    -1496,
    -1278,
    -1070,
    -870,
    -680,
    -504, /* -40, -35, ... -5 */
    -347,
    -214,
    -109,
    -37,
    0,
    0,
    -37,
    -111, /* 0, 5, ... 35 */
    -218,
    -356,
    -521,
    -709,
    -917,
    -1143,
    -1388,
    -1656, /* 40, 45, ... 75 */
    -1957,
    -2293,
    -2657,
    -3048,
    -3467,
    -3913,
    -4386,
    -4887, /* 80, 85, ... 115 */
    -5416,
    -5971,
    -6555}; /* 120, 125, 130 */
#endif
static int iCapDeltaCalculation_kHz_x1000(int32_t iTemp)
{
    uint32_t Ind_A_32k;
    uint32_t Ind_B_32k;
    int32_t a_Temp_32k_x1000;
    int32_t b_Temp_32k_x1000;

    /* Get index into array for temperature value at or below the ATE
    * temperature, but not above. For case where ATE temperature is at
    * highest index in array, drop back by 1 because we want to get the
    * next index later */
    if (iTemp >= HW_32k_LOAD_VS_TEMP_MAX) {
        Ind_A_32k = ((HW_32k_LOAD_VS_TEMP_MAX - HW_32k_LOAD_VS_TEMP_MIN) / HW_32k_LOAD_VS_TEMP_STEP) - 1;
    }
    else if (iTemp < HW_32k_LOAD_VS_TEMP_MIN) {
        Ind_A_32k = 0;
    }
    else {
        Ind_A_32k = (iTemp - HW_32k_LOAD_VS_TEMP_MIN) / HW_32k_LOAD_VS_TEMP_STEP;
    }

    /* Get index after the selected one */
    Ind_B_32k = Ind_A_32k + 1;

    /* Linear fit coefficients calculation */
    a_Temp_32k_x1000 = (CLOCK_ai32kXtalIecLoadFfVsTemp[Ind_B_32k] - CLOCK_ai32kXtalIecLoadFfVsTemp[Ind_A_32k]) /
                       HW_32k_LOAD_VS_TEMP_STEP;
    b_Temp_32k_x1000 = CLOCK_ai32kXtalIecLoadFfVsTemp[Ind_A_32k] -
                       a_Temp_32k_x1000 * (HW_32k_LOAD_VS_TEMP_MIN + HW_32k_LOAD_VS_TEMP_STEP * Ind_A_32k);

    return iTemp * a_Temp_32k_x1000 + b_Temp_32k_x1000;
}

static const ClockCapacitanceCompensation_t BOARD_Clock32kCapacitanceCharacteristics = {
    .clk_XtalIecLoadpF_x100    = CLOCK_32kfXtalIecLoadpF_x100,
    .clk_XtalPPcbParCappF_x100 = CLOCK_32kfXtalPPcbParCappF_x100,
    .clk_XtalNPcbParCappF_x100 = CLOCK_32kfXtalNPcbParCappF_x100,
};

static int32_t Calculate_32kOscCapCompensation(int32_t iTemperature)
{
    int32_t xo_32k_osc_cap_delta_fF = 0;
#if (defined gTcxo32k_ModeEn_c) && (gTcxo32k_ModeEn_c != 0)
    int16_t i16AteTemp;

    /* LUT Normalization to ATE temperature */
    i16AteTemp = i16GetAteTemp();

    if (i16AteTemp != 23) {
        xo_32k_osc_cap_delta_fF = iCapDeltaCalculation_kHz_x1000(i16AteTemp);
    }
    else {
        xo_32k_osc_cap_delta_fF = 0;
    }

    xo_32k_osc_cap_delta_fF = iCapDeltaCalculation_kHz_x1000(iTemperature) - xo_32k_osc_cap_delta_fF;
#endif
    return xo_32k_osc_cap_delta_fF;
}
#endif

#endif

#if defined gXo32M_Trim_c && (gXo32M_Trim_c == 1)

static const ClockCapacitanceCompensation_t BOARD_Clock32MCapacitanceCharacteristics = {
    .clk_XtalIecLoadpF_x100    = CLOCK_32MfXtalIecLoadpF_x100,
    .clk_XtalPPcbParCappF_x100 = CLOCK_32MfXtalPPcbParCappF_x100,
    .clk_XtalNPcbParCappF_x100 = CLOCK_32MfXtalNPcbParCappF_x100};

#endif

/*******************************************************************************
* Local variables
******************************************************************************/
#if gAdcUsed_d
static adc_config_t adcConfigStruct;
static adc_conv_seq_config_t adcConvSeqConfigStruct;
#endif

#if (gKBD_KeysCount_c > 0)
const gpioInputPinConfig_t dk6_button_io_pins[] = {[0] =
                                                       {
                                                           .gpioPort            = gpioPort_A_c,
                                                           .gpioPin             = BOARD_USER_BUTTON1_GPIO_PIN,
                                                           .pullSelect          = pinPull_Disabled_c,
                                                           .interruptModeSelect = pinInt_FallingEdge_c,
                                                           .pinIntSelect        = kPINT_PinInt0,
                                                           .inputmux_attach_id  = kINPUTMUX_GpioPort0Pin1ToPintsel,
                                                           .is_wake_source      = TRUE,
                                                       },
#if (gKBD_KeysCount_c > 1)
    [1] =
        {
            .gpioPort            = gpioPort_A_c,
            .gpioPin             = BOARD_USER_BUTTON2_GPIO_PIN,
            .pullSelect          = pinPull_Disabled_c,
            .interruptModeSelect = pinInt_FallingEdge_c,
            .pinIntSelect        = kPINT_PinInt1,
            .inputmux_attach_id  = kINPUTMUX_GpioPort0Pin5ToPintsel,
            .is_wake_source      = TRUE,
        }
#endif
};

const iocon_group_t dk6_buttons_io[] = {
    [0] =
        {
            .port     = 0,
            .pin      = BOARD_USER_BUTTON1_GPIO_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_USER_BUTTON_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        },
#if (gKBD_KeysCount_c > 1)
    [1] =
        {
            .port     = 0,
            .pin      = BOARD_USER_BUTTON2_GPIO_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_USER_BUTTON_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        }
#endif
};
#endif

#if (gLEDsOnTargetBoardCnt_c > 0)

const gpioOutputPinConfig_t dk6_leds_io_pins[] = {
    [0] =
        {
            .gpioPort      = gpioPort_A_c,
            .gpioPin       = BOARD_LED_RED_GPIO_PIN,
            .outputLogic   = 1, /* negated output 0 :  means switch on */
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#if (gLEDsOnTargetBoardCnt_c > 1)
    [1] =
        {
            .gpioPort      = gpioPort_A_c,
            .gpioPin       = BOARD_LED_GREEN_GPIO_PIN,
            .outputLogic   = 1, /* negated output 0 :  means switch on */
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },

#if (gLEDsOnTargetBoardCnt_c > 2)
    [2] =
        {
            .gpioPort      = gpioPort_A_c,
            .gpioPin       = BOARD_LED_BLUE_GPIO_PIN,
            .outputLogic   = 1, /* negated output 0 :  means switch on */
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#endif
#endif
};

const iocon_group_t dk6_leds_io[] = {
    [0] =
        {
            .port     = 0,
            .pin      = BOARD_LED_RED_GPIO_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_LED_MODE_FUNC) | IOCON_MODE_INACT | IOCON_DIGITAL_EN,
        },
#if (gLEDsOnTargetBoardCnt_c > 1)
    [1] =
        {
            .port     = 0,
            .pin      = BOARD_LED_GREEN_GPIO_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_LED_MODE_FUNC) | IOCON_MODE_INACT | IOCON_DIGITAL_EN,
        },
#if (gLEDsOnTargetBoardCnt_c > 2)
    [2] =
        {
            .port     = 0,
            .pin      = BOARD_LED_BLUE_GPIO_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_LED_MODE_FUNC) | IOCON_MODE_INACT | IOCON_DIGITAL_EN,
        },
#endif
#endif

};
#endif

#if gDbgUseDbgIos
#if (gDbgIoCfg_c == 1) || (gDbgIoCfg_c == 2)

#define IOCON_DBG_PIN 17
#define NB_DBG_IO     (22 - IOCON_DBG_PIN)

#define IOCON_DBGIO_MODE_FUNC 0

const gpioOutputPinConfig_t dk6_dbg_io_pins[] = {
#if NB_DBG_IO >= 1
    [0] =
        {
            .gpioPort      = 0,
            .gpioPin       = IOCON_DBG_PIN,
            .outputLogic   = 1,
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#if NB_DBG_IO >= 2
    [1] =
        {
            .gpioPort      = 0,
            .gpioPin       = IOCON_DBG_PIN + 1,
            .outputLogic   = 1,
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#if NB_DBG_IO >= 3

    [2] =
        {
            .gpioPort      = 0,
            .gpioPin       = IOCON_DBG_PIN + 2,
            .outputLogic   = 1,
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#if NB_DBG_IO >= 4
    [3] =
        {
            .gpioPort      = 0,
            .gpioPin       = IOCON_DBG_PIN + 3,
            .outputLogic   = 1,
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#if NB_DBG_IO >= 5
    [4] =
        {
            .gpioPort      = 0,
            .gpioPin       = IOCON_DBG_PIN + 4,
            .outputLogic   = 1,
            .slewRate      = pinSlewRate_Slow_c,
            .driveStrength = pinDriveStrength_Low_c,
        },
#endif
#endif
#endif
#endif
#endif
};

const iocon_group_t dk6_dbg_io[] = {
#if NB_DBG_IO >= 1
    [0] =
        {
            .port     = 0,
            .pin      = IOCON_DBG_PIN,
            .modefunc = IOCON_PIO_FUNC(IOCON_DBGIO_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        },
#if NB_DBG_IO >= 2
    [1] =
        {
            .port     = 0,
            .pin      = IOCON_DBG_PIN + 1,
            .modefunc = IOCON_PIO_FUNC(IOCON_DBGIO_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        },
#if NB_DBG_IO >= 3
    [2] =
        {
            .port     = 0,
            .pin      = IOCON_DBG_PIN + 2,
            .modefunc = IOCON_PIO_FUNC(IOCON_DBGIO_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        },
#if NB_DBG_IO >= 4
    [3] =
        {
            .port     = 0,
            .pin      = IOCON_DBG_PIN + 3,
            .modefunc = IOCON_PIO_FUNC(IOCON_DBGIO_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        },
#if NB_DBG_IO >= 5
    [4] =
        {
            .port     = 0,
            .pin      = IOCON_DBG_PIN + 4,
            .modefunc = IOCON_PIO_FUNC(IOCON_DBGIO_MODE_FUNC) | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN,
        }
#endif
#endif
#endif
#endif
#endif
};
#endif
#endif

/*****************************************************************************
* Local functions
****************************************************************************/

static bool BOARD_IsADCEnabled(void);

/*****************************************************************************
* temperature sensor driver
****************************************************************************/
#if (ADC_TEMP_SENSOR_DRIVER_EN)

//Comment the define if you want to use specific calibration values or want to debug
//#define LOAD_CAL_FROM_FLASH

/*Types*/
typedef struct
{
    int16_t ref0v0;
    int16_t ref0v8;
    int16_t adcout_ref0v0_vbat3v3_sum8;
    int16_t adcout_ref0v8_vbat3v3_sum8;
    int16_t adcout_ref0v0_vbat1v8_sum8;
    int16_t adcout_ref0v8_vbat1v8_sum8;
    int16_t adcout_Tref_vbat3v3_sum8;
    int16_t adcout_Tref_vbat1v8_sum8;
    int8_t nlfit_Toffset;
    int8_t nlfit_gain;
    int16_t Tref;
    uint16_t vbatsum_offset;
    int16_t vbatsum_gain;
    int16_t adcfs_vbat3v3;
    int16_t slopefitinv_gain;
    int16_t slopefitinv_vbatgain;
} CfgTempSensor_tt;

/*Defines*/
#define ADC_GPADC_CTRL0_GPADC_TSAMP_Pos 9 /*!< ADC GPADC_CTRL0: GPADC_TSAMP Position */
#define ADC_GPADC_CTRL0_GPADC_TSAMP_Msk \
    (0x1fUL << ADC_GPADC_CTRL0_GPADC_TSAMP_Pos)                       /*!< ADC GPADC_CTRL0: GPADC_TSAMP Mask     */
#define ADC_GPADC_CTRL0_TEST_Pos 14                                   /*!< ADC GPADC_CTRL0: TEST Position        */
#define ADC_GPADC_CTRL0_TEST_Msk (0x03UL << ADC_GPADC_CTRL0_TEST_Pos) /*!< ADC GPADC_CTRL0: TEST Mask            */

#define ADC_GPADC_CTRL1_OFFSET_CAL_Pos 0 /*!< ADC GPADC_CTRL1: OFFSET_CAL Position  */
#define ADC_GPADC_CTRL1_OFFSET_CAL_Msk \
    (0x000003ffUL << ADC_GPADC_CTRL1_OFFSET_CAL_Pos) /*!< ADC GPADC_CTRL1: OFFSET_CAL Mask      */
#define ADC_GPADC_CTRL1_GAIN_CAL_Pos 10              /*!< ADC GPADC_CTRL1: GAIN_CAL Position    */
#define ADC_GPADC_CTRL1_GAIN_CAL_Msk \
    (0x000003ffUL << ADC_GPADC_CTRL1_GAIN_CAL_Pos) /*!< ADC GPADC_CTRL1: GAIN_CAL Mask        */

#define FLASH_CFG_GPADC_TRIM_VALID_Pos         0
#define FLASH_CFG_GPADC_TRIM_VALID_Width       1
#define FLASH_CFG_GPADC_OFFSETCAL_Pos          1
#define FLASH_CFG_GPADC_OFFSETCAL_Width        10
#define FLASH_CFG_GPADC_GAINCAL_Pos            11
#define FLASH_CFG_GPADC_GAINCAL_Width          10
#define FLASH_CFG_GPADC_UNITY_OFFSET_CAL_Pos   21
#define FLASH_CFG_GPADC_UNITY_OFFSET_CAL_Width 10

#define FLASH_CFG_GPADC_UNITY_OFFSET_VOUT_VAL_Pos   0
#define FLASH_CFG_GPADC_UNITY_OFFSET_VOUT_VAL_Width 12
#define FLASH_CFG_GPADC_UNITY_GAIN_INPUT_VAL_Pos    12
#define FLASH_CFG_GPADC_UNITY_GAIN_INPUT_VAL_Width  12

#define FLASH_CFG_TEMPSENSOR_TRIM_VALID_Pos   0
#define FLASH_CFG_TEMPSENSOR_TRIM_VALID_Width 1

#define FLASH_CFG_TEMP_SENSOR_REF0V0_Pos   0
#define FLASH_CFG_TEMP_SENSOR_REF0V0_Width 16
#define FLASH_CFG_TEMP_SENSOR_REF0V8_Pos   16
#define FLASH_CFG_TEMP_SENSOR_REF0V8_Width 16

#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V0_VBAT3V3_SUM8_Pos   0
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V0_VBAT3V3_SUM8_Width 16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V8_VBAT3V3_SUM8_Pos   16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V8_VBAT3V3_SUM8_Width 16

#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V0_VBAT1V8_SUM8_Pos   0
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V0_VBAT1V8_SUM8_Width 16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V8_VBAT1V8_SUM8_Pos   16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_REF0V8_VBAT1V8_SUM8_Width 16

#define FLASH_CFG_TEMP_SENSOR_ADCOUT_TREF_VBAT3V3_SUM8_Pos   0
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_TREF_VBAT3V3_SUM8_Width 16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_TREF_VBAT1V8_SUM8_Pos   16
#define FLASH_CFG_TEMP_SENSOR_ADCOUT_TREF_VBAT1V8_SUM8_Width 16

#define FLASH_CFG_TEMP_SENSOR_NLFIT_TOFFSET_Pos   0
#define FLASH_CFG_TEMP_SENSOR_NLFIT_TOFFSET_Width 8
#define FLASH_CFG_TEMP_SENSOR_NLFIT_GAIN_Pos      8
#define FLASH_CFG_TEMP_SENSOR_NLFIT_GAIN_Width    8
#define FLASH_CFG_TEMP_SENSOR_NLFIT_TREF_Pos      16
#define FLASH_CFG_TEMP_SENSOR_NLFIT_TREF_Width    16

#define FLASH_CFG_TEMP_SENSOR_VBATSUM_OFFSET_Pos   0
#define FLASH_CFG_TEMP_SENSOR_VBATSUM_OFFSET_Width 12
#define FLASH_CFG_TEMP_SENSOR_VBATSUM_GAIN_Pos     12
#define FLASH_CFG_TEMP_SENSOR_VBATSUM_GAIN_Width   10
#define FLASH_CFG_TEMP_SENSOR_ADCFS_ERR_Pos        22
#define FLASH_CFG_TEMP_SENSOR_ADCFS_ERR_Width      10

#define FLASH_CFG_TEMP_SENSOR_SLOPEFITINV_GAIN_Pos       0
#define FLASH_CFG_TEMP_SENSOR_SLOPEFITINV_GAIN_Width     16
#define FLASH_CFG_TEMP_SENSOR_SLOPEFITINV_VBATGAIN_Pos   16
#define FLASH_CFG_TEMP_SENSOR_SLOPEFITINV_VBATGAIN_Width 16

#define FIELD_VAL(wrd, _FLD) (((wrd) >> (_FLD##_Pos)) & ((1 << (_FLD##_Width)) - 1))

#define CONFIG_PAGE_BASE_ADDRESS      0x9FC00
#define ADC_TRIM_PARAM_OFFSET         (0x80)
#define TEMP_SENSOR_TRIM_VALID_OFFSET (0x8C)
#define TEMP_SENSOR_TRIM_PARAM_OFFSET (0xA0)

#define VBAT_CHANNEL                 6
#define TEMPERATURE_SENSOR_CHANNEL   7U
#define DEMO_ADC_SAMPLE_TEST_CHANNEL 0U

#ifdef LOAD_CAL_FROM_FLASH
static CfgTempSensor_tt temp_sensor_calibration;
static CfgGpAdcTrim_t adc_calibration;
#else
static CfgTempSensor_tt temp_sensor_calibration = {
    0, 26214, 68, 29288, 68, 29256, 21001, 20968, 41, (int8_t)154, 3354, 3756, 1008, 1011, (int16_t)39453, 1401};
#if 0
static CfgGpAdcTrim_t adc_calibration =    {
                                            646, 489, 0, 0, 0
                                        };
#endif
#endif

static CfgTempSensor_tt *cal = &temp_sensor_calibration;
//adc_conv_seq_config_t     adcConvSeqConfigStruct;
//adc_config_t             adcConfigStruct;
static bool is_valid_calibration_avail = false;

#endif /* #if(ADC_TEMP_SENSOR_DRIVER_EN) */

/*******************************************************************************
 * Public variables
 *******************************************************************************/

jmp_buf *exception_buf;

/*******************************************************************************
 * Private functions
 *******************************************************************************/

#if (ADC_TEMP_SENSOR_DRIVER_EN)

/* Function shall be called prior reading the temperature.
 * Returns 1 if calibration data is available else returns 0.
 * If there is no calibration data available for ADC or temperature sensor, computation of temperature with
 * get_temperature() is not possible.*/
static bool load_calibration_param_from_flash(ADC_Type *base)
{
    bool is_valid = false;
    do {
        is_valid = is_valid_calibration_avail;
#ifdef LOAD_CAL_FROM_FLASH
        if (is_valid)
            break; /* was done already */
        //Load ADC calibration data from flash
        is_valid = flashConfig_ExtractGpAdcTrimValues(&adc_calibration);
        if (!is_valid) {
            PRINTF("Calibration data loading error: no ADC calibration data available!\r\n");
            break;
        }

        //Load temperature sensor calibration data from flash
        is_valid = flashConfig_ExtractTempSensorTrimValues(&temp_sensor_calibration);
        if (!is_valid) {
            PRINTF("Calibration data loading error: no temperature sensor calibration data available!\r\n");
            break;
        }

        if (temp_sensor_calibration.vbatsum_gain & (1 << 9)) {
            temp_sensor_calibration.vbatsum_gain = (int16_t)(temp_sensor_calibration.vbatsum_gain | 0xFC00);
        }
        if (temp_sensor_calibration.adcfs_vbat3v3 & (1 << 9)) {
            temp_sensor_calibration.adcfs_vbat3v3 = (int16_t)(temp_sensor_calibration.adcfs_vbat3v3 | 0xFC00);
        }

        //Initialize ADC with calibration data
        uint32_t reg = 0;
        reg = ((adc_calibration.offset_cal << ADC_GPADC_CTRL1_OFFSET_CAL_Pos) & ADC_GPADC_CTRL1_OFFSET_CAL_Msk);
        reg |= ((adc_calibration.gain_cal << ADC_GPADC_CTRL1_GAIN_CAL_Pos) & ADC_GPADC_CTRL1_GAIN_CAL_Msk);

        base->GPADC_CTRL1 = reg;
#else
        is_valid = true;
#endif
    } while (0);
    is_valid_calibration_avail = is_valid;
    return is_valid;
}

static void update_ctrl0_adc_register(ADC_Type *base, uint8_t mode, uint8_t tsamp)
{
    //read the GPADC_CTRL0 register and update settings
    uint32_t read_reg = base->GPADC_CTRL0;

    //clear the "TEST" and "TSAMP" fields
    read_reg &= ~(ADC_GPADC_CTRL0_TEST_Msk | ADC_GPADC_CTRL0_GPADC_TSAMP_Msk);

    read_reg |= ((tsamp << ADC_GPADC_CTRL0_GPADC_TSAMP_Pos) & ADC_GPADC_CTRL0_GPADC_TSAMP_Msk);
    read_reg |= ((mode << ADC_GPADC_CTRL0_TEST_Pos) & ADC_GPADC_CTRL0_TEST_Msk);

    base->GPADC_CTRL0 = read_reg;
}

static uint16_t compute_estimated_sum(ADC_Type *base, uint8_t nb_samples, uint8_t channel_id)
{
    uint8_t count          = 0U;
    uint16_t estimated_sum = 0U;
    adc_result_info_t adcResultInfoStruct;
    while (count < nb_samples) {
        /*  trigger the converter by software. */
        ADC_DoSoftwareTriggerConvSeqA(base);
        //At that step in CTRl register, START = 1
        /* Wait for the converter to be done. */
        while (!ADC_GetChannelConversionResult(base, channel_id, &adcResultInfoStruct)) {
        }
        estimated_sum += adcResultInfoStruct.result;
#ifdef DBG_DRIVER
        PRINTF("adc_result=%d\r\n", adcResultInfoStruct.result);
#endif
        count++;
    }
    return estimated_sum;
}

static int32_t compute_nlfit_corr(int32_t input)
{
    int32_t signed_T_linearfit = input >> 7;

    //nlfit_corr_tlinearfit_soft_mult128 = (2**7)*(signed_tsens_nlfit_gain*1.0/(2**19))*((T_linearfit-signed_tsens_nlfit_Toffset)**2)
    int32_t coefnlfit1        = (signed_T_linearfit - cal->nlfit_Toffset) * (signed_T_linearfit - cal->nlfit_Toffset);
    int32_t coefnlfit1_shift6 = coefnlfit1 >> 6;

    return (cal->nlfit_gain * coefnlfit1_shift6) >> 6;
}

static int32_t temp_computation(uint16_t adcout_vbat_lsb_sum8, uint16_t tsens_adcout_T_sum8, uint8_t nb_samples)
{
    int16_t diff_adcout_vbat_lsb_mult8 = adcout_vbat_lsb_sum8 - (cal->vbatsum_offset * nb_samples);

    /*
    tsens_slopefitinv_vbatgain is 16 bit signed value
    tsens_slopefitinv_gain is 16 bit signed value

    SlopeFitInv_oC_lsb= (signed_tsens_slopefitinv_gain*1.0/(2**18))*(1-(signed_tsens_slopefitinv_vbatgain*(adcout_vbat_lsb-tsens_vbatsum_offset))*1.0/(2**31))
    Trunc under 16 bits: 15 bit + sign bit
    */
    int32_t signed_val_coef1_mult8 = (cal->slopefitinv_vbatgain * diff_adcout_vbat_lsb_mult8);

    int32_t signed_val_coef1_shift16 = signed_val_coef1_mult8 >> 16;
    int32_t signed_val_coef2_mult8   = ((1 << 18) - signed_val_coef1_shift16);
    //Divide by 8 now as the result of signed_val_coef3_shift16 below may not fit in a signed int
    int32_t signed_val_coef2 = ((1 << 18) - signed_val_coef1_shift16) / nb_samples;

    //Rounding: if decimal part >= 0.5
    if (signed_val_coef2_mult8 & (1 << 2)) {
        if (signed_val_coef1_mult8 & (1 << 31)) {
            signed_val_coef2--;
        }
        else {
            signed_val_coef2++;
        }
    }

    //SlopeFitInv_oC_lsb= (signed_tsens_slopefitinv_gain*1.0/(2**18))*signed_val_coef2/2**15
    //SlopeFitInv_oC_lsb= signed_tsens_slopefitinv_gain*signed_val_coef2/(2**15 * 2**18)
    int32_t signed_val_coef3_shift16 = (cal->slopefitinv_gain * signed_val_coef2) >> 16;

    int32_t signed_val_coef4         = (tsens_adcout_T_sum8 - cal->adcout_Tref_vbat3v3_sum8) * signed_val_coef3_shift16;
    int32_t signed_val_coef4_shift13 = signed_val_coef4 >> 13;

    int32_t T_linearfit_soft_mult128 = signed_val_coef4_shift13 + cal->Tref;

    int32_t Test_Vbatsumcorr_soft_mult128 = ((cal->vbatsum_gain * diff_adcout_vbat_lsb_mult8) / nb_samples) >> 9;

    //T_linearfit : No decimal value would be enough
    //nlfit_corr_tlinearfit = (signed_tsens_nlfit_gain*1.0/(2**19))*((T_linearfit-signed_tsens_nlfit_Toffset)**2)
    //nlfit_corr_tref =  (signed_tsens_nlfit_gain*1.0/(2**19))*((signed_tsens_Tref-signed_tsens_nlfit_Toffset)**2)

    int32_t nlfit_corr_tlinearfit_soft_mult128 = compute_nlfit_corr(T_linearfit_soft_mult128);
    int32_t nlfit_corr_tref_soft_mult128       = compute_nlfit_corr(cal->Tref);

    int32_t Test_final_soft_mult128 = T_linearfit_soft_mult128 + nlfit_corr_tlinearfit_soft_mult128 -
                                      nlfit_corr_tref_soft_mult128 - Test_Vbatsumcorr_soft_mult128;

    return Test_final_soft_mult128;
}

static void ADC_ClockPower_Configuration(void)
{
    /* Enable ADC clock */
    CLOCK_EnableClock(kCLOCK_Adc0);

    //CLOCK_EnableClock(kCLOCK_PVT);

    /* Power on the ADC converter. */
    POWER_EnablePD(kPDRUNCFG_PD_LDO_ADC_EN);

    /* Enable the clock. */
    CLOCK_AttachClk(kXTAL32M_to_ADC_CLK);
}

static bool ADC_Configuration(ADC_Type *base)
{
    ADC_ClockPower_Configuration();

    int valid = 1;
#if 1
    /* Configure the converter. */
    adcConfigStruct.clockMode = kADC_ClockAsynchronousMode; /* Using async clock source. */
    adc_clock_src_t adc_clk_src =
        (adc_clock_src_t)((SYSCON->ADCCLKSEL) & SYSCON_ADCCLKSEL_SEL_MASK >> SYSCON_ADCCLKSEL_SEL_SHIFT);
    switch (adc_clk_src) {
    case kCLOCK_AdcXtal32M: {
        CLOCK_SetClkDiv(kCLOCK_DivAdcClk, 8, true);
        break;
    }
    case kCLOCK_AdcFro12M: {
        CLOCK_SetClkDiv(kCLOCK_DivAdcClk, 3, true);
        break;
    }
    case kCLOCK_AdcNoClock:
    default: {
        valid = 0;
        PRINTF("ADC configuration error: no clock selected in asynchronous mode\r\n");
        break;
    }
    }

    if (!valid) {
        return valid;
    }
#endif

    /* Configure the converter. */
    //adcConfigStruct.clockMode = kADC_ClockSynchronousMode; /* Using sync clock source. */
    adcConfigStruct.resolution = kADC_Resolution12bit;
#if defined(FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL) & FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL
    adcConfigStruct.enableBypassCalibration = false;
#endif /* FSL_FEATURE_ADC_HAS_CTRL_BYPASSCAL. */
    adcConfigStruct.sampleTimeNumber = 0U;
    ADC_Init(base, &adcConfigStruct);

    /* Enable conversion in Sequence A. */
    /* Channel to be used is set up at the beginning of each sequence */
    adcConvSeqConfigStruct.channelMask      = 0;
    adcConvSeqConfigStruct.triggerMask      = 0U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = false;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachSequence;
    ADC_SetConvSeqAConfig(base, &adcConvSeqConfigStruct);

    return valid;
}

/* Nb_samples shall be a power of 2. The temperature is reported in 128th degree Celsius.
 * Return value is 1 if temperature is computed else 0 is returned (no valid calibration data).*/
static void get_temperature(ADC_Type *base, uint8_t channel, uint8_t nb_samples, int32_t *temperature)
{
    /**********************************************/
    /*ADCout from temperature sensor input in div1*/
    /**********************************************/
#ifdef DBG_DRIVER
    PRINTF("ADC output code from current temperature\r\n");
#endif
    //Setup the temperature sensor to on
    ADC_EnableTemperatureSensor(base, true);
    //Add some delay to prevent first ADC acquisition before temperature sensor is ready
    CLOCK_uDelay(100);

    //Set test mode=2 (ADC in unity gain) and TSAMP = 0x1f
    update_ctrl0_adc_register(base, 0x2, 0x1f);

    //run a 8 sample acquisition
    /* Enable conversion in Sequence A. */
    /* Channel to be used is set up at the beginning of each sequence */
    adcConvSeqConfigStruct.channelMask      = (1 << channel);
    adcConvSeqConfigStruct.triggerMask      = 2U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = true;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachConversion;
    ADC_SetConvSeqAConfig(base, &adcConvSeqConfigStruct);

    //At that step in CTRl register, TRIGGER=2 (PWM9), TRIGPOL=1, SYNCBYPASS=1, MODE=0
    ADC_EnableConvSeqA(base, true);
    //At that step in CTRl register, SEQ_ENA = 1

    uint16_t tsens_adcout_T_sum8 = compute_estimated_sum(base, nb_samples, channel);

#ifdef DBG_DRIVER
    PRINTF("tsens_adcout_T_sum8 = %d\r\n", tsens_adcout_T_sum8);
    PRINTF("tsens_adcout_T = %d\r\n", tsens_adcout_T_sum8 / nb_samples);
    PRINTF("\r\n");
#endif

    ADC_EnableTemperatureSensor(base, false);
    ADC_EnableConvSeqA(base, false);

    /*************************************/
    /*ADCout from from Vbat input in div4*/
    /*************************************/
#ifdef DBG_DRIVER
    PRINTF("ADC output code from current VBAT\r\n");
#endif

    //Set test mode=0 (ADC in normal mode => /4) and TSAMP = 0x14
    update_ctrl0_adc_register(base, 0x0, 0x14);

    /* Channel to be used is set up at the beginning of each sequence */
    adcConvSeqConfigStruct.channelMask      = (1 << VBAT_CHANNEL);
    adcConvSeqConfigStruct.triggerMask      = 2U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = true;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachConversion;
    ADC_SetConvSeqAConfig(base, &adcConvSeqConfigStruct);

    ADC_EnableConvSeqA(base, true);

    uint16_t adc12b_estimated_sum = compute_estimated_sum(base, nb_samples, VBAT_CHANNEL);

#ifdef DBG_DRIVER
    PRINTF("adcout_vbat_sum = %d\r\n", adc12b_estimated_sum);
    PRINTF("adcout_vbat_lsb = %d\r\n", adc12b_estimated_sum / nb_samples);
#endif

    *temperature = temp_computation(adc12b_estimated_sum, tsens_adcout_T_sum8, nb_samples);
    //*temperature = temp_computation(30740, 20752, nb_samples);
}
#endif /* #if(ADC_TEMP_SENSOR_DRIVER_EN) */

#if gAdcUsed_d
/*
 * ISR for ADC conversion sequence A done.
 */
static bool BOARD_IsADCEnabled(void)
{
    bool result = false;
    if (ADC0->STARTUP & ADC_STARTUP_ADC_ENA_MASK) {
        result = true;
    }
    return result;
}

/*!
 * @brief Set ADC channel.
 *
 * @param number Channel to be set.
 * @param enable Enable or disable the ADC channel.
 */
static void BOARD_ADCChannelSet(uint32_t number, bool enable)
{
    if (enable == true) {
        adcConvSeqConfigStruct.channelMask |= (1 << number);
    }
    else {
        adcConvSeqConfigStruct.channelMask &= ~(1 << number);
    }
    ADC_SetConvSeqAConfig(ADC0, &adcConvSeqConfigStruct);
    ADC_EnableConvSeqA(ADC0, true);
}
#endif

static void FaultRecovery(void)
{
    if (exception_buf != NULL) {
        longjmp(*exception_buf, BUS_EXCEPTION);
    }
    while (1)
        ;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initialize debug console. */
status_t BOARD_InitDebugConsole(void)
{
    status_t result = kStatus_Fail;

#if gUartDebugConsole_d | gUartAppConsole_d
    uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    result = DbgConsole_Init(
        DEBUG_SERIAL_INTERFACE_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
    //BOARD_InitHostInterface();
#endif
    return result;
}

/* DeInitialize debug console. */
status_t BOARD_DeinitDebugConsole(void)
{
    status_t result;
    result = DbgConsole_Deinit();
    return result;
}

#if gAdcUsed_d
uint8_t BOARD_GetBatteryLevel(void)
{
    uint8_t battery_lvl;
    adc_result_info_t adcResultInfoStruct;
    ADC_DBG_LOG("");
    if (!BOARD_IsADCEnabled()) {
        ADC_DBG_LOG("ADC not enabled");
        BOARD_InitAdc();
        CLOCK_uDelay(ADC_WAIT_TIME_US);
    }

    BOARD_ADCChannelSet(ADC_BAT_LEVEL_CHANNEL, true);

    ADC_DoSoftwareTriggerConvSeqA(ADC0);
    while (!ADC_GetChannelConversionResult(ADC0, ADC_BAT_LEVEL_CHANNEL, &adcResultInfoStruct)) {
    }

    BOARD_ADCChannelSet(ADC_BAT_LEVEL_CHANNEL, false);

    /* battery_lvl = ADCoutputData/4095*ADCFullScale/ADCFrontEndGain/FullVoltageRange*100*/
    uint32_t voltageValMv = ADC_TO_MV(adcResultInfoStruct.result);
    battery_lvl           = (uint8_t)ADC_MV_TO_PERCENT(voltageValMv);

    //PRINTF("BOARD_GetBatteryLevel =%d \r\n", battery_lvl);
    ADC_DBG_LOG("Battery level=%d", battery_lvl);

    return battery_lvl;
}

int32_t BOARD_GetTemperature(void)
{
    uint32_t status                     = 0;
    int32_t temperature_in_128th_degree = 0;
    int32_t temp_int_part               = 0;

#if (ADC_TEMP_SENSOR_DRIVER_EN)
    int32_t temp_dec_part = 0;

    status = load_calibration_param_from_flash(ADC0);

    if (status) {
        if (!BOARD_IsADCEnabled()) {
            ADC_DBG_LOG("ADC not enabled");
            BOARD_InitAdc();
            // A problem with the ADC requires a delay after setup, see RFT 1340
            CLOCK_uDelay(ADC_WAIT_TIME_US);
        }

        get_temperature(ADC0, TEMPERATURE_SENSOR_CHANNEL, 8, &temperature_in_128th_degree);

        last_temperature_report_in_128th_of_degree = temperature_in_128th_degree;

        //integer part
        temp_int_part = temperature_in_128th_degree / 128;
        /* Update Temperature in XCVR for calibration purpose */
        //decimal part
        temp_dec_part = (((temperature_in_128th_degree - (temp_int_part * 128))) * 10) / 128;
        PRINTF("Temperature = %d.%d C\r\n", (short int)temp_int_part, (short int)temp_dec_part);
        ADC_DBG_LOG("Temperature=%d.%d", temp_int_part, temp_dec_part);
        //volatile int vole = true;
        //while (vole);
    }
    else {
        ADC_DBG_LOG("Wrong ADC configuration or missing calibration data");
        PRINTF("FAIL: temperature measurement failed due to wrong ADC configuration or missing calibration data!\r\n");
    }
#endif
    return temp_int_part;
}

void BOARD_InitAdc(void)
{
    ADC_DBG_LOG("");
    BOARD_DbgLpIoSet(1, 0);

    if (!BOARD_IsADCEnabled()) {
        ADC_DBG_LOG("ADC not enabled");
        ADC_Configuration(ADC0);

        /* Enabling ADC should be done at least 230us later */
    }
    BOARD_DbgLpIoSet(1, 1);
}

void BOARD_EnableAdc(void)
{
    BOARD_DbgLpIoSet(1, 0);

    /* Make sure 230us has elapsed since BOARD_InitAdc() has been called
     * ADC requires a delay after setup, see RFT 1340 */
    //CLOCK_uDelay(ADC_WAIT_TIME_US);
    ADC_DBG_LOG("");
    ADC_EnableConvSeqA(ADC0, true); /* Enable the conversion sequence A. */

    BOARD_DbgLpIoSet(1, 1);
}

void BOARD_DeInitAdc(void)
{
    ADC_DBG_LOG("");
    /* Power off the ADC converter. */
    POWER_DisablePD(kPDRUNCFG_PD_LDO_ADC_EN);
}
#endif

uint32_t BOARD_GetUsartClock(int8_t instance)
{
    uint32_t ret = -1;
    do {
        if (instance >= FSL_FEATURE_SOC_USART_COUNT)
            break;
#if gUartDebugConsole_d
        if (instance == DEBUG_SERIAL_INTERFACE_INSTANCE) {
            ret = BOARD_DEBUG_UART_CLK_FREQ;
            break;
        }
#endif
#if gUartAppConsole_d
        if (instance == APP_SERIAL_INTERFACE_INSTANCE) {
            ret = BOARD_APP_UART_CLK_FREQ;
        }
#endif
    } while (0);
    return ret;
}

uint32_t BOARD_GetCtimerClock(CTIMER_Type *timer)
{
    return CLOCK_GetApbCLkFreq();
}

uint16_t BOARD_GetPotentiometerLevel(void)
{
    uint32_t random = 0;
    RNG_GetRandomNo(&random);

    return (uint16_t)(random & 0x0FFF);
}

void BOARD_SetFaultBehaviour(void)
{
    /* In the absence of a proper setting in SHCSR, all faults go to a HardFault
     * Since we want a finer discrimination for BusFaults in order to catch Faults when
     * accessing the Flash on pages in error, lets set  BUSFAULTENA.
     * The other may turnout usefull later
     */
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
    // SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk ;
    // SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk ;
}

__attribute__((section(".after_vectors"))) void BusFault_Handler(void)
{
    /* Retrieve the stack pointer */
    /* At this point LR contains the EXC_RETURN whose value tells
     * which stack pointer to restore : MSP Main Stack pointer or
     * PSP Process Stack pointer*/
    asm volatile(
        "TST   lr, #0x4\n"
        "ITE   EQ\n"
        "MRSEQ r0, MSP\n"
        "MRSNE r0, PSP\n"
        :
        :
        : "r0");

    /* Set the exception return address to recovery function
     * Force execution of the Fault Recovery function and patch the LR to be reloaded
     * in the stack frame.
     * As the exception fired SP got decreased by 8 32 bit words as it stored:
     * PSR, PC, LR, R13, R3-R0
     * From the stack bottom LR is to be found at offset 6 * 4 i.e. 24, still here we have
     * gone deeper in the stack by 1 more word becasue r7 gets  pushed on the stack too
     *  thence the 28 offset
     * */
    asm volatile(
        "MOV   r1, %0\n"
        "STR   r1, [r0, #28]\n"
        :
        : "r"(FaultRecovery)
        : "r0", "r1");
}

uint32_t BOARD_GetSpiClock(uint32_t instance)
{
    panic(0, 0, 0, 0);
    return 0;
}

#if gDbgUseDbgIos
static void BOARD_DbgSetIo(const gpioOutputPinConfig_t *cfg_array, int pin_id, int val)
{
    const gpioOutputPinConfig_t *pin_def = NULL;
    pin_def                              = &cfg_array[pin_id];

    if (pin_def) {
        if (val != 0)
            GpioSetPinOutput(pin_def);
        else
            GpioClearPinOutput(pin_def);
    }
}
#endif

void BOARD_DbgLpIoSet(int pinid, int val)
{
#if (gDbgIoCfg_c == 1)
    BOARD_DbgSetIo(dk6_dbg_io_pins, pinid, val);
#else
    NOT_USED(pinid);
    NOT_USED(val);
#endif
}

void BOARD_DbgIoSet(int pinid, int val)
{
#if (gDbgIoCfg_c == 2)
    BOARD_DbgSetIo(dk6_dbg_io_pins, pinid, val);
#else
    NOT_USED(pinid);
    NOT_USED(val);
#endif
}

/* Required for backward compatibility with previous link controller libraries
 * should be deprecated
*/
void BOARD_DbgSetIoUp(int pin)
{
    BOARD_DbgLpIoSet(pin, 1);
}

void BOARD_DbgSetIoDown(int pin)
{
    BOARD_DbgLpIoSet(pin, 0);
}

#if gDbgUseLLDiagPort
/* BOARD_DIAG_PORT_MODE 32 bit word is used to set up to 4 DBG_FUNC modes at a time.
 * It is used to set the Link Layer DIAGCTRL register telling it to output debug
 * information of IOs.
 * The value of each 8-bit tranche composing this 32-bit word is the configuration corresponds
 * to a DIAG mode: DIAG0 and DIAG2 can be muxed towards PIO0..PIO7 port whereas DIAG1..DIAG3
 * are routed to PIO8..PIO15 if configured.
 * The IOCON configuration allows the selection.
 * Beware PIO[8:9] are USART0 so might be lost
 * Beware PIO[10:11] are USART1 so might be lost too, also these 2 are I2C specific IOs
 * Beware PIO[12:13] are SWD so is very painful to lose
 * For each PIO in the IOCON, DBG_FUNC is coded on 4 bits however only values 1 and 2 make sense for
 * SW control of LL diagnostic traces. The DIAG port consists of 16 outputs, so let's consider a 2 bit code to say:
 *   - 0 means not a debug pin,
 *   - 1 routes DIAG0 signals to PIO [0..7] and DIAG1 ones to PIO[8..15]
 *   - 2 routes DIAG2 signals to PIO [0..7] and DIAG3 ones to PIO[8..15]
 *   Combining 16 of these 2 bit codes fits in a single 32bit word.
 * In order to mux to PIO[0..7] signals from either DIAG0 or DIAG2, configure DIAG_PIN_DBGCFG(pin, 1 or 2)
 * likewise the muxing to PIO[8..15] from DIAG1 or DIAG3 can be configured using  DIAG_PIN_DBGCFG(pin, 1 or 2)
 * Note that in order to preserve the debug UART functionality you may wish to prevent routing of LL DIAG signals
 * to pins 8 and 9. This is done using by configuring DIAG_PIN_DBGCFG(8, 0) since USART0 Tx is pin 8 with pin mode #2.
 * We could move USART0 to pin18 and 19 if we configured them in mode #5 thus potentially freeing pin 8 and 9 for LL DIAG.
 * The same kind of issue exists for pins 10 - 11 that are USART1's default.
 * Be warned that SWD interface makes use of pins 12 and 13. Setting the pinmux to make them belong to the DIAG port,
 * prevents the use of a debugger.
 * If we need to manually set a mix of groups
 * Diag 0 & Diag 2 -
 * Use the DIAG_PIN_DBGCFG macro below to select the pins that are to be directed to the DIAG port.
 * DIAG_PIN_DBGCFG(pin, 1) maps a Diag0 or Diag1 signal to IO pin
 * DIAG_PIN_DBGCFG(pin, 2) to map a Diag2 or Diag3 signal to IO the IO pin
 * For instance:
 *        GPIO 0:3    DIAGCNTL 0x03 TX,RX, sync win, sync found
 *        GPIO 4:6    DIAGCNTL 0x0B  exchange memory accesses
 *        GPIO 7      DIAGCNTL 0x03  ble_error_irq
 *        GPIO 11:12  DIAGCNTL 0x07 625us, prefetch
 *        GPIO 15     DIAGCNTL 0x1F event_in_process
 * This results in BOARD_DIAG_PORT_MODE defined as  0x9F8B8783
 *
 * #define DIAG_IOCON_SETTING \
 *       (DIAG_PIN_DBGCFG(0, 1) |\
 *        DIAG_PIN_DBGCFG(1, 1) |\
 *        DIAG_PIN_DBGCFG(2, 1) |\
 *        DIAG_PIN_DBGCFG(3, 1) |\
 *        DIAG_PIN_DBGCFG(4, 2) |\
 *        DIAG_PIN_DBGCFG(5, 2) |\
 *        DIAG_PIN_DBGCFG(6, 2) |\
 *        DIAG_PIN_DBGCFG(7, 1) |\
 *        DIAG_PIN_DBGCFG(8, 0) |\
 *        DIAG_PIN_DBGCFG(9, 0) |\
 *        DIAG_PIN_DBGCFG(10, 0) |\
 *      DIAG_PIN_DBGCFG(11, 1) |\ <- the breaks the USART1
 *      DIAG_PIN_DBGCFG(12, 1) |\ <- note that this setting ruins the SWD usage
 *        DIAG_PIN_DBGCFG(15, 2))
 *
**/

#define DIAG_PIN_DBGCFG(pin, cfg) (((cfg)&0x3) << (pin * 2))

#define DIAG_IOCON_SETTING                                                                                \
    (DIAG_PIN_DBGCFG(0, 1) | DIAG_PIN_DBGCFG(1, 1) | DIAG_PIN_DBGCFG(2, 1) | DIAG_PIN_DBGCFG(3, 1) |      \
        DIAG_PIN_DBGCFG(4, 1) | DIAG_PIN_DBGCFG(5, 1) | DIAG_PIN_DBGCFG(6, 1) | DIAG_PIN_DBGCFG(7, 1) |   \
        DIAG_PIN_DBGCFG(8, 0) | DIAG_PIN_DBGCFG(9, 0) | DIAG_PIN_DBGCFG(10, 0) | DIAG_PIN_DBGCFG(11, 0) | \
        DIAG_PIN_DBGCFG(12, 0) | DIAG_PIN_DBGCFG(13, 0) | DIAG_PIN_DBGCFG(14, 1) | DIAG_PIN_DBGCFG(15, 1))

void BOARD_DbgDiagIoConf(void)
{
    int i;
    uint32_t diag_iocon_val = DIAG_IOCON_SETTING;

    uint32_t dbg_func;
    uint32_t val;

    /* PIO[0..7] settings */
    if (!(BOARD_DIAG_PORT_MODE & gDbgLLDiagPort0Msk))
        diag_iocon_val &= 0xffff0000;
    /* PIO[8..15] settings */
    if (!(BOARD_DIAG_PORT_MODE & gDbgLLDiagPort1Msk))
        diag_iocon_val &= 0x0000ffff;

    for (i = 0; i < 16; i++) {
        dbg_func = diag_iocon_val & 0x3;
        if (dbg_func == 1 || dbg_func == 2) {
            /* pins 10 and 11 are I2C capable and have a special programming */
            val              = (i == 10 || i == 11) ? IOCON_I2C_CFG(dbg_func) : IOCON_CFG(dbg_func);
            IOCON->PIO[0][i] = val;
        }
        else {
            val = IOCON->PIO[0][i];
            /* pins 10 and 11 are I2C capable and have a special programming */
            if (i == 10 || i == 11)
                val &= ~(uint32_t)IOCON_PIO_I2C_DBG_MODE_MASK;
            else
                val &= ~(uint32_t)IOCON_PIO_DBG_MODE_MASK;
            IOCON->PIO[0][i] = val;
        }
        diag_iocon_val >>= 2;
    }
}
#define BLE_BASE_ADDR 0x400A0000

#define REG_BLE_RD(offs)      (*(volatile uint32_t *)(BLE_BASE_ADDR + (offs)))
#define REG_BLE_WR(offs, val) (*(volatile uint32_t *)(BLE_BASE_ADDR + (offs))) = (val);

#define BLE_DIAGCNTL_OFFSET 0x00000050

void BOARD_DbgDiagEnable(void)
{
    REG_BLE_WR(BLE_DIAGCNTL_OFFSET, BOARD_DIAG_PORT_MODE);
}
#else
void BOARD_DbgDiagIoConf(void)
{
}
void BOARD_DbgDiagEnable(void)
{
}
#endif

void BOARD_InitPMod_SPI_I2C(void)
{
    panic(0, 0, 0, 0);
}

void BOARD_InitSPI(void)
{
    panic(0, 0, 0, 0);
}

#if gEepromType_d == gEepromDevice_MX25R8035F_c
void BOARD_InitSPIFI(void)
{
    uint32_t divisor;

    BOARD_InitSPIFIPins();
    RESET_SetPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
    /* Enable clock to block and set divider */
    CLOCK_AttachClk(kMAIN_CLK_to_SPIFI);
    divisor = CLOCK_GetSpifiClkFreq() / BOARD_SPIFI_CLK_RATE;
    CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, divisor ? divisor : 1, false);
    RESET_ClearPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
}
#endif

#if gDbgUseDbgIos
void BOARD_InitDbgIo(void)
{
    const gpioOutputPinConfig_t *pinArray = &dk6_dbg_io_pins[0];
    const iocon_group_t *p_iocon          = &dk6_dbg_io[0];
    uint32_t nb_dbg_io                    = sizeof(dk6_dbg_io) / sizeof(iocon_group_t);
    gpio_pin_config_t dk6_output          = {.pinDirection = kGPIO_DigitalOutput};
    for (uint32_t i = 0; i < nb_dbg_io; i++) {
        dk6_output.outputLogic = pinArray->outputLogic;
        IOCON_PinMuxSet(IOCON, p_iocon->port, p_iocon->pin, p_iocon->modefunc);

        GPIO_PinInit(GPIO, pinArray->gpioPort, pinArray->gpioPin, &dk6_output);
        pinArray++;
        p_iocon++;
    }
    GpioOutputPinInit(dk6_dbg_io_pins, nb_dbg_io);

    // Toggle all Dbg IOs
    for (int i = 0; i < nb_dbg_io; i++) {
        BOARD_DbgSetIo(dk6_dbg_io_pins, i, 0);
        BOARD_DbgSetIo(dk6_dbg_io_pins, i, 1);
        BOARD_DbgSetIo(dk6_dbg_io_pins, i, 0);
    }

    /* To output RFTX and RFRX on 20 & 21 */
    //    IOCON -> PIO[0][20] = 0x85;
    //    IOCON -> PIO[0][21] = 0x85;

    /* To output Clock on 17 */
    //    SYSCON -> CLKOUTSEL = SYSCON_CLKOUTSEL_SEL(0); // MAINCLK output to CLK_OUT
    //    SYSCON -> CLKOUTDIV = SYSCON_CLKOUTDIV_DIV(3) | SYSCON_CLKOUTDIV_HALT(0);
    //    IOCON -> PIO[0][17] =  0x3BD;
}
#endif
#if (gKBD_KeysCount_c > 0)

void BOARD_UnInitButtons(void)
{
    INPUTMUX_Init(INPUTMUX);
    for (int i = 0; i < sizeof(dk6_button_io_pins) / sizeof(gpioInputPinConfig_t); i++) {
        INPUTMUX_AttachSignal(INPUTMUX,
            dk6_button_io_pins[i].pinIntSelect,
            (inputmux_connection_t)INPUTMUX_GpioPortPinToPintsel(0, 0x1f));
        PINT_PinInterruptConfig(PINT, dk6_button_io_pins[i].pinIntSelect, (pint_pin_enable_t)pinInt_Disabled_c, NULL);
    }
    INPUTMUX_Deinit(INPUTMUX); /* Can be gated now because used only for changes */
}

void BOARD_SetButtons_LowPowerEnter(void)
{
    const iocon_group_t *pinArray = &dk6_buttons_io[0];
    int nb_buttons                = sizeof(dk6_buttons_io) / sizeof(iocon_group_t);
    for (int i = 0; i < nb_buttons; i++) {
        IOCON_PinMuxSet(IOCON, pinArray->port, pinArray->pin, IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN);
        GPIO_PinInit(GPIO, pinArray->port, pinArray->pin, &((const gpio_pin_config_t){kGPIO_DigitalInput, 1}));
        pinArray++;
    }
}
#endif

#if (gLEDsOnTargetBoardCnt_c > 0)
void BOARD_SetLEDs_LowPower(void)
{
    const iocon_group_t *pinArray = &dk6_leds_io[0];
    uint32_t nb_leds              = sizeof(dk6_leds_io) / sizeof(iocon_group_t);
    for (uint32_t i = 0; i < nb_leds; i++) {
        if (IS_DEBUG_SESSION && pinArray->pin == SWO_PIN) {
            // SWO pin is shared with a LED. LED functionality disabled in debug session
            continue;
        }

        IOCON_PinMuxSet(IOCON, pinArray->port, pinArray->pin, IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
        GPIO_PinInit(GPIO, pinArray->port, pinArray->pin, &((const gpio_pin_config_t){kGPIO_DigitalInput, 1}));
        pinArray++;
    }
}
#endif

/* This API shall be called with the proper Voltage setting on the LDO PMU, LDO CORE and LDO MEM
 * For instance, if the frequency is increased to 1.1V, then the voltage of these LDOs shall be set to 1.1v */

void BOARD_CpuClockUpdate(void)
{
#if gPWR_CpuClk_48MHz
    /* Selection to 48MHz FRO */
    FLASH_SetReadMode(FLASH, true);
#else
    /* Selection to 32MHz XO or FRO*/
    FLASH_SetReadMode(FLASH, false);

#if (BOARD_TARGET_CPU_FREQ == BOARD_MAINCLK_XTAL32M)
    while ((ASYNC_SYSCON->RADIOSTATUS & ASYNC_SYSCON_RADIOSTATUS_PLLXOREADY_MASK) == 0) {
    }
#endif
#endif

    SYSCON->MAINCLKSEL = SYSCON_MAINCLKSEL_SEL(BOARD_TARGET_CPU_FREQ);
}

#if gLoggingActive_d || gRtcLockupWorkAround

void Dbg_Start32kCounter(void)
{
    WTIMER_Init();
    WTIMER_StartTimer(WTIMER_TIMER0_ID, ~0UL);
}

uint32_t Dbg_Get32kTimestamp(void)
{
    return WTIMER_ReadTimer(WTIMER_TIMER0_ID);
}

void Dbg_Stop32kCounter(void)
{
    WTIMER_StopTimer(WTIMER_TIMER0_ID);
    WTIMER_DeInit();
}

#endif

#if gDbgUseDbgIos

void force_all_dbg_io_high(void)
{
    for (int i = 0; i < 5; i++) {
        BOARD_DbgLpIoSet(i, 1);
    }
}
/* Function that gives a pattern to be easily recognized when called */
void toggle_io_debug(void)
{
    /* CPU active (except IRQ) */
    BOARD_DbgLpIoSet(0, 0);
    BOARD_DbgLpIoSet(0, 1);

    /* power down */
    BOARD_DbgLpIoSet(1, 0);
    BOARD_DbgLpIoSet(1, 1);

    /* LL IRQ */
    BOARD_DbgLpIoSet(2, 0);
    BOARD_DbgLpIoSet(2, 1);
    BOARD_DbgLpIoSet(2, 0);

    /* LL IRQ in sleep if LL irq down,  with error if LL irq is up */
    BOARD_DbgLpIoSet(3, 0);
    BOARD_DbgLpIoSet(3, 1);
    BOARD_DbgLpIoSet(3, 0);

    /* wakeup */
    BOARD_DbgLpIoSet(4, 0);
    BOARD_DbgLpIoSet(4, 1);
}
#endif

#if gXo32M_Trim_c
static void BOARD_tcxo32M_compensation_run(uint32_t temperature_change_threshold, uint32_t us_delay_post_tcxo_adj)
{
    static int32_t prev_compensation_temperature_in_celsius = TEMP_ZERO_K;
    int32_t XO_32M_OSC_CAP_Delta_x1000                      = 0;
    int32_t latest_temperature_in_celsius;
    int32_t delta = 0;

    latest_temperature_in_celsius = last_temperature_report_in_128th_of_degree / 128;

    if (latest_temperature_in_celsius == TEMP_ZERO_K) /* Temperature never monitored yet */
    {
        latest_temperature_in_celsius = 25;
    }
    else {
        delta = latest_temperature_in_celsius - prev_compensation_temperature_in_celsius;
        delta = ABSOLUTE_VALUE(delta);
    }
    if (delta >= temperature_change_threshold) {
        prev_compensation_temperature_in_celsius = latest_temperature_in_celsius;
        XO_32M_OSC_CAP_Delta_x1000               = Calculate_32MOscCapCompensation(latest_temperature_in_celsius);

        CLOCK_Xtal32M_Trim(XO_32M_OSC_CAP_Delta_x1000, &BOARD_Clock32MCapacitanceCharacteristics);

        CLOCK_uDelay(us_delay_post_tcxo_adj); /* delay to be determined but should be fast */
    }
}
#endif

#if gXo32k_Trim_c
static void BOARD_tcxo32k_compensation_run(uint32_t temperature_change_threshold, uint32_t us_delay_post_tcxo_adj)
{
    static int32_t prev_compensation_temperature_in_celsius = TEMP_ZERO_K;
    int32_t OSC_CAP_Delta_x1000                             = 0;
    int32_t latest_temperature_in_celsius;
    int32_t delta = 0;

    latest_temperature_in_celsius = last_temperature_report_in_128th_of_degree / 128;

    if (latest_temperature_in_celsius == TEMP_ZERO_K) /* Temperature never monitored yet */
    {
        latest_temperature_in_celsius = 25;
    }
    else {
        delta = latest_temperature_in_celsius - prev_compensation_temperature_in_celsius;
        delta = ASBOLUTE_VALUE(delta);
    }
    if (delta >= temperature_change_threshold) {
        prev_compensation_temperature_in_celsius = latest_temperature_in_celsius;
        xo_32k_osc_cap_delta_fF                  = Calculate_32kOscCapCompensation(latest_temperature_in_celsius);

        CLOCK_Xtal32k_Trim(xo_32k_osc_cap_delta_fF, &BOARD_Clock32kCapacitanceCharacteristics);

        CLOCK_uDelay(us_delay_post_tcxo_adj); /* delay to be determined but should be fast */
    }
}
#endif

void BOARD_common_hw_init(void)
{
    reset_cause_t reset_cause;
    /* Switch to FRO32M to speed up initialization -
     * careful : XTAL32M may not be ready */

    SYSCON->MAINCLKSEL = BOARD_MAINCLK_FRO32M;

    /* MODEM master priority = 3 – highest */
    SYSCON->AHBMATPRIO = 0x00000300;

    /* Security code to allow debug access */
    SYSCON->CODESECURITYPROT = 0x87654320;

    BOARD_SetFaultBehaviour();

#if gOTA_externalFlash_d
    /* Allows to write in the qspi in each 256 blocks */
    SYSCON->MEMORYREMAP = (SYSCON->MEMORYREMAP & 0xfffff) | 0xe400000;
#endif

    /* Enable APB bridge access */
    CLOCK_EnableAPBBridge();
#if gLoggingActive_d
    /* Need to start 32k clock before starting Logging service because of WTIMER timestamp */
    CLOCK_EnableClock(CLOCK_32k_source);
    //    DbgLogInit(RAM2_BASE, RAM2_SIZE);
    DbgLogInit(RAM2_BASE + gLogRingPlacementOffset_c, RAM2_SIZE - gLogRingPlacementOffset_c);

#elif gRtcLockupWorkAround
    CLOCK_EnableClock(CLOCK_32k_source);
    Dbg_Start32kCounter();
    Dbg_Stop32kCounter();
#endif

#if gDbgUseDbgIos
    BOARD_InitDbgIo();
    //force_all_dbg_io_high();
    toggle_io_debug();
#endif
    /* check if we come from power down */
    reset_cause = POWER_GetResetCause();
    INIT_DBG_LOG("Reset Cause=%x PMC reset cause:%x", reset_cause, PMC->RESETCAUSE);

#ifdef DUAL_MODE_APP
    /* Configure the optimized active voltages, set up the BODs */
    POWER_Init();
#endif
#ifdef FOR_BOD_DEBUG
    /* Set up the BODs
     * Note : This function shall be safe in term of LDO voltage - no Voltage scaling yet */
    POWER_BodSetUp();
#endif

    /* Need to be done only at power On
     * Configure the trimmed default active voltages */
    if (reset_cause != RESET_WAKE_PD) {
        POWER_SetTrimDefaultActiveVoltage();
    }

    /* TODO : Trim and bias the XTAL32M on Radio : do we need to be sure XTAL is ready first ? */
    BOARD_DbgLpIoSet(1, 0);
#if gXo32M_Trim_c
    BOARD_tcxo32M_compensation_run(0, 0);
#else
    CLOCK_XtalBasicTrim();
#endif

#if gXo32k_Trim_c
    BOARD_tcxo32k_compensation_run(0, 0);
#endif
#ifndef DUAL_MODE_APP
    vRadio_SkipXTALInit();
    vRadio_DisableZBRadio();
#endif
    vRadio_ActivateXtal32MRadioBiasing();
    BOARD_DbgLpIoSet(1, 1);

#ifdef FOR_BOD_DEBUG
    /* need to wait for 27us for the LDO BOD core to set up (Done during POWER_BodSetUp()) - so do it after XTAL trimming/biasing */
    POWER_BodActivate();
#endif

    /* It is mandatory to reset FLEXCOM peripherals because FLEXCOM is not reset on wakeup from power down
     * Except if FLEXCOM is retained during power down mode
     * No need to reset other blocks, these are already reset by Hardware */
    if ((reset_cause != RESET_WAKE_PD) || !(PMC->PDSLEEPCFG & PMC_PDSLEEPCFG_PDEN_PD_COMM0_MASK)) {
        RESET_PeripheralReset(kUSART0_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kI2C0_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kSPI0_RST_SHIFT_RSTn);
#ifdef DUAL_MODE_APP
        RESET_PeripheralReset(kGPIO0_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kADC0_RST_SHIFT_RSTn);
#endif
    }

    /* For power optimization, we need a limited clock enabling specific to this application
     * This replaces BOARD_BootClockRUN() usually called in other application    */
    BOARD_InitClocks();
    BOARD_SetPinsForRunMode();

#if gUartDebugConsole_d
    BOARD_InitDebugConsole();
#endif

#if gDbgUseLLDiagPort
    BOARD_DbgDiagIoConf();
    BOARD_DbgDiagEnable();
#endif

    /* remove comment to keep Debug port IOs on wake up from power down
     * Note : this will disable the button after power down   */
#if (BOARD_DIAG_PORT_MODE & gDbgLLDiagPort0Msk)
    if ((reset_cause == RESET_WAKE_PD)) {
#if gKeyBoardSupported_d
        KBD_ShutOff();
#endif
    }
#endif

    /* Switch to target frequency set by application
     * Configure correctly the Flash wait state  */
    BOARD_CpuClockUpdate();

#if gPWR_LdoVoltDynamic
    /* now revert to 1.0V since the current peak is done */
    POWER_ApplyLdoActiveVoltage(PM_LDO_VOLT_1_0V);
#endif

    /* Always call this in case CPU frequency was updated during/before hardware_init() */
    SystemCoreClockUpdate();
}

#if gUartDebugConsole_d
void BOARD_InitDEBUG_UART(void)
{
    BOARD_InitSwoPins();
}

void BOARD_SetDEBUG_UART_LowPower(void)
{
    BOARD_DeInitDebugUARTPins();
    BOARD_DeinitDebugConsole();
}
#endif

#if gUartAppConsole_d

extern const iocon_group_t dk6_hostif_usart_io[2];

void BOARD_InitHostInterface(void)
{
    /* For now let's deal with USART only */
    /* USART RX/TX pin */
    BOARD_InitAppUARTPins();
}

void BOARD_SetHostIf_UART_LowPower(void)
{
    /* Reconfigure USART RX/TX pins */
    BOARD_DeInitAppUARTPins();
}

#endif

bool BOARD_CheckSwdAttached(void)
{
    const uint32_t gpioCfg = (/* Pin is configured as SWO */
        IOCON_PIO_FUNC0 |
        /* Selects pull-up function */
        IOCON_PIO_MODE_PULLDOWN |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW0_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Input filter disabled */
        IOCON_PIO_INPFILT_OFF |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW1_STANDARD |
        /* Open drain is disabled */
        IOCON_PIO_OPENDRAIN_DI |
        /* SSEL is disabled */
        IOCON_PIO_SSEL_DI);

    const uint32_t swdioCfg = (/* Pin is configured as SWO */

        IOCON_PIO_FUNC2 |
        /* No addition pin function */
        IOCON_PIO_MODE_INACT |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW0_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Input filter disabled */
        IOCON_PIO_INPFILT_OFF |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW1_STANDARD |
        /* Open drain is disabled */
        IOCON_PIO_OPENDRAIN_DI |
        /* SSEL is disabled */
        IOCON_PIO_SSEL_DI);

    gpio_pin_config_t pinGpioCfg = {
        kGPIO_DigitalInput,
        0,
    };

    IOCON_PinMuxSet(IOCON, 0, 13, gpioCfg);
    GPIO_PinInit(GPIO, 0, 13, &pinGpioCfg);
    bool attached = GPIO_PinRead(GPIO, 0, 13);
    IOCON_PinMuxSet(IOCON, 0, 13, swdioCfg);

    return attached;
}

void BOARD_BatteryLevelPinInit(void)
{
    /* Set up the Analog input */
    IOCON_PinMuxSet(IOCON, 0, gADC0BatLevelInputPin, IOCON_ANALOG_EN);
}

void BOARD_SetPinsForRunMode(void)
{
    SYSCON->RETENTIONCTRL &= ~SYSCON_RETENTIONCTRL_IOCLAMP_MASK;

    INPUTMUX_Init(INPUTMUX);
    PINT_Init(PINT);
    GPIO_PortInit(GPIO, 0);

    BOARD_InitSwoPins();
    LED_Init();
    BOARD_InitHeliosPins();
    UWB_GpioInit();
}

void BOARD_SetPinsForPowerDown(void)
{
#if gUartDebugConsole_d
    BOARD_SetDEBUG_UART_LowPower();
#endif
#if gUartAppConsole_d
    BOARD_SetHostIf_UART_LowPower();
#endif
#if (gLEDsOnTargetBoardCnt_c > 0)
    LED_SetRgbLed(LED_RGB, 0x00, 0x00, 0x00);
    BOARD_SetLEDs_LowPower(); /* Set outputs as inputs to stop current output */
#endif
#if (gKBD_KeysCount_c > 0)
    BOARD_SetButtons_LowPowerEnter(); /* already inputs could do nothing */
#endif

    uint8_t pullupPins[]    = {0, 16, 19, 20, 21};
    uint8_t pulldownPins[]  = {9, 17};
    uint8_t openDrainPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 14, 15};

    const uint32_t openDrain = (
        /* pin as GPIO */
        IOCON_FUNC0 |
        /* internal pull resistor inactive*/
        IOCON_MODE_INACT |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW0_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Input filter disabled */
        IOCON_PIO_INPFILT_OFF |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW1_STANDARD |
        /* Enable open drain */
        IOCON_OPENDRAIN_EN |
        /* SSEL is disabled */
        IOCON_PIO_SSEL_DI);

    const uint32_t pullDown = (
        /* pin as GPIO */
        IOCON_FUNC0 |
        /* pull up */
        IOCON_MODE_PULLDOWN |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW0_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Input filter disabled */
        IOCON_PIO_INPFILT_OFF |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW1_STANDARD |
        /* SSEL is disabled */
        /*IOCON_PIO_SSEL_DI*/
        IOCON_PIO_SSEL_DI);

    const uint32_t pullUp = (
        /* pin as GPIO */
        IOCON_FUNC0 |
        /* pull up */
        IOCON_MODE_PULLUP |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW0_STANDARD |
        /* Input function is not inverted */
        IOCON_PIO_INV_DI |
        /* Enables digital function */
        IOCON_PIO_DIGITAL_EN |
        /* Input filter disabled */
        IOCON_PIO_INPFILT_OFF |
        /* Standard mode, output slew rate control is disabled */
        IOCON_PIO_SLEW1_STANDARD |
        /* SSEL is disabled */
        /*IOCON_PIO_SSEL_DI*/
        IOCON_PIO_SSEL_DI);

    for (int i = 0; i < sizeof(openDrainPins) / sizeof(openDrainPins[0]); i++) {
        IOCON_PinMuxSet(IOCON, 0, openDrainPins[i], openDrain);
    }

    for (int i = 0; i < sizeof(pullupPins) / sizeof(pullupPins[0]); i++) {
        IOCON_PinMuxSet(IOCON, 0, pullupPins[i], pullUp);
    }

    for (int i = 0; i < sizeof(pulldownPins) / sizeof(pulldownPins[0]); i++) {
        IOCON_PinMuxSet(IOCON, 0, pulldownPins[i], pullDown);
    }

    SYSCON->RETENTIONCTRL |= SYSCON_RETENTIONCTRL_IOCLAMP_MASK;
}

void board_specific_action_on_idle(void)
{
#if gOtaEepromPostedOperations_d
    /* Checks if any Eeprom Write transactions are pending and resume them if the
     * status is no busy */
    OTA_TransactionResume();
#endif
#if ((defined gTcxo32k_ModeEn_c) && (gTcxo32k_ModeEn_c != 0))
    BOARD_tcxo32k_compensation_run(2, 0);
#endif
#if ((defined gTcxo32M_ModeEn_c) && (gTcxo32M_ModeEn_c != 0))
    BOARD_tcxo32M_compensation_run(2, 10); /* 2 degrees - wait for 10us */
#endif
}
#if 0
#define NumberOfElements(x) (sizeof(x) / sizeof((x)[0]))
typedef struct {
    const char * reg_name;
    const volatile uint32_t *reg_addr;
} reg_desc_t;

static const reg_desc_t RtcDesc[] = {
    {"CTRL",  &RTC->CTRL },
    {"MATCH", &RTC->MATCH },
    {"COUNT", &RTC->COUNT },
    {"WAKE",  &RTC->WAKE },
};

void dump_rtc(void)
{
    PRINTF("RTC\r\n");
    for (int i = 0; i < NumberOfElements(RtcDesc); i++)
     {
         PRINTF("\t%s = %x\r\n", RtcDesc[i].reg_name, *RtcDesc[i].reg_addr);
     }
}
static const reg_desc_t Async_SysconDesc[] = {
    {"ASYNCPRESETCTRL",        &ASYNC_SYSCON->ASYNCPRESETCTRL },
    {"ASYNCAPBCLKCTRL",        &ASYNC_SYSCON->ASYNCAPBCLKCTRL },
    {"ASYNCAPBCLKSELA",        &ASYNC_SYSCON->ASYNCAPBCLKSELA },
    {"ASYNCAPBCLKDIV",         &ASYNC_SYSCON->ASYNCAPBCLKDIV },
    {"ASYNCCLKOVERRIDE",       &ASYNC_SYSCON->ASYNCCLKOVERRIDE },
    {"TEMPSENSORCTRL",         &ASYNC_SYSCON->TEMPSENSORCTRL },
    {"NFCTAGPADSCTRL",         &ASYNC_SYSCON->NFCTAGPADSCTRL },
    {"XTAL32MLDOCTRL",         &ASYNC_SYSCON->XTAL32MLDOCTRL },
    {"XTAL32MCTRL",            &ASYNC_SYSCON->XTAL32MCTRL },
    {"ANALOGID",               &ASYNC_SYSCON->ANALOGID },
    {"RADIOSTATUS",            &ASYNC_SYSCON->RADIOSTATUS },
    {"DIGITALSTATUS",          &ASYNC_SYSCON->DIGITALSTATUS },
    {"DCBUSCTRL",              &ASYNC_SYSCON->DCBUSCTRL },
    {"FREQMECTRL",             &ASYNC_SYSCON->FREQMECTRL },
    {"NFCTAGINTSTATUS",        &ASYNC_SYSCON->NFCTAGINTSTATUS },
    {"NFCTAG_VDD",             &ASYNC_SYSCON->NFCTAG_VDD },
};

void dump_async_syscon(void)
{
    PRINTF("ASYNC_SYSCON\r\n");
    for (int i = 0; i < NumberOfElements(Async_SysconDesc); i++)
     {
         PRINTF("\t%s = %x\r\n", Async_SysconDesc[i].reg_name, *Async_SysconDesc[i].reg_addr);
     }
}



static const reg_desc_t SysconDesc[] = {
    {"MEMORYREMAP",       &SYSCON->MEMORYREMAP },
    {"AHBMATPRIO",        &SYSCON->AHBMATPRIO },
    {"BUFFERINGAHB2VPB0", &SYSCON->BUFFERINGAHB2VPB[0] },
    {"BUFFERINGAHB2VPB1", &SYSCON->BUFFERINGAHB2VPB[1] },
    {"SYSTCKCAL",         &SYSCON->SYSTCKCAL },
    {"NMISRC",            &SYSCON->NMISRC },
    {"ASYNCAPBCTRL",      &SYSCON->ASYNCAPBCTRL },
    {"PRESETCTRL0",       &SYSCON->PRESETCTRL[0] },
    {"PRESETCTRL1",       &SYSCON->PRESETCTRL[1] },
    {"AHBCLKCTRL0",       &SYSCON->AHBCLKCTRL[0] },
    {"AHBCLKCTRL1",       &SYSCON->AHBCLKCTRL[1] },
    {"MAINCLKSEL",        &SYSCON->MAINCLKSEL },
    {"OSC32CLKSEL",       &SYSCON->OSC32CLKSEL },
    {"CLKOUTSEL",         &SYSCON->CLKOUTSEL },
    {"SPIFICLKSEL",       &SYSCON->SPIFICLKSEL },
    {"ADCCLKSEL",         &SYSCON->ADCCLKSEL },
    {"USARTCLKSEL",       &SYSCON->USARTCLKSEL },
    {"I2CCLKSEL",         &SYSCON->I2CCLKSEL },
    {"SPICLKSEL",         &SYSCON->SPICLKSEL },
    {"IRCLKSEL",          &SYSCON->IRCLKSEL },
    {"PWMCLKSEL",         &SYSCON->PWMCLKSEL },
    {"WDTCLKSEL",         &SYSCON->WDTCLKSEL },
    {"MODEMCLKSEL",       &SYSCON->MODEMCLKSEL },
    {"FRGCLKSEL",         &SYSCON->FRGCLKSEL },
    {"DMICCLKSEL",        &SYSCON->DMICCLKSEL },
    {"WKTCLKSEL",         &SYSCON->WKTCLKSEL },
    {"ISO7816CLKSEL",     &SYSCON->ISO7816CLKSEL },
    {"SYSTICKCLKDIV",     &SYSCON->SYSTICKCLKDIV },
    {"TRACECLKDIV",       &SYSCON->TRACECLKDIV },
    {"WDTCLKDIV",         &SYSCON->WDTCLKDIV },
    {"IRCLKDIV",          &SYSCON->IRCLKDIV },
    {"AHBCLKDIV",         &SYSCON->AHBCLKDIV },
    {"CLKOUTDIV",         &SYSCON->CLKOUTDIV },
    {"SPIFICLKDIV",       &SYSCON->SPIFICLKDIV },
    {"ADCCLKDIV",         &SYSCON->ADCCLKDIV },
    {"RTCCLKDIV",         &SYSCON->RTCCLKDIV },
    {"ISO7816CLKDIV",&SYSCON->ISO7816CLKDIV },
    {"FRGCTRL",&SYSCON->FRGCTRL },
    {"DMICCLKDIV",&SYSCON->DMICCLKDIV },
    {"RTC1HZCLKDIV",&SYSCON->RTC1HZCLKDIV },
    {"CLOCKGENUPDATELOCKOUT",&SYSCON->CLOCKGENUPDATELOCKOUT },
    {"EFUSECLKCTRL",&SYSCON->EFUSECLKCTRL },
    {"RNGCTRL",&SYSCON->RNGCTRL },
    {"RNGCLKCTRL",&SYSCON->RNGCLKCTRL },
    {"SRAMCTRL",&SYSCON->SRAMCTRL },
    {"SRAMCTRL0",&SYSCON->SRAMCTRL0 },
    {"SRAMCTRL1",&SYSCON->SRAMCTRL1 },
    {"ROMCTRL",&SYSCON->ROMCTRL },
    {"MODEMCTRL",&SYSCON->MODEMCTRL },
    {"MODEMSTATUS",&SYSCON->MODEMSTATUS },
    {"XTAL32KCAP",&SYSCON->XTAL32KCAP },
    {"XTAL32MCTRL",&SYSCON->XTAL32MCTRL },
    {"STARTER0",&SYSCON->STARTER[0] },
    {"STARTER1",&SYSCON->STARTER[1] },
    {"RETENTIONCTRL",&SYSCON->RETENTIONCTRL },
    {"POWERDOWNSAFETY",&SYSCON->POWERDOWNSAFETY },
    {"MAINCLKSAFETY",&SYSCON->MAINCLKSAFETY },
    {"HARDWARESLEEP",&SYSCON->HARDWARESLEEP },
    {"CPUCTRL",&SYSCON->CPUCTRL },
    {"CPBOOT",&SYSCON->CPBOOT },
    {"CPSTACK",&SYSCON->CPSTACK },
    {"CPSTAT",&SYSCON->CPSTAT },
    {"GPIOSECIN",&SYSCON->GPIOSECIN },
    {"GPIOSECOUT",&SYSCON->GPIOSECOUT },
    {"GPIOSECDIR",&SYSCON->GPIOSECDIR },
    {"ANACTRL_CTRL",&SYSCON->ANACTRL_CTRL },
    {"ANACTRL_VAL",&SYSCON->ANACTRL_VAL },
    {"ANACTRL_STAT",&SYSCON->ANACTRL_STAT },
    {"ANACTRL_INTENSET",&SYSCON->ANACTRL_INTENSET },
    {"ANACTRL_INTSTAT",&SYSCON->ANACTRL_INTSTAT },
    {"CLOCK_CTRL",&SYSCON->CLOCK_CTRL },
    {"WKT_CTRL",&SYSCON->WKT_CTRL },
    {"WKT_LOAD_WKT0_LSB",&SYSCON->WKT_LOAD_WKT0_LSB },
    {"WKT_LOAD_WKT0_MSB",&SYSCON->WKT_LOAD_WKT0_MSB },
    {"WKT_LOAD_WKT1",&SYSCON->WKT_LOAD_WKT1 },
    {"WKT_VAL_WKT0_LSB",&SYSCON->WKT_VAL_WKT0_LSB },
    {"WKT_VAL_WKT0_MSB",&SYSCON->WKT_VAL_WKT0_MSB },
    {"WKT_VAL_WKT1",&SYSCON->WKT_VAL_WKT1 },
    {"WKT_STAT",&SYSCON->WKT_STAT },
    {"WKT_INTENSET",&SYSCON->WKT_INTENSET },
    {"WKT_INTSTAT",&SYSCON->WKT_INTSTAT },
    {"AUTOCLKGATEOVERRIDE",&SYSCON->AUTOCLKGATEOVERRIDE },
    {"GPIOPSYNC",&SYSCON->GPIOPSYNC },
    {"INVERTMAINCLK",&SYSCON->INVERTMAINCLK },
    {"DIEID",&SYSCON->DIEID },
    {"CPUCFG",&SYSCON->CPUCFG },
    {"CONFIGLOCKOUT",&SYSCON->CONFIGLOCKOUT },
};

void dump_syscon(void)
{
    PRINTF("SYSCON\r\n");
    for (int i = 0; i < NumberOfElements(SysconDesc); i++)
    {
        PRINTF("\t%s = %x\r\n", SysconDesc[i].reg_name, *SysconDesc[i].reg_addr);
    }

}

const reg_desc_t PmcDesc[] = {
    {"CTRL",         &PMC->CTRL  },
    {"DCDC0",        &PMC->DCDC0 },
    {"DCDC1",        &PMC->DCDC1 },
    {"BIAS",         &PMC->BIAS },
    {"LDOPMU",       &PMC->LDOPMU },
    {"LDOMEM",       &PMC->LDOMEM  },
    {"LDOCORE",      &PMC->LDOCORE },
    {"LDOFLASHNV",   &PMC->LDOFLASHNV },
    {"LDOFLASHCORE", &PMC->LDOFLASHCORE },
    {"LDOADC",       &PMC->LDOADC },
    {"BODVBAT",      &PMC->BODVBAT },
    {"BODMEM",       &PMC->BODMEM },
    {"BODCORE",      &PMC->BODCORE },
    {"FRO192M",      &PMC->FRO192M },
    {"FRO1M",        &PMC->FRO1M },
    {"FRO32K",       &PMC->FRO32K },
    {"XTAL32K",      &PMC->XTAL32K },
    {"ANAMUXCOMP",   &PMC->ANAMUXCOMP  },
    {"PWRSWACK",     &PMC->PWRSWACK },
    {"DPDWKSRC",     &PMC->DPDWKSRC },
    {"STATUSPWR",    &PMC->STATUSPWR },
    {"STATUSCLK",    &PMC->STATUSCLK },
    {"RESETCAUSE",   &PMC->RESETCAUSE },
    {"AOREG0",       &PMC->AOREG0 },
    {"AOREG1",       &PMC->AOREG1 },
    {"AOREG2",       &PMC->AOREG2 },
    {"FUSEOVW",      &PMC->FUSEOVW },
    {"DUMMYCTRL",    &PMC->DUMMYCTRL },
    {"DUMMYSTATUS",  &PMC->DUMMYSTATUS },
    {"DPDCTRL",      &PMC->DPDCTRL },
    {"PIOPORCAP",    &PMC->PIOPORCAP },
    {"PIORESCAP",    &PMC->PIORESCAP },
    {"TIMEOUTEVENTS",&PMC->TIMEOUTEVENTS },
    {"TIMEOUT",      &PMC->TIMEOUT },
    {"PDSLEEPCFG",   &PMC->PDSLEEPCFG },
    {"PDRUNCFG",     &PMC->PDRUNCFG },
    {"WAKEIOCAUSE",  &PMC->WAKEIOCAUSE },
    {"EFUSE0",       &PMC->EFUSE0 },
    {"EFUSE1",       &PMC->EFUSE1 },
    {"EFUSE2",       &PMC->EFUSE2 },
    {"CTRLNORST",    &PMC->CTRLNORST },
};

void dump_pmc(void)
{
    PRINTF("PMC\r\n");
    for (int i = 0; i < NumberOfElements(PmcDesc); i++)
    {
        PRINTF("\t%s = %x\r\n", PmcDesc[i].reg_name, *PmcDesc[i].reg_addr);
    }
}

const reg_desc_t BleDpDesc[] = {
    {"DP_TOP_SYSTEM_CTRL",  &BLE_DP_TOP->DP_TOP_SYSTEM_CTRL },
    {"PROP_MODE_CTRL",      &BLE_DP_TOP->PROP_MODE_CTRL },
    {"ACCESS_ADDRESS",      &BLE_DP_TOP->ACCESS_ADDRESS },
    {"ANT_PDU_DATA0",       &BLE_DP_TOP->ANT_PDU_DATA0 },
    {"ANT_PDU_DATA1",       &BLE_DP_TOP->ANT_PDU_DATA1 },
    {"ANT_PDU_DATA2",       &BLE_DP_TOP->ANT_PDU_DATA2 },
    {"ANT_PDU_DATA3",       &BLE_DP_TOP->ANT_PDU_DATA3 },
    {"ANT_PDU_DATA4",       &BLE_DP_TOP->ANT_PDU_DATA4 },
    {"ANT_PDU_DATA5",       &BLE_DP_TOP->ANT_PDU_DATA5 },
    {"ANT_PDU_DATA6",       &BLE_DP_TOP->ANT_PDU_DATA6 },
    {"ANT_PDU_DATA7",       &BLE_DP_TOP->ANT_PDU_DATA7 },
    {"CRC_SEED",            &BLE_DP_TOP->CRC_SEED },
    {"DP_FUNCTION_CTRL",    &BLE_DP_TOP->DP_FUNCTION_CTRL },
    {"DP_TEST_CTRL",        &BLE_DP_TOP->DP_TEST_CTRL },
    {"BLE_DP_STATUS1",      &BLE_DP_TOP->BLE_DP_STATUS1 },
    {"BLE_DP_STATUS2",      &BLE_DP_TOP->BLE_DP_STATUS2 },
    {"BLE_DP_STATUS3",      &BLE_DP_TOP->BLE_DP_STATUS3 },
    {"BLE_DP_STATUS4",      &BLE_DP_TOP->BLE_DP_STATUS4 },
    {"RX_FRONT_END_CTRL1",  &BLE_DP_TOP->RX_FRONT_END_CTRL1 },
    {"RX_FRONT_END_CTRL2",  &BLE_DP_TOP->RX_FRONT_END_CTRL2 },
    {"FREQ_DOMAIN_CTRL1",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL1 },
    {"FREQ_DOMAIN_CTRL2",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL2 },
    {"FREQ_DOMAIN_CTRL3",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL3 },
    {"FREQ_DOMAIN_CTRL4",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL4 },
    {"FREQ_DOMAIN_CTRL5",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL5 },
    {"FREQ_DOMAIN_CTRL6",   &BLE_DP_TOP->FREQ_DOMAIN_CTRL6 },
    {"HP_MODE_CTRL1",       &BLE_DP_TOP->HP_MODE_CTRL1},
    {"HP_MODE_CTRL2",       &BLE_DP_TOP->HP_MODE_CTRL2 },
    {"FREQ_DOMAIN_STATUS1", &BLE_DP_TOP->FREQ_DOMAIN_STATUS1 },
    {"FREQ_DOMAIN_STATUS2", &BLE_DP_TOP->FREQ_DOMAIN_STATUS2 },
    {"DP_AA_ERROR_CTRL",    &BLE_DP_TOP->DP_AA_ERROR_CTRL },
    {"DP_INT",              &BLE_DP_TOP->DP_INT },
    {"DP_AA_ERROR_TH",      &BLE_DP_TOP->DP_AA_ERROR_TH },
    {"DP_ANTENNA_CTRL",     &BLE_DP_TOP->DP_ANTENNA_CTRL },
    {"ANTENNA_MAP01",       &BLE_DP_TOP->ANTENNA_MAP01 },
    {"ANTENNA_MAP23",       &BLE_DP_TOP->ANTENNA_MAP23 },
    {"ANTENNA_MAP45",       &BLE_DP_TOP->ANTENNA_MAP45 },
    {"ANTENNA_MAP67",       &BLE_DP_TOP->ANTENNA_MAP67 },
    {"LL_EM_BASE_ADDRESS",  &BLE_DP_TOP->LL_EM_BASE_ADDRESS },
    {"RX_EARLY_EOP",        &BLE_DP_TOP->RX_EARLY_EOP },
    {"ANT_DIVERSITY",       &BLE_DP_TOP->ANT_DIVERSITY },
    {"TX_M_TEST_CTRL",      &BLE_DP_TOP->TX_M_TEST_CTRL },
};
void dump_BleDp(void)
{
    PRINTF("BLE_TOP_DP\r\n");
    for (int i = 0; i < NumberOfElements(BleDpDesc); i++)
    {
        PRINTF("\t%s = %x\r\n", BleDpDesc[i].reg_name, *BleDpDesc[i].reg_addr);
    }
}
#endif

static psector_page_data_t *mPage0Hdl = NULL;
#if PRELOAD_PFLASH
static psector_page_data_t *mPFlashHdl = NULL;
#endif
psector_page_data_t *psector_GetPage0Contents(void)
{
    static psector_page_data_t mPage0;
    psector_page_data_t *page = &mPage0;

    psector_page_state_t page_state;
    do {
        page_state = psector_ReadData(PSECTOR_PAGE0_PART, 0, 0, sizeof(psector_page_t), page);
        if (page_state < PAGE_STATE_DEGRADED) {
            page = NULL;
            panic(0, 0, 0, 0);
            break;
        }
    } while (0);

    return page;
}

#if PRELOAD_PFLASH
psector_page_data_t *psector_GetPFlashContents(void)
{
    static psector_page_data_t mpFlash;
    psector_page_data_t *page = &mpFlash;

    psector_page_state_t page_state;
    do {
        page_state = psector_ReadData(PSECTOR_PFLASH_PART, 0, 0, sizeof(psector_page_t), page);
        if (page_state < PAGE_STATE_DEGRADED) {
            page = NULL;
            panic(0, 0, 0, 0);
            break;
        }
    } while (0);

    return page;
}
#endif

psector_page_data_t *psector_GetPage0Handle(void)
{
    psector_page_data_t *hdl = NULL;
    ;
    do {
        if (mPage0Hdl != NULL) {
            hdl = mPage0Hdl;
            break;
        }

        if ((mPage0Hdl = psector_GetPage0Contents()) != NULL) {
            hdl = mPage0Hdl;
            break;
        }
    } while (0);
    return hdl;
}

int psector_CommitPageUpdates(psector_page_data_t *page, psector_partition_id_t id)
{
    int status = -1;
    do {
        if (!page)
            break;

        page->hdr.version++;
        page->hdr.checksum = psector_CalculateChecksum((psector_page_t *)page);

        if (psector_WriteUpdatePage(id, (psector_page_t *)page) != WRITE_OK) {
            break;
        }
        status = 0;
    } while (0);

    return status;
}

int psector_SetOtaEntry(image_directory_entry_t *ota_entry, bool commit)
{
    int res = -1;

    do {
        mPage0Hdl = psector_GetPage0Handle();
        if (mPage0Hdl == NULL)
            break;

        mPage0Hdl->page0_v3.ota_entry.img_base_addr = ota_entry->img_base_addr;
        mPage0Hdl->page0_v3.ota_entry.img_nb_pages  = ota_entry->img_nb_pages;
        mPage0Hdl->page0_v3.ota_entry.flags         = ota_entry->flags;
        mPage0Hdl->page0_v3.ota_entry.img_type      = ota_entry->img_type;

        if (commit) {
            if (psector_CommitPageUpdates(mPage0Hdl, PSECTOR_PAGE0_PART) < 0)
                break;
        }
        res = 0;

    } while (0);
    return res;
}

bool psector_SetPreferredApp(
    uint8_t preferred_app, bool commit_now, pfImgValidationCb imgValCb, void *imgValidationArgs)
{
    bool res = false;
    do {
        uint32_t image_addr = IMAGE_INVALID_ADDR;

        mPage0Hdl = psector_GetPage0Handle();
        if (mPage0Hdl == NULL) {
            res = false;
            break;
        }

        if (preferred_app == mPage0Hdl->page0_v3.preferred_app_index) {
            res = false; /* do nothing : already set so do not reset */
            break;
        }
        /* Try to find an entry in the img directory */
        image_directory_entry_t *dir_entry          = NULL;
        image_directory_entry_t *dir_entry_iterator = NULL;
        for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++) {
            dir_entry_iterator = mPage0Hdl->page0_v3.img_directory + i;
            if (dir_entry_iterator->img_type == preferred_app) {
                dir_entry = dir_entry_iterator;
                break;
            }
        }
        if (dir_entry && (dir_entry->flags & IMG_FLAG_BOOTABLE)) {
            /* Do a quick image validation : pass no certificate pointer */
            if (imgValCb != NULL)
                image_addr = (*imgValCb)(dir_entry->img_base_addr, imgValidationArgs);
        }
        if (image_addr == IMAGE_INVALID_ADDR)
            break;

        mPage0Hdl->page0_v3.preferred_app_index = preferred_app;
        if (commit_now) {
            if (psector_CommitPageUpdates(mPage0Hdl, PSECTOR_PAGE0_PART) != 0)
                break;
        }
        res = true;
    } while (0);
    return res;
}

/*
 * BOARD_Get_BLE_MAC_Id tries to find a valid BLE MAC address by looking successively into
 * the PFLASH, if an address is founnd it exists. Otherwise if keeps looking for one into the
 * CONFIG page (page N-2), if that fails too then it generates a random address whose OUI is
 * forced (3 MSB bytes of the address).
 *
 */
#if !defined CPU_JN518X_REV || (CPU_JN518X_REV > 1)

int psector_WriteBleMacAddress(uint8_t *src_mac_address)
{
    int res = -1;
    psector_page_data_t page; /* Stored in the stack temporarily */

    do {
        psector_page_state_t pg_state;

        pg_state = psector_ReadData(PSECTOR_PFLASH_PART, 0, 0, sizeof(psector_page_data_t), &page);
        if (pg_state < PAGE_STATE_DEGRADED)
            break;

        uint8_t *dst = (uint8_t *)&page.pFlash.ble_mac_id;

        for (int i = 0; i < sizeof(uint64_t); i++) {
            dst[i] = src_mac_address[i];
        }
        if (psector_CommitPageUpdates(&page, PSECTOR_PFLASH_PART) < 0)
            break;

        res = 0;

    } while (0);

    return res;
}

int BOARD_Get_BLE_MAC_Id(uint8_t mac_addr[BLE_MACID_SZ])
{
    int res = -1;
    uint8_t buf[8];

    FLib_MemSet((uint8_t *)mac_addr, 0, BLE_MACID_SZ);

    do {
        /* Try to read form pFlash Customer MAC address */
        psector_page_state_t pg_state;
#define CUSTOMER_BLE_MACID_OFFSET ((size_t) & ((psector_page_data_t *)0)->pFlash.ble_mac_id)
        pg_state = psector_ReadData(PSECTOR_PFLASH_PART, 0, CUSTOMER_BLE_MACID_OFFSET, sizeof(uint64_t), buf);
        if (pg_state > 1) {
            if (!FLib_MemCmpToVal((uint8_t *)buf, 0, sizeof(uint64_t))) {
                res = 0;
                break;
            }
        }
        /* If unset by Customer continue to read Manufacturer's */

        FLib_MemCpy((uint8_t *)buf, MANUFACTURER_BLE_MACID_ADRESS, sizeof(uint64_t));
        if (!FLib_MemCmpToVal(buf, 0, sizeof(uint64_t))) {
            res = 1;
            FLib_MemCpy((uint8_t *)mac_addr, (uint8_t *)&buf[2], BLE_MACID_SZ);
            break;
        }
        else {
            uint8_t randomNb[4];
            const uint8_t oui[3] = {gBD_ADDR_NXP_OUI_c};
            RNG_GetRandomNo((uint32_t *)&randomNb[0]);
            buf[0] = buf[1] = 0;
            FLib_MemCpy(&buf[2], (void *)oui, 3);
            FLib_MemCpy(&buf[5], (void *)&randomNb[0], 3);

            if (psector_WriteBleMacAddress(&buf[0]) < 0) {
                res = -1;
                break;
            }
            res = 2;
            break;
        }
    } while (0);
    if (res >= 0) {
        for (int i = 0; i < BLE_MACID_SZ; i++) {
            mac_addr[i] = buf[BLE_MACID_SZ + 1 - i];
        }
    }
    return res;
}

void BOARD_DisplayMCUUid(uint8_t mac_id[BLE_MACID_SZ])
{
    PRINTF("Device_MAC_ID = ");
    for (int i = 0; i < BLE_MACID_SZ - 1; i++) {
        PRINTF("%02x:", mac_id[BLE_MACID_SZ - 1 - i]);
    }
    PRINTF("%02x\r\n", mac_id[0]);
}

void BOARD_GetMCUUid(uint8_t *aOutUid16B, uint8_t *pOutLen)
{
    uint8_t mac_id[BD_ADDR_SIZE] = {0};

    if (BOARD_Get_BLE_MAC_Id(mac_id) >= 0) {
        static int already_displayed = 0;
        if (!already_displayed) {
            BOARD_DisplayMCUUid(mac_id);
            already_displayed = 1;
        }

        *pOutLen = BD_ADDR_SIZE;
        FLib_MemCpy(aOutUid16B, mac_id, BD_ADDR_SIZE);
    }
}
#endif

//void BOARD_SetPinsForRunMode(void)
//{
//    SYSCON->RETENTIONCTRL &= ~SYSCON_RETENTIONCTRL_IOCLAMP_MASK;
//
//    INPUTMUX_Init(INPUTMUX);
//    PINT_Init(PINT);
//    GPIO_PortInit(GPIO, 0);
//
//}
