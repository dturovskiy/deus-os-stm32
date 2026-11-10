/* ESP01-010: STM32F103 USART2 <-> ESP-01 RX timestamp driver CANDIDATE.
 * PRIVATE OFF-GIT SOURCE ONLY. DO NOT LINK OR FLASH WITHOUT ARM BUDGET.
 * PA2 TX / PA3 RX, PCLK1 36MHz, 115200 8N1, IRQ38.
 */
#ifndef DRIVERS_USART2_NODE_H
#define DRIVERS_USART2_NODE_H
#include <stdint.h>
#define USART2_NODE_RX_CAPACITY 256u
#define USART2_NODE_MAX_TX_BYTES 40u
typedef struct {
    uint8_t byte;
    uint16_t arrival_ms_low;
} usart2_node_sample_t;
typedef struct {
    uint32_t depth;
    uint32_t dropped;
    uint32_t errors;
} usart2_node_diagnostics_t;
/* Call once at boot AFTER clock init; IRQ38 vector and event service separately. */
void usart2_node_init(void);
/* Task0-only: keep RX interrupts OFF when no Node session exists.
 * Arm BEFORE first HELLO byte TX; disarm on fail-closed/timeout/CRC error.
 * Both calls are finite PRIMASK-protected, preserve dropped/error counters.
 */
void usart2_node_rx_arm(void);
void usart2_node_rx_disarm(void);
/* Called only from USART2_IRQHandler; wakes task0 when nonzero. */
int usart2_node_irq_service(uint32_t arrival_ms);
/* -1 = loss: queue was atomically discarded; caller MUST reset parser.
 *  0 = no data; 1 = one sample; -2 = invalid sample pointer.
 * IRQs are masked only for an O(1) index/sample operation.
 */
int usart2_node_try_read(usart2_node_sample_t *sample);
/* Thread-only bounded write, at most 40 bytes. No transmission in IRQ.
 * Returns 1 after accepting all bytes into USART DR, else 0 (may be partial).
 */
int usart2_node_write_frame(const uint8_t *data,uint32_t length);
void usart2_node_diagnostics_get(usart2_node_diagnostics_t *out);
#endif
