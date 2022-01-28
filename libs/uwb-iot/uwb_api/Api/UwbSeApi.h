/*
 *
 * Copyright 2018-2020 NXP.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading,installing, activating and/or otherwise
 * using the software, you are agreeing that you have read,and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 *
 */

#ifndef UWB_SEAPI_H
#define UWB_SEAPI_H

#include "phUwb_BuildConfig.h"

#if defined(ENABLE_NFC) && (ENABLE_NFC == TRUE)

/**
 * \brief write directly to the NFCC
 *                              BLOCKING - this will block until the write is
 * complete
 *
 * \param dataLen   - [in] number of bytes to be written to the NFCC
 * \param pData     - [in] pointer to a buffer containing the data that will be
 *                    written
 * \param pRespSize - [in] the size of the response buffer being passed
 *                    in, [out] the number of bytes actually written
 *                    into the respBuf
 * \param pRespBuf  - [out] pointer to a buffer to receive the response from the NFCC
 *
 * \retval #UWB_SEAPI_STATUS_OK - if successful
 * \retval #UWB_SEAPI_STATUS_INVALID_PARAM - if invalid parameters are passed
 * \retval #UWB_SEAPI_STATUS_NOT_INITIALIZED if SE is not initialized
 * \retval #UWB_SEAPI_STATUS_FAILED otherwise
 */
EXTERNC UINT8 UwbSeApi_NciRawCmdSend(UINT16 dataLen, UINT8 *pData, UINT16 *pRespSize, UINT8 *pRespBuf);

#endif

#endif
