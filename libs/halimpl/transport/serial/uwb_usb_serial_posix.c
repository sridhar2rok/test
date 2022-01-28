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

#if defined(__linux__) || defined(__APPLE__)

#include "uwb_usb_serial.h"

int usb_serial_open(const char *port, long baudRate)
{
    return -1;
}

void usb_serial_close()
{
}

int usb_serial_write(uint8_t *data, int len)
{
    return 0;
}

int usb_serial_read(uint8_t *data, int len)
{
    return 0;
}

#endif /* defined(__linux__) || defined(__APPLE__) */
