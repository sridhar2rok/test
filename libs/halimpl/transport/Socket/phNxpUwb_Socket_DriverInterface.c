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

#include <phNxpUwb_DriverInterface.h>
#include "phUwbErrorCodes.h"

#define PRINT_ENTRY() printf("DBG:TODO:%s\n", __FUNCTION__)
