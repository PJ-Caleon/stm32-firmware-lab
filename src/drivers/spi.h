#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void spi_init(void);
void spi_xfer(uint8_t reg_addr, uint8_t *rx_data, int rx_size);

#endif
