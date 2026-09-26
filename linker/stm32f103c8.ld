ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 54K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

_application_flash_origin = ORIGIN(FLASH);
_application_flash_end = ORIGIN(FLASH) + LENGTH(FLASH);
_relocation_headroom_start = 0x0800D800;
_relocation_headroom_end = 0x0800F800;
_persistence_slot_a = 0x0800F800;
_persistence_slot_b = 0x0800FC00;
_persistence_flash_end = 0x08010000;

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

    ASSERT(_application_flash_origin == 0x08000000,
           "Asset phase application origin must remain 0x08000000")
    ASSERT(_application_flash_end == _relocation_headroom_start,
           "Application Flash must end at 54 KiB ceiling")
    ASSERT(_relocation_headroom_end == _persistence_slot_a,
           "Relocation headroom must end at persistence slot A")
    ASSERT(_persistence_slot_b + 0x400 == _persistence_flash_end,
           "Persistence slot B must end at physical Flash end")
    ASSERT((LOADADDR(.data) + SIZEOF(.data)) <= _application_flash_end,
           "FLASH overflow: loadable image overlaps relocation headroom")
    ASSERT(_msp_stack_section_start == _smsp_stack,
           "MSP stack section placement mismatch")
    ASSERT(_emsp_stack == _estack,
           "MSP stack must end at _estack")
    ASSERT(_ebss <= _smsp_stack,
           "RAM overflow: .bss overlaps reserved MSP stack")
}
