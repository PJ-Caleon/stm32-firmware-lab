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
  CLEAR_BIT(SPI1->CR1, (3 << 0));
  SET_BIT(SPI1->CR1, SPI_MODE2);
  SET_BIT(SPI1->CR1, (1 << 2)); // Master Mode = 1 
  SET_BIT(SPI1->CR1, (3 << 3)); // Baud Rate[2:0] = 011
  CLEAR_BIT(SPI1->CR1, (1 << 7)); // LSBFirst = 0 
  SET_BIT(SPI1->CR1, (1 << 8)); // SSM = 1 
  SET_BIT(SPI1->CR1, (1 << 9)); // SSI = 1
  CLEAR_BIT(SPI1->CR1, (1 << 10)); // RXONLY = 0, full duplex 
  CLEAR_BIT(SPI1->CR1, (1 << 11)); // DFF = 0, 8 bit data 
  SPI1->CR2 = 0;

  SET_BIT(SPI1->CR1, (1 << 6)); // Enable SPI Peripheral, SPE = 1
}

/*
  SPI Transmit 
  1. Wait for TXE bit to set in the status register 
  2. Write data to the data register 
  3. After data transmit, wait for BSY bit to reset Status Register 
  4. Clear overrun flag by reading DR and SR 
*/

/*
  SPI Receive 
  1. Wait for BSY bit to reset in Status Register 
  2. Send dummy data before reading the data 
  3. Wait for RXNE bit to set in the status register 
  4. Read data from data register 
*/

void spi_xfer(uint8_t *tx_data, uint8_t *rx_data, int rx_size){
  for (int i = 0; i < rx_size; i++){
    while (!(SPI1->SR & (1 << 1))) {}; // Wait for TXE buffer to be empty 
    
    if (tx_data != NULL){
      SPI1->DR = tx_data[i];
    } else{
      SPI1->DR = 0x00; // Send dummy data 
    }

    while (!(SPI1->SR & (1 << 0))) {}; // Wait for RXE buffer to not be empty 
    
    // Read data from DR 
    if (rx_data != NULL){
      rx_data[i] = SPI1->DR;
    } else{
      (void)SPI1->DR;
    }
  }
  while (SPI1->SR & (1 << 7)) {}; // Wait for communication to end 
  (void)SPI1->DR; 
  (void)SPI1->SR;
}
