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

#ifndef __SWUP_UPDATE_H__
#define __SWUP_UPDATE_H__

#include <SwupApi.h>
#include <stdint.h>
#include <stdio.h>

#define GO_CLENAUP_IF_NOT_ACTIVE            \
    if (STATUS_CMD_SUCCESS != error) {      \
        goto cleanup;                       \
    }                                       \
    if (swupStatus != SWUP_STATUS_ACTIVE) { \
        goto cleanup;                       \
    }

#define GO_CLENAUP_IF_NOT_TRANSFER            \
    if (STATUS_CMD_SUCCESS != error) {        \
        goto cleanup;                         \
    }                                         \
    if (swupStatus != SWUP_STATUS_TRANSFER) { \
        goto cleanup;                         \
    }

SwupResponseStatus_t Swup_Execute(
    const uint8_t swup_pkg[], const uint32_t size_swup_pkg, const SwupKeyVersion_t key_version);
SwupResponseStatus_t SwupUpdate(
    const uint8_t swup_pkg[], const uint32_t size_swup_pkg, const SwupKeyVersion_t key_version);

#endif // __SWUP_UPDATE_H__
