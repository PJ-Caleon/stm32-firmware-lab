#include "stm32f103.h"
#include "watchdog.h"
#include <stdint.h>

void watchdog_init(void){
  // Enable write access to PR and RLR 
  IWDG->KR = 0x5555;

  // Set prescaler to divide by 32, 0x03 corresponds to /32
  // page 495 of manual
  IWDG->PR = 0x03;

  // Set reload value 2500 for 2s on 40 kHz
  IWDG->RLR = 2500;

  // Initial reload 
  IWDG->KR = 0xAAAA;

  // Start watchdog 
  IWDG->SR = 0xCCCC;
}

void watchdog_pat(void){
  IWDG->KR = 0xAAAA;
}

