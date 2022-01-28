/*
  * Copyright (C) 2012-2020 NXP Semiconductors
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

#include <stdio.h>
#include "phOsalUwb.h"
#include "phNxpUwb_Common.h"
#include "cmsis_gcc.h"

#ifdef UWBIOT_USE_FTR_FILE
#include "uwb_iot_ftr.h"
#else
#include "uwb_iot_ftr_default.h"
#endif

#if (UWBIOT_UWBD_SR040)

/*
 *  Backoff Delay helps in improving the performance of complete system
 *          +
 *  Retry   |
 *  count   |                                         _________________
 *          |                                        |
 *          |                                   _____|
 *          |                                  |
 *          |                             _____|
 *          |                            |
 *          |                      ______|
 *          |                     /
 *          |                    /
 *          |                   /
 *          |                  /
 *          |                 /
 *          |  ______________/
 *          | /
 *          +----------------------------------------------------------------------+
 *            |<--- NOP ---->|<-------------ms---------------->|<-Timeout->|
 *
 */

void phNxpUwb_BackoffDelayReset(uint16_t *stepDelay)
{
    *stepDelay = 0;
}

int phNxpUwb_BackoffDelay(uint16_t *stepDelay)
{
    int timeout = 0;
    int msDelay = 0;
    if (*stepDelay <= 100) {
        __NOP();
        __NOP();
    }
    else if (*stepDelay <= 120) {
        /* Wait for millisec in incremental way till 10ms */
        msDelay = ((*stepDelay) % 100);
        msDelay = (msDelay <= 10) ? msDelay : 10;
        phOsalUwb_Delay(msDelay);
    }
    else if (*stepDelay > 120 && *stepDelay <= 150) {
        /* Wait for millisec in steps till 50ms
         *
         * +---------------------+----------------------+
         * | stepDelay           | Backoff              |
         * +---------------------+----------------------+
         * | 121 - 129           | 20ms                 |
         * | 130 - 139           | 30ms                 |
         * | 140 - 149           | 40ms                 |
         * | 150                 | 50ms                 |
         * +---------------------+----------------------+
         *
         */
        msDelay = ((*stepDelay % 100) - ((*stepDelay % 100) % 10));
        phOsalUwb_Delay(msDelay);
    }
    else {
        /* Wait for max time before backoff timeout */
        phOsalUwb_Delay(50);
        if ((*stepDelay) > BACKOFF_TIMEOUT_VALUE) {
            timeout = 1;
        }
    }
    (*stepDelay)++;
    return timeout;
}
#endif
