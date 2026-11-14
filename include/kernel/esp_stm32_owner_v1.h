#ifndef KERNEL_ESP_STM32_OWNER_V1_H
#define KERNEL_ESP_STM32_OWNER_V1_H
#include <stdint.h>

/* Bound to a source-locked, electrically verified STM32-owned ESP board.
 * Default firmware is inert: no GPIO writes and no bridge admission.
 * The compile-time 1 option is NEVER a USB or Host flag. */
#ifndef DEUS_ESP_STM32_BOARD_QUALIFIED
#define DEUS_ESP_STM32_BOARD_QUALIFIED 0
#endif
/* BENCH transitional source profile: CH340 alone owns ESP BOOT/EN,
 * STM32 alone owns UART data. No STM32 PB0/PB1 driver is allowed.
 * This is selected in the exact signed firmware build, not by a USB frame. */
#ifndef DEUS_ESP_SPLIT_BOOT_ADAPTER
#define DEUS_ESP_SPLIT_BOOT_ADAPTER 0
#endif
#if DEUS_ESP_SPLIT_BOOT_ADAPTER != 0 && DEUS_ESP_SPLIT_BOOT_ADAPTER != 1
#error "DEUS_ESP_SPLIT_BOOT_ADAPTER must be 0 or 1"
#endif
#if DEUS_ESP_SPLIT_BOOT_ADAPTER && DEUS_ESP_STM32_BOARD_QUALIFIED
#error "ESP boot ownership conflict: CH340 and STM32 must be exclusive"
#endif
#if DEUS_ESP_STM32_BOARD_QUALIFIED != 0 && DEUS_ESP_STM32_BOARD_QUALIFIED != 1
#error "DEUS_ESP_STM32_BOARD_QUALIFIED must be 0 or 1"
#endif

void esp_stm32_owner_v1_init(uint32_t now_ms);
int esp_stm32_owner_v1_can_enter(void);
int esp_stm32_owner_v1_enter(uint32_t now_ms);
int esp_stm32_owner_v1_rom_tx_ready(void);
void esp_stm32_owner_v1_service(uint32_t now_ms);
/* uncertain=1 for ambiguous/aborted UART, holds EN LOW; normal CLOSE
 * returns ESP to application boot without assuming HELLO succeeded. */
void esp_stm32_owner_v1_leave(uint8_t uncertain, uint32_t now_ms);
#endif
