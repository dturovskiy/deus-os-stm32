#ifndef KERNEL_ESP_POWER_MONITOR_V0_H
#define KERNEL_ESP_POWER_MONITOR_V0_H
#include <stdint.h>

/* Optional INA226 on shared I2C1: OLED=0x3C, monitor proposal=0x40.
 * SOURCE-ONLY, NOT LINKED OR WIRED. NEVER synthesize 0 mA for an
 * unavailable sensor. Rshunt MUST match the real fitted resistor.
 * This driver does not replace verifying supply capacity.
 */
#define ESP_POWER_INA226_ADDR 0x40u
#define ESP_POWER_SHUNT_REG 0x01u
#define ESP_POWER_BUS_REG 0x02u
#define ESP_POWER_DEFAULT_SHUNT_MOHM 50u /* DESIGN EXAMPLE ONLY; not auto-active */
#define ESP_POWER_MAX_AGE_MS 1000u

typedef enum {
    ESP_POWER_NOT_AVAILABLE = 0,
    ESP_POWER_VALID = 1,
    ESP_POWER_STALE = 2,
    ESP_POWER_IO_ERROR = 3,
    ESP_POWER_OUT_OF_RANGE = 4
} esp_power_status_v0_t;

typedef struct {
    /* Return 1=valid sample, 0=NACK/device absent, -1=I2C fault.
     * Owns bus arbitration with SSD1306 and ACK/error handling. */
    int (*read_reg16)(void *ctx, uint8_t addr, uint8_t reg, uint16_t *value);
    void *ctx;
} esp_power_bus_v0_t;

typedef struct {
    esp_power_status_v0_t status;
    uint32_t sampled_ms;
    uint32_t bus_mv;
    int32_t current_ma;
    int32_t power_mw;
    uint8_t has_sample;
    uint16_t shunt_mohm; /* 0 => NOT_CONFIGURED; MUST match fitted resistor */
} esp_power_monitor_v0_t;

void esp_power_monitor_v0_init(esp_power_monitor_v0_t *s);
/* Explicit trusted board configuration. No default shunt assumptions. */
int esp_power_monitor_v0_configure(esp_power_monitor_v0_t *s,
                                  uint16_t fitted_shunt_mohm);
/* No I2C hardware is accessed if callbacks absent; absence is observable. */
void esp_power_monitor_v0_sample(
    esp_power_monitor_v0_t *s, const esp_power_bus_v0_t *bus,
    uint32_t now_ms);
/* Report freshness and validity, not an inferred current estimate. */
esp_power_status_v0_t esp_power_monitor_v0_status(
    const esp_power_monitor_v0_t *s, uint32_t now_ms);
#endif
