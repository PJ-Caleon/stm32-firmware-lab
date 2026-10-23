#ifndef STM32_H
#define STM32_H

#include <stdint.h>

// GPIO Struct 
typedef struct Gpio{
  volatile uint32_t CRL;
  volatile uint32_t CRH;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t BRR;
  volatile uint32_t LCKR;
} gpio;

// Mapped at GPIOC 0x40011000
#define GPIOC ((gpio *) 0x40011000)

// Mapped at GPIOA 0x40010800 
#define GPIOA ((gpio *) 0x40010800)

// RCC Struct 
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

// FLASH Struct
typedef struct Flash{
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
} flash;

#define FLASH ((flash *) 0x40022000)

// External Interrupt/Event GPIO 
typedef struct Afio{
  volatile uint32_t EVRC;
  volatile uint32_t MAPR;
  volatile uint32_t EXTICR[4];
  uint32_t RESERVED0;
  volatile uint32_t MAPR2;
} afio;

#define AFIO ((afio *) 0x40010000UL)

// External Interrupt Peripheral
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

// Nested Vectored Interrupt Controller 
typedef struct Nvic{
  volatile uint32_t ISER[8];
  uint32_t RESERVED0[24];
  volatile uint32_t ICER[8];
  uint32_t RESERVED1[24];
  volatile uint32_t ISPR[8];
} nvic;

#define NVIC ((nvic *) 0xE000E100UL)

// Data WatchPoint and Trace
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


// Helper Macros 
#define SET_BIT(REG, BIT)  ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) ((REG) = (((REG) & (~(CLEARMASK))) | (SETMASK)))

#endif 
