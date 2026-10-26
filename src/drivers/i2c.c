#include <stdint.h>
#include "stm32f103.h"

#define I2C_TIMEOUT_MAX 50000UL

void i2c_init(void){
  // Enable I2C1 clock 
  SET_BIT(RCC->APB1ENR, (RCC_APB1ENR_I2C1EN));

  /*
    Configure I2C1
    CR2.FREQ = 36; the stm is running at 72 MHz
    APB1 runs at half the system clock speed

    CCR = 180 for 100 kHz
    CCR = (36 MHz / 2)/0.1 MHz = 180

    TRISE = 37 
    Set PE, this enables I2C1 peripheral
  */
  MODIFY_REG(I2C1->CR2, (0x3F << 0), (36 << 0));
  I2C1->CCR = 180;
  I2C1->TRISE = 37;
  SET_BIT(I2C1->CR1, (1UL << 0));
}

/*
  i2c read and write 
  START, address + write bit 
  wait for ACK
  data bytes 
  STOP 
  every waut loop needs a timeout that returns an error code 
*/
void i2c_write(uint8_t addr, uint8_t *buf, uint16_t len){
  uint32_t timeout;

  // Start condition 
  SET_BIT(I2C1->CR1, (I2C_CR1_START));
  timeout = I2C_TIMEOUT_MAX;
  while(!(I2C1->SR1 & I2C_SR1_SB)){
    if (--timeout == 0) return; // timeout error 
  }

  // Send slave address with write bit 0 
  I2C1->DR = (addr << 1) & ~0x1;
  timeout = I2C_TIMEOUT_MAX;
  while(!(I2C1->SR1 & I2C_SR1_AF)){
    // Check ACK 
    if (I2C1->SR1 & I2C_SR1_AF){
      CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
      SET_BIT(I2C1->CR1, I2C_CR1_STOP);
      return; // NACK error 
    }
    if (--timeout == 0) return;
  }

  // Clear ADDR flag by reading SR1 then SR2 
  (void)I2C1->SR1;
  (void)I2C1->SR2;

  // Send data bytes 
  for (uint16_t i =0; i < len; i++){
    timeout = I2C_TIMEOUT_MAX;
    while(!(I2C1->SR1 & I2C_SR1_TXE)){
      if (--timeout == 0) return;
    }
    I2C1->DR = buf[i];
  }

  // Wait for byte transfer finished 
  timeout = I2C_TIMEOUT_MAX;
  while(!(I2C1->SR1 & I2C_SR1_BTF)){
    if (--timeout == 0) return;
  }

  // Generate STOP condition 
  SET_BIT(I2C1->CR1, I2C_CR1_STOP);
}

void i2c_read(uint8_t addr, uint8_t *buf, uint16_t len){
  uint32_t timeout;

  if (len == 0) return;

  SET_BIT(I2C1->CR1, I2C_CR1_ACK);

  // Generate START condition
  SET_BIT(I2C1->CR1, I2C_CR1_START);
  timeout = I2C_TIMEOUT_MAX;
  while (!(I2C1->SR1 & I2C_SR1_SB)) {
    if (--timeout == 0) return;
  }

  // Send Slave Address with Read bit 1 
  I2C1->DR = (addr << 1) | 0x1;

  if (len == 1) {
    CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
  }

  timeout = I2C_TIMEOUT_MAX;
  while (!(I2C1->SR1 & I2C_SR1_ADDR)) {
    if (I2C1->SR1 & I2C_SR1_AF) {
      CLEAR_BIT(I2C1->SR1, I2C_SR1_AF);
      SET_BIT(I2C1->CR1, I2C_CR1_STOP);
      return;
    }
    if (--timeout == 0) return;
  }

  if (len == 1) {
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    SET_BIT(I2C1->CR1, I2C_CR1_STOP);
  } else if (len == 2) {
    CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    SET_BIT(I2C1->CR1, I2C_CR1_STOP);
  } else {
    (void)I2C1->SR1;
    (void)I2C1->SR2;
  }

  // Read data bytes 
  for (uint16_t i = 0; i < len; i++){
    if (len > 2 && i == len - 2){
      timeout = I2C_TIMEOUT_MAX;
      while (!(I2C1->SR1 & I2C_SR1_BTF)){
        if (--timeout == 0) return;
      }

      CLEAR_BIT(I2C1->CR1, I2C_CR1_ACK);
      buf[i++] = I2C1->DR;
      SET_BIT(I2C1->CR1, (I2C_CR1_STOP));
      buf[i] = I2C1->DR;
      break;
    }
    timeout = I2C_TIMEOUT_MAX;
    while (!(I2C1->SR1 & I2C_SR1_RXNE)){
      if (--timeout == 0) return;
    }
    buf[i] = I2C1->DR;
  }
}
