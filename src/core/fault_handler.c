#include <stdint.h>
#include "stm32f103.h"
#include "fault_handler.h"

/*
  Initialize the flags and specific handler for debugging the 
  div by zero, unaligned access, other faults 
*/
void fault_handler_init(void){
  // Enable Fault handlers 
  SET_BIT(SCB->SHCSR, (SCB_SHCSR_USGFAULTENA_Msk));
  SET_BIT(SCB->SHCSR, (SCB_SHCSR_BUSFAULTENA_Msk));
  SET_BIT(SCB->SHCSR, (SCB_SHCSR_MEMFAULTENA_Msk));

  /*
    Enable trapping 
    Bit 3: UNALIGN_TRP
    Bit 4: DIV_0_TRP
  */
  SET_BIT(SCB->CCR, (SCB_CCR_UNALIGN_TRP_Msk));
  SET_BIT(SCB->CCR, (SCB_CCR_DIV_0_TRP_Msk));
}

void usagefault_Handler(void){
  // Configurable fault status register
  volatile uint32_t cfsr = SCB->CFSR;
  (void)cfsr;
  while(1){}
}

void bufault_handler(void){
  volatile uint32_t cfsr = SCB->CFSR;
  volatile uint32_t bfar = SCB->BFAR; // Fault address 
  (void)cfsr;
  (void)bfar;
  while(1){}
}

void memmanage_handler(void){
  volatile uint32_t cfsr = SCB->CFSR;
  volatile uint32_t mmfar = SCB->MMFAR; // Memory management fault address 
  (void)cfsr;
  (void)mmfar;
  while(1){}
}

void fault_handler_c(sContextStateFrame *frame){
  volatile uint32_t hfsr = SCB->HFSR;
  volatile uint32_t cfsr = SCB->CFSR;

  volatile uint32_t fault_pc = frame->return_address;
  volatile uint32_t fault_lr = frame->lr;
  volatile uint32_t stacked_r0 = frame->r0;

  (void)hfsr;
  (void)cfsr;
  (void)fault_pc;
  (void)fault_lr;
  (void)stacked_r0;

  // Check if debugger is attached, to break 
  HALT_IF_DEBUGGING();

  while(1){}
}

__attribute__((naked)) void hardfault_handler(void){
  __asm volatile(
    "tst lr, #4   \n\t" // test bit 2 of LR(EXC_RETURN)
    "ite eq       \n\t" // If then else 
    "mrseq r0, msp \n\t"  // if bit 2 is 0, stack is MSP. Move MSP to r0 
    "mrsne r0, psp \n\t"  // if bit 2 is 1, stack is PSP, move PSP to r0 
    "b fault_handler_c \n\t" // Branch to c function, passing r0 as argument pointer
  );
}
