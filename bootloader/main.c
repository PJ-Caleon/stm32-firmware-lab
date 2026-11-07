#include <stdint.h>
#include "../src/core/stm32f103.h"
#include "../src/core/crc.h"
#include "../src/drivers/port_pin.h"
#include "../src/drivers/clock.h"
#include "../src/app/button.h"
#include "flash.h"
#include "uart_boot.h"

#define APP_START_ADDRESS    0x08001400
#define META_START_ADDRESS   0x08001000
#define BOOT_MAGIC           0x424F4F54
#define MAX_APP_SIZE         0x6C00  
#define BOOT_HOLD_TIMEOUT_MS 1000    

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t crc32;
} app_meta_t;

typedef void (*pFunction)(void);

static void boot_delay_ms(uint32_t ms){
  uint32_t start = millis();
  while((millis() - start) < ms){}
}

// Function to trap execution in the bootloader and blink fast
static void bootloader_lock_fail(void) {
    while (1) {
        led_on();             
        boot_delay_ms(100);
        led_off();            
        boot_delay_ms(100);
    }
}

void boot_jump_to_app(uint32_t app_address) {
    uint32_t app_msp = *(volatile uint32_t *)app_address;
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_address + 4);

    __asm volatile("cpsid i" : : : "memory");
    __asm volatile("msr msp, %0" : : "r" (app_msp) : "memory");

    pFunction app_entry = (pFunction)app_reset_handler;
    app_entry();
}

int main(void) {
    setClock();
    systick_init();
    port_pin_init();

    uart_boot_init(72000000UL);
    uart_boot_echo_loop();

    boot_delay_ms(10);

    // Check if the button on PA1 is held down for the duration of BOOT_HOLD_TIMEOUT_MS at reset
    if ((GPIOA->IDR & PINA1) == 0) {
        uint32_t press_start = millis();
        uint8_t held_confirmed = 0;

        while ((GPIOA->IDR & PINA1) == 0) {
            if ((millis() - press_start) >= BOOT_HOLD_TIMEOUT_MS) {
                held_confirmed = 1;
                break;
            }
        }

        if (held_confirmed) {
            bootloader_lock_fail(); // Stay in bootloader (fast blink) if held
        }
    }

    app_meta_t *meta = (app_meta_t *)META_START_ADDRESS;

    // Validate Magic Number
    if (meta->magic != BOOT_MAGIC) {
        bootloader_lock_fail();
    }

    // Validate Size Bounds
    if (meta->size == 0 || meta->size > MAX_APP_SIZE) {
        bootloader_lock_fail();
    }

    // Validate Stack Pointer bounds (must point into RAM: 0x20000000)
    uint32_t app_msp = *(volatile uint32_t *)APP_START_ADDRESS;
    if ((app_msp & 0xFFF00000) != 0x20000000) {
        bootloader_lock_fail();
    }

    // Compute CRC32
    uint32_t computed_crc = crc32_bit_by_bit((const uint8_t *)APP_START_ADDRESS, meta->size, 0);
    
    // Verify CRC integrity
    if (computed_crc != meta->crc32) {
        bootloader_lock_fail();
    }

    // Flash 
  flash_status status = flash_unlock();
  if (status == FLASH_OK) {
    flash_erase_page(0x08002000);

    flash_write_halfword(0x08002000, 0x1234);
    flash_write_halfword(0x08002002, 0x5678);
    flash_write_halfword(0x08002004, 0x9ABC);
    flash_write_halfword(0x08002006, 0xDEF0);

    flash_lock();
  }

    // All checks passed! Perform the handover to the application.
    boot_jump_to_app(APP_START_ADDRESS);

    while(1);
}
