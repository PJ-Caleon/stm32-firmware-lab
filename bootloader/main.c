#include <stdint.h>
#include "../src/core/stm32f103.h"
#include "../src/core/crc.h"


#define APP_START_ADDRESS    0x08001400
#define META_START_ADDRESS   0x08001000
#define BOOT_MAGIC           0x424F4F54
#define MAX_APP_SIZE         0x6C00 

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t crc32;
    uint32_t version;
} app_meta_t;

// Function pointer type for the application entry point
typedef void (*pFunction)(void);

void boot_jump_to_app(uint32_t app_address) {
    // Extract the Application's Initial Stack Pointer and Reset Handler
    uint32_t app_msp = *(volatile uint32_t *)app_address;
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_address + 4);

    // Disable global interrupts before altering core registers
    __asm volatile("cpsid i");

    // Set the Main Stack Pointer to the application's stack
    __asm volatile("msr msp, %0" : : "r" (app_msp));

    // Branch to the application's Reset Handler
    pFunction app_entry = (pFunction)app_reset_handler;
    app_entry();
}

int main(void) {
    app_meta_t *meta = (app_meta_t *)META_START_ADDRESS;

    // Validate Magic Number
    if (meta->magic != BOOT_MAGIC) {
        while(1); // Trap in bootloader on failure
    }

    // Validate Size Bounds (Protects against uninitialized/corrupted flash)
    if (meta->size == 0 || meta->size > MAX_APP_SIZE) {
        while(1);
    }

    // Validate Stack Pointer bounds (must point into RAM: 0x20000000)
    uint32_t app_msp = *(volatile uint32_t *)APP_START_ADDRESS;
    if ((app_msp & 0xFFF00000) != 0x20000000) {
        while(1);
    }

    // Compute CRC32 using your implementation in crc.c (with 0xFFFFFFFF initial seed)
    uint32_t computed_crc = crc32_bit_by_bit((const uint8_t *)APP_START_ADDRESS, meta->size, 0);
    
    // Verify CRC integrity against the metadata file
    if (computed_crc != meta->crc32) {
        while(1);
    }

    // All checks passed! Perform the handover to the application.
    boot_jump_to_app(APP_START_ADDRESS);

    while(1);
}
