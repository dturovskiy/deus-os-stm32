#ifndef MOCK_UART_H
#define MOCK_UART_H
#include <stddef.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "esp_err.h"
typedef int uart_port_t;
enum { UART_NUM_0=0, UART_DATA_8_BITS=8, UART_PARITY_DISABLE=0,
       UART_STOP_BITS_1=1, UART_HW_FLOWCTRL_DISABLE=0 };
typedef struct {
 int baud_rate;
 int data_bits;
 int parity;
 int stop_bits;
 int flow_ctrl;
} uart_config_t;
esp_err_t uart_param_config(uart_port_t port,const uart_config_t *config);
esp_err_t uart_driver_install(uart_port_t port,int rx_buffer_size,
 int tx_buffer_size,int queue_size,void *queue,int no_use);
int uart_read_bytes(uart_port_t port,uint8_t *out,uint32_t length,
 TickType_t ticks);
int uart_write_bytes(uart_port_t port,const char *data,size_t length);
#endif
