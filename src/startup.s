.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global NMI_Handler
.global HardFault_Handler
.global MemManage_Handler
.global BusFault_Handler
.global UsageFault_Handler

.extern kernel_main
.extern SVC_Handler
.extern SysTick_Handler
.extern fault_capture

.section .isr_vector, "a", %progbits
.word _estack
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler
.word 0
.word 0
.word 0
.word 0
.word SVC_Handler          /* SVCall */
.word Default_Handler      /* DebugMon */
.word 0
.word Default_Handler      /* PendSV */
.word SysTick_Handler      /* SysTick */

.section .text.Reset_Handler, "ax", %progbits
.thumb_func
Reset_Handler:

    /* Copy .data from Flash to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

copy_data:
    cmp r1, r2
    bcs clear_bss
    ldr r3, [r0], #4
    str r3, [r1], #4
    b copy_data

clear_bss:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

clear_bss_loop:
    cmp r1, r2
    bcs start_kernel
    str r3, [r1], #4
    b clear_bss_loop

start_kernel:
    bl kernel_main

hang:
    b hang

.thumb_func
Default_Handler:
    b Default_Handler

/*
 * Cortex-M exception entry has already stacked:
 * r0, r1, r2, r3, r12, lr, pc, xPSR.
 *
 * EXC_RETURN bit 2 selects the interrupted stack:
 *   0 -> MSP
 *   1 -> PSP
 *
 * r0 = pointer to stacked frame
 * r1 = EXC_RETURN
 * r2 = exception number
 */
.section .text.Fault_Handlers, "ax", %progbits
.thumb_func
NMI_Handler:
    movs r2, #2
    b Fault_Common

.thumb_func
HardFault_Handler:
    movs r2, #3
    b Fault_Common

.thumb_func
MemManage_Handler:
    movs r2, #4
    b Fault_Common

.thumb_func
BusFault_Handler:
    movs r2, #5
    b Fault_Common

.thumb_func
UsageFault_Handler:
    movs r2, #6
    b Fault_Common

.thumb_func
Fault_Common:
    tst lr, #4
    beq 1f
    mrs r0, psp
    b 2f
1:
    mrs r0, msp
2:
    mov r1, lr
    b fault_capture
