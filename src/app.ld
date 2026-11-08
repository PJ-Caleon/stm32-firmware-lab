_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

MEMORY
{
  /*
   * 0x08001400 - 0x08007800
   * 0x08007800 - 0x08007C00 are for EEPROM 
   * */
  ROM (rx)  : ORIGIN = 0x08001400, LENGTH = 25K
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 10K
}

STACK_SIZE = 0x400;

SECTIONS
{
  /* 
   * Constraint check: ROM should not cross 0x7800 
   * */

  ASSERT((ORIGIN(ROM) + LENGTH(ROM)) <= 0x08007800, "App Exceed")

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

  _la_data = LOADADDR(.data);

  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .; 
  } > RAM AT >ROM

  __stack_limit = _estack - STACK_SIZE;

  .stack (NOLOAD):
  {
    . = ALIGN(8);
    . = . + STACK_SIZE;
    . = ALIGN(8);
  } > RAM 

  _end = . ;
  PROVIDE(end = _end);
}
