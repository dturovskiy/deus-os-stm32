ENTRY(Reset_Handler)
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 8K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_boot_msp_size = 0x400;
_boot_stack_bottom = _estack - _boot_msp_size;

SECTIONS
{
 .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } > FLASH
 .text : { . = ALIGN(4); *(.text*) *(.rodata*) *(.ARM.extab*) . = ALIGN(4); _etext = .; } > FLASH
 .ARM.exidx : { *(.ARM.exidx*) } > FLASH
 _etext_load = .;

 .data : AT(_etext_load) { . = ALIGN(4); _sdata = .; *(.data*) . = ALIGN(4); _edata = .; } > RAM
 _sidata = LOADADDR(.data);

 .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss*) *(COMMON) . = ALIGN(4); _ebss = .; } > RAM

 .boot_stack _boot_stack_bottom (NOLOAD) :
 {
   . = ALIGN(8);
   _sboot_stack = .;
   . += _boot_msp_size;
   _eboot_stack = .;
 } > RAM

 ASSERT((LOADADDR(.data) + SIZEOF(.data)) <= 0x08002000, "bootloader exceeds 8 KiB")
 ASSERT((SIZEOF(.data) + SIZEOF(.bss)) <= 1024, "bootloader data+bss exceeds 1024")
 ASSERT(_ebss <= _boot_stack_bottom, "bootloader RAM overlaps MSP")
 ASSERT(_eboot_stack == _estack, "bootloader MSP placement")
}
