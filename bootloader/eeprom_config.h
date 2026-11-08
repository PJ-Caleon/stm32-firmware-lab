#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_MAGIC 0x43464731
#define CONFIG_PAGE_ADDR1 0x08007800
#define CONFIG_PAGE_ADDR2 0x08007C00 
#define CONFIG_SIZE 1024
#define NUM_PAGES 2

typedef struct {
  uint32_t device_id;
  uint32_t baud_rate;
  uint32_t calibration_mode;
  uint8_t reserved[3];
} AppConfig;

typedef struct {
  uint32_t magic;
  uint32_t sequence_number;
  AppConfig config;
  uint32_t crc32;
} ConfigRecord;

bool EEPPROM_Read(AppConfig *config_out);
bool EEPPROM_Write(AppConfig *new_config);

#endif

