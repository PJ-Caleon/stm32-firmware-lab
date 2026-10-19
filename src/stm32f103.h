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

// Helper Macros 
#define SET_BIT(REG, BIT)  ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)  ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#define MODIFY_REG(REG, CLEARMASK, SETMASK) ((REG) = (((REG) & (~(CLEARMASK))) | (SETMASK)))

#endif 
