.syntax unified
.cpu cortex-m3
.thumb
.global Reset_Handler
.extern boot_main
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss
.extern _estack

.section .isr_vector,"a",%progbits
.word _estack
.word Reset_Handler
.rept 14
.word Default_Handler
.endr

.section .text.Reset_Handler,"ax",%progbits
.thumb_func
Reset_Handler:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    bcs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
3:
    cmp r1, r2
    bcs 4f
    str r3, [r1], #4
    b 3b
4:
    bl boot_main
5:
    b 5b

.thumb_func
Default_Handler:
    b Default_Handler
