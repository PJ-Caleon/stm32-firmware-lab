#include <stdint.h>
#include "stm32f103.h"

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
