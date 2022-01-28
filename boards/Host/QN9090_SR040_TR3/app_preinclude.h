/*! *********************************************************************************
 * \defgroup app
 * @{
 ********************************************************************************** */
/*!
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 *
 * \file
 *
 * This file is the app configuration file which is pre included.
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

#ifndef _APP_PREINCLUDE_H_
#define _APP_PREINCLUDE_H_

/*! *********************************************************************************
 * 	Board Configuration
 ********************************************************************************** */

/* Debug only */
#define gLoggingActive_d 0
#define DBG_PWR          0
#define DBG_TMR          0

/* Defines the number of available keys for the keyboard module */
#define gKBD_KeysCount_c 2

// #define gUartDebugConsole_d 0

/*! *********************************************************************************
 * 	App Configuration
 ********************************************************************************** */
/*! Number of connections supported by the application */
#define gAppMaxConnections_c 1

/*! Enable/disable use of bonding capability */
#define gAppUseBonding_d 0

/*! Enable/disable use of pairing procedure */
#define gAppUsePairing_d 0

/*! Enable/disable use of privacy */
#define gAppUsePrivacy_d 0

#define gPasskeyValue_c 999999

#if (gAppUseBonding_d) && (!gAppUsePairing_d)
#error "Enable pairing to make use of bonding"
#endif

/* Enable HS Clock to support 2Mbps PHY mode setting */
#define gBleUseHSClock2MbpsPhy_c 0

/* Enable/Disable FSCI Low Power Commands*/
#define gFSCI_IncludeLpmCommands_c 0

/*! ADV interval in Slots : 1600 matches to 1s  */
#define gAppAdvertisingInterval 1600

/*! ADV connectable - Set to 0 if ADV is not connectable*/
#define gAppConnectableAdv_d 0

#define gTimerMgrUseLpcRtc_c 1

/*! *********************************************************************************
 * 	Framework Configuration
 ********************************************************************************** */
/* Use the Lowpower module from the framework :
 * Default lowpower mode for the lowpower module is WFI only
 * For full power down mode, cPWR_FullPowerDownMode shall be set to 1
 */
#define cPWR_UsePowerDownMode 1

/* Enable Power down modes
 * Need cPWR_UsePowerDownMode to be set to 1 first */
#define cPWR_FullPowerDownMode 1

/* Settings that apply only when cPWR_FullPowerDownMode is set */
#if cPWR_FullPowerDownMode

/* Prevent from disabling the power down mode when switching in connected mode
 * If not set, the powerdown mode will be disabled when going into connected mode */
#define cPWR_NoPowerDownDisabledOnConnect 1

/* Go to power down even if a timer (not lower timer) is running
 * Warning : timers are lost and not yet recovered on wakeup  */
#define cPWR_DiscardRunningTimerForPowerDown 1

/* Use Timebase drift compensation algo in Linklayer when power Down mode is
 * enabled (cPWR_UsePowerDownMode set to 3)
 * To be used with FRO32K (gClkUseFro32K)
 * Allow to decrease Always ON LDO voltage in powerdown for additional power saving
 * in connected mode */
#define gPWR_UseAlgoTimeBaseDriftCompensate gClkUseFro32K

/* Switch CPU clock to 48MHz FRO at startup - 32MHz (FRO or XTAL) default */
#define gPWR_CpuClk_48MHz 1

/* Allow Frequency scaling @FRO 12MHz when CPU goes to WFI */
//#define gPWR_FreqScalingWFI                  1

/* Control the power saving of the Flash Controller when CPU is in WFI state
 * 1 - Power down the flash controller and Switch OFF the 48Mhz clock to Flash Controller
 * 2 - Not supported */
//#define gPWR_FlashControllerPowerDownInWFI   0

/* Apply LDO MEM voltage to 0.9V instead of 1.1V in power down
 * Shall not be used over full operating range of the device  */
#define gPWR_LDOMEM_0_9V_PD 1

/* Reduce LDO Core/Mem/Ao Voltage down to 1.0V during Active
 * except at wakeup and when increasing CPU clock */
#define gPWR_LdoVoltDynamic 1

/* Optimize Advertising interslot interval in us - Default is 1500us if not set */
#define gPWR_AdvertisingInterSlotInt 1328

/* Not supported */
#define gPWR_SerialUartRxWakeup 0
#endif

/* Use FRO32K instead of XTAL32K in active and power down modes - XTAL32K no longer required */
#define gClkUseFro32K 0

/* Enable/disable NTAG feature */
#define gAppNtagSupported_d 0

/*! *********************************************************************************
 * 	Framework Configuration
 ********************************************************************************** */
/* enable NVM to be used as non volatile storage management by the host stack */
#define gAppUseNvm_d 0

/* If set, enables Kmod data saving in PDM (requires PDM library) */
#define gRadioUsePdm_d 1

/* gUsePdm_d is not synonymous to gAppUseNvm_d because PDM is used by Radio driver independantly from NVM */
#define gUsePdm_d (gAppUseBonding_d | gAppUsePairing_d | gRadioUsePdm_d)

