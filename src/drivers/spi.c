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

/*
  SPI Transmit 
  1. Wait for TXE bit to set in the status register 
  2. Write data to the data register 
  3. After data transmit, wait for BSY bit to reset Status Register 
  4. Clear overrun flag by reading DR and SR 
*/
void spi_xfer(uint8_t *data, int size){
  int i = 0;
  while (i < size){
    while (!((SPI1->SR)&(1<<1))) {}; // wait for TXE bit to set. Tells if buffer is empty 
    SPI1->DR = data[i]; // Load data to data register 
    i++;
  }
  while(!((SPI1->SR)&(1 << 1))) {}; 
  while(!((SPI1->SR)&(1 << 7))) {}; // Wait for (Busy) BSY bit to reset. Tells the SPI is not busy in communcation 
  
  // Clear overrun flag by reading DR and SR 
  uint8_t temp = SPI1->DR;
  temp = SPI1->SR;
}
