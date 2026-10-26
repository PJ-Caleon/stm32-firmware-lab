#include <stdint.h>
#include "stm32f103.h"

void i2c_init(void){
  // Enable I2C1 clock 
  SET_BIT(RCC->APB1ENR, (RCC_APB1ENR_I2C1EN));
}
