/* Host-only SDK mock regression tests for ESP UART0 startup and RX error paths.
 * Includes actual production adapter; no UART device is opened. */
#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../main/uart0_adapter.c"

enum { M_INIT_CONFIG, M_INIT_INSTALL, M_READ_ERROR,
       M_VALID_HELLO, M_TX_FAILURE };
static jmp_buf escape;
static int test_mode;
static unsigned configs, installs, reads, delays, writes;
static TickType_t last_delay;
static uint8_t incoming[DNL_MAX_FRAME];
static size_t incoming_length;
static uint8_t transmitted[DNL_MAX_FRAME];
static size_t transmitted_length;
static int exit_reason;
static unsigned passed;

#define CHECK(expr) do{if(!(expr)){  fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#expr);return 1;}  }while(0)

static void reset_test(int mode){
 test_mode=mode;
 configs=installs=reads=delays=writes=0;
 last_delay=0;
 incoming_length=transmitted_length=0;
 exit_reason=0;
 memset(incoming,0,sizeof(incoming));
 memset(transmitted,0,sizeof(transmitted));
}

esp_err_t uart_param_config(uart_port_t port,const uart_config_t *config){
 ++configs;
 if(port!=UART_NUM_0 || !config || config->baud_rate!=115200 ||
    config->data_bits!=UART_DATA_8_BITS)return ESP_FAIL;
 if(test_mode==M_INIT_CONFIG && configs==1)return ESP_FAIL;
 return ESP_OK;
}
esp_err_t uart_driver_install(uart_port_t port,int rx,int tx,
                              int depth,void *queue,int reserved){
 ++installs;
 if(port!=UART_NUM_0 || rx!=256 || tx!=256 ||
    depth!=0 || queue!=NULL || reserved!=0)return ESP_FAIL;
 if(test_mode==M_READ_ERROR){
  g_node.stream.has_started=1;
  g_node.stream.used=3;
 }
 if(test_mode==M_INIT_INSTALL && installs==1)return ESP_FAIL;
 return ESP_OK;
}
TickType_t xTaskGetTickCount(void){return 17;}
void vTaskDelay(TickType_t ticks){
 ++delays;
 last_delay=ticks;
 if(delays>3){
  exit_reason=99;
  longjmp(escape,1);
 }
}
int uart_read_bytes(uart_port_t port,uint8_t *out,uint32_t length,
                    TickType_t ticks){
 ++reads;
 if(port!=UART_NUM_0 || !out || length<incoming_length || !ticks){
  exit_reason=98;
  longjmp(escape,1);
 }
 if(test_mode==M_READ_ERROR && reads==1)return -1;
 if((test_mode==M_VALID_HELLO || test_mode==M_TX_FAILURE) && reads==1){
  memcpy(out,incoming,incoming_length);
  return (int)incoming_length;
 }
 exit_reason=1;
 longjmp(escape,1);
}
int uart_write_bytes(uart_port_t port,const char *buf,size_t length){
 ++writes;
 if(port!=UART_NUM_0 || !buf || length>DNL_MAX_FRAME)return -1;
 memcpy(transmitted,buf,length);
 transmitted_length=length;
 if(test_mode==M_TX_FAILURE)return -1;
 return (int)length;
}
static void drive(void){
 if(setjmp(escape)==0)app_main();
}
static int test_init_retries(void){
 reset_test(M_INIT_CONFIG);
 drive();
 CHECK(exit_reason==1 && configs==2 && installs==1);
 CHECK(delays==1 && last_delay==10);
 CHECK(reads==1 && writes==0);
 reset_test(M_INIT_INSTALL);
 drive();
 CHECK(exit_reason==1 && configs==2 && installs==2);
 CHECK(delays==1 && last_delay==10);
 CHECK(reads==1 && writes==0);
 return 0;
}
static int test_negative_read_backoff_and_frame_reset(void){
 reset_test(M_READ_ERROR);
 drive();
 CHECK(exit_reason==1 && configs==1 && installs==1);
 CHECK(reads==2 && delays==1 && last_delay==1);
 CHECK(g_node.stream.used==0 && g_node.stream.has_started==0);
 return 0;
}
static int hello_prepare(void){
 dnl_frame_t f;size_t n=0;
 memset(&f,0,sizeof(f));
 f.type=DNL_HELLO;
 f.sequence=1;
 f.payload_length=4;
 f.session_id=0;
 f.payload[0]=0x78;f.payload[1]=0x56;f.payload[2]=0x34;f.payload[3]=0x12;
 if(dnl_encode(&f,incoming,sizeof(incoming),&n)!=DNL_OK)return 1;
 incoming_length=n;
 return 0;
}
static int test_runtime_full_response(void){
 dnl_frame_t response;
 reset_test(M_VALID_HELLO);
 CHECK(hello_prepare()==0);
 drive();
 CHECK(exit_reason==1 && reads==2 && delays==0 && writes==1);
 CHECK(transmitted_length==24);
 CHECK(dnl_decode(transmitted,transmitted_length,&response)==DNL_OK);
 CHECK(response.type==DNL_HELLO_ACK && response.sequence==1);
 CHECK(response.session_id==0x12345678u);
 CHECK(response.payload_length==8);
 CHECK(g_node.frames_received==1 && g_node.frames_replied==1);
 CHECK(g_node.tx_failed==0);
 return 0;
}
static int test_uart_write_failure_counted(void){
 reset_test(M_TX_FAILURE);
 CHECK(hello_prepare()==0);
 drive();
 CHECK(exit_reason==1 && reads==2 && writes==1);
 CHECK(g_node.frames_received==1 && g_node.frames_replied==1);
 CHECK(g_node.tx_failed==1);
 return 0;
}
static int test_delay_minimum(void){
 CHECK(wait_ticks(1)==1);
 CHECK(wait_ticks(9)==1);
 CHECK(wait_ticks(10)==1);
 CHECK(wait_ticks(100)==10);
 return 0;
}
int main(void){
 static const struct{const char *name;int(*fn)(void);} cases[]={
  {"retries UART parameter and driver startup failures",test_init_retries},
  {"backs off and clears partial RX frame on SDK read error",test_negative_read_backoff_and_frame_reset},
  {"real codec HELLO produces valid SDK UART0 ACK",test_runtime_full_response},
  {"short UART TX failure increments saturating counter",test_uart_write_failure_counted},
  {"every deliberate delay yields at least one RTOS tick",test_delay_minimum}
 };
 size_t i;
 for(i=0;i<sizeof(cases)/sizeof(cases[0]);++i){
  if(cases[i].fn())return 1;
  ++passed;
  printf("UART_ADAPTER_MOCK_PASS %zu %s\n",i+1,cases[i].name);
 }
 printf("ESP01_UART_ADAPTER_OFFLINE_RESULT=PASS CASES=%u HARDWARE=NO\n",passed);
 return 0;
}
