#include "flash.h"

typedef struct {
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  volatile uint32_t RESERVED;
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
} flash_t;

#define FLASH ((flash_t *) 0x40022000UL)
#define FLASH_KEY1 0x45670123UL
#define FLASH_KEY2 0xCDEF89ABUL

#define FLASH_SR_BSY (1UL << 0)
#define FLASH_SR_PGERR (1UL << 2)
#define FLASH_SR_WRPRT (1UL << 4)

#define FLASH_CR_PG (1UL << 0)
#define FLASH_CR_PER (1UL << 1)
#define FLASH_CR_STRT (1UL << 6)
#define FLASH_CR_LOCK (1UL << 7)

#define FLASH_TIMEOUT_COUNT 0x00050000UL

static flash_status flash_wait_ready(void){
  uint32_t timeout = FLASH_TIMEOUT_COUNT;
  while((FLASH->SR & FLASH_SR_BSY) != 0){
    if(--timeout == 0){
      return FLASH_ERR_TIMEOUT;
    }
  }
  return FLASH_OK;
}

flash_status flash_unlock(void){
  if((FLASH->CR & FLASH_CR_LOCK) == 0){
    return FLASH_OK;
  }

  FLASH->KEYR = FLASH_KEY1;
  FLASH->KEYR = FLASH_KEY2;

  if ((FLASH->CR & FLASH_CR_LOCK) != 0){
    return FLASH_ERR_LOCKED;
  }
  return FLASH_OK;
}

flash_status flash_lock(void){
  FLASH->CR |= FLASH_CR_LOCK;
  return FLASH_OK;
}

flash_status flash_erase_page(uint32_t page_address){
  flash_status status = flash_wait_ready();
  if (status != FLASH_OK) return status;

  // Clear previous error flags 
  FLASH->SR = (FLASH_SR_PGERR | FLASH_SR_WRPRT);

  // Set page error bit 
  FLASH->CR |= FLASH_CR_PER; 

  // Set target address 
  FLASH->AR = page_address;

  // Start erase operation 
  FLASH->CR |= FLASH_CR_STRT; 

  status = flash_wait_ready();

  // Clear page erase bit 
  FLASH->CR &= ~FLASH_CR_PER;

  if (status != FLASH_OK) return status;

  if (FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPRT)){
    return FLASH_ERR_PROGRAM;
  }
  return FLASH_OK;
}

flash_status flash_write_halfword(uint32_t address, uint16_t data){
  flash_status status = flash_wait_ready();
  if (status != FLASH_OK) return status;

  // Clear previous error flags
  FLASH->SR = (FLASH_SR_PGERR | FLASH_SR_WRPRT);

  // Set Programming bit
  FLASH->CR |= FLASH_CR_PG;

  // Perform half-word write
  *(volatile uint16_t *)address = data;

  status = flash_wait_ready();

  // Clear Programming bit
  FLASH->CR &= ~FLASH_CR_PG;

  if (status != FLASH_OK) return status;

  if (FLASH->SR & (FLASH_SR_PGERR | FLASH_SR_WRPRT)) {
        return FLASH_ERR_PROGRAM;
  }

  // Verify written data
  if (*(volatile uint16_t *)address != data) {
      return FLASH_ERR_PROGRAM;
  }

  return FLASH_OK;
}

void flash_init(void){

}
