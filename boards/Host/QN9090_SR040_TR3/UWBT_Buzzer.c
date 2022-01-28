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
#include "UWBT_BuildConfig.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "fsl_pwm.h"
#include "fsl_debug_console.h"
#include "driver_config.h"

#include "UWBT_Buzzer.h"

#define BUZZER_QUEUE_LEN 25
#define BUZZER_CLK_FREQ  320000

typedef struct
{
    uint32_t freq;
    uint32_t ms;
} UWB_BuzzerAction;

static QueueHandle_t buzzQueue;
static TimerHandle_t buzzTimer;
static volatile bool buzzIdle = true;
static pwm_setup_t pwmChan;

static void UWBT_BuzzerFlush(void)
{
    xQueueReset(buzzQueue);
    PWM_StopTimer(BUZZER_PWM, BUZZER_PWM_CH);
    buzzIdle = true;
}

static void UWBT_PlayNext(void)
{
    UWB_BuzzerAction action;
    //uint32_t pwmSourceClockFreq;
    //uint32_t pwmChannelClockFreq;
    uint16_t closestFreq;

    if (xQueueReceive(buzzQueue, &action, 0) == pdFALSE) {
        PRINTF("ERROR: could not retrieve next BUZZER action\n");
        return;
    }

    if (xTimerChangePeriod(buzzTimer, pdMS_TO_TICKS(action.ms), 0) == pdFALSE) {
        PRINTF("ERROR: could not start BUZZER timer\n");
        return;
    }

    if (action.freq > 0) {
        closestFreq = BUZZER_CLK_FREQ / action.freq;
    }
    else {
        closestFreq = 0;
    }

    /* Get the default source clock frequency */
    //pwmSourceClockFreq = CLOCK_GetFreq(kCLOCK_Pwm);

    /* Set up PWM channel to generate PWM pulse of 10ms with a starting 100% duty cycle */
    pwmChan.pol_ctrl      = kPWM_SetHighOnMatchLowOnPeriod;
    pwmChan.dis_out_level = kPWM_SetLow;
    pwmChan.prescaler_val = 99;
    //pwmChannelClockFreq   = pwmSourceClockFreq / (1 + pwmChan.prescaler_val);
    pwmChan.period_val = closestFreq;
    /* Compare value starts same as period value to give a 100% starting duty cycle */
    pwmChan.comp_val = closestFreq / 2;
    PWM_SetupPwm(BUZZER_PWM, BUZZER_PWM_CH, (pwm_setup_t *)&pwmChan);

    /* Clear interrupt status for PWM channel */
    PWM_ClearStatusFlags(BUZZER_PWM, BUZZER_PWM_CH);

    /* Clear interrupt status for PWM channel */
    PWM_ClearStatusFlags(BUZZER_PWM, BUZZER_PWM_CH);

    /* Enable IRQ in NVIC for PWM channel 3 */
    //EnableIRQ(BUZZER_PWM_IRQ_NAME);

    /* Enable PWM channel interrupt */
    //PWM_EnableInterrupts(BUZZER_PWM, BUZZER_PWM_CH);

    /* Start the PWM generation channel */
    PWM_StartTimer(BUZZER_PWM, BUZZER_PWM_CH);

    buzzIdle = false;
}

bool UWBT_BuzzerPlayTone(uint32_t freq, uint32_t ms)
{
    UWB_BuzzerAction action = {
        .freq = freq,
        .ms   = ms,
    };

    if (xQueueSend(buzzQueue, &action, 0) == pdFALSE) {
        PRINTF("ERROR: could not push BUZZER action\n");
        return false;
    }

    if (buzzIdle) {
        UWBT_PlayNext();
    }
    return true;
}

void UWBT_TimerCallback(TimerHandle_t xTimer)
{
    if (uxQueueMessagesWaiting(buzzQueue)) {
        UWBT_PlayNext();
    }
    else {
        PWM_StopTimer(BUZZER_PWM, BUZZER_PWM_CH);
        buzzIdle = true;
    }
}

bool UWBT_BuzzerInit(void)
{
    pwm_config_t pwmConfig;

    buzzTimer = xTimerCreate("BUZZ TMR", 1, false, NULL, UWBT_TimerCallback);
    if (!buzzTimer) {
        PRINTF("ERROR: could not create BUZZER timer\n");
        return false;
    }

    buzzQueue = xQueueCreate(BUZZER_QUEUE_LEN, sizeof(UWB_BuzzerAction));
    if (!buzzQueue) {
        PRINTF("ERROR: could not create BUZZER queue\n");
        return false;
    }

    /* Get default configuration */
    PWM_GetDefaultConfig(&pwmConfig);

    /* PWM Init with default clock selected */
    PWM_Init(BUZZER_PWM, &pwmConfig);

    return true;
}

bool UWBT_BuzzerPlayMelody(UWB_BuzzerMelody_t melody)
{
    bool ok = true;
    switch (melody) {
    case UWBT_POWER_UP:
        ok = ok && UWBT_BuzzerPlayTone(C7, 150);
        ok = ok && UWBT_BuzzerPlayTone(E7, 150);
        ok = ok && UWBT_BuzzerPlayTone(G7, 150);
        ok = ok && UWBT_BuzzerPlayTone(C8, 300);
        break;

    case UWBT_POWER_DOWN:
        ok = ok && UWBT_BuzzerPlayTone(C8, 150);
        ok = ok && UWBT_BuzzerPlayTone(G7, 150);
        ok = ok && UWBT_BuzzerPlayTone(E7, 150);
        ok = ok && UWBT_BuzzerPlayTone(C7, 300);
        break;

    case UWBT_FINDME: {
        for (int i = 0; i < 6; i++) {
            ok = ok && UWBT_BuzzerPlayTone(G7, 75);
            ok = ok && UWBT_BuzzerPlayTone(F7, 75);
            ok = ok && UWBT_BuzzerPlayTone(E7, 75);
            ok = ok && UWBT_BuzzerPlayTone(SILENCE, 125);
        }
    } break;

    default:
        ok = false;
    }

    if (!ok) {
        UWBT_BuzzerFlush();
        PRINTF("ERROR: could not play melody %02X\n", melody);
    }

    return ok;
}

void BUZZER_PWM_IRQ_HANDLER(void)
{
    /* Re-apply the channel setup to update the compare value */
    PWM_SetupPwm(BUZZER_PWM, BUZZER_PWM_CH, (pwm_setup_t *)&pwmChan);

    /* Handle PWM channel interrupt, clear interrupt status */
    PWM_ClearStatusFlags(BUZZER_PWM, BUZZER_PWM_CH);
}
