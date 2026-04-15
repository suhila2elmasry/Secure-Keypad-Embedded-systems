#include "app.h"
#include "Rcc.h"
#include "Gpio.h"
#include "Keypad.h"
#include "SevenSeg.h"
#include "Utils.h"
#include "Exti.h"

#define NO_KEY_PRESSED 0xFF
#define FAILURE_LIMIT 3

void SM_Init(void);
void SM_Update(void);
void SM_SetResetFlagFromISR(void);
void SM_SetDoorBellFlagFromISR(void);
void App_EmergencyResetCallback(void);
void App_DoorBellCallback(void);

// Static variables
static uint8 input_buffer[PASSWORD_LENGTH];
static uint8 input_index = 0;
static uint8 failure_count = 0;

static void InitClocks(void) {
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_GPIOD);
    Rcc_Enable(RCC_SYSCFG);
}

static void InitLeds(void) {
    Gpio_Init(PROGRESS_LED0, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED1, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED2, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED3, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(SUCCESS_LED, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(Alarm_LED, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(DOORBELL_LED, GPIO_OUTPUT, GPIO_PUSH_PULL);
}

static void InitLockButton(void) {
    Gpio_Init(LOCK_BUTTON_PORT, LOCK_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);
}

static void InitExternalInterrupts(void) {
    Gpio_Init(EMERGENCY_BUTTON_PORT, EMERGENCY_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);
    Exti_Init(EMERGENCY_EXTI_LINE, EMERGENCY_EXTI_PORT, EXTI_EDGE_FALLING, App_EmergencyResetCallback);
    Exti_Enable(EMERGENCY_EXTI_LINE);

    Gpio_Init(DOORBELL_BUTTON_PORT, DOORBELL_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);
    Exti_Init(DOORBELL_EXTI_LINE, DOORBELL_EXTI_PORT, EXTI_EDGE_FALLING, App_DoorBellCallback);
    Exti_Enable(DOORBELL_EXTI_LINE);
}

static void TurnOffAllLeds(void) {
    Gpio_WritePin(PROGRESS_LED0, LOW);
    Gpio_WritePin(PROGRESS_LED1, LOW);
    Gpio_WritePin(PROGRESS_LED2, LOW);
    Gpio_WritePin(PROGRESS_LED3, LOW);
    Gpio_WritePin(SUCCESS_LED, LOW);
    Gpio_WritePin(Alarm_LED, LOW);
    Gpio_WritePin(DOORBELL_LED, LOW);
}

static void TurnOnProgressLed(uint8 position) {
    switch (position) {
        case 0:
            Gpio_WritePin(PROGRESS_LED0, HIGH);
            break;
        case 1:
            Gpio_WritePin(PROGRESS_LED1, HIGH);
            break;
        case 2:
            Gpio_WritePin(PROGRESS_LED2, HIGH);
            break;
        case 3:
            Gpio_WritePin(PROGRESS_LED3, HIGH);
            break;
        default:
            break;
    }
}

static void ResetInputState(void) {
    input_index = 0;
}

static void ClearInputBuffer(void) {
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        input_buffer[i] = 0;
    }
}

static void ResetForNewAttempt(void) {
    Gpio_WritePin(PROGRESS_LED0, LOW);
    Gpio_WritePin(PROGRESS_LED1, LOW);
    Gpio_WritePin(PROGRESS_LED2, LOW);
    Gpio_WritePin(PROGRESS_LED3, LOW);
    Gpio_WritePin(SUCCESS_LED, LOW);
    Gpio_WritePin(Alarm_LED, LOW);
    Gpio_WritePin(DOORBELL_LED, LOW);
    ResetInputState();
}

static void WaitForKeyRelease(void) {
    while (Keypad_Scan() != NO_KEY_PRESSED) {
        delay_ms(10);
    }
}

static uint8 IsPasswordComplete(void) {
    return (input_index == PASSWORD_LENGTH);
}

static uint8 IsPasswordCorrect(void) {
    for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
        if (input_buffer[i] != CORRECT_PASSWORD[i]) {
            return 0;
        }
    }

    return 1;
}

static uint8 IsValidKeyInput(uint8 key) {
    return (key != NO_KEY_PRESSED) && (input_index < PASSWORD_LENGTH);
}

static void StoreKeyAndUpdateProgress(uint8 key) {
    input_buffer[input_index] = key;
    input_index++;
    TurnOnProgressLed(input_index - 1);
}

static void ProcessKeypadInput(void) {
    uint8 key = Keypad_Scan();

    if (!IsValidKeyInput(key)) {
        return;
    }

    StoreKeyAndUpdateProgress(key);
    WaitForKeyRelease();
}

void App_Init(void) {
    InitClocks();
    InitLeds();
    InitLockButton();
    InitExternalInterrupts();
    TurnOffAllLeds();

    Keypad_Init();
    SevenSeg_Init();
    SevenSeg_Display(failure_count);

    ClearInputBuffer();
    ResetInputState();

    SM_Init();
}

void App_Run(void) {
    while (1) {
        SM_Update();
        delay_ms(100);
    }
}

uint8 App_ProcessKeypadInputStep(void) {
    uint8 previous_index = input_index;
    ProcessKeypadInput();
    return (input_index != previous_index);
}

uint8 App_IsPasswordComplete(void) {
    return IsPasswordComplete();
}

uint8 App_IsPasswordCorrect(void) {
    return IsPasswordCorrect();
}

void App_TransitionToUnlockedOutput(void) {
    ResetForNewAttempt();
    ClearInputBuffer();
    Gpio_WritePin(SUCCESS_LED, HIGH);
}

uint8 App_HandleInvalidSequenceOutput(void) {
    failure_count++;
    SevenSeg_Display(failure_count);
    ResetForNewAttempt();
    ClearInputBuffer();

    if (failure_count >= FAILURE_LIMIT) {
        Gpio_WritePin(Alarm_LED, HIGH);
        return 1;
    }

    return 0;
}

void App_HandleLockCommandOutput(void) {
    ResetForNewAttempt();
    ClearInputBuffer();
    failure_count = 0;
    SevenSeg_Display(failure_count);
}

void App_HandleDoorBellOutput(void) {
    Gpio_WritePin(DOORBELL_LED, HIGH);
    delay_ms(1000);
    Gpio_WritePin(DOORBELL_LED, LOW);

}

void App_HandleEmergencyResetOutput(void) {

    failure_count = 0;
    SevenSeg_Display(failure_count);
    ClearInputBuffer();
    ResetInputState();
    TurnOffAllLeds();
}

uint8 App_IsLockButtonPressed(void) {
    static uint8 previous_state = HIGH;
    uint8 current_state = Gpio_ReadPin(LOCK_BUTTON_PORT, LOCK_BUTTON_PIN);
    uint8 is_pressed_edge = (previous_state == HIGH) && (current_state == LOW);

    previous_state = current_state;
    return is_pressed_edge;
}

void App_EmergencyResetCallback(void) {
    SM_SetResetFlagFromISR();
}

void App_DoorBellCallback(void) {
    SM_SetDoorBellFlagFromISR();
}
