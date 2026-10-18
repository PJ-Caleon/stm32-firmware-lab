_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

MEMORY{
  ROM (rx): ORIGIN = 0x08000000, LENGTH = 32K 
  RAM (rwx): ORIGIN = 0x20000000, LENGTH = 10K
}

STACK_SIZE = 0x400;

SECTIONS{
  .isr_vector (READONLY) : 
  {
    KEEP(*(.isr_vector))
  } > ROM

  .text (READONLY) :
  {
    KEEP(*(.vectors .vectors.*))
    *(.text*)
    *(.rodata*)
    _etext = .;
  } > ROM
  
  .bss (NOLOAD) :
  {
    _sbss = . ;
    *(.bss*)
    *(COMMON)
    _ebss = . ;
  } > RAM

  _la_data = LOADADDR(.DATA);

  .DATA :
  {
    _sdata = .;
    *(.data*);
    _edata = .; 
  } > RAM AT >ROM 

  .stack (NOLOAD):
  {
    . = ALIGN(8);
    . = . + STACK_SIZE;
    . = ALIGN(8);
  } > RAM 

  _end = . ;
}
