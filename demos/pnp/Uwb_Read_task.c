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
#include "phUwb_BuildConfig.h"

#if 1 //( UWBCORE_SDK_BUILDCONFIG == UWB_BUILD_PLUG_AND_PLAY_MODE )

#include "UwbCore_Types.h"
#include <stdint.h>
/* Freescale includes */
#include "fsl_debug_console.h"
#if UWBIOT_UWBD_SR100T
#include "fsl_port.h"
#include "fsl_lpspi.h"
#include "UWB_Hbci.h"
#endif
#include "fsl_gpio.h"
/* UWB includes */
#include "UWB_Spi_Driver_Interface.h"
#include "UWB_GpioIrq.h"
#include "UWB_Evt.h"
#include "UwbPnpInternal.h"

void UCI_ReaderTask(void *args)
{
    static uint8_t Buffer[2176];

    while (1) {
        int numRead = 0;
        numRead     = UWB_SpiUciRead(Buffer);
        if (numRead == 0) {
            DEBUGOUT("Spi Read Error, Zero bytes read\n");
            continue;
        }
        DEBUGOUT("received uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
#if (ENABLE_UWB_RESPONSE == ENABLED)
        DEBUGOUT("read returned count is %d\n", numRead);
        DEBUGOUT("UCI rsp:\n");
        for (int i = 0; i < numRead; i++) {
            DEBUGOUT(" %02x", Buffer[i]);
        }
        DEBUGOUT("\n");
#endif
        if (!Uwb_Is_Hif_Active()) {
            if ((Buffer[0] & 0xF0) == 0x40) {
                uint32_t error;
                xSemaphoreTake(mHifWriteMutex, portMAX_DELAY);
                DEBUGOUT("sending rsp: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
                error = UWB_Hif_SendUCIRsp(Buffer, numRead);
                if (error != 0) {
                    DEBUGOUT("UCI_READER: error sending over HIF [%d]\n", error);
                }
                xSemaphoreGive(mHifWriteMutex);
            }
            else {
                if (uxQueueSpacesAvailable(mHifWriteQueue) > 0) {
                    tlv_t tlv;
                    tlv.size  = numRead;
                    tlv.value = (void *)pvPortMalloc(tlv.size * sizeof(uint8_t));
                    if (tlv.value != NULL) {
                        memcpy((uint8_t *)tlv.value, Buffer, tlv.size);
                        xQueueSend(mHifWriteQueue, &tlv, 0 /*No Wait time if queue is Full*/);
                    }
                    else {
                        DEBUGOUT("UCI_ReaderTask: Unable to Allocate Memory of %d, Memory Full:\n", tlv.size);
                    }
                }
                else {
                    DEBUGOUT("Queue is FULL, ignoring notification\n");
                }
            }
        }
        else {
            DEBUGOUT("USB detached, USB channel reset.\n");
            DEBUGOUT("missed uci rsp/ntf: 0x%X 0x%X 0x%X 0x%X\n", Buffer[0], Buffer[1], Buffer[2], Buffer[3]);
            Uwb_Reset_Hif_State(false);
        }
    }
}

#endif
