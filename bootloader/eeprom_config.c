#include "eeprom_config.h"
#include "flash.h"
#include <string.h>

extern uint32_t crc32_bit_by_bit(const uint8_t *data, uint32_t length, uint32_t previous_crc);

bool EEPROM_Read(AppConfig *config_out) {
    const uint32_t pages[NUM_PAGES] = {CONFIG_PAGE_ADDR1, CONFIG_PAGE_ADDR2};
    ConfigRecord latest_record = {0};
    uint32_t highest_seq = 0;
    bool found_valid = false;

    for (int p = 0; p < NUM_PAGES; p++) {
        uint32_t addr = pages[p];
        uint32_t page_end = addr + CONFIG_SIZE;

        while (addr + sizeof(ConfigRecord) <= page_end) {
            ConfigRecord *candidate = (ConfigRecord *)addr;

            if (candidate->magic == 0xFFFFFFFF) {
                break;
            }

            if (candidate->magic == CONFIG_MAGIC) {
                size_t crc_data_len = offsetof(ConfigRecord, crc32);
                uint32_t computed_crc = crc32_bit_by_bit((const uint8_t *)candidate, crc_data_len, 0);
              
                if (computed_crc == candidate->crc32) {
                    if (!found_valid || candidate->sequence_number > highest_seq) {
                        highest_seq = candidate->sequence_number;
                        latest_record = *candidate;
                        found_valid = true;
                    }
                }
            }
            addr += sizeof(ConfigRecord);
        }
    }
    
    if (found_valid) {
        *config_out = latest_record.config; 
        return true;
    } 
    return false;
}

bool EEPROM_Write(const AppConfig *new_config) {
    uint32_t next_seq = 1;
    const uint32_t pages[NUM_PAGES] = {CONFIG_PAGE_ADDR1, CONFIG_PAGE_ADDR2};
    uint32_t target_slot_addr = 0;
    bool slot_found = false;

    // Scan pages to find the highest sequence number and an empty slot
    for (int p = 0; p < NUM_PAGES; p++) {
        uint32_t addr = pages[p];
        uint32_t page_end = addr + CONFIG_SIZE;

        while (addr + sizeof(ConfigRecord) <= page_end) {
            ConfigRecord *candidate = (ConfigRecord *)addr;
            
            // Unwritten flash slot found (erased state is 0xFFFFFFFF)
            if (candidate->magic == 0xFFFFFFFF) {
                target_slot_addr = addr;
                slot_found = true;
                break;
            }

            if (candidate->magic == CONFIG_MAGIC) {
                size_t crc_data_len = offsetof(ConfigRecord, crc32);
                if (crc32_bit_by_bit((const uint8_t *)candidate, crc_data_len, 0) == candidate->crc32) {
                    if (candidate->sequence_number >= next_seq) {
                        next_seq = candidate->sequence_number + 1;
                    }
                }
            }
            addr += sizeof(ConfigRecord);
        }
        if (slot_found) break;
    }

    // If both pages are full, perform a wear-leveling reset/erase on both pages
    if (!slot_found) {
        target_slot_addr = CONFIG_PAGE_ADDR1;
        
        if (flash_unlock() != FLASH_OK) return false;
        if (flash_erase_page(CONFIG_PAGE_ADDR1) != FLASH_OK) {
            flash_lock();
            return false;
        }
        if (flash_erase_page(CONFIG_PAGE_ADDR2) != FLASH_OK) {
            flash_lock();
            return false;
        }
        flash_lock();
    }

    // Build the new record
    ConfigRecord new_record;
    new_record.magic = CONFIG_MAGIC;
    new_record.sequence_number = next_seq;
    new_record.config = *new_config;
    
    size_t crc_data_len = offsetof(ConfigRecord, crc32);
    new_record.crc32 = crc32_bit_by_bit((const uint8_t *)&new_record, crc_data_len, 0);

    if (flash_unlock() != FLASH_OK) return false;

    const uint16_t *data_ptr = (const uint16_t *)&new_record;
    uint32_t byte_size = sizeof(ConfigRecord);

    for (uint32_t i = 0; i < byte_size; i += 2) {
        uint16_t half_word = *data_ptr++;
        if (flash_write_halfword(target_slot_addr + i, half_word) != FLASH_OK) {
            flash_lock();
            return false;
        }
    }

    flash_lock();
    return true;
}
