#ifndef FLASH_H
#define FLASH_H 

#include <stdint.h>

typedef enum {
  FLASH_OK = 0,
  FLASH_ERR_LOCKED,
  FLASH_ERR_BUSY,
  FLASH_ERR_TIMEOUT,
  FLASH_ERR_PROGRAM,
  FLASH_ERR_PROTECTION
} flash_status;

void flash_init(void);
flash_status flash_unlock(void);
flash_status flash_lock(void);
flash_status flash_erase_page(uint32_t page_address);
flash_status flash_write_halfword(uint32_t address, uint16_t data);

#endif
