#include <stdint.h>
#include "stm32f103.h"
#include "fault_handler.h"
#include "uart.h"

extern uint32_t __stack_limit;
extern uint32_t _estack;

void stack_guard_init(void){
  volatile uint32_t *stack_ptr = &__stack_limit;
  volatile uint32_t *stack_top = &_estack;

  while (stack_ptr < stack_top){
    *stack_ptr = 0xDEADBEEF;
    stack_ptr++;
  }
}

/*
  Initialize the flags and specific handler for debugging the 
  div by zero, unaligned access, other faults 
*/
void fault_handler_init(void){
  stack_guard_init();

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

void fault_handler_c(sContextStateFrame *frame){
  volatile uint32_t hfsr = SCB->HFSR;
  volatile uint32_t cfsr = SCB->CFSR;
  volatile uint32_t bfar = SCB->BFAR;
  volatile uint32_t mmfar = SCB->MMFAR; 

  volatile uint32_t fault_pc = frame->return_address;
  volatile uint32_t fault_lr = frame->lr;
  volatile uint32_t stacked_r0 = frame->r0;

  (void)hfsr;
  (void)cfsr;
  (void)bfar;
  (void)mmfar;
  (void)fault_pc;
  (void)fault_lr;
  (void)stacked_r0;

  // Check if debugger is attached, to break 
  HALT_IF_DEBUGGING();

  while(1){}
}

__attribute__((naked)) void HardFault_Handler(void){
  __asm volatile(
    "tst lr, #4   \n\t" // test bit 2 of LR(EXC_RETURN)
    "ite eq       \n\t" // If then else 
    "mrseq r0, msp \n\t"  // if bit 2 is 0, stack is MSP. Move MSP to r0 
    "mrsne r0, psp \n\t"  // if bit 2 is 1, stack is PSP, move PSP to r0 
    "b fault_handler_c \n\t" // Branch to c function, passing r0 as argument pointer
  );
}

__attribute__((naked)) void UsageFault_Handler(void){
  __asm volatile(
    "tst lr, #4      \n\t"
    "ite eq          \n\t"
    "mrseq r0, msp   \n\t"
    "mrsne r0, psp   \n\t"
    "b fault_handler_c \n\t"
  );
}

__attribute__((naked)) void BusFault_Handler(void){
  __asm volatile(
    "tst lr, #4      \n\t"
    "ite eq          \n\t"
    "mrseq r0, msp   \n\t"
    "mrsne r0, psp   \n\t"
    "b fault_handler_c \n\t"
  );
}

__attribute__((naked)) void MemManage_Handler(void){
  __asm volatile(
    "tst lr, #4      \n\t"
    "ite eq          \n\t"
    "mrseq r0, msp   \n\t"
    "mrsne r0, psp   \n\t"
    "b fault_handler_c \n\t"
  );
}

// Helper to print decimal line numbers at UART 
static void print_dec(int num){
  char buf[12];
  int i = 10;
  buf[11] = '\0';
  if (num == 0){
    uart_puts("0");
    return;
  }
  while (num > 0 && i >= 0){
    buf[i--] = (num % 10) + '0';
    num /= 10;
  }
  uart_puts(&buf[i + 1]);
}

void assert_failed(const char *file, int line){
  __asm__ volatile("cpsid i" : : : "memory");

  uart_puts("ASSERTION FAILED\r\n");
  uart_puts("File: ");
  uart_puts((char *)file);
  uart_puts("\r\nLine: ");
  print_dec(line);

  HALT_IF_DEBUGGING();
  while(1){}
}
