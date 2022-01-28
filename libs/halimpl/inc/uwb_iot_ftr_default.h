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

#ifndef UWB_IOT_FTR_H_
#define UWB_IOT_FTR_H_

/* ************************************************************************** */
/* Defines                                                                    */
/* ************************************************************************** */

/* clang-format off */


/* # CMake Features : Start */


/** UWBIOT_UWBD : The UWB Device
 *
 * The device to which we are connecting
 */

/** Helios SR100T */
#define UWBIOT_UWBD_SR100T 0

/** SR040 */
#define UWBIOT_UWBD_SR040 1

#if (( 0                             \
    + UWBIOT_UWBD_SR100T             \
    + UWBIOT_UWBD_SR040              \
    ) > 1)
#        error "Enable only one of 'UWBIOT_UWBD'"
#endif


#if (( 0                             \
    + UWBIOT_UWBD_SR100T             \
    + UWBIOT_UWBD_SR040              \
    ) == 0)
#        error "Enable at-least one of 'UWBIOT_UWBD'"
#endif



/** UWBIOT_TML : Interface used for connection
 */

/** Plug And Play mode on Rhodes */
#define UWBIOT_TML_PNP 0

/** UART Mode of communication with S32K */
#define UWBIOT_TML_S32UART 0

/** Native SPI Communication */
#define UWBIOT_TML_SPI 1

/** Using network Sockets
 *
 * NXP Internal, for testing purpose.
 * Onging development. Not yet ready
 * NXP Internal
 */
#define UWBIOT_TML_SOCKET 0

#if (( 0                             \
    + UWBIOT_TML_PNP                 \
    + UWBIOT_TML_S32UART             \
    + UWBIOT_TML_SPI                 \
    + UWBIOT_TML_SOCKET              \
    ) > 1)
#        error "Enable only one of 'UWBIOT_TML'"
#endif


#if (( 0                             \
    + UWBIOT_TML_PNP                 \
    + UWBIOT_TML_S32UART             \
    + UWBIOT_TML_SPI                 \
    + UWBIOT_TML_SOCKET              \
    ) == 0)
#        error "Enable at-least one of 'UWBIOT_TML'"
#endif



/** UWBIOT_OS : Operating system used
 */

/** running with FreeRTOS Implementation. Direct or Simuation. */
#define UWBIOT_OS_FREERTOS 1

/** Native Implementation. Using pthread */
#define UWBIOT_OS_NATIVE 0

#if (( 0                             \
    + UWBIOT_OS_FREERTOS             \
    + UWBIOT_OS_NATIVE               \
    ) > 1)
#        error "Enable only one of 'UWBIOT_OS'"
#endif


#if (( 0                             \
    + UWBIOT_OS_FREERTOS             \
    + UWBIOT_OS_NATIVE               \
    ) == 0)
#        error "Enable at-least one of 'UWBIOT_OS'"
#endif


/* ====================================================================== *
 * == Feature selection/values ========================================== *
 * ====================================================================== */


/* ====================================================================== *
 * == Computed Options ================================================== *
 * ====================================================================== */



/** Deprecated items. Used here for backwards compatibility. */


/* # CMake Features : END */

/* clang-format on */

#endif /* UWB_IOT_FTR_H_ */
