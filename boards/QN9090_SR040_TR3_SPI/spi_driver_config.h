/* Copyright 2018-2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#ifndef UWB_HELIOS_H
#define UWB_HELIOS_H

// #include "phUwb_BuildConfig.h"

#define MAX_NAME_LEN               20
#define MAX_FW_VERSION_LEN         10
#define HBCI_HEADER_SIZE           4
#define UWB_MAX_HELIOS_CMD_SIZE    (2048 + HBCI_HEADER_SIZE)
#define UWB_MAX_HELIOS_RSP_SIZE    64
#define kSPI_MasterPcsContinuous   //kLPSPI_MasterPcsContinuous
#define SPI_MasterTransferBlocking //LPSPI_MasterTransferBlocking
#define spi_transfer_t             //lpspi_transfer_t
#define spi_master_config_t        //lpspi_master_config_t
#define SPI_MasterInit             //LPSPI_MasterInit

#endif
