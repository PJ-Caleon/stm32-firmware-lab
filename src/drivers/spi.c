#include <stdint.h>
#include "stm32f103.h"
#include "spi.h"

/*
  Enable SPI clock
  Configure control register 1 
  configure CR2 
*/
void spi_init(void){
  // Enable SPI1 clock
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_SPI1EN));

  /*
    Set CR1 to master mode 
    software slave management 
    baud rate divider 
    CPOL/CPHA 
  
    Mode 3: CPOL 1; CPHA 1
  */
  SET_BIT(SPI1->CR1, (1 << 0)); // CPOL = 1 
  SET_BIT(SPI1->CR1, (1 << 1)); // CPHA = 1
  SET_BIT(SPI1->CR1, (1 << 2)); // Master Mode 
  SET_BIT(SPI1->CR1, (3 << 3)); // Baud Rate[2:0] = 011
  CLEAR_BIT(SPI1->CR1, (1 << 7)); // LSBFirst = 0 
  SET_BIT(SPI1->CR1, (1 << 8)); // SSM = 1 
  SET_BIT(SPI1->CR1, (1 << 9)); // SSI = 1
  CLEAR_BIT(SPI1->CR1, (1 << 10)); // RXONLY = 0, full duplex 
  CLEAR_BIT(SPI1->CR1, (1 << 11)); // DFF = 0, 8 bit data 
  SPI1->CR2 = 0;
}
