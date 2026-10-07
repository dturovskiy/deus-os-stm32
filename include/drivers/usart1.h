#ifndef DRIVERS_USART1_H
#define DRIVERS_USART1_H

#include <stdint.h>

#define USART1_RX_RING_CAPACITY 128u

typedef struct
{
    uint32_t capacity;
    uint32_t depth;
    uint32_t irq_count;
    uint32_t byte_count;
    uint32_t drop_count;
    uint32_t error_count;
    uint32_t high_water;
} usart1_diagnostics_t;

void usart1_init(void);
int usart1_write_byte(uint8_t byte);
int usart1_try_read(char *value);
int usart1_irq_service(void);
void usart1_diagnostics_get(usart1_diagnostics_t *diagnostics);

#endif
