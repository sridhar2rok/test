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

#ifndef _PHNXPLOG_SWUP_H
#define _PHNXPLOG_SWUP_H

#include "uwb_logging.h"

/* Check if we are double defining these macros */
#if defined(LOG_E) || defined(LOG_W) || defined(LOG_I) || defined(LOG_D)
/* This should not happen.  The only reason this could happn is double inclusion of different log files. */
#error "LOG_ macro already defined"
#endif

/* Logging Level used by SWUP module */
#define SWUP_LOG_LEVEL   UWB_LOG_INFO_LEVEL
#define SWUP_MODULE_NAME "SWUP"

/*
 * Use the following macros.
 */

#if (SWUP_LOG_LEVEL >= UWB_LOG_ERROR_LEVEL)
#define LOG_E(...)                      UWB_LOG_ERROR(SWUP_MODULE_NAME, __VA_ARGS__)
#define LOG_X8_E(VALUE)                 UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_E(VALUE)                 UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X16_E(VALUE)                UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_E(VALUE)                UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X32_E(VALUE)                UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_E(VALUE)                UWB_LOG_ERROR(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_E(ARRAY, LEN)           UWB_LOG_AU8_ERROR(SWUP_MODULE_NAME, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_E(MESSAGE, ARRAY, LEN) UWB_LOG_AU8_ERROR(SWUP_MODULE_NAME, MESSAGE, ARRAY, LEN)
#else
#define LOG_E(...)
#define LOG_X8_E(VALUE)
#define LOG_U8_E(VALUE)
#define LOG_X16_E(VALUE)
#define LOG_U16_E(VALUE)
#define LOG_X32_E(VALUE)
#define LOG_U32_E(VALUE)
#define LOG_AU8_E(ARRAY, LEN)
#define LOG_MAU8_E(MESSAGE, ARRAY, LEN)
#endif

#if (SWUP_LOG_LEVEL >= UWB_LOG_WARN_LEVEL)
#define LOG_W(...)                      UWB_LOG_WARN(SWUP_MODULE_NAME, __VA_ARGS__)
#define LOG_X8_W(VALUE)                 UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_W(VALUE)                 UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X16_W(VALUE)                UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_W(VALUE)                UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X32_W(VALUE)                UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_W(VALUE)                UWB_LOG_WARN(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_W(ARRAY, LEN)           UWB_LOG_AU8_WARN(SWUP_MODULE_NAME, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_W(MESSAGE, ARRAY, LEN) UWB_LOG_AU8_WARN(SWUP_MODULE_NAME, MESSAGE, ARRAY, LEN)
#else
#define LOG_W(...)
#define LOG_X8_W(VALUE)
#define LOG_U8_W(VALUE)
#define LOG_X16_W(VALUE)
#define LOG_U16_W(VALUE)
#define LOG_X32_W(VALUE)
#define LOG_U32_W(VALUE)
#define LOG_AU8_W(ARRAY, LEN)
#define LOG_MAU8_W(MESSAGE, ARRAY, LEN)
#endif

#if (SWUP_LOG_LEVEL >= UWB_LOG_INFO_LEVEL)
#define LOG_I(...)                      UWB_LOG_INFO(SWUP_MODULE_NAME, __VA_ARGS__)
#define LOG_X8_I(VALUE)                 UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_I(VALUE)                 UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X16_I(VALUE)                UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_I(VALUE)                UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X32_I(VALUE)                UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_I(VALUE)                UWB_LOG_INFO(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_I(ARRAY, LEN)           UWB_LOG_AU8_INFO(SWUP_MODULE_NAME, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_I(MESSAGE, ARRAY, LEN) UWB_LOG_AU8_INFO(SWUP_MODULE_NAME, MESSAGE, ARRAY, LEN)
#else
#define LOG_I(...)
#define LOG_X8_I(VALUE)
#define LOG_U8_I(VALUE)
#define LOG_X16_I(VALUE)
#define LOG_U16_I(VALUE)
#define LOG_X32_I(VALUE)
#define LOG_U32_I(VALUE)
#define LOG_AU8_I(ARRAY, LEN)
#define LOG_MAU8_I(MESSAGE, ARRAY, LEN)
#endif

#if (SWUP_LOG_LEVEL >= UWB_LOG_DEBUG_LEVEL)
#define LOG_D(...)                      UWB_LOG_DEBUG(SWUP_MODULE_NAME, __VA_ARGS__)
#define LOG_X8_D(VALUE)                 UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=0x%02X", #VALUE, VALUE)
#define LOG_U8_D(VALUE)                 UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X16_D(VALUE)                UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=0x%04X", #VALUE, VALUE)
#define LOG_U16_D(VALUE)                UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_X32_D(VALUE)                UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=0x%08X", #VALUE, VALUE)
#define LOG_U32_D(VALUE)                UWB_LOG_DEBUG(SWUP_MODULE_NAME, "%s=%u", #VALUE, VALUE)
#define LOG_AU8_D(ARRAY, LEN)           UWB_LOG_AU8_DEBUG(SWUP_MODULE_NAME, #ARRAY, ARRAY, LEN)
#define LOG_MAU8_D(MESSAGE, ARRAY, LEN) UWB_LOG_AU8_DEBUG(SWUP_MODULE_NAME, MESSAGE, ARRAY, LEN)
#else
#define LOG_D(...)
#define LOG_X8_D(VALUE)
#define LOG_U8_D(VALUE)
#define LOG_X16_D(VALUE)
#define LOG_U16_D(VALUE)
#define LOG_X32_D(VALUE)
#define LOG_U32_D(VALUE)
#define LOG_AU8_D(ARRAY, LEN)
#define LOG_MAU8_D(MESSAGE, ARRAY, LEN)
#endif

#endif /* _PHNXPLOG_SWUP_H */
