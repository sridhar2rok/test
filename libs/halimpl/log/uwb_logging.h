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

#ifndef LOGGING_H
#define LOGGING_H

#include "fsl_debug_console.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
void phUwb_LogInit(void);
void phUwb_LogDeInit(void);

#define LOG(...)         \
    PRINTF(__VA_ARGS__); \
    PRINTF("\n");

#define ALOGD(...)          LOG(__VA_ARGS__)
#define ALOGE(...)          LOG(__VA_ARGS__)
#define ALOGD_IF(cond, ...) LOG(__VA_ARGS__)
// Make Zero to disable
// Define Logging Levels
#define UWB_LOG_DEFAULT_LEVEL 0x00
#define UWB_LOG_ERROR_LEVEL   0x01
#define UWB_LOG_WARN_LEVEL    0x02
#define UWB_LOG_INFO_LEVEL    0x03
#define UWB_LOG_DEBUG_LEVEL   0x04
#define UWB_LOG_TX_LEVEL      0x05
#define UWB_LOG_RX_LEVEL      0x06

/*
 * Define Global Log level. Depending on this, sub module levels are enabled/disabled.
 * To switch off all the logging part then set the macro to 0 value.
 */
#define UWB_GLOBAL_LOG_LEVEL UWB_LOG_INFO_LEVEL

#define LOG_PRINT phUwb_LogPrint

void phUwb_LogPrint(const char *tag, int level, const char *fmt, ...);
void phUwb_LogPrint_au8(const char *tag, int level, const char *message, const unsigned char *array, size_t array_len);
/* show a spinnner that keeps roatiting at one fixed place, just to show progress */
void phUwb_LogPrint_Spinner(void);

/* for log call back from test frameworks
 * Not to be used in production */
typedef void (*fpUwb_LogPrintCb_t)(const char *szString);
void phUwb_LogPrintSetCb(fpUwb_LogPrintCb_t fpCbLogSzFn);

#if (UWB_GLOBAL_LOG_LEVEL >= UWB_LOG_ERROR_LEVEL)
#define UWB_LOG_ERROR(Component, ...) LOG_PRINT(Component, UWB_LOG_ERROR_LEVEL, __VA_ARGS__)
#define UWB_LOG_AU8_ERROR(COMP, Message, Array, Size) \
    phUwb_LogPrint_au8(COMP, UWB_LOG_ERROR_LEVEL, Message, Array, Size)
#else
#define UWB_LOG_ERROR(...)
#define UWB_LOG_AU8_ERROR(...)
#endif

#if (UWB_GLOBAL_LOG_LEVEL >= UWB_LOG_WARN_LEVEL)
#define UWB_LOG_WARN(Component, ...)                 LOG_PRINT(Component, UWB_LOG_WARN_LEVEL, __VA_ARGS__)
#define UWB_LOG_AU8_WARN(COMP, Message, Array, Size) phUwb_LogPrint_au8(COMP, UWB_LOG_WARN_LEVEL, Message, Array, Size)
#else
#define UWB_LOG_WARN(...)
#define UWB_LOG_AU8_WARN(...)
#endif

#if (UWB_GLOBAL_LOG_LEVEL >= UWB_LOG_INFO_LEVEL)
#define UWB_LOG_INFO(Component, ...)                 LOG_PRINT(Component, UWB_LOG_INFO_LEVEL, __VA_ARGS__)
#define UWB_LOG_AU8_INFO(COMP, Message, Array, Size) phUwb_LogPrint_au8(COMP, UWB_LOG_INFO_LEVEL, Message, Array, Size)
#else
#define UWB_LOG_INFO(...)
#define UWB_LOG_AU8_INFO(...)
#endif

#if (UWB_GLOBAL_LOG_LEVEL >= UWB_LOG_DEBUG_LEVEL)
#define UWB_LOG_DEBUG(Component, ...) LOG_PRINT(Component, UWB_LOG_DEBUG_LEVEL, __VA_ARGS__)
#define UWB_LOG_AU8_DEBUG(COMP, Message, Array, Size) \
    phUwb_LogPrint_au8(COMP, UWB_LOG_DEBUG_LEVEL, Message, Array, Size)
#else
#define UWB_LOG_DEBUG(...)
#define UWB_LOG_AU8_DEBUG(...)
#endif

#define LOG_PRI phUwb_LogPrint

#define phUwb_LogMsg(trace_set_mask, ...) LOG(__VA_ARGS__)

void phUwb_LogSetColor(int level);
void phUwb_LogReSetColor(void);

#ifdef __cplusplus
} /* extern "c" */
#endif
#endif
