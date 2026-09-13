ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_msp_stack_size = 0x800;
_msp_stack_guard_size = 0x40;
_smsp_stack = _estack - _msp_stack_size;
_emsp_guard = _smsp_stack + _msp_stack_guard_size;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        *(.ARM.extab*)
        *(.ARM.exidx*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : AT(_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .msp_stack _smsp_stack (NOLOAD) :
    {
        . = ALIGN(8);
        _msp_stack_section_start = .;
        . += _msp_stack_size;
        _emsp_stack = .;
    } > RAM

    ASSERT(_msp_stack_section_start == _smsp_stack,
           "MSP stack section placement mismatch")
    ASSERT(_emsp_stack == _estack,
           "MSP stack must end at _estack")
    ASSERT(_ebss <= _smsp_stack,
           "RAM overflow: .bss overlaps reserved MSP stack")
}
