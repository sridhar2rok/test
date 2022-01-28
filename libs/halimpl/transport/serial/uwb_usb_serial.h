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
#ifndef _USB_SERIAL_H_
#define _USB_SERIAL_H_

#include <stdint.h>

#include <time.h>

#if __cplusplus
extern "C" {
#endif

int usb_serial_open(const char *port, long baudRate);
int usb_serial_write(uint8_t *data, int len);
int usb_serial_read(uint8_t *data, int len);
void usb_serial_close(void);

#if __cplusplus
}
#endif

#endif
