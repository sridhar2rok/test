/*
 * Copyright 2019,2020 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* System includes */
#include <string.h>
#include <stdint.h>
#include "UWB_GpioIrq.h"
#include "board.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Freescale includes */

/* UWB includes */
#include "driver_config.h"
#include "UwbCore_Types.h"

#include "phNxpUwb_SpiTransport.h"
#include "phUwb_BuildConfig.h"
#include "phOsalUwb.h"
#include "phUwbErrorCodes.h"
#include <phNxpUwb_DriverInterface.h>
#include <uwb_usb_serial.h>
#include "phNxpLogApis_TmlUwb.h"

#define PRINT_ENTRY() printf("DBG:TODO:%s\n", __FUNCTION__)

int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len)
{
    int ret = RETURNED_FAILURE;
    int write_status;
    write_status = usb_serial_write(data, len);
    if (write_status > 0) {
        ret = RETURNED_SUCCESS;
    }
    return ret;
}

int phNxpUwb_SpiRead(uint8_t *data, uint16_t len)
{
    int ret = RETURNED_FAILURE;
    int write_status;
    write_status = usb_serial_read(data, len);
    if (write_status > 0) {
        ret = RETURNED_SUCCESS;
    }
    return ret;
}

int phNxpUwb_SpiInit(void)
{
    int ret               = RETURNED_SUCCESS;
    char comPortName[32]  = {0};
    size_t comPortNameLen = sizeof(comPortName) - 1;
    board_SerialGetCOMPort(comPortName, &comPortNameLen);

    int status = usb_serial_open(comPortName, board_SerialGetBaudRate());
    if (status == -1) {
        NXPLOG_UWB_TML_E("Could not open Com port '%s'", comPortName);
        NXPLOG_UWB_TML_I("NOTE: You can set '%s' to your port at run Time", UWBIOT_ENV_COM);
        usb_serial_close();
        ret = RETURNED_FAILURE;
    }
    else {
        ret = RETURNED_SUCCESS;
    }
    return ret;
}

int phNxpUwb_SpiDeInit(void)
{
    usb_serial_close();
    return RETURNED_SUCCESS;
}
