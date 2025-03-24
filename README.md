# STMInvaders

A remake of the classic Space Invaders game implemented on **STM32** microcontrollers. This project showcases how an embedded system can deliver smooth, real-time gameplay using hardware inputs (accelerometer for tilt), Kalman filtering, and UART communication for ASCII-based rendering. Enjoy tilting your board to steer a spaceship, firing bullets at descending alien invaders, and taking advantage of real-time sound effects generated via a DAC output.

---

## Table of Contents
1. [Overview](#overview)  
2. [Features](#features)  
3. [Hardware Requirements](#hardware-requirements)  
4. [Software Requirements](#software-requirements)  
5. [Project Structure](#project-structure)  
6. [How It Works](#how-it-works)  
    - [System Overview](#system-overview)  
    - [Hardware Integration](#hardware-integration)  
    - [Game Logic and Interface](#game-logic-and-interface)  
    - [Accelerometer-Based Controls](#accelerometer-based-controls)  
    - [Kalman Filter for Noise Reduction](#kalman-filter-for-noise-reduction)  
    - [Sound Effects](#sound-effects)  
7. [Build and Run Instructions](#build-and-run-instructions)  
8. [Results](#results)  
9. [Future Improvements](#future-improvements)  
10. [License](#license)

---

## Overview
**STMInvaders** is a bare-bones clone of Space Invaders for the STM32 microcontroller platform. By combining a UART ASCII rendering system, an accelerometer for movement, and DAC-generated sound, the project demonstrates how classical gaming concepts can be reimagined on modern embedded hardware. In the course of two weeks, the team crafted a working example of real-time user interaction and smooth game performance within the computational limitations of an STM32.

For an in-depth look at the hardware schematics, detailed software design, and results analysis, please refer to the [PDF documentation](./Space_Invaders___ECSE444_Final_Project.pdf).

---

## Features
- **Accelerometer Controls**: Tilt the microcontroller left or right to move the spaceship horizontally.
- **ASCII-based Rendering**: The game plays on a 25×60 grid rendered in a serial terminal at high baud rates.
- **Kalman Filter**: Smooths raw accelerometer data for precise, responsive movement.
- **UART Communication**: Sends ASCII frames to a connected terminal (921600 bps) for minimal flicker.
- **Button Input**: A user button triggers bullet firing and navigates the main menu.
- **DAC-based Sound**: Plays a 2 kHz tone when bullets are fired or certain events occur.
- **Win/Lose Screens**: Custom “Game Over” and “You Win” ASCII art for end-of-game feedback.

---

## Hardware Requirements
1. **STM32 Microcontroller** (e.g., STM32L4S5VIT6 or similar).  
2. **Built-in or External Accelerometer** to sense tilt.  
3. **User Button** wired to a GPIO pin for firing bullets.  
4. **Pressure Sensor** (optional) for additional input if desired (e.g., map selection).  
5. **Speaker/Buzzer** connected to DAC output pins for sound effects.  
6. **USB-UART** connection to display the ASCII game in a terminal at 921600 bps.

---

## Software Requirements
1. **STM32CubeMX** (optional) for pin configuration and initial project setup.  
2. **STM32CubeIDE** or **GCC ARM Toolchain** to build and flash the firmware.  
3. **Serial Terminal** (e.g., PuTTY, Minicom, screen) configured at 921600 bps.  
4. **FreeRTOS** (optional) if you want to replicate the exact multitasking environment—though you could adapt the code for a bare-metal loop.

---

## Project Structure
An example project layout might look like this:
STMInvaders/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── kalman_filter.h
│   │   ├── game_logic.h
│   │   └── …
│   └── Src/
│       ├── main.c
│       ├── kalman_filter.c
│       ├── game_logic.c
│       └── …
├── Drivers/
│   └── …
├── FreeRTOS/
│   └── …
├── README.md
├── LICENSE

- **Core/Inc/**: Header files for main application logic (Kalman filter, game logic, etc.).  
- **Core/Src/**: Source files including the main loop, interrupt handlers, and peripheral initialization.  
- **Drivers/**: Vendor-provided HAL drivers.  
- **FreeRTOS/**: If using RTOS-based threading, place OS source here.  
- **Space_Invaders___ECSE444_Final_Project.pdf**: In-depth PDF report with schematics and code snippets.

---

## How It Works

### System Overview
1. **Accelerometer** readings are continuously acquired to determine tilt angle.  
2. **Kalman Filter** processes raw accelerometer data, reducing jitter/noise.  
3. **Button Inputs** detect firing actions and menu selection changes.  
4. **UART** outputs ASCII art for the game—each frame updates spaceship, bullets, and aliens on a 25×60 grid.  
5. **DAC** generates simple sound effects (2 kHz tone).  

### Hardware Integration
- **Accelerometer (U3 MEMS)**: Communicates via I2C/SPI to measure acceleration along x, y, and z axes.  
- **UART Pins (PB6, PB7)**: Configured at 921600 bps for fast serial output to a terminal.  
- **Button (PC13)**: Configured as GPIO input with interrupt to handle bullet firing.  
- **DAC Pins (PA4, PA5)**: Outputs 2 kHz sine wave samples to a speaker or buzzer.  

### Game Logic and Interface
- **Menu Screen**: Displayed upon reset, user can select a map by cycling through choices using the button or a pressure sensor.  
- **Gameplay**:
  - Aliens spawn at the top rows and move downward in waves.  
  - Each bullet fired travels upward; if it collides with an alien, the alien is destroyed.  
  - If aliens reach the bottom row, the game ends with “Game Over.”  
  - If all aliens in a wave are destroyed, “You Win” is displayed.  

### Accelerometer-Based Controls
- **Tilt Detection**: Calculated using the formula  
  \[
    \text{Pitch} = \arctan\!\Big(\frac{a_x}{\sqrt{a_y^2 + a_z^2}}\Big) \times \frac{180}{\pi}.
  \]  
- **Movement Thresholds**: If pitch exceeds positive/negative limits, the spaceship moves right/left accordingly.  
- **Interrupt Handling**: Button presses are captured in an EXTI callback, toggling a flag (`fireBullet`) in the main loop.

### Kalman Filter for Noise Reduction
- The game code employs a simple Kalman filter to smooth the accelerometer data:
  ```c
  float kalman_filter(kalman_state *kstate, float measurement) {
      // Predict
      float p_temp = kstate->p + kstate->q;
      // Update
      float k = p_temp / (p_temp + kstate->r);
      float x_temp = kstate->x + k * (measurement - kstate->x);
      kstate->p = (1.0f - k) * p_temp;

      kstate->x = x_temp;
      kstate->k = k;
      return kstate->x;
  }
	•	kstate->x holds the current estimated value of the accelerometer reading.
	•	kstate->p is the error covariance, updated each iteration.
	•	kstate->q and kstate->r are the process and measurement noise covariances, tuned experimentally.

Sound Effects
	•	DAC Approach: A precomputed 22-sample sine wave is played via the DAC at ~44 kHz, producing a 2 kHz tone.
	•	Triggering: A sound flag is set on bullet fire or other in-game events, and a dedicated function (or RTOS task) starts the DMA transfer to the DAC.

## Build and Run Instructions
	1.	Clone the repository:
 	2.	Open the project in STM32CubeIDE or import into your preferred environment.
	3.	Adjust Pin Mappings (if your board differs from the default pin assignments).
	4.	Build and Flash the project onto your STM32 board.
	5.	Connect a serial terminal (e.g., PuTTY) at 921600 bps, 8-N-1, to the board’s UART pins.
	6.	Reset or power-cycle the STM32. The main menu screen should appear in the terminal.
	7.	Play: Use the button to cycle through maps, apply pressure if configured, and tilt the board to move the spaceship.

## Results
	•	Smooth Gameplay: Thanks to high-speed UART, updates display with minimal flicker.
	•	Accurate Movement: The Kalman filter ensures quick yet stable tilt detection.
	•	Responsive Fire: Button interrupts register near-instantly, reducing lag between pressing and bullet firing.
	•	Audio Feedback: The 2 kHz tone effectively signals events like firing without noticeable latency.

## Future Improvements
	•	Enhanced Graphics: Replace the ASCII rendering with an LCD or OLED display for richer visuals.
	•	Advanced Audio: Implement multiple sound effects or even background music with more DAC channels or waveforms.
	•	Game Expansion: Add new alien formations, boss battles, or power-ups.
	•	High Score Saving: Write scores to onboard Flash/EEPROM.
	•	Network Connectivity: Integrate Wi-Fi/Bluetooth for online leaderboards or multiplayer modes.
