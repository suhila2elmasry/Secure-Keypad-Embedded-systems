#include "state_machine.h"

#include "app.h"

typedef enum {
    EVENT_NONE = 0,
    EVENT_VALID_SEQUENCE_INPUT,
    EVENT_VALID_SEQUENCE_COMPLETE,
    EVENT_INVALID_SEQUENCE_COMPLETE,
    EVENT_LOCK_COMMAND_TRIGGERED,
    EVENT_EMERGENCY_RESET_TRIGGERED
} SM_Event;

static SM_State current_state = STATE_LOCKED;
static volatile uint8 reset_flag = 0;
// static volatile uint8 doorbell_flag = 0;

static SM_Event DetectEvent(void) {
    if (reset_flag) {
        reset_flag = 0;
        return EVENT_EMERGENCY_RESET_TRIGGERED;
    }

    if (App_IsLockButtonPressed()) {
        return EVENT_LOCK_COMMAND_TRIGGERED;
    }

    if (current_state == STATE_LOCKED) {
        if (App_ProcessKeypadInputStep()) {
            if (!App_IsPasswordComplete()) {
                return EVENT_VALID_SEQUENCE_INPUT;
            }

            if (App_IsPasswordCorrect()) {
                return EVENT_VALID_SEQUENCE_COMPLETE;
            }

            return EVENT_INVALID_SEQUENCE_COMPLETE;
        }
    }

    return EVENT_NONE;
}

void SM_Init(void) {
    current_state = STATE_LOCKED;
    reset_flag = 0;
}

void SM_Update(void) {
    SM_Event event = DetectEvent();

    switch (current_state) {
        case STATE_LOCKED:
            switch (event) {
                case EVENT_VALID_SEQUENCE_INPUT:
                    // Mealy output already happened during keypad processing.
                    break;

                case EVENT_VALID_SEQUENCE_COMPLETE:
                    App_TransitionToUnlockedOutput();
                    current_state = STATE_UNLOCKED;
                    break;

                case EVENT_INVALID_SEQUENCE_COMPLETE:
                    if (App_HandleInvalidSequenceOutput()) {
                        current_state = STATE_ALARM;
                    }
                    break;

                // case EVENT_EMERGENCY_RESET_TRIGGERED:
               // // App_HandleEmergencyResetOutput();
                //     current_state = STATE_LOCKED;
                //     break;

                case EVENT_LOCK_COMMAND_TRIGGERED:
                case EVENT_NONE:
                default:
                    break;
            }
            break;

        case STATE_UNLOCKED:
            if (event == EVENT_LOCK_COMMAND_TRIGGERED) {
                App_HandleLockCommandOutput();
                current_state = STATE_LOCKED;
            }
            break;
            // case EVENT_EMERGENCY_RESET_TRIGGERED:
            // // App_HandleEmergencyResetOutput();
            //     current_state = STATE_LOCKED;
            //     break;


        case STATE_ALARM:

            if (event == EVENT_EMERGENCY_RESET_TRIGGERED) {
                // App_HandleEmergencyResetOutput();
                current_state = STATE_LOCKED;
            }
            break;

        default:
            current_state = STATE_LOCKED;
            break;
    }
}

void SM_SetResetFlagFromISR(void) {
    reset_flag = 1;
}

SM_State SM_GetState(void) {
    return current_state;
}
