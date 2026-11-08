#include <stdint.h>
#include <stdbool.h>
#include "../src/core/stm32f103.h"
#include "../src/drivers/clock.h"
#include "../src/drivers/uart.h"
#include "../src/drivers/port_pin.h"
#include "flash.h"
#include "uart_boot.h"
#include "eeprom_config.h"

#define META_START_ADDRESS 0x08001000
#define APP_START_ADDRESS  0x08001400
#define MAX_APP_SIZE       0x6C00
#define BOOT_MAGIC         0x424F4F54

AppConfig g_device_config;

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t crc32;
    uint32_t version;
} app_meta_t;

extern uint32_t crc32_bit_by_bit(const uint8_t *data, uint32_t length, uint32_t previous_crc);

static void boot_delay_ms(uint32_t ms) {
    uint32_t start = millis();
    while ((millis() - start) < ms);
}

static void boot_jump_to_app(uint32_t app_address) {
    typedef void (*pFunction)(void);
    
    // Disable global interrupts before handover
    __asm volatile ("cpsid i" : : : "memory");

    // Extract Application Stack Pointer from vector table
    uint32_t app_stack = *(volatile uint32_t *)app_address;

    // Extract Application Reset Handler entry point
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_address + 4);

    // Set Vector Table Offset Register (VTOR) to application start
    SCB->VTOR = app_address;

    // Initialize Main Stack Pointer (MSP) for the application
    __asm volatile ("msr msp, %0" : : "r" (app_stack) : "memory");

    // Jump directly to the application's Reset Handler
    pFunction app_entry = (pFunction)app_reset_handler;
    app_entry();
}

int main(void) {
    setClock();
    systick_init();
    port_pin_init();
    flash_init();

    boot_delay_ms(10);

    if (EEPROM_Read(&g_device_config)){

    } else{
      g_device_config.baud_rate = 115200;
      g_device_config.device_id = 0x0001;
      EEPROM_Write(&g_device_config);
    }

    // Check if the button on PA1 is held down to force update mode
    bool force_update = false;
    if ((GPIOA->IDR & PINA1) == 0) {
        // Debounce / hold check
        uint32_t press_start = millis();
        while ((GPIOA->IDR & PINA1) == 0) {
            if ((millis() - press_start) > 50) { // Held for 50ms+
                force_update = true;
                break;
            }
        }
    }

    // If button was NOT held, validate existing application metadata and jump if valid
    if (!force_update) {
        app_meta_t *meta = (app_meta_t *)META_START_ADDRESS;

        // Validate Magic Number
        if (meta->magic == BOOT_MAGIC) {
            // Validate Size Bounds
            if (meta->size > 0 && meta->size <= MAX_APP_SIZE) {
                // Validate Stack Pointer bounds (must point into SRAM: 0x20000000)
                uint32_t app_msp = *(volatile uint32_t *)APP_START_ADDRESS;
                if ((app_msp & 0xFFF00000) == 0x20000000) {
                    // Compute and Verify CRC32
                    uint32_t computed_crc = crc32_bit_by_bit((const uint8_t *)APP_START_ADDRESS, meta->size, 0);
                    if (computed_crc == meta->crc32) {
                        // All checks passed! Perform handover to the application.
                        boot_jump_to_app(APP_START_ADDRESS);
                    }
                }
            }
        }
    }

    // Fallback: Initialize UART and enter the protocol loop for firmware updates
    uart_boot_init(72000000UL);
    uart_boot_protocol_loop();

    while (1);
}
