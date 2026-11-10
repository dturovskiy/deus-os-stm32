/* INA226 conversion-only monitor: SOURCE-ONLY, not in current ROM build.
 * INA226 bus voltage LSB 1.25mV; shunt LSB 2.5uV.
 * With explicit example Rshunt=50milliohm, current LSB=0.05mA.
 * No sensor read means NOT_AVAILABLE, not "0 mA".
 */
#include "kernel/esp_power_monitor_v0.h"
void esp_power_monitor_v0_init(esp_power_monitor_v0_t *s)
{
    if (s == (esp_power_monitor_v0_t *)0) return;
    s->status=ESP_POWER_NOT_AVAILABLE;
    s->sampled_ms=0u;
    s->bus_mv=0u;
    s->current_ma=0;
    s->power_mw=0;
    s->has_sample=0u;
    s->shunt_mohm=0u;
}
int esp_power_monitor_v0_configure(esp_power_monitor_v0_t *s,
                                  uint16_t fitted_shunt_mohm)
{
    if (s == (esp_power_monitor_v0_t *)0 ||
        fitted_shunt_mohm < 10u || fitted_shunt_mohm > 1000u) return 0;
    s->shunt_mohm=fitted_shunt_mohm;
    s->has_sample=0u;
    s->status=ESP_POWER_NOT_AVAILABLE;
    return 1;
}
void esp_power_monitor_v0_sample(
    esp_power_monitor_v0_t *s, const esp_power_bus_v0_t *bus,
    uint32_t now_ms)
{
    uint16_t shunt, voltage;
    int rv,rs;
    int32_t mv, ma;
    if (s == (esp_power_monitor_v0_t *)0) return;
    if (s->shunt_mohm==0u || bus == (const esp_power_bus_v0_t *)0 ||
        bus->read_reg16 == (int (*)(void *,uint8_t,uint8_t,uint16_t *))0) {
        s->status=ESP_POWER_NOT_AVAILABLE;
        s->has_sample=0u;
        return;
    }
    rv=bus->read_reg16(bus->ctx,ESP_POWER_INA226_ADDR,
                        ESP_POWER_BUS_REG,&voltage);
    if (rv==1) {
        rs=bus->read_reg16(bus->ctx,ESP_POWER_INA226_ADDR,
                          ESP_POWER_SHUNT_REG,&shunt);
    } else rs=rv;
    if (rv!=1 || rs!=1) {
        s->status=(rv==0 || rs==0) ? ESP_POWER_NOT_AVAILABLE :
            ESP_POWER_IO_ERROR;
        s->has_sample=0u;
        return;
    }
    /* 1.25mV LSB gives full uint16 scale <81920mV, safe in int32. */
    mv=((int32_t)voltage*5)/4;
    /* 2.5uV/50mOhm = 0.05mA/LSB; round toward zero for raw signed. */
    ma=((int32_t)(int16_t)shunt*5)/((int32_t)s->shunt_mohm*2);
    s->sampled_ms=now_ms;
    s->bus_mv=(uint32_t)mv;
    s->current_ma=ma;
    s->power_mw=(mv*ma)/1000;
    s->has_sample=1u;
    /* Conservatively flag voltage anomaly without claiming it uniquely
     * determines regulator/USB overcurrent capacity. */
    s->status=(mv>=3000 && mv<=3600) ?
        ESP_POWER_VALID : ESP_POWER_OUT_OF_RANGE;
}
esp_power_status_v0_t esp_power_monitor_v0_status(
    const esp_power_monitor_v0_t *s, uint32_t now_ms)
{
    if (s == (const esp_power_monitor_v0_t *)0)
        return ESP_POWER_NOT_AVAILABLE;
    if (s->has_sample != 0u &&
        (uint32_t)(now_ms-s->sampled_ms)>ESP_POWER_MAX_AGE_MS)
        return ESP_POWER_STALE;
    return s->status;
}
