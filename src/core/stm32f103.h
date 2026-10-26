#ifndef STM32_H
#define STM32_H

#include <stdint.h>
#include <stddef.h>

/*
  stm32f103.h - register map + bit names for the STM32F103.

  RULE: this is the ONLY place that knows register addresses and bit positions.
  Drivers (clock.c, uart.c, ...) include this and contain logic only.
*/

#define SET_BIT(REG, BIT)  ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) ((REG) = (((REG) & (~(CLEARMASK))) | (SETMASK)))

// GPIO 
typedef struct Gpio{
  volatile uint32_t CRL;
  volatile uint32_t CRH;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t BRR;
  volatile uint32_t LCKR;
} gpio;

// Mapped at GPIOA 0x40010800
#define GPIOA ((gpio *) 0x40010800)
// Mapped at GPIOB 0x40010C00 
#define GPIOB ((gpio *) 0x40010C00)
// Mapped at GPIOC 0x40011000
#define GPIOC ((gpio *) 0x40011000)

// RCC 
typedef struct Rcc{
  volatile uint32_t CR;
  volatile uint32_t CFGR;
  volatile uint32_t CIR;
  volatile uint32_t APB2RSTR;
  volatile uint32_t APB1RSTR;
  volatile uint32_t AHBENR;
  volatile uint32_t APB2ENR;
  volatile uint32_t APB1ENR;
  volatile uint32_t BDCR;
  volatile uint32_t CSR;
} rcc;

// Mapped at RCC 0x40021000
#define RCC ((rcc *) 0x40021000)

// RCC->CR
#define RCC_CR_HSEON (1U << 16)
#define RCC_CR_HSERDY (1U << 17)
#define RCC_CR_PLLON (1U << 24)
#define RCC_CR_PLLRDY (1U << 25)

// RCC->CFGR
#define RCC_CFGR_PLLSRC (1U << 16)
#define RCC_CFGR_PLLXTPRE (1U << 17)
#define RCC_CFGR_PLLMULL (0xF << 18)
#define RCC_CFGR_PLLMULL9 (7U << 18)
#define RCC_CFGR_PPRE1_DIV2 (4U << 8)
#define RCC_CFGR_SW (3U << 0)
#define RCC_CFGR_SW_PLL (2U << 0)
#define RCC_CFGR_SWS_PLL (2U << 2)

// RCC->AHBENR / APB2ENR (peripheral clock enables)
#define RCC_AHBENR_DMA1EN (1U << 0)
#define RCC_APB2ENR_AFIOEN (1U << 0)
#define RCC_APB2ENR_IOPAEN (1U << 2)
#define RCC_APB2ENR_IOPBEN (1U << 3)
#define RCC_APB2ENR_IOPCEN (1U << 4)
#define RCC_APB2ENR_ADC1EN (1U << 9)
#define RCC_APB2ENR_USART1EN (1U << 14)
#define RCC_APB1ENR_TIM3EN (1UL << 1)
#define RCC_APB1ENR_I2C1EN (1UL << 21)

// Flash 
typedef struct Flash{
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  uint32_t RESERVED0;       // 0x18 is unused, OBR really lives at 0x1C
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
} flash;

#define FLASH ((flash *) 0x40022000)

#define FLASH_ACR_PRFTBE (1U << 4)
#define FLASH_ACR_LATENCY (0x7)
#define FLASH_ACR_LATENCY_2 (0x2) // 48 - 72 MHz
#define FLASH_ACR_LATENCY_1 (0x1) // 24 - 48 MHz
#define FLASH_ACR_LATENCY_0 (0x0) // 0 - 24 MHz

// Systick 
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

// AFIO & EXTI
typedef struct Afio{
  volatile uint32_t EVRC;
  volatile uint32_t MAPR;
  volatile uint32_t EXTICR[4];
  uint32_t RESERVED0;
  volatile uint32_t MAPR2;
} afio;

