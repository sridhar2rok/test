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

/* Basic type definitions */
#include "phTmlUwb.h"
#include "phUwbTypes.h"

#define SR100_MAGIC          0xEA
#define SR100_SET_PWR        _IOW(SR100_MAGIC, 0x01, long)
#define SR100_SET_DBG        _IOW(SR100_MAGIC, 0x02, long)
#define SR100_GET_THROUGHPUT _IOW(SR100_MAGIC, 0x05, long)

#define PWR_DISABLE        0
#define PWR_ENABLE         1
#define ABORT_READ_PENDING 2

#define NORMAL_MODE_HEADER_LEN 4
#define NORMAL_MODE_LEN_OFFSET 3

#define EXTENDED_SIZE_LEN_OFFSET 1
#define UCI_EXTENDED_PKT_MASK    0xC0
#define UCI_EXTENDED_SIZE_SHIFT  6
#define UCI_NORMAL_PKT_SIZE      0
#define UCI_EXT_PKT_SIZE_512B    1
#define UCI_EXT_PKT_SIZE_1K      2
#define UCI_EXT_PKT_SIZE_2K      3

#define UCI_PKT_SIZE_512B 512
#define UCI_PKT_SIZE_1K   1024
#define UCI_PKT_SIZE_2K   2048

/* Function declarations */
UWBSTATUS phTmlUwb_spi_open_and_configure(pphTmlUwb_Config_t pConfig, void **pLinkHandle);
void phTmlUwb_spi_close();
int phTmlUwb_spi_read(void *pDevHandle, uint8_t *pBuffer, int nNbBytesToRead);
int phTmlUwb_spi_write(void *pDevHandle, uint8_t *pBuffer, uint16_t nNbBytesToWrite);
int phTmlUwb_spi_reset(void *pDevHandle, long level);
