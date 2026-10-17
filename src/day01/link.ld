_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY{
  FLASH (rx): ORIGIN = 0x08000000, LENGTH = 32K 
  RAM (rwx): ORIGIN = 0x20000000, LENGTH = 10K
}

SECTIONS{
  .isr_vector (READONLY) : 
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text (READONLY) :
  {
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH 

  .data (READONLY) : 
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH 
  _sidata = LOADADDR(.data);

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM 
}
