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

#ifndef _APP_RECOVERY_H__
#define _APP_RECOVERY_H__

enum errorScenario
{
    TIME_OUT = 1,
    FW_CRASH,
    APP_CLEANUP
};

typedef enum errorScenario errorScenario_t;

void Start_AppRecoveryTask();
void Handle_ErrorScenario(errorScenario_t scenario);

#endif
