/*
 * Copyright (C) 2019,2020 NXP
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

#ifndef UWB_CORE_TYPES_H_
#define UWB_CORE_TYPES_H_

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>

#if UWBIOT_OS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if UWBIOT_OS_NATIVE
typedef long BaseType_t;
#endif

#ifndef _MSC_VER
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int32_t INT32;
typedef int8_t INT8;
typedef int16_t INT16;
#endif /* _MSC_VER */

typedef unsigned char BOOLEAN;
#if 0
typedef uintptr_t UINTPTR;
typedef UINT32 TIME_STAMP;
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

typedef uint8_t UBYTE;

#ifdef __arm
#define PACKED __packed
#ifdef ANDROID_MW
#define INLINE __inline
#endif
#else
#define PACKED
#ifdef ANDROID_MW
#define INLINE
#endif
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN FALSE
#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/** API Parameters */
#ifdef __GNUC__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

#define USLEEP(time_in_ms) vTaskDelay(pdMS_TO_TICKS(time_in_ms / 1000))

#endif /* UWB_CORE_TYPES_H_ */
