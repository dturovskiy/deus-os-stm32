/* ESP8266 RTOS SDK v3.4 UART0 adapter. Source-only; NO FLASH OPS. */
#include "node_runtime.h"
/* ESP8266 SDK queue.h needs FreeRTOS macros before driver/uart.h. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_err.h"
#include <stdint.h>

#define DEUS_ESP01_UART UART_NUM_0
#define DEUS_ESP01_UART_RX_RING 256
#define DEUS_ESP01_UART_TX_RING 256
#define DEUS_ESP01_UART_CHUNK 32

static deus_esp01_node_runtime_t g_node;

/* Keep scheduling even if SDK tick resolution is coarser than the delay.
 * No logs on UART0: the port carries binary Node Link REV2 frames. */
static TickType_t wait_ticks(uint32_t ms){
 TickType_t n=pdMS_TO_TICKS(ms);
 return n ? n : (TickType_t)1;
}

/* Explicit API boundary: SDK UART driver owns its internal queues/heap.
 * This application never mallocs; no UART0 debug logs may be emitted by us.
 * ROM boot messages may appear before app_main, hence peer resynchronization.
 * The contract is source-only; no claim of target compilers or timing tests. */
static int init_uart0(void){
 uart_config_t config={
  .baud_rate=115200,
  .data_bits=UART_DATA_8_BITS,
  .parity=UART_PARITY_DISABLE,
  .stop_bits=UART_STOP_BITS_1,
  .flow_ctrl=UART_HW_FLOWCTRL_DISABLE
 };
 if(uart_param_config(DEUS_ESP01_UART,&config)!=ESP_OK)return 0;
 if(uart_driver_install(DEUS_ESP01_UART,
         DEUS_ESP01_UART_RX_RING,DEUS_ESP01_UART_TX_RING,
         0,NULL,0)!=ESP_OK)return 0;
 return 1;
}
static void app_process_uart(void){
 uint8_t rx[DEUS_ESP01_UART_CHUNK];
 uint8_t tx[DNL_MAX_FRAME];
 for(;;){
  /* Finite blocking interval avoids busy looping when link is idle. */
  int n=uart_read_bytes(DEUS_ESP01_UART,rx,sizeof(rx),pdMS_TO_TICKS(10));
  int i;
  if(n<0){
   /* A failed SDK read must not spin forever or retain a partial frame. */
   dnl_stream_init(&g_node.stream);
   vTaskDelay(wait_ticks(10));
   continue;
  }
  if(n==0)continue;
  for(i=0;i<n && i<(int)sizeof(rx);++i){
   size_t tx_length=0;
   uint32_t now_ms=(uint32_t)xTaskGetTickCount()*(uint32_t)portTICK_PERIOD_MS;
   int status=deus_esp01_node_runtime_feed(&g_node,rx[i],now_ms,
                                            tx,sizeof(tx),&tx_length);
   if(status==DNL_OK && tx_length!=0u){
    /* uart_write_bytes returns bytes buffered, not proof of physical delivery.
     * Failure is recorded; controller retries permitted idempotent packets. */
    int count=uart_write_bytes(DEUS_ESP01_UART,(const char *)tx,tx_length);
    if(count!=(int)tx_length && g_node.tx_failed!=UINT32_MAX)++g_node.tx_failed;
   }
  }
 }
}
void app_main(void){
 deus_esp01_node_runtime_init(&g_node);
 /* Returning here permanently kills Node Link if UART init transiently fails.
  * Retry with a bounded non-spinning delay; do not reboot or change GPIO. */
 while(!init_uart0()){
  vTaskDelay(wait_ticks(100));
 }
 app_process_uart();
}
