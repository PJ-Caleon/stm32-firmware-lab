#include <stdint.h>

extern uint32_t _estack;

extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _la_data;

void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler (void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler (void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler (void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler (void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler (void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler (void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler (void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler (void) __attribute__((weak, alias("Default_Handler")));

void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

void DMA1_Channel1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

int main(void);

const uint32_t vectors[] __attribute__((section(".isr_vector"))) = {
  (uint32_t) &_estack,
  (uint32_t) &Reset_Handler,
  (uint32_t) &NMI_Handler,
  (uint32_t) &HardFault_Handler,
  (uint32_t) &MemManage_Handler,
  (uint32_t) &BusFault_Handler,
  (uint32_t) &UsageFault_Handler,
  0,
  0,
  0,
  0,
  (uint32_t) &SVC_Handler,
  (uint32_t) &DebugMon_Handler,
  0,
  (uint32_t) &PendSV_Handler,
  (uint32_t) &SysTick_Handler,
  0,
  0,
  0,
  0,
  0,
  0,
  (uint32_t) &EXTI0_IRQHandler,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // IRQ 7-16
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // IRQ 17-26
  0, 0, 0, 0,                   // IRQ 27-30
  (uint32_t) &DMA1_Channel1_IRQHandler, 
  (uint32_t) &DMA1_Channel2_IRQHandler, 
  (uint32_t) &DMA1_Channel3_IRQHandler,
  (uint32_t) &DMA1_Channel4_IRQHandler, 
  (uint32_t) &DMA1_Channel5_IRQHandler, 
  (uint32_t) &UART1_IRQHandler,         
};

void Reset_Handler(void){
  // Copy the section .data in the SRAM 
  uint32_t size = (uint32_t) &_edata - (uint32_t) &_sdata; 
  uint8_t *pSrc = (uint8_t *) &_la_data; // source is ROM load address  
  uint8_t *pDst = (uint8_t *) &_sdata; // destination is RAM start address 

  for (uint32_t i=0; i<size; i++){
    *pDst++ = *pSrc++;
  }

  // Init the content of .bss to 0 in SRAM 
  uint32_t size_b = (uint32_t) &_ebss - (uint32_t) &_sbss;
  pDst = (uint8_t *) &_sbss;

  for (uint32_t i=0; i < size_b; i++){
    *pDst++ = 0;
  }

  // Call the main()
  main();
}

void Default_Handler(void){
  while(1);
}
