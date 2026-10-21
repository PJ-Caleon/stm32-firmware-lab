#ifndef CLOCK_H
#define CLOCK_H

void setClock(void);

typedef struct Systick {
  volatile uint32_t CTRL; // Control and Status Register 
  volatile uint32_t LOAD; // Reload value register 
  volatile uint32_t VAL; // Current value register 
  volatile uint32_t CALIB; // Calibration register 
} SysTick;

#define SYSTICK ((SysTick *) 0xE000E010)
#define SYSTICK_EN (1U << 0)
#define SYSTICK_TICKINT (1U << 1)
#define SYSTICK_CLKSRC (1U << 2)

#define RCC_CR_HSEON (1U << 16)

#define RCC_CFGR_PLLSRC (1U << 16)
#define RCC_CFGR_PLLXTRE (1U << 17)
#define RCC_CFGR_PLLMULL (0xF << 18)
#define RCC_CFGR_PLLMULL9 (7U << 18)
#define RCC_CFGR_PPRE1_DIV2 (4U << 8)
#define RCC_CR_PILLON (1U << 24)
#define RCC_CR_PLLRDY (1U << 25)
#define RCC_CFGR_SW (3U << 0) 
#define RCC_CFGR_SW_PLL (2U << 0)

#define FLASH_ACR_PRFTBE (1U << 4) 
#define FLASH_ACR_LATENCY (0x7)
#define FLASH_ACR_LATENCY_2 (0x2) // 48 - 72 MHz 
#define FLASH_ACR_LATENCY_1 (0x1) // 24 - 48 MHz 
#define FLASH_ACR_LATENCY_0 (0x0) // 0 - 24 MHz 

#endif
