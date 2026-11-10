/* ESP8266 RTOS SDK v3.4 UART0 + on-demand Wi-Fi scan candidate.
 * SOURCE ONLY: must replace uart0_adapter.c in an ISOLATED OFFLINE build.
 * Not in accepted ESP v2 firmware/main/component.mk or installed device.
 * UART0 is strictly binary DNL; no SDK/application log strings emitted.
 * No credentials, STA association, NVS network secrets, GPIO/BOOT/reset.
 */
#include "wifi_node_dispatch_v0.h"
#include "wifi_node_sdk_bridge_v0.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_err.h"
#include <stdint.h>

#define NODE_UART UART_NUM_0
#define NODE_RX_RING 256
#define NODE_TX_RING 256
#define NODE_RX_CHUNK 32

static deus_wifi_node_dispatch_v0_t g_node;
static uint32_t g_uart_tx_failed;

static TickType_t wait_ticks(uint32_t ms)
{
    TickType_t t=pdMS_TO_TICKS(ms);
    return t?t:(TickType_t)1;
}
static int uart0_initialize(void)
{
    uart_config_t config={
        .baud_rate=115200,
        .data_bits=UART_DATA_8_BITS,
        .parity=UART_PARITY_DISABLE,
        .stop_bits=UART_STOP_BITS_1,
        .flow_ctrl=UART_HW_FLOWCTRL_DISABLE
    };
    if (uart_param_config(NODE_UART,&config)!=ESP_OK)
        return 0;
    if (uart_driver_install(NODE_UART,NODE_RX_RING,NODE_TX_RING,
                            0,NULL,0)!=ESP_OK)
        return 0;
    return 1;
}
static uint32_t monotonic_milliseconds(void)
{
    return (uint32_t)xTaskGetTickCount()*
           (uint32_t)portTICK_PERIOD_MS;
}
static void app_process_uart(void)
{
    uint8_t rx[NODE_RX_CHUNK],tx[DNL_MAX_FRAME];
    for (;;) {
        uint32_t ms;
        int count=uart_read_bytes(NODE_UART,rx,sizeof(rx),wait_ticks(10u));
        int i;
        ms=monotonic_milliseconds();
        if(count<0) {
            /* UART0 adapter is the only stream owner; on driver error,
             * clear incomplete framing, preserving authenticated session.
             */
            dnl_stream_init(&g_node.stream);
            vTaskDelay(wait_ticks(10u));
            deus_wifi_node_dispatch_v0_poll(&g_node,ms);
            continue;
        }
        if(count>0) {
            if(count>(int)sizeof(rx))
                count=(int)sizeof(rx);
            for(i=0;i<count;++i) {
                size_t out_length=0u;
                int rc=deus_wifi_node_dispatch_v0_feed(
                    &g_node,rx[i],monotonic_milliseconds(),
                    tx,sizeof(tx),&out_length);
                if(rc==DNL_OK && out_length!=0u) {
                    int queued=uart_write_bytes(NODE_UART,
                                                (const char *)tx,
                                                out_length);
                    /* Buffered-write acknowledgment is not proof that
                     * STM32 physically received the frame. Controller
                     * retries trigger worker byte-identical cache.
                     */
                    if(queued!=(int)out_length &&
                       g_uart_tx_failed!=UINT32_MAX)
                        ++g_uart_tx_failed;
                }
            }
        }
        /* The scan SDK runs on this task, never in UART IRQ/callback.
         * The UART driver ring tolerates bounded task-side scan work;
         * target timing/overrun acceptance remains required before Flash.
         */
        deus_wifi_node_dispatch_v0_poll(&g_node,
                                         monotonic_milliseconds());
    }
}
void app_main(void)
{
    deus_wifi_node_ops_v0_t ops;
    /* Initializes only RAM state; radio is not touched until explicit
     * authenticated SCAN_START. No network credentials are provisioned.
     */
    deus_wifi_node_sdk_bridge_v0_init();
    ops=deus_wifi_node_sdk_bridge_v0_ops();
    deus_wifi_node_dispatch_v0_init(&g_node,&ops);
    g_uart_tx_failed=0u;
    while(!uart0_initialize())
        vTaskDelay(wait_ticks(100u));
    app_process_uart();
}
