/* Copyright 2019,2020 NXP
 *
 * This software is owned or controlled by NXP and may only be used
 * strictly in accordance with the applicable license terms.  By expressly
 * accepting such terms or by downloading, installing, activating and/or
 * otherwise using the software, you are agreeing that you have read, and
 * that you agree to comply with and are bound by, such license terms.  If
 * you do not agree to be bound by the applicable license terms, then you
 * may not retain, install, activate or otherwise use the software.
 */

#include "AppStateManagement.h"
#include "AppInternal.h"
#include "phOsalUwb.h"

AppContext_t *curAppContext;

void initAppStateInfo(AppContext_t *pAppContext)
{
    if (pAppContext != NULL) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            pAppContext->appStates[count].session_id   = 0;
            pAppContext->appStates[count].currentState = UWBAPI_SESSION_ERROR;
            pAppContext->appStates[count].opType       = 0xFF;
        }
    }
    curAppContext = pAppContext;
}

BOOLEAN addStateInfo(AppContext_t *pAppContext, AppStateInfo_t *pCurrentState)
{
    BOOLEAN status = FALSE;
    if (pAppContext != NULL && pCurrentState != NULL && pCurrentState->session_id != 0) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].currentState == UWBAPI_SESSION_ERROR) {
                pAppContext->appStates[count].session_id   = pCurrentState->session_id;
                pAppContext->appStates[count].opType       = pCurrentState->opType;
                pAppContext->appStates[count].currentState = pCurrentState->currentState;
                status                                     = TRUE;
                break;
            }
        }
    }
    return status;
}

BOOLEAN removeStateInfo(AppContext_t *pAppContext, UINT32 seesionId)
{
    BOOLEAN status = FALSE;
    if (pAppContext != NULL) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].session_id == seesionId) {
                pAppContext->appStates[count].session_id   = 0;
                pAppContext->appStates[count].opType       = 0xFF;
                pAppContext->appStates[count].currentState = UWBAPI_SESSION_ERROR;
                status                                     = TRUE;
                break;
            }
        }
    }
    return status;
}

BOOLEAN updateState(AppContext_t *pAppContext, UINT32 sessionId, UINT8 state)
{
    BOOLEAN status = FALSE;
    if (pAppContext != NULL && sessionId != 0) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].session_id == sessionId) {
                pAppContext->appStates[count].currentState = state;
                status                                     = TRUE;
                break;
            }
        }
    }
    return status;
}

UINT8 getCurrentState(AppContext_t *pAppContext, UINT32 sessionId)
{
    UINT8 state = UWBAPI_SESSION_ERROR;
    if (pAppContext != NULL && sessionId != 0) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].session_id == sessionId) {
                state = pAppContext->appStates[count].currentState;
                break;
            }
        }
    }
    return state;
}

void getActiveRangingSessionCount(AppContext_t *pAppContext, UINT32 sessionIds[], UINT8 *pNoOfActiveSessions)
{
    UINT8 sessionCount = 0;
    if (pAppContext != NULL && pNoOfActiveSessions != NULL) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].opType == UWBD_RANGING_SESSION &&
                pAppContext->appStates[count].currentState == UWBAPI_SESSION_ACTIVATED) {
                sessionIds[sessionCount++] = pAppContext->appStates[count].session_id;
            }
        }
        *pNoOfActiveSessions = sessionCount;
    }
}

void getActiveSessionIds(AppContext_t *pAppContext, UINT32 sessionIds[], UINT8 *pNoOfActiveSessions)
{
    UINT8 sessionCount = 0;

    if (pAppContext != NULL && pNoOfActiveSessions != NULL) {
        for (UINT8 count = 0; count < MAX_SESSIONS; count++) {
            if (pAppContext->appStates[count].currentState == UWBAPI_SESSION_ACTIVATED ||
                pAppContext->appStates[count].currentState == UWBAPI_SESSION_INIT_SUCCESS ||
                pAppContext->appStates[count].currentState == UWBAPI_SESSION_IDLE) {
                sessionIds[sessionCount++] = pAppContext->appStates[count].session_id;
            }
        }
        *pNoOfActiveSessions = sessionCount;
    }
}
void cleanUpAppContext()
{
    UINT8 activeSessionCount = 0;
    UINT32 sessionId[MAX_SESSIONS];
    UINT8 status;

    memset(sessionId, 0x00, sizeof(sessionId));
    getActiveSessionIds(curAppContext, sessionId, &activeSessionCount);
    for (UINT8 i = 0; i < activeSessionCount; i++) {
        status = UwbApi_SessionDeinit(sessionId[i]);
        if (status != UWBAPI_STATUS_OK) {
            Log("Session DeInint failed");
        }
        else {
            removeStateInfo(curAppContext, sessionId[i]);
        }
    }

    initAppStateInfo(curAppContext);
}
