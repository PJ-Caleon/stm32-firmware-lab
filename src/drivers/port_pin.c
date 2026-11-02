#include "stm32f103.h"
#include "port_pin.h"

void port_pin_init(void){
  // 1. Enable Clocks for AFIO, GPIOA, GPIOB, and GPIOC
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN));
  
  // CRITICAL: Read back RCC->APB2ENR to ensure clock propagation delay completes 
  // before writing to GPIO registers (prevents BusFault on Cortex-M3)
  volatile uint32_t temp = RCC->APB2ENR;
  (void)temp;

  /*
    PC13 as 2 MHz push-pull output
    Pin 13 uses bits 23:20 in CRH (20 is the start bit for pin 13)
    LED is active-low, so drive it high (off) BEFORE it becomes an output
  */
  GPIOC->BSRR = PINC13;
  MODIFY_REG(GPIOC->CRH, (0xF << 20), (0x2 << 20)); // 0b0010 (output max 2 MHz)

  /*
    PA1 as input with internal pull-up
    CRL bits 7:4 = 1000 (CNF=10, MODE=00), then ODR bit = 1 selects pull-up
  */
  MODIFY_REG(GPIOA->CRL, (0xF << 4), (0x8 << 4));
  SET_BIT(GPIOA->ODR, PINA1);

  /*
    PA9 (USART1 TX): alternate function push-pull, 50 MHz -> nibble 0xB
    PA10 (USART1 RX): floating input                      -> nibble 0x4
  */
  MODIFY_REG(GPIOA->CRH, (0xF << 4), (0xB << 4));
  MODIFY_REG(GPIOA->CRH, (0xF << 8), (0x4 << 8));

  /*
    Set PA2 to analog mode (MODE2 = 00, CNF2 = 00)
  */
  MODIFY_REG(GPIOA->CRL, (0xF << 8), (0x0 << 8));

  /* 
    Set PB6 to I2C1_SCL [27:24], PB7 to I2C1_SDA [31:28]
    Alternate function open drain with 50 MHz 
  */
  MODIFY_REG(GPIOB->CRL, (0xF << 24), (0xF << 24));
  MODIFY_REG(GPIOB->CRL, (0xF << 28), (0xF << 28));

  /*
    Consolidated configuration for SPI pins and CS on GPIOA (PA4 - PA7):
    - PA4 (Bits 19:16) = 0x2 (Output 2 MHz)
    - PA5 (Bits 23:20) = 0xB (AF Push-Pull 50 MHz)
    - PA6 (Bits 27:24) = 0x4 (Floating Input)
    - PA7 (Bits 31:28) = 0xB (AF Push-Pull 50 MHz)
  */
  MODIFY_REG(GPIOA->CRL, 
             (0xFFFF << 16), 
             ((0x2 << 16) | (0xB << 20) | (0x4 << 24) | (0xB << 28)));
}

// LED is active-LOW: BRR (reset) lights it, BSRR (set) turns it off
void led_on(void){
  GPIOC->BRR = PINC13;
}

void led_off(void){
  GPIOC->BSRR = PINC13;
}
