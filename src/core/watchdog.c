#include "stm32f103.h"
#include "watchdog.h"
#include <stdint.h>

#if defined(__arm__) || defined(__thumb__)
#define CRITICAL_ENTER() uint32_t primask; __asm__ volatile ("mrs %0, primask\ncpsid i" : "=r" (primask) :: "memory")
#define CRITICAL_EXIT()  __asm__ volatile ("msr primask, %0" :: "r" (primask) : "memory")
#else 
#define CRITICAL_ENTER() do {} while(0)
#define CRITICAL_EXIT()  do {} while(0) 
#endif

volatile uint32_t alive_flags = 0;

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
  IWDG->KR = 0xCCCC;

  // Enable freeze during debug 
  (*(volatile uint32_t *)0xE0042008) |= (1 << 10);
}

void watchdog_pat(void){
  IWDG->KR = 0xAAAA;
}

void supervisor_kick_bit(uint32_t flag_mask){
  CRITICAL_ENTER();
  alive_flags |= flag_mask;
  CRITICAL_EXIT();
}

// Hz supervisor task: eval flag, clears them, and pats IWDG 
void supervisor_task(void){
  CRITICAL_ENTER();
  uint32_t current_flags = alive_flags;
  alive_flags = 0;
  CRITICAL_EXIT();

  if ((current_flags & EXPECTED_FLAGS) == EXPECTED_FLAGS){
    watchdog_pat();
  }
}
