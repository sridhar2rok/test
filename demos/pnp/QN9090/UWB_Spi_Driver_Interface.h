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

#ifndef _UWB_SPI_PNP_H_
#define _UWB_SPI_PNP_H_

#include "UwbCore_Types.h"
#include "phUwb_BuildConfig.h"
#include "phNxpUwb_SpiTransport.h"
#include "phNxpUwb_DriverInterface.h"
#include "UwbPnpInternal.h"

#define RCI_COMMAND_LENGTH 256

EXTERNC uint8_t UWB_HeliosSpiInit(void);
EXTERNC void UWB_HeliosSpiDeInit(void);
EXTERNC uint8_t UWB_SpiHbciXfer(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t *rspLen);
EXTERNC uint8_t UWB_SpiHbciXferWithLen(uint8_t *data, uint16_t len, uint8_t *rsp, uint16_t rspLen);
EXTERNC uint16_t UWB_SpiUciWrite(uint8_t *data, uint16_t len);
EXTERNC uint16_t UWB_SpiUciRead(uint8_t *rsp);
EXTERNC void UWB_HeliosIrqEnable(void);
EXTERNC uint16_t UWB_SpiRciTransceive(uint8_t *data, uint16_t len);

/*UCI/SWUP interface configuration*/
typedef struct
{
    interface_handler_t interface_mode;
} interface_config_t;

#endif // _UWB_SPI_PNP_H_
