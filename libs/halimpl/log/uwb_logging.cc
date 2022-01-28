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

#include "phNxpLogApis_HalUci.h"
#include "phOsalUwb.h"
#include <stdarg.h>
#include <stdio.h>
#include <UwbCore_Types.h>
#include <inttypes.h>
#if defined(__clang__)
#include <io.h> /* for isatty */
#endif

#if defined(_MSC_VER)
#include <windows.h>
#include <time.h>
#endif

#define COLOR_RED    "\033[0;31m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE   "\033[0;34m"
#define COLOR_RESET  "\033[0m"

#define szCRLF "\r\n"
#define szLF   "\n"

#if defined(_MSC_VER)
#define SHOW_TIMESTAMP_IN_LOGS 1
static HANDLE sStdOutConsoleHandle = INVALID_HANDLE_VALUE;
static void msvc_setColor(int level);
static void msvc_reSetColor(void);
#define szEOL szLF
#endif

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#include <unistd.h>
#include <time.h>
static void ansi_setColor(int level);
static void ansi_reSetColor(void);
#if AX_EMBEDDED
#define szEOL szCRLF
#else
#define szEOL szLF
#endif
#endif /* __GNUC__ && !defined(__ARMCC_VERSION) */

#ifndef szEOL
#define szEOL szCRLF
#endif

#if __APPLE__
#define USE_COLORED_LOGS 0
#else
#define USE_COLORED_LOGS 1
#endif

static const char *szLevel[] = {"ERROR", "WARN ", "INFO ", "DEBUG", "TX > ", "RX < "};

#if AX_EMBEDDED
#define TAB_SEPRATOR "\t"
#else
#define TAB_SEPRATOR "   "
#endif

/* Allocated in stack */
#define CLOG_BUFFER_SIZE 800
char cLogBuffer[CLOG_BUFFER_SIZE];

void *mLogMutex = NULL;

/* Call back pointer to give log data to other layes,
 * e.g. test framework to log data to common file */
fpUwb_LogPrintCb_t gfpLogPrintCb;

void phUwb_LogInit(void)
{
    if (phOsalUwb_CreateMutex(&mLogMutex) != UWBSTATUS_SUCCESS) {
        LOG_E("Error: phUwb_LogInit(), could not create mutex mLogMutex\n");
    }
}

void phUwb_LogDeInit()
{
    phOsalUwb_DeleteMutex(&mLogMutex);
    mLogMutex = NULL;
}

#if defined(SHOW_TIMESTAMP_IN_LOGS)
void phUwb_ConsoleTimeStamp(void)
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    PRINTF("%02u:%02u:%02u:%03u : ", lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
}
#endif

size_t phUwb_LogPrint_FormatBuffer_String(char *cLogbufferptr, size_t buffersize, const char *message, size_t totalSize)
{
    size_t alignmentSize = 0;
    size_t i;
    size_t totalCount = 0;
    size_t messageLen = strlen(message);
    if (messageLen < totalSize) {
        alignmentSize = (totalSize - messageLen);
    }

    for (i = 0; i < messageLen && buffersize > 1 && buffersize <= CLOG_BUFFER_SIZE; i++) {
        *cLogbufferptr++ = message[i];
        buffersize -= 1;
        totalCount += 1;
    }

    for (i = 0; i < alignmentSize && buffersize > 1 && buffersize < CLOG_BUFFER_SIZE; i++) {
        *cLogbufferptr++ = ' ';
        buffersize -= 1;
        totalCount += 1;
    }

    return totalCount;
}

size_t phUwb_LogPrint_FormatBuffer_Hex(
    char *cLogbufferptr, size_t buffersize, const unsigned char *array, size_t array_len)
{
    const char hex[] = "0123456789ABCDEF";
    size_t i;
    size_t totalCount = 0;
    for (i = 0; i < array_len && buffersize > 1 && buffersize < (CLOG_BUFFER_SIZE); ++i) {
        *cLogbufferptr++ = hex[(array[i] >> 4)];
        *cLogbufferptr++ = hex[array[i] & 0xF];
        buffersize -= 2;
        totalCount += 2;

        if (0 == ((i + 1) % 4)) {
            buffersize -= 1;
            totalCount += 1;
            *cLogbufferptr++ = ' ';
            *cLogbufferptr   = '\0';
        }
    }
    totalCount += 1;
    *cLogbufferptr = '\0';
    return totalCount;
}

void phUwb_LogPrint(const char *tag, int level, const char *fmt, ...)
{
    size_t buffersize = sizeof(cLogBuffer) - 1;
    size_t offset;
    char *cLogbufferptr    = cLogBuffer;
    const char seperator[] = {':', '\0'};

    phOsalUwb_LockMutex(mLogMutex);
    phUwb_LogSetColor(level);

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, tag, 8);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, seperator, 1);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, szLevel[level - 1], 5);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, seperator, 1);
    cLogbufferptr += offset;
    buffersize -= offset;

    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(cLogbufferptr, buffersize, fmt, args);
        va_end(args);
    }
#if defined(SHOW_TIMESTAMP_IN_LOGS)
    phUwb_ConsoleTimeStamp();
#endif
    LOG("%s", cLogBuffer);
    if (gfpLogPrintCb != NULL) {
#if defined(SHOW_TIMESTAMP_IN_LOGS)
        /* 16 is timestamp header */
        char timestamp_and_logmsg[16 + CLOG_BUFFER_SIZE];
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        snprintf(timestamp_and_logmsg,
            sizeof(timestamp_and_logmsg),
            "%02d:%02d:%02d:%03d : %s",
            lt.wHour,
            lt.wMinute,
            lt.wSecond,
            lt.wMilliseconds,
            cLogBuffer);
        gfpLogPrintCb(timestamp_and_logmsg);
#else
        gfpLogPrintCb(cLogBuffer);
#endif
        gfpLogPrintCb("\n");
    }
    phUwb_LogReSetColor();
    phOsalUwb_UnlockMutex(mLogMutex);
}