#define AFIO ((afio *) 0x40010000UL)

typedef struct Exti{
  volatile uint32_t IMR;
  volatile uint32_t EMR;
  volatile uint32_t RTSR;
  volatile uint32_t FTSR;
  volatile uint32_t SWIER;
  volatile uint32_t PR;
} exti;

#define EXTI ((exti *) 0x40010400UL)

#define EXTI_PORTA (0x0UL)
#define EXTI_PORTB (0x1UL)
#define EXTI_PORTC (0x2UL)
#define EXTI_LINE0 (1U << 0)
#define EXTI_LINE1 (1U << 1)

// NVIC
typedef struct Nvic{
  volatile uint32_t ISER[8];
  uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];
  uint32_t RESERVED1[24];
  volatile uint32_t ISPR[8];
} nvic;

#define NVIC ((nvic *) 0xE000E100UL)

// IRQ numbers (position in the vector table is 16 + IRQ number)
#define EXTI0_IRQn 6
#define EXTI1_IRQn 7
#define EXTI2_IRQn 8 
#define EXTI3_IRQn 9 
#define EXTI4_IRQn 10 
#define DMA1_Channel1_IRQn 11   // channels 1..7 are IRQ 11..17
#define EXTI9_5_IRQn 23
#define USART1_IRQn 37
#define EXTI15_10_IRQn 40

static inline void nvic_enable_irq(uint32_t irq){
  NVIC->ISER[irq >> 5] = (1UL << (irq & 0x1F));  // write-1-to-set
}

// DWT - Cycle Counter 
#define DEMCR (*(volatile uint32_t*)0xE000EDFCUL)
#define DWT_CTRL (*(volatile uint32_t*)0xE0001000UL)
#define DWT_CYCCNT  (*(volatile uint32_t*)0xE0001004UL)
#define TRCENA (1U << 24)
#define CYCCNTENA (1UL << 0)

// USART 
typedef struct Usart{
  volatile uint32_t SR;
  volatile uint32_t DR;
  volatile uint32_t BRR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t CR3;
  volatile uint32_t GTPR;
} usart;

#define UART1 ((usart *) 0x40013800)

#define USART_CR1_RE (1U << 2)
#define USART_CR1_TE (1U << 3)
#define USART_CR1_UE (1U << 13)
#define USART_CR3_DMAR (1U << 6)
#define USART_CR3_DMAT (1U << 7)

// DMA 
/*
  Each channel's register block is 5 words (0x14 bytes) apart: CCR, CNDTR,
  CPAR, CMAR and one RESERVED word. Without RESERVED, every channel after
  the first lands on the wrong registers.
*/
typedef struct Dma_Channel{
  volatile uint32_t CCR;
  volatile uint32_t CNDTR;
  volatile uint32_t CPAR;
  volatile uint32_t CMAR;
  uint32_t RESERVED;
} dma_channel;

typedef struct Dma{
  volatile uint32_t ISR;
  volatile uint32_t IFCR;
  dma_channel CH[7];
} dma;

// Compile-time guards: build will fail if struct is wrong
_Static_assert(sizeof(dma_channel) == 0x14, "DMA channel block must be 0x14 bytes");
_Static_assert(offsetof(dma, CH[4].CCR) == 0x58, "DMA1 CCR5 must be at offset 0x58");
_Static_assert(offsetof(flash, OBR) == 0x1C, "FLASH OBR must be at offset 0x1C");

#define DMA1 ((dma *) 0x40020000UL)

#define DMA_CCR_EN (1U << 0)
#define DMA_CCR_TCIE (1U << 1)
#define DMA_CCR_DIR (1U << 4)
#define DMA_CCR_CIRC (1U << 5)
#define DMA_CCR_MINC (1U << 7)
#define DMA_CCR_PL_0 (1U << 12)

