_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

MEMORY{
  ROM (rx)  : ORIGIN = 0x08000000, LENGTH = 32K 
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}

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

  _end = . ;
  PROVIDE(end = _end);

  STACK_SIZE = 0x400;

  /* Explicitly reserve the top of RAM for the stack and check limits */
  __stack_limit = _estack - STACK_SIZE;

  .stack __stack_limit (NOLOAD):
  {
    . = ALIGN(8);
    . = . + STACK_SIZE;
    . = ALIGN(8);
  } > RAM
}