/* Defines Num of Serial Manager interfaces */
#define gSerialManagerMaxInterfaces_c 0

/* Defines Size for Timer Task*/
#define gTmrTaskStackSize_c 700

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define AppPoolsDetails_c                                                                   \
    _block_size_ 32 _number_of_blocks_ 60 _eol_ _block_size_ 64 _number_of_blocks_ 30 _eol_ \
        _block_size_ 128 _number_of_blocks_ 30 _eol_ _block_size_ 512 _number_of_blocks_ 20 _eol_

/* Defines number of timers needed by the application */
#define gTmrApplicationTimers_c 4

/* Defines number of timers needed by the protocol stack */
#define gTmrStackTimers_c 5

/* Set this define TRUE if the PIT frequency is an integer number of MHZ */
#define gTMR_PIT_FreqMultipleOfMHZ_d 0

/* Enable/Disable Periodical Interrupt Timer */
#define gTMR_PIT_Timestamp_Enabled_d 0

/* Enables / Disables the precision timers platform component */
#define gTimestamp_Enabled_d 0

/* Enable/Disable PANIC catch */
#define gUsePanic_c 1

/* Debug only */
#define gLoggingWithExtraTs 0

/* Keyboard not supported */
#define gKeyBoardSupported_d 0

/*! *********************************************************************************
 * 	RTOS Configuration
 ********************************************************************************** */
/* Defines the RTOS used */
#define FSL_RTOS_FREE_RTOS 1

/*defines the tickless mode*/
#define mAppUseTickLessMode_c 1

/* Defines number of OS events used */
#define osNumberOfEvents 5

/* Defines main task stack size */
#define gMainThreadStackSize_c 1100

/* Defines controller task stack size */
#define gControllerTaskStackSize_c 2048

/* Defines total heap size used by the OS - 12k */
#define gTotalHeapSize_c 12288
/*! *********************************************************************************
 *     BLE Stack Configuration
 ********************************************************************************** */
/* Configure the maximum number of bonded devices. If maximum bonded devices reached,
 * user should remove an old bonded device to store new bonded information. Otherwise,
 * demo application will pair with new deivce with No Bonding type.
 */
#define gMaxBondedDevices_c     16
#define gMaxResolvingListSize_c 6

/*! *********************************************************************************
 * 	NVM Module Configuration - gAppUseNvm_d shall be defined aboved as 1 or 0
 ********************************************************************************** */
/* USER DO NOT MODIFY THESE MACROS DIRECTLY. */
#define gAppMemPoolId_c 0
#if gAppUseNvm_d
#define gNvmMemPoolId_c 1
#if gUsePdm_d
#define gPdmMemPoolId_c 2
#endif
#else
#if gUsePdm_d
#define gPdmMemPoolId_c 1
#endif
#endif

#if gAppUseNvm_d
#define gNvmOverPdm_d 1
/* Defines NVM pools by block size and number of blocks. Must be aligned to 4 bytes.*/
#define NvmPoolsDetails_c                                                          \
    _block_size_ 32 _number_of_blocks_ 20 _pool_id_(gNvmMemPoolId_c)               \
        _eol_ _block_size_ 60 _number_of_blocks_ 10 _pool_id_(gNvmMemPoolId_c)     \
            _eol_ _block_size_ 80 _number_of_blocks_ 10 _pool_id_(gNvmMemPoolId_c) \
                _eol_ _block_size_ 100 _number_of_blocks_ 2 _pool_id_(gNvmMemPoolId_c) _eol_

/* configure NVM module */
#define gNvStorageIncluded_d         (1)
#define gNvFragmentation_Enabled_d   (1)
#define gUnmirroredFeatureSet_d      (0)
#define gNvRecordsCopiedBufferSize_c (512)
#else
#define NvmPoolsDetails_c
#endif

#if gUsePdm_d
#define gPdmNbSegments 63 /* number of sectors contained in PDM storage */

#define PdmInternalPoolsDetails_c                                                                            \
    _block_size_ 512 _number_of_blocks_ 2 _pool_id_(gPdmMemPoolId_c) _eol_ _block_size_(gPdmNbSegments * 12) \
        _number_of_blocks_ 1 _pool_id_(gPdmMemPoolId_c) _eol_
#else
#define PdmInternalPoolsDetails_c
#endif

/*! *********************************************************************************
 * 	Memory Pools Configuration
 ********************************************************************************** */

/* Defines pools by block size and number of blocks. Must be aligned to 4 bytes.
 * DO NOT MODIFY THIS DIRECTLY. CONFIGURE AppPoolsDetails_c
 * If gMaxBondedDevices_c increases, adjust NvmPoolsDetails_c
*/

#if gAppUseNvm_d
#define PoolsDetails_c AppPoolsDetails_c NvmPoolsDetails_c PdmInternalPoolsDetails_c
#elif gUsePdm_d /* Radio drivers uses PDM but no NVM over PDM */
#define PoolsDetails_c AppPoolsDetails_c PdmInternalPoolsDetails_c
#else
#define PoolsDetails_c AppPoolsDetails_c
#endif

#endif /* _APP_PREINCLUDE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
