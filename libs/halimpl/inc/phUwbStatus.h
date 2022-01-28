/*
 * Copyright (C) 2012-2019 NXP Semiconductors
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

/*
 * UWB Status Values - Function Return Codes
 */

#ifndef PHUWBSTATUS_H
#define PHUWBSTATUS_H

#include <phUwbTypes.h>

/* Internally required by PHUWBSTVAL. */
#define PHUWBSTSHL8 (8U)
/* Required by PHUWBSTVAL. */
#define PHUWBSTBLOWER ((UWBSTATUS)(0x00FFU))

/*
 *  UWB Status Composition Macro
 *
 *  This is the macro which must be used to compose status values.
 *
 *  phUwbCompID Component ID, as defined in phUwbCompId.h .
 *  phUwbStatus Status values, as defined in phUwbStatus.h .
 *
 *  The macro is not required for the UWBSTATUS_SUCCESS value.
 *  This is the only return value to be used directly.
 *  For all other values it shall be used in assignment and conditional
 * statements, e.g.:
 *     UWBSTATUS status = PHUWBSTVAL(phUwbCompID, phUwbStatus); ...
 *     if (status == PHUWBSTVAL(phUwbCompID, phUwbStatus)) ...
 */
#define PHUWBSTVAL(phUwbCompID, phUwbStatus)  \
    (((phUwbStatus) == (UWBSTATUS_SUCCESS)) ? \
            (UWBSTATUS_SUCCESS) :             \
            ((((UWBSTATUS)(phUwbStatus)) & (PHUWBSTBLOWER)) | (((uint16_t)(phUwbCompID)) << (PHUWBSTSHL8))))

/*
 * PHUWBSTATUS
 * Get grp_retval from Status Code
 */
#define PHUWBSTATUS(phUwbStatus) ((phUwbStatus)&0x00FFU)
#define PHUWBCID(phUwbStatus)    (((phUwbStatus)&0xFF00U) >> 8)

/*
 *  Status Codes
 *
 *  Generic Status codes for the UWB components. Combined with the Component ID
 *  they build the value (status) returned by each function.
 *  Example:
 *      grp_comp_id "Component ID" -  e.g. 0x10, plus
 *      status code as listed in this file - e.g. 0x03
 *      result in a status value of 0x0003.
 */

/*
 * The function indicates successful completion
 */
#define UWBSTATUS_SUCCESS (0x0000)

/*
 *  The function indicates successful completion
 */
#define UWBSTATUS_OK (UWBSTATUS_SUCCESS)

/*
 * At least one parameter could not be properly interpreted
 */
#define UWBSTATUS_INVALID_PARAMETER (0x0001)

/*
 * The buffer provided by the caller is too small
 */
#define UWBSTATUS_BUFFER_TOO_SMALL (0x0003)

/*

 * Device specifier/handle value is invalid for the operation
 */
#define UWBSTATUS_INVALID_DEVICE (0xFFFF)
/*
 * The function executed successfully but could have returned
 * more information than space provided by the caller
 */
#define UWBSTATUS_MORE_INFORMATION (0x0008)

/*
 * Not enough resources Memory, Timer etc(e.g. allocation failed.)
 */
#define UWBSTATUS_INSUFFICIENT_RESOURCES (0x000C)

/*
 * A non-blocking function returns this immediately to indicate
 * that an internal operation is in progress
 */
#define UWBSTATUS_PENDING (0x000D)

/*
 * A board communication error occurred
 * (e.g. Configuration went wrong)
 */
#define UWBSTATUS_BOARD_COMMUNICATION_ERROR (0x000F)

/*
 * Invalid State of the particular state machine
 */
#define UWBSTATUS_INVALID_STATE (0x0011)

/*
 * This Layer is Not initialized, hence initialization required.
 */
#define UWBSTATUS_NOT_INITIALISED (0x0031)

/*
 * The Layer is already initialized, hence initialization repeated.
 */
#define UWBSTATUS_ALREADY_INITIALISED (0x0032)

/*
 * Feature not supported
 */
#define UWBSTATUS_FEATURE_NOT_SUPPORTED (0x0033)

/*  The Unregistration command has failed because the user wants to unregister
 * on
 * an element for which he was not registered
 */
#define UWBSTATUS_NOT_REGISTERED (0x0034)

/* The Registration command has failed because the user wants to register on
 * an element for which he is already registered
 */
#define UWBSTATUS_ALREADY_REGISTERED (0x0035)

/*  Single Tag with Multiple
    Protocol support detected */
#define UWBSTATUS_MULTIPLE_PROTOCOLS (0x0036)

/*
 * Feature not supported
 */
#define UWBSTATUS_MULTIPLE_TAGS (0x0037)

/*
 * A DESELECT event has occurred
 */
#define UWBSTATUS_DESELECTED (0x0038)

/*
 * A RELEASE event has occurred
 */
#define UWBSTATUS_RELEASED (0x0039)

/*
 * The operation is currently not possible or not allowed
 */
#define UWBSTATUS_NOT_ALLOWED (0x003A)

/*
 * FW version error while performing FW download,
 * FW major version mismatch (cannot downgrade FW major version) or FW version
 * already upto date
 * User may be trying to flash Mobile FW on top of Infra FW, which is not
 * allowed
 * Download appropriate version of FW
 */
#define UWBSTATUS_FW_VERSION_ERROR (0x003C)

/*
 *  The system is busy with the previous operation.
 */
#define UWBSTATUS_BUSY (0x006F)

/* NDEF Mapping error codes */

/* The remote device (type) is not valid for this request. */
#define UWBSTATUS_INVALID_REMOTE_DEVICE (0x001D)

