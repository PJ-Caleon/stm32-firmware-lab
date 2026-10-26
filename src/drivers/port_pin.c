#include "stm32f103.h"
#include "port_pin.h"

void port_pin_init(void){
  // Clocks for AFIO (bit 0), GPIOA (bit 2), GPIOB (bit 3), GPIOC (bit 4)
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN));
  (void)RCC->APB2ENR; // read back so the clock is running before touching GPIO registers

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
    CRH: pin 8 = bits 3:0, pin 9 = bits 7:4, pin 10 = bits 11:8
  */
  MODIFY_REG(GPIOA->CRH, (0xF << 4), (0xB << 4));
  MODIFY_REG(GPIOA->CRH, (0xF << 8), (0x4 << 8));

  /*
    Set PA2 to analog mode
    Analog Input is MODE2 = 00 
    CNF2 = 00
  */
  MODIFY_REG(GPIOA->CRL, (0xF << 8), (0x0 << 8));

  /*
    Set PA6 to Alternate Function Push pull 
    CNF[1:0] = 10 
    MODE[1:0] = 11 
    Combined: 1011 or 0xB
    27:24
  */
  MODIFY_REG(GPIOA->CRL, (0xF << 24), (0xB << 24));

  /*
    Set PB6 - to I2C1_SCL [27:24] 
    Set PB7 to I2C1_SDA [31:28]
    Alternate function open drain with 50 MHz 
  */
  MODIFY_REG(GPIOB->CRL, (0xF << 24), (0xF << 24));
  MODIFY_REG(GPIOB->CRL, (0xF << 28), (0xF << 28));
}

// LED is active-LOW: BRR (reset) lights it, BSRR (set) turns it off
void led_on(void){
  GPIOC->BRR = PINC13;
}

void led_off(void){
  GPIOC->BSRR = PINC13;
}
