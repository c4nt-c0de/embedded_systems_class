# STM32 Staircase Timer & Dual 7-Segment Controller

An embedded systems project built for an **STM32VL Discovery** board (ARM Cortex-M3) featuring a multiplexed dual 2-digit 7-segment display driver built with serial-in parallel-out shift registers and local user input handling[cite: 2].

### Initial State (Setting Time)
*Adding time to the timer, subtracting, and configuring duration.*

![Showcase 1](showcase1-ezgif.com-optimize.gif)

### Active Timer Countdown
*Demonstrating that the system remains responsive even while the countdown is actively running.*

![Showcase 2](showcase2-ezgif.com-optimize.gif)

---

## Features & Functional Overview

- **Dual 2-Digit 7-Segment Displays:** 
  - One display shows the currently set countdown time in seconds (adjustable up to 99 seconds)[cite: 2].
  - The second display shows the remaining time after triggering the timer[cite: 2].
- **Multiplexed Display Control:** Driven using cascaded **74HC164** shift registers for segment data and high-side transistor switches (NPN/PNP pairs) for digit anodes, refreshed at high frequency (>50–100 Hz) to maintain persistent visual output without flicker[cite: 2, 5, 6].
- **Local Control Interface:** Three tactile input buttons for adjusting time parameters (*Increment*, *Decrement*, *Confirm*) featuring software debounce[cite: 2].
- **Bare-Metal Implementation:** Developed in Keil uVision using C without heavy hardware abstraction layer (HAL) frameworks[cite: 2].

---

## Hardware Architecture (Bare Minimum)

- **Microcontroller:** STM32F100RB (STM32VL Discovery evaluation board)[cite: 2].
- **Shift Registers:** Cascaded **74HC164** 8-bit serial-in parallel-out shift registers to stream segment data[cite: 5, 6].
- **Display Modules:** Two dual-digit common-anode 7-segment LED displays (e.g., OSL20361)[cite: 2, 4].
- **Transistor Switches:** High-side switching configuration utilizing NPN (e.g., BC337) and PNP (e.g., BC327) transistors to drive common anodes[cite: 3, 5].
- **Passive Components:** Resistor networks for current-limiting segment cathodes (270Ω/470Ω) and transistor base biasing[cite: 3].

---

## Project Structure

- `Source/` — Main C source files and application logic.
- `Libraries/` — Low-level device drivers and peripheral support.
- `demo.uvproj` — Keil uVision project file.
- `demo.uvopt` — Keil target options and debug configuration.
