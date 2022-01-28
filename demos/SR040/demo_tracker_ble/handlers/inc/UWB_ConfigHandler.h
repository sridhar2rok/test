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

#ifndef _CONFIG_HANDLER_H_
#define _CONFIG_HANDLER_H_

#include "TLV_Defs.h"
#include "TLV_Builder.h"
/* Standard library */
#include <stdbool.h>
#include <string.h>

typedef struct
{
    uint32_t configId;
    uint8_t *configValue;
    uint8_t configLen;
} UWBT_ConfigRequest_t;

void handleConfigCmd(tlv_t *tlv, bool *error);

#endif /* _CONFIG_HANDLER_H_ */
