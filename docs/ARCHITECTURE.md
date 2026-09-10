# Architecture

## Boot flow

```text
Reset
  |
  v
Cortex-M3 reads initial MSP from 0x08000000
  |
  v
Cortex-M3 reads Reset_Handler address from 0x08000004
  |
  v
Reset_Handler
  |
  +-- copy .data from Flash to SRAM
  +-- clear .bss
  |
  v
kernel_main()
```

## Memory map

```text
Flash
0x08000000
   |
   +-- vector table
   +-- kernel code
   +-- read-only data
   +-- initial .data image
   |
0x08010000

SRAM
0x20000000
   |
   +-- .data
   +-- .bss
   +-- future kernel state / task stacks
   |
0x20005000  <- initial stack pointer
```

## Current boot image

The current linked image starts with:

```text
0x08000000: 0x20005000   initial MSP
0x08000004: 0x08000041   Reset_Handler | Thumb bit
```

The Reset_Handler code initializes C runtime memory and enters `kernel_main()`.

## Planned kernel layers

```text
startup / vector table
        |
        v
exceptions / fault handling
        |
        v
clock + SysTick + timers
        |
        v
scheduler + task context
        |
        v
drivers
        |
        +-- GPIO
        +-- I2C
        +-- UART
        +-- SSD1306 console
        |
        v
services / IPC / networking bridge
```

## Networking direction

Networking is deferred until the local kernel baseline is stable.

A future ESP-01 / ESP8266 can be attached over UART. The ESP module should own
Wi-Fi-specific behavior while STM32 communicates with it through a narrow
transport/protocol boundary.
