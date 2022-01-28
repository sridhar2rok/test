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

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

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
#include <nxEnsure.h>

#if UWBIOT_TML_SOCKET
#include <windows.h>
#endif

#define READ_WRITE_WAIT_MUTEX_TIMEOUT_MS (500)

static HANDLE comMutex = INVALID_HANDLE_VALUE;

int phNxpUwb_SpiWrite(uint8_t *data, uint16_t len)
{
    int ret = RETURNED_FAILURE;
    int write_status;
    DWORD dwWaitResult = 0;
    ENSURE_OR_GO_EXIT(INVALID_HANDLE_VALUE != comMutex);

    dwWaitResult = WaitForSingleObject(comMutex, // handle to mutex
        (2 * READ_WRITE_WAIT_MUTEX_TIMEOUT_MS)); // no time-out interval
    ENSURE_OR_GO_EXIT(WAIT_OBJECT_0 == dwWaitResult);
    write_status = usb_serial_write(data, len);
    if (write_status > 0) {
        ret = RETURNED_SUCCESS;
    }
    ReleaseMutex(comMutex);
exit:
    return ret;
}

int phNxpUwb_SpiRead(uint8_t *data, uint16_t len)
{
    int ret            = RETURNED_FAILURE;
    int read_status    = 0;
    DWORD dwWaitResult = 0;
    ENSURE_OR_GO_EXIT(INVALID_HANDLE_VALUE != comMutex);
    dwWaitResult = WaitForSingleObject(comMutex, // handle to mutex
        (READ_WRITE_WAIT_MUTEX_TIMEOUT_MS));     // no time-out interval
    ENSURE_OR_GO_EXIT(WAIT_OBJECT_0 == dwWaitResult);
    read_status = usb_serial_read(data, len);
    if (read_status > 0) {
        ret = RETURNED_SUCCESS;
    }

    ReleaseMutex(comMutex);

exit:
    if (read_status <= 0) {
#if UWBIOT_TML_SOCKET
        /* Uwb tests is not built with freertos currently. So vTaskDelay will not work */
        Sleep(100); /* wait for some time for writer */
#else
        vTaskDelay(pdMS_TO_TICKS(100)); /* wait for some time for writer */
#endif
    }
    return ret;
}

int phNxpUwb_SpiInit(void)
{
    int ret               = RETURNED_SUCCESS;
    char comPortName[32]  = {0};
    size_t comPortNameLen = sizeof(comPortName) - 1;

    comMutex = CreateMutex(NULL, FALSE, NULL);
    ENSURE_OR_GO_EXIT(INVALID_HANDLE_VALUE != comMutex);
    board_SerialGetCOMPort(comPortName, &comPortNameLen);

    int status = usb_serial_open(comPortName, board_SerialGetBaudRate());
    if (status == -1) {
        NXPLOG_UWB_TML_E("Could not open Com port '%s'", comPortName);
        NXPLOG_UWB_TML_I("NOTE: You can set '%s' to your port at run Time", UWBIOT_ENV_COM);
        usb_serial_close();
        CloseHandle(comMutex);
        comMutex = INVALID_HANDLE_VALUE;
        ret      = RETURNED_FAILURE;
    }
    else {
        ret = RETURNED_SUCCESS;
    }
exit:
    return ret;
}

int phNxpUwb_SpiDeInit(void)
{
    usb_serial_close();
    if (comMutex != INVALID_HANDLE_VALUE)
        CloseHandle(comMutex);
    comMutex = INVALID_HANDLE_VALUE;
    return RETURNED_SUCCESS;
}
