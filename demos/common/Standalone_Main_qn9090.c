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

#include "phUwb_BuildConfig.h"

#if (UWBCORE_SDK_BUILDCONFIG != UWB_BUILD_PLUG_AND_PLAY_MODE)

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
// Kinetis includes
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "UWB_GpioIrq.h"
#include "UWBT_PowerMode.h"
#include "LED.h"
#include "GPIO.h"
#include "UWBT_Buzzer.h"
#include "AppInternal.h"
#include "AppRecovery.h"

int gapp_argc              = 1;
const char gapp_argv[2][4] = {"NA", ""};

extern void hardware_init(void);

UWBOSAL_TASK_HANDLE testTaskHandle;
void Init();
void StandaloneTask(void *args);

/* Allocate the memory for the heap. */
uint8_t __attribute__((section(".bss.$SRAM1"))) ucHeap[configTOTAL_HEAP_SIZE];
/*
 * @brief   Application entry point.
 */
int main(void)
{
    /* Init board hardware. */

    hardware_init();

    UWBT_PowerModesInit();

    /* Call UWB_GpioInit() and LED_Init() in this order */
    UWBT_BuzzerInit();

    BOARD_InitBuzzerPins();

    LED_Init();
    GPIO_InitTimer();
    /* Override pin configurations with SWO, if in debug session */
    //BOARD_InitSwoPins();

    Init();

#if 1 // UWB_BUILD_STANDALONE_DEFAULT
    Start_AppRecoveryTask();
#endif

    if (xTaskCreate(StandaloneTask, TEST_TASK_NAME, TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &testTaskHandle) !=
        TRUE) {
        PRINTF("Task created Failed");
    }

    vTaskStartScheduler();

    return 0;
}

#endif
