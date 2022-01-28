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
#define C8      4186
#define B7      3951
#define Ash7    3729
#define A7      3520
#define Gsh7    3322
#define G7      3136
#define Fsh7    2960
#define F7      2794
#define E7      2637
#define Dsh7    2489
#define D7      2349
#define Csh7    2217
#define C7      2093
#define B6      1975
#define Ash6    1865
#define A6      1760
#define SILENCE 0

typedef enum
{
    UWBT_POWER_UP,
    UWBT_POWER_DOWN,
    UWBT_FINDME,
} UWB_BuzzerMelody_t;

bool UWBT_BuzzerInit(void);
bool UWBT_BuzzerPlayTone(uint32_t freq, uint32_t ms);
bool UWBT_BuzzerPlayMelody(UWB_BuzzerMelody_t seq);
