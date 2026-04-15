#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "Std_Types.h"

typedef enum {
    STATE_LOCKED = 0,
    STATE_UNLOCKED,
    STATE_ALARM
} SM_State;

void SM_Init(void);
void SM_Update(void);

// ISR-facing setters: callbacks only set flags, no logic.
void SM_SetResetFlagFromISR(void);
void SM_SetDoorBellFlagFromISR(void);

SM_State SM_GetState(void);

#endif // STATE_MACHINE_H
