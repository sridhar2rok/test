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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "uwb_usb_serial.h"

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include <windows.h>
#include <stdbool.h>

static int usb_serial_configuration(long baudRate);

static HANDLE comHandle = INVALID_HANDLE_VALUE;

int usb_serial_open(const char *port, long baudRate)
{
    int retStatus            = -1;
    static char portPath[32] = {0};
    if (port[0] != '\\') {
        _snprintf(portPath, sizeof(portPath) - 1, "\\\\.\\%s", port);
        port = portPath;
    }
    comHandle = CreateFileA(port, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (!comHandle || comHandle == INVALID_HANDLE_VALUE) {
        comHandle = INVALID_HANDLE_VALUE;
    }
    else {
        FlushFileBuffers(comHandle);
        retStatus = usb_serial_configuration(baudRate);
    }

    return retStatus;
}

void usb_serial_close()
{
    if (comHandle != INVALID_HANDLE_VALUE)
        CloseHandle(comHandle);
    comHandle = INVALID_HANDLE_VALUE;
}

static int usb_serial_configuration(long baudRate)
{
    COMMTIMEOUTS timeouts;
    DCB dcb               = {0};
    dcb.DCBlength         = sizeof(DCB);
    dcb.BaudRate          = baudRate;
    dcb.fBinary           = true;
    dcb.fParity           = false;
    dcb.fOutxCtsFlow      = false;
    dcb.fOutxDsrFlow      = false;
    dcb.fDtrControl       = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity   = false;
    dcb.fTXContinueOnXoff = true;
    dcb.fOutX             = false;
    dcb.fInX              = false;
    dcb.fErrorChar        = false;
    dcb.fNull             = false;
    dcb.fRtsControl       = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError     = false;
    dcb.XonLim            = 0;
    dcb.XoffLim           = 0;
    dcb.ByteSize          = 8;
    dcb.Parity            = NOPARITY;
    dcb.StopBits          = ONESTOPBIT;

    if (!SetCommState(comHandle, &dcb)) {
        return -1;
    }
    EscapeCommFunction(comHandle, SETDTR);

#if UWBIOT_TML_PNP && UWBIOT_UWBD_SR100T
    timeouts.ReadIntervalTimeout = 10;
#elif UWBIOT_TML_PNP && UWBIOT_UWBD_SR040
    timeouts.ReadIntervalTimeout = 5;
#elif UWBIOT_TML_S32UART
    timeouts.ReadIntervalTimeout = 5;
#else
    timeouts.ReadIntervalTimeout = 1;
#endif
    timeouts.ReadTotalTimeoutMultiplier  = 1;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant   = 100;

    if (!SetCommTimeouts(comHandle, &timeouts)) {
        return -1;
    }
    return 0;
}

int usb_serial_write(uint8_t *data, int len)
{
    DWORD bytesWrote = 0;
    int retStatus    = 0;
    if (!WriteFile(comHandle, data, len, &bytesWrote, NULL)) {
        retStatus = -1;
    }
    else {
        retStatus = bytesWrote;
    }

    return retStatus;
}

int usb_serial_read(uint8_t *data, int len)
{
    DWORD bytesRead = 0;
    int retStatus   = 0;
    if (!ReadFile(comHandle, data, len, &bytesRead, NULL)) {
        retStatus = -1;
    }
    else {
        retStatus = bytesRead;
    }

    return retStatus;
}

#endif /* _MSC_VER */
