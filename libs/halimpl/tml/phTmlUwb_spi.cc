/*
 * Copyright 2012-2020 NXP.
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
#include "phNxpUwb_SpiTransport.h"
#include <stdlib.h>

#include "phNxpUciHal.h"
#include "phNxpUciHal_utils.h"

#include "phTmlUwb_spi.h"
#include "phUwbStatus.h"
#include "phNxpLogApis_TmlUwb.h"
#include "phUwb_BuildConfig.h"
/*********************** Global Variables *************************************/
/* UCI HAL Control structure */
extern phNxpUciHal_Control_t nxpucihal_ctrl;

/*******************************************************************************
**
** Function         phTmlUwb_spi_open_and_configure
**
** Description      Open and configure SR100 device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - open_and_configure operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
UWBSTATUS phTmlUwb_spi_open_and_configure(pphTmlUwb_Config_t pConfig, void **pLinkHandle)
{
    UWBSTATUS retStatus = UWBSTATUS_INVALID_DEVICE;
    static int initDone = 0xFF;
    if (phNxpUwb_HeliosInit() == UWBSTATUS_SUCCESS) {
        initDone  = 0;
        retStatus = UWBSTATUS_SUCCESS;
    }
    *pLinkHandle = (void *)&initDone;
    return retStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_close
**
** Description      SPI Cleanup
**
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_spi_close()
{
    phNxpUwb_HeliosDeInit();
}
/*******************************************************************************
**
** Function         phTmlUwb_spi_read
**
** Description      Reads requested number of bytes from SR100 device into given
**                  buffer
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlUwb_spi_read(void *pDevHandle, uint8_t *pBuffer, int nNbBytesToRead)
{
    int numRead = 0;
    numRead     = phNxpUwb_UciRead(pBuffer);
    return numRead;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR100 device
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlUwb_spi_write(void *pDevHandle, uint8_t *pBuffer, uint16_t nNbBytesToWrite)
{
    int numWrote = 0;
    numWrote     = phNxpUwb_UciWrite(pBuffer, nNbBytesToWrite);

    if (numWrote <= 0) {
        return -1;
    }
    return numWrote;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_reset
**
** Description      Reset SR100 device, using VEN pin
**
** Parameters       pDevHandle     - valid device handle
**                  level          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int phTmlUwb_spi_reset(void *pDevHandle, long level)
{
    NXPLOG_UWB_TML_D("phTmlUwb_spi_reset(), VEN level %ld", level);
    phTmlUwb_ReadAbort();
    if (NULL == pDevHandle) {
        return -1;
    }
    return 1;
}
