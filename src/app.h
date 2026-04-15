#ifndef APP_H
#define APP_H

#include "Std_Types.h"
#include "Gpio.h"
#include "Exti.h"

// LED definitions
#define PROGRESS_LED0  GPIO_B, 0
#define PROGRESS_LED1  GPIO_B, 1
#define PROGRESS_LED2  GPIO_B, 2
#define PROGRESS_LED3  GPIO_B, 3
#define SUCCESS_LED    GPIO_B, 4
#define Alarm_LED    GPIO_B, 5
#define DOORBELL_LED   GPIO_B, 6

// Lock button (active-low with pull-up)
#define LOCK_BUTTON_PORT GPIO_C
#define LOCK_BUTTON_PIN  13

// Emergency Reset button (active-low with pull-up)
#define EMERGENCY_BUTTON_PORT GPIO_A
#define EMERGENCY_BUTTON_PIN  1
#define EMERGENCY_EXTI_LINE   EXTI_LINE_1
#define EMERGENCY_EXTI_PORT   EXTI_PORT_A

// Door Bell button (active-low with pull-up)
#define DOORBELL_BUTTON_PORT GPIO_A
#define DOORBELL_BUTTON_PIN   2
#define DOORBELL_EXTI_LINE    EXTI_LINE_2
#define DOORBELL_EXTI_PORT    EXTI_PORT_A

// Correct password
#define CORRECT_PASSWORD "1234"
#define PASSWORD_LENGTH 4

// Function prototypes
void App_Init(void);
void App_Run(void);

// State-machine integration API (implemented in app.c)
uint8 App_ProcessKeypadInputStep(void);
uint8 App_IsPasswordComplete(void);
uint8 App_IsPasswordCorrect(void);
void App_TransitionToUnlockedOutput(void);
uint8 App_HandleInvalidSequenceOutput(void);
void App_HandleLockCommandOutput(void);
void App_HandleDoorBellOutput(void);
void App_HandleEmergencyResetOutput(void);
uint8 App_IsLockButtonPressed(void);

#endif // APP_H