// Per-channel status/clear bits, ch = 1..7
#define DMA_ISR_TCIF(ch)   (1U << (((ch) - 1U) * 4U + 1U))
#define DMA_IFCR_CGIF(ch)  (1U << (((ch) - 1U) * 4U))      // clears ALL flags of the channel
#define DMA_IFCR_CTCIF(ch) (1U << (((ch) - 1U) * 4U + 1U))

// ADC 
typedef struct Adc{
  volatile uint32_t SR;
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t SMPR1;
  volatile uint32_t SMPR2;
  volatile uint32_t JOFR1;
  volatile uint32_t JOFR2;
  volatile uint32_t JOFR3;
  volatile uint32_t JOFR4;
  volatile uint32_t HTR; 
  volatile uint32_t LTR;
  volatile uint32_t SQR1;
  volatile uint32_t SQR2;
  volatile uint32_t SQR3;
  volatile uint32_t JSQR;
  volatile uint32_t JDR1;
  volatile uint32_t JDR2;
  volatile uint32_t JDR3;
  volatile uint32_t JDR4;
  volatile uint32_t DR;
} Adc;

#define ADC1 ((Adc *) 0x40012400U)

#define ADC_PRESCALER_MASK (3U << 14)
#define ADC_PRESCALER_DIV6 (2U << 14)
#define ADC_CR2_ADON (1U << 0)
#define ADC_CR2_SWSTART (1U << 22)
#define ADC_CR2_RSTCAL (1U << 3)
#define ADC_CR2_CAL (1U << 2)
#define ADC_SR_EOC (1U << 1)
#define ADC_SQR3_SQ1_POS 0 
#define ADC_SQR3_SQ1_MASK (0x1FU << 0)
#define ADC_CR2_TSVREFE (1U << 23)

#define ADC_CR1_SCAN (1U << 8)
#define ADC_CR2_CONT (1U << 1)
#define ADC_CR2_DMA (1U << 8)
#define ADC_SQR3_SQ2_POS 5 
#define ADC_SQR3_SQ2_MASK (0x1FU << 5)

#define DMA_CCR_PSIZE_16 (1U << 8)
#define DMA_CCR_MSIZE_16 (1U << 10)

// Timer Register
typedef struct Tim{
  volatile uint32_t CR1;
  volatile uint32_t CR2;
  volatile uint32_t SMCR;
  volatile uint32_t DIER;
  volatile uint32_t SR;
  volatile uint32_t EGR;
  volatile uint32_t CCMR1;
  volatile uint32_t CCMR2;
  volatile uint32_t CCER;
  volatile uint32_t CNT;
  volatile uint32_t PSC;
  volatile uint32_t ARR;
  volatile uint32_t Reserved1;
  volatile uint32_t CCR1;
  volatile uint32_t CCR2;
  volatile uint32_t CCR3;
  volatile uint32_t CCR4;
} Tim;

#define APB1PERIPH_BASE 0x40000000UL
#define TIM3_BASE (APB1PERIPH_BASE + 0x0400UL)
#define TIM3 ((Tim *) TIM3_BASE)

// I2C Register 
typedef struct I2c{
  volatile uint16_t CR1;
  uint16_t RESERVED0;
  volatile uint16_t CR2;
  uint16_t RESERVED1;
  volatile uint16_t OAR1;
  uint16_t RESERVED2;
  volatile uint16_t OAR2;
  uint16_t RESERVED3;
  volatile uint16_t DR;
  uint16_t RESERVED4;
  volatile uint16_t SR1;
  uint16_t RESERVED5;
  volatile uint16_t SR2;
  uint16_t RESERVED6;
  volatile uint16_t CCR;
  uint16_t RESERVED7;
  volatile uint16_t TRISE;
  uint16_t RESERVED8;
} i2c;

#define I2C1_BASE (APB1PERIPH_BASE + 0x5400UL)
#define I2C1 ((i2c *) I2C1_BASE)

#endif
