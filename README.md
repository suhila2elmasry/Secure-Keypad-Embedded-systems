
---

# 🔐 Secure Keypad System (Embedded Project)

## 📌 Overview

This project implements a **password-based locking system** using an embedded platform.
The system is designed using a **Mealy State Machine** and integrates **hardware interfacing**, **external interrupts**, and **modular drivers**.

The system is fully **simulated using Proteus** to validate both hardware behavior and system logic.

---

## 🧠 System Features

* 4x4 Keypad for password input

* 7-Segment display for failure count

* LEDs for:

  * Progress indication
  * Success (Unlocked)
  * Alarm state

* External interrupts (EXTI):

  * Emergency Reset
  * Door Bell

---

## 🧩 System Architecture

```text
Drivers Layer (GPIO, RCC, Keypad, SevenSeg, EXTI)
        ↓
Application Layer (app.c)
        ↓
State Machine (state_machine.c)
```

---

## 🧪 Simulation (Proteus)

The system is implemented and tested using **Proteus simulation environment**.

### Simulation Setup:

* Microcontroller (STM32 or equivalent)

* 4x4 Keypad connected via GPIO

* External buttons:

  * Emergency Reset (EXTI)
  * Door Bell (EXTI)
  * Lock Command Button (GPIO - Polling)

* 7-Segment Display for failure count

* LEDs for:

  * Progress indication
  * Success state
  * Alarm state
  * Door Bell indicator

---

## ⚙️ Hardware Components

### 🔢 Keypad (GPIO)

* Used for password input
* Implemented using polling

---

### 🔘 Lock Command Button (GPIO - Polling)

* Regular push button
* **Not connected to interrupts (NO EXTI)**
* Used only when system is in **UNLOCKED state**

Behavior:

* When pressed:

  * System locks again
  * Clears success indication
  * Returns to LOCKED state

---

### 🚨 External Interrupts (EXTI)

#### 1. Emergency Reset

* Configured as **high-priority external interrupt**

Behavior:

* ISR sets:

```c
reset_flag = 1;
```

* State machine handles:

  * Turn OFF alarm LED
  * Clear progress LEDs
  * Reset failure counter
  * Clear 7-segment display
  * Clear input buffer
    → Transition to STATE_LOCKED

---

#### 2. Door Bell

* Configured as external interrupt

Behavior:

* ISR sets:

```c
doorbell_flag = 1;
```

* State machine:

  * Generates pulse (LED)

IMPORTANT:

* Does NOT affect system state
* Does NOT interrupt password process

---

## 🧠 Interrupt Design

* ISR contains **NO logic**
* Only sets flags
* All behavior handled inside state machine

---

## ⚡ Priority Concept

* Emergency Reset → **Highest priority**
* Door Bell → **Lower priority**

Behavior:

* If Door Bell interrupt is running and Emergency Reset occurs
  → Emergency Reset is handled first

* After finishing
  → system resumes Door Bell execution

---

## 🔁 State Machine Design (Mealy Model)

### States:

1. STATE_LOCKED (Initial)
2. STATE_UNLOCKED
3. STATE_ALARM

---

### 🔒 STATE_LOCKED

#### Event: Valid Sequence Input Detected

* Output:

  * Update progress indicator

* Condition Check:

  * If Sequence_Complete = TRUE
    → Transition to STATE_UNLOCKED

  * Else
    → Remain in STATE_LOCKED

---

#### Event: Invalid Sequence Input Detected

* Output:

  * Clear progress
  * Increment failure counter
  * Update 7-segment

* Condition Check:

  * If failures < threshold
    → Remain in STATE_LOCKED

  * If failures == threshold
    → Transition to STATE_ALARM

---

#### Event: Door Bell Triggered (EXTI)

* Output:

  * Pulse doorbell indicator

* Next State:
  → Remain in STATE_LOCKED

---

### 🔓 STATE_UNLOCKED

#### Event: Lock Command Triggered (GPIO Button - Polling)

* This event is generated from a **regular button using polling (NOT interrupt)**

* Output:

  * Turn OFF success LED
  * Clear progress indicators

* Next State:
  → Transition to STATE_LOCKED

---

#### Event: Any Sequence Input Detected

* Output:

  * None (Ignored)

* Next State:
  → Remain in STATE_UNLOCKED

---

### 🚨 STATE_ALARM

* Triggered after reaching failure threshold

---

#### Event: Emergency Reset Triggered (EXTI)

* Output:

  * Clear alarm indicator
  * Clear progress
  * Reset failure history
  * Clear input buffer

* Next State:
  → Transition to STATE_LOCKED

---

#### Event: Any Sequence Input Detected

* Output:

  * None (Ignored)

* Next State:
  → Remain in STATE_ALARM

---

## 📁 Project Structure

```text
/drivers
    gpio.c / gpio.h
    rcc.c / rcc.h
    keypad.c / keypad.h
    sevenseg.c / sevenseg.h
    exti.c / exti.h

/app
    app.c / app.h

/state_machine
    state_machine.c / state_machine.h

main.c
```

---

## 🔧 Key Design Decisions

* Mealy State Machine for control logic
* Interrupt-driven external events (EXCEPT Lock button → polling)
* Clear separation between hardware and logic
* No logic inside ISR
* Use of flags for interrupt communication

---