void phUwb_LogPrint_au8(const char *tag, int level, const char *message, const unsigned char *array, size_t array_len)
{
    size_t buffersize = CLOG_BUFFER_SIZE;
    size_t offset;
    char *cLogbufferptr    = cLogBuffer;
    const char seperator[] = {':', '\0'};

    phOsalUwb_LockMutex(mLogMutex);
    phUwb_LogSetColor(level);

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, tag, 8);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, seperator, 1);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, szLevel[level - 1], 5);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, seperator, 1);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, message, 20);
    cLogbufferptr += offset;
    buffersize -= offset;

    offset = phUwb_LogPrint_FormatBuffer_String(cLogbufferptr, buffersize, seperator, 1);
    cLogbufferptr += offset;
    buffersize -= offset;

    (void)phUwb_LogPrint_FormatBuffer_Hex(cLogbufferptr, buffersize, array, array_len);

#if defined(SHOW_TIMESTAMP_IN_LOGS)
    phUwb_ConsoleTimeStamp();
#endif
    LOG("%s", cLogBuffer);
    if (gfpLogPrintCb != NULL) {
#if defined(SHOW_TIMESTAMP_IN_LOGS)
        /* 16 is timestamp header */
        char timestamp_and_log[16 + CLOG_BUFFER_SIZE];
        SYSTEMTIME lt;
        GetLocalTime(&lt);
        snprintf(timestamp_and_log,
            sizeof(timestamp_and_log),
            "%02d:%02d:%02d:%03d : %s",
            lt.wHour,
            lt.wMinute,
            lt.wSecond,
            lt.wMilliseconds,
            cLogBuffer);
        gfpLogPrintCb(timestamp_and_log);
#else
        gfpLogPrintCb(cLogBuffer);
#endif
        gfpLogPrintCb("\n");
    }
    phUwb_LogReSetColor();
    phOsalUwb_UnlockMutex(mLogMutex);
}

void phUwb_LogPrint_Spinner(void)
{
    const char spin_pattern[] = {'[', '|', ']', '#', '[', '|', ']', '(', '+', ')'};
    static size_t spin_index  = 0;
    PUTCHAR(spin_pattern[spin_index++]);
    PUTCHAR('\r');
    if (spin_index >= sizeof(spin_pattern)) {
        spin_index = 0;
    }
}

void phUwb_LogPrintSetCb(fpUwb_LogPrintCb_t fpCbLogSzFn)
{
    gfpLogPrintCb = fpCbLogSzFn;
}

void phUwb_LogSetColor(int level)
{
#if defined(_MSC_VER)
    msvc_setColor(level);
#endif
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
    ansi_setColor(level);
#endif
}

void phUwb_LogReSetColor(void)
{
#if defined(_MSC_VER)
    msvc_reSetColor();
#endif
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
    ansi_reSetColor();
#endif
}

#if defined(_MSC_VER) && USE_COLORED_LOGS
static void msvc_setColor(int level)
{
#if USE_COLORED_LOGS
    WORD wAttributes = 0;
    if (sStdOutConsoleHandle == INVALID_HANDLE_VALUE) {
        sStdOutConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    switch (level) {
    case UWB_LOG_ERROR_LEVEL:
        wAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    case UWB_LOG_WARN_LEVEL:
        wAttributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
    case UWB_LOG_INFO_LEVEL:
        wAttributes = FOREGROUND_GREEN;
        break;
    case UWB_LOG_DEBUG_LEVEL:
        /* As of now put color here. All normal printfs would be in WHITE
             * Later, remove this color.
             */
        wAttributes = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case UWB_LOG_TX_LEVEL:
        wAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN;
        break;
    case UWB_LOG_RX_LEVEL:
        wAttributes = FOREGROUND_BLUE;
        break;
    default:
        wAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }
    SetConsoleTextAttribute(sStdOutConsoleHandle, wAttributes);
#endif // USE_COLORED_LOGS
}

static void msvc_reSetColor()
{
#if USE_COLORED_LOGS
    msvc_setColor(-1 /* default */);
#endif // USE_COLORED_LOGS
}
#endif

#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
static void ansi_setColor(int level)
{
#if USE_COLORED_LOGS
    if (isatty(fileno(stdout))) {
        return;
    }

    switch (level) {
    case UWB_LOG_ERROR_LEVEL:
        PRINTF(COLOR_RED);
        break;
    case UWB_LOG_WARN_LEVEL:
        PRINTF(COLOR_YELLOW);
        break;
    case UWB_LOG_INFO_LEVEL:
        PRINTF(COLOR_BLUE);
        break;
    case UWB_LOG_DEBUG_LEVEL:
        /* As of now put color here. All normal printfs would be in WHITE
             * Later, remove this color.
             */
        PRINTF(COLOR_GREEN);
        break;
    case UWB_LOG_TX_LEVEL:
        PRINTF(COLOR_BLUE);
        break;
    case UWB_LOG_RX_LEVEL:
        PRINTF(COLOR_GREEN);
        break;
    default:
        PRINTF(COLOR_RESET);
    }
#endif // USE_COLORED_LOGS
}

static void ansi_reSetColor()
{
#if USE_COLORED_LOGS
    if (isatty(fileno(stdout))) {
        return;
    }
    PRINTF(COLOR_RESET);
#endif // USE_COLORED_LOGS
}
#endif
