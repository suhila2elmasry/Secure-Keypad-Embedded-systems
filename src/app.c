#include "app.h"
#include "Rcc.h"
#include "Gpio.h"
#include "Keypad.h"
#include "SevenSeg.h"
#include "Utils.h"

// Static variables
static uint8 input_buffer[PASSWORD_LENGTH];
static uint8 input_index = 0;
static uint8 failure_count = 0;

void App_Init(void) {
    // Initialize RCC
    Rcc_Init();
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_GPIOD);

    // Initialize LEDs as output
    Gpio_Init(PROGRESS_LED0, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED1, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED2, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(PROGRESS_LED3, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(SUCCESS_LED, GPIO_OUTPUT, GPIO_PUSH_PULL);

    // Turn off all LEDs initially
    Gpio_WritePin(PROGRESS_LED0, LOW);
    Gpio_WritePin(PROGRESS_LED1, LOW);
    Gpio_WritePin(PROGRESS_LED2, LOW);
    Gpio_WritePin(PROGRESS_LED3, LOW);
    Gpio_WritePin(SUCCESS_LED, LOW);

    // Initialize Keypad
    Keypad_Init();

    // Initialize Seven Segment
    SevenSeg_Init();

    // Display initial failure count (0)
    SevenSeg_Display(failure_count);

    // Reset input
    input_index = 0;
}

void App_Run(void) {
    uint8 key;

    while (1) {
        key = Keypad_Scan();

        if (key != 0xFF && input_index < PASSWORD_LENGTH) {
            // Store the digit
            input_buffer[input_index] = key;
            input_index++;

            // Turn on progress LED based on position
            switch (input_index - 1) {
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
            }

            // Wait for key release to avoid multiple inputs
            while (Keypad_Scan() != 0xFF) {
                delay_ms(10);
            }
        }

        if (input_index == PASSWORD_LENGTH) {
            // Compare password
            uint8 correct = 1;
            for (uint8 i = 0; i < PASSWORD_LENGTH; i++) {
                if (input_buffer[i] != CORRECT_PASSWORD[i]) {
                    correct = 0;
                    break;
                }
            }

            if (correct) {
                // Password correct
                Gpio_WritePin(SUCCESS_LED, HIGH);
                // Keep system in success state (loop forever or something)
                while (1) {
                    // Maybe blink or just stay on
                }
            } else {
                // Password wrong
                failure_count++;
                SevenSeg_Display(failure_count);

                // Turn off all LEDs
                Gpio_WritePin(PROGRESS_LED0, LOW);
                Gpio_WritePin(PROGRESS_LED1, LOW);
                Gpio_WritePin(PROGRESS_LED2, LOW);
                Gpio_WritePin(PROGRESS_LED3, LOW);
                Gpio_WritePin(SUCCESS_LED, LOW);

                // Reset input
                input_index = 0;
            }
        }

        delay_ms(100); // Small delay to prevent tight loop
    }
}