/* Read operation failed */
#define UWBSTATUS_READ_FAILED (0x0014)

/*
 * Write operation failed
 */
#define UWBSTATUS_WRITE_FAILED (0x0015)

/* Non Ndef Compliant */
#define UWBSTATUS_NO_NDEF_SUPPORT (0x0016)

/* Could not proceed further with the write operation: reached card EOF*/
#define UWBSTATUS_EOF_NDEF_CONTAINER_REACHED (0x001A)

/* Incorrect number of bytes received from the card*/
#define UWBSTATUS_INVALID_RECEIVE_LENGTH (0x001B)

/* The data format/composition is not understood/correct. */
#define UWBSTATUS_INVALID_FORMAT (0x001C)

/* There is not sufficient storage available. */
#define UWBSTATUS_INSUFFICIENT_STORAGE (0x001F)

/* The Ndef Format procedure has failed. */
#define UWBSTATUS_FORMAT_ERROR (0x0023)

/* The UCI Cedit error */
#define UWBSTATUS_CREDIT_TIMEOUT (0x0024)

/*
 * Response Time out for the control message(UWBC not responded)
 */
#define UWBSTATUS_RESPONSE_TIMEOUT (0x0025)

/*
 * Device is already connected
 */
#define UWBSTATUS_ALREADY_CONNECTED (0x0026)

/*
 * Device is already connected
 */
#define UWBSTATUS_ANOTHER_DEVICE_CONNECTED (0x0027)

/*
 * Single Target Detected and Activated
 */
#define UWBSTATUS_SINGLE_TAG_ACTIVATED (0x0028)

/*
 * Single Target Detected
 */
#define UWBSTATUS_SINGLE_TAG_DISCOVERED (0x0029)

/*
 * Secure element Detected and Activated
 */
#define UWBSTATUS_SECURE_ELEMENT_ACTIVATED (0x0028)

/*
 * Unknown error Status Codes
 */
#define UWBSTATUS_UNKNOWN_ERROR (0x00FE)

/*
 * Status code for failure
 */
#define UWBSTATUS_FAILED (0x00FF)

/*
 * The function/command has been aborted
 */
#define UWBSTATUS_CMD_ABORTED (0x0002)

/*
 * No target found after poll
 */
#define UWBSTATUS_NO_TARGET_FOUND (0x000A)

/* Attempt to disconnect a not connected remote device. */
#define UWBSTATUS_NO_DEVICE_CONNECTED (0x000B)

/* External RF field detected. */
#define UWBSTATUS_EXTERNAL_RF_DETECTED (0x000E)

/* Message is not allowed by the state machine
 * (e.g. configuration went wrong)
 */
#define UWBSTATUS_MSG_NOT_ALLOWED_BY_FSM (0x0010)

/*
 * No access has been granted
 */
#define UWBSTATUS_ACCESS_DENIED (0x001E)

/* No registry node matches the specified input data. */
#define UWBSTATUS_NODE_NOT_FOUND (0x0017)

#if (UWB_NXP_ESE == TRUE)
#define UWBSTATUS_SMX_SPI_STATE (0x00F0)

/* The current module is free ; one might use it */
#define UWBSTATUS_SMX_IDLE_STATE (0x00F1)

/* The current module is busy with wired; one might use it */
#define UWBSTATUS_SMX_WIRED_STATE (0x00F3)

/* The current module is free ; one might use it */
#define UWBSTATUS_UWBC_DWNLD_STATE (0x00F4)

#else
/* The current module is busy ; one might retry later */
#define UWBSTATUS_SMX_BAD_STATE (0x00F0)

#endif

/* The Abort mechanism has failed for unexpected reason: user can try again*/
#define UWBSTATUS_ABORT_FAILED (0x00F2)

/* The Registration command has failed because the user wants to register as
 * target
 * on a operating mode not supported
 */
#define UWBSTATUS_REG_OPMODE_NOT_SUPPORTED (0x00F5)

/*
 * Shutdown in progress, cannot handle the request at this time.
 */
#define UWBSTATUS_SHUTDOWN (0x0091)

/*
 * Target is no more in RF field
 */
#define UWBSTATUS_TARGET_LOST (0x0092)

/*
 * Request is rejected
 */
#define UWBSTATUS_REJECTED (0x0093)

/*
 * Target is not connected
 */
#define UWBSTATUS_TARGET_NOT_CONNECTED (0x0094)

/*
 * Invalid handle for the operation
 */
#define UWBSTATUS_INVALID_HANDLE (0x0095)

/*
 * Process aborted
 */
#define UWBSTATUS_ABORTED (0x0096)

/*
 * Requested command is not supported
 */
#define UWBSTATUS_COMMAND_NOT_SUPPORTED (0x0097)

/*
 * Tag is not NDEF compilant
 */
#define UWBSTATUS_NON_NDEF_COMPLIANT (0x0098)

/*
 * Not enough memory available to complete the requested operation
 */
#define UWBSTATUS_NOT_ENOUGH_MEMORY (0x001F)

/*
 * Indicates incoming connection
 */
#define UWBSTATUS_INCOMING_CONNECTION (0x0045)

/*
 * Indicates Connection was successful
 */
#define UWBSTATUS_CONNECTION_SUCCESS (0x0046)

/*
 * Indicates Connection failed
 */
#define UWBSTATUS_CONNECTION_FAILED (0x0047)

#define UWBSTATUS_FILE_NOT_FOUND (0x0048)

#define UWBSTATUS_FORCE_FWDWNLD (0x0049)

#endif /* PHUWBSTATUS_H */
