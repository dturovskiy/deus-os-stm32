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
    uint32_t rx_bytes; /* accepted IRQ38 samples; includes later-discarded bytes */
    uint32_t tx_bytes; /* bytes submitted to USART2_DR; not wire ACK */
    uint32_t tx_frames; /* all bytes accepted by USART2_DR */
    /* Latched last *single* ROM-only SYNC session; never cumulative.
     * TX: [31:16]=actual BRR at TX; [15]=attempted; [14]=RXNEIE;
     * [13]=USART UE|TE|RE; [12]=TC after last DR; [11]=TXE/TC timeout;
     * [10]=all 46 DR submitted; [5:0]=DR writes (0..46).
     * RX: [31]=accepted-byte count overflow; [30]=UART errors occurred;
     * [29]=session closed; [28]=session started; [27:20]=dropped (sat255);
     * [19:16]=PE/FE/NE/ORE SR flags; [15:0]=RX samples (sat65535).
     * An asserted TC proves USART shift register completion, NOT ESP RX
     * pad voltage, boot-ROM autobaud or a received SLIP reply. */
    uint32_t rom_sync_tx;
    uint32_t rom_sync_rx;
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
/* ROM-only exact 46B SYNC; fail closed on ANY other payload/length. */
int usart2_node_write_rom_sync46(const uint8_t *data,uint32_t length);
void usart2_node_diagnostics_get(usart2_node_diagnostics_t *out);
/* ROM-only 74880-baud startup capture (before EN release).
 * Status bits occupy unused high bits of persistent ROM_PIN_TRACE:
 *  [21:20]=boot mode (0 unknown,1 UART download,3 flash app);
 *  [19]=RX UART error, [31:24]=early RX count saturated 255.
 * Low GPIO stage bits [18:0] are unchanged. 115200 restored before SYNC.
 * This is received UART evidence, NOT a physical ESP voltage reading. */
void usart2_node_rom_boot_capture_begin(void);
void usart2_node_rom_boot_capture_end(void);
uint32_t usart2_node_rom_boot_status_bits(void);
/* Task0 bridge ROM-only lifecycle: begin AFTER arming 115200 RX,
 * end BEFORE disarming on CLOSE/abort. Never invoked for NodeLink. */
void usart2_node_rom_sync_trace_begin(void);
void usart2_node_rom_sync_trace_end(void);
#endif
