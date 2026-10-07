#ifndef DRIVERS_I2C1_H
#define DRIVERS_I2C1_H

#include <stdint.h>

void i2c1_init(void);
int i2c1_probe(uint8_t address);
int i2c1_write(uint8_t address, const uint8_t *data, uint32_t length);

#endif
