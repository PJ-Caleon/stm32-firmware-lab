#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdio.h>
#include "port_pin.h"

void i2c_init(void);
void i2c_write(uint8_t addr, uint8_t *buf, uint16_t len);
void i2c_read(uint8_t addr, uint8_t *buf, uint16_t len);
void i2c_scan(void);
uint16_t i2c_device_ready(uint8_t addr);

#endif
