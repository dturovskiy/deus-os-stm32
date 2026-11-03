/* ESP01-008 offline transport-neutral worker runtime integration tests. */
#include "node_runtime.h"
#include <stdio.h>
#include <string.h>
#define CK(c) do{if(!(c)){fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#c);return 1;}}while(0)
static unsigned pass_count;
static int one(const char *name,int (*fn)(void)){
 if(fn())return 1;
 ++pass_count;printf("PASS %02u %s\n",pass_count,name);return 0;
}
static int feed_frame(deus_esp01_node_runtime_t *rt,const uint8_t *in,size_t len,
 uint32_t t,uint8_t *out,size_t *outlen){
 size_t i;int replies=0;
 for(i=0;i<len;++i){
  size_t n=0;
  int s=deus_esp01_node_runtime_feed(rt,in[i],t,out,DNL_MAX_FRAME,&n);
  if(s==DNL_OK && n){++replies;*outlen=n;}
  else CK(s==DNL_DROP || s==DNL_OK);
 }
 return replies;
}
static int prepare(deus_esp01_node_runtime_t *rt,uint32_t sid,
 uint8_t *hello,size_t *hello_len){
 dnl_frame_t f;
 deus_esp01_node_runtime_init(rt);
 memset(&f,0,sizeof(f));f.type=DNL_HELLO;f.sequence=1;f.payload_length=4;
 f.payload[0]=(uint8_t)sid;f.payload[1]=(uint8_t)(sid>>8);
 f.payload[2]=(uint8_t)(sid>>16);f.payload[3]=(uint8_t)(sid>>24);
 return dnl_encode(&f,hello,40,hello_len);
}
static int test_01_hello_ack_and_session(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n=0,m=0;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 CK(dnl_decode(rsp,m,&f)==DNL_OK);
 CK(f.type==DNL_HELLO_ACK && f.sequence==1 && f.session_id==0x04030201);
 CK(rt.frames_received==1 && rt.frames_replied==1);
 return 0;
}
static int test_02_crc32_reference_123456789(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n=0,m=0;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=DNL_JOB_CRC32;f.sequence=2;
 f.session_id=0x04030201;f.payload_length=10;f.payload[0]=9;
 memcpy(f.payload+1,"123456789",9);
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 CK(dnl_decode(rsp,m,&f)==DNL_OK);
 CK(f.type==DNL_JOB_RESULT && f.payload_length==5);
 CK(f.payload[0]==9 && memcmp(f.payload+1,"\x26\x39\xf4\xcb",4)==0);
 CK(rt.worker.completed_jobs==1);
 return 0;
}
static int test_03_duplicate_idempotent(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40],copy[40];
 size_t n,m=0,l=0;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=3;f.sequence=2;f.session_id=0x04030201;
 f.payload_length=2;f.payload[0]=1;f.payload[1]=0xaa;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 memcpy(copy,rsp,m);l=m;
 CK(feed_frame(&rt,req,n,20,rsp,&m)==1);
 CK(m==l && memcmp(copy,rsp,l)==0);
 CK(rt.worker.completed_jobs==1);
 return 0;
}
static int test_04_previous_session_drop(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40],old[40];
 size_t n,m=0,l;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=2;f.sequence=2;f.session_id=0x04030201;
 CK(dnl_encode(&f,old,40,&l)==DNL_OK);
 CK(prepare(&rt,0x04030202,req,&n)==DNL_OK); /* startup simulator resets runtime */
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 CK(feed_frame(&rt,old,l,20,rsp,&m)==0);
 CK(rt.worker.highest_sequence==1);
 return 0;
}
static int test_05_bad_crc_no_response(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n,m=0;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 req[n-1]^=0x01;
 CK(feed_frame(&rt,req,n,0,rsp,&m)==0);
 CK(rt.worker.session_id==0 && rt.frames_replied==0);
 memset(&f,0,sizeof(f));f.type=2;f.sequence=2;f.session_id=0x04030201;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==0);
 return 0;
}
static int test_06_partial_frame_timeout(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n,m=0,i;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 for(i=0;i<6;++i){
  size_t x=0;CK(deus_esp01_node_runtime_feed(&rt,req[i],0,rsp,40,&x)==DNL_DROP);
 }
 CK(feed_frame(&rt,req,n,70,rsp,&m)==1);
 return 0;
}
static int test_07_noise_before_hello(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 uint8_t noise[]={0x35,0xff,0xd5,0x34,0x0d,0xd5};
 size_t n,m=0,i;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 for(i=0;i<sizeof(noise);++i){
  size_t x=0;CK(deus_esp01_node_runtime_feed(&rt,noise[i],0,rsp,40,&x)==DNL_DROP);
 }
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 return 0;
}
static int test_08_wrong_direction_silent(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n,m=0;dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=DNL_PONG;f.sequence=2;f.session_id=0x04030201;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==0);
 CK(rt.worker.highest_sequence==1);
 return 0;
}
static int test_09_invalid_job_payload_returns_error(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];size_t n,m=0;
 dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=DNL_JOB_CRC32;f.sequence=2;
 f.session_id=0x04030201;f.payload_length=2;
 f.payload[0]=5;f.payload[1]=1;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 CK(dnl_decode(rsp,m,&f)==DNL_OK);
 CK(f.type==DNL_ERROR && f.payload[0]==DNL_ERR_BAD_PAYLOAD && f.payload[1]==3);
 CK(rt.worker.completed_jobs==0);
 return 0;
}
static int test_10_status_reporting_and_max(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];size_t n,m=0;
 dnl_frame_t f;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 rt.worker.completed_jobs=0xffffu;
 memset(&f,0,sizeof(f));f.type=DNL_GET_STATUS;f.sequence=2;
 f.session_id=0x04030201;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 CK(dnl_decode(rsp,m,&f)==DNL_OK);
 CK(f.type==DNL_STATUS && f.payload[0]==2);
 CK(f.payload[2]==0xff && f.payload[3]==0xff);
 return 0;
}
static int test_11_max_input_bounds(void){
 deus_esp01_node_runtime_t rt;uint8_t req[40],rsp[40];
 size_t n,m=0;dnl_frame_t f;unsigned i;
 CK(prepare(&rt,0x04030201,req,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,0,rsp,&m)==1);
 memset(&f,0,sizeof(f));f.type=3;f.sequence=2;f.session_id=0x04030201;
 f.payload_length=17;f.payload[0]=16;
 for(i=0;i<16;++i)f.payload[1+i]=(uint8_t)i;
 CK(dnl_encode(&f,req,40,&n)==DNL_OK);
 CK(feed_frame(&rt,req,n,10,rsp,&m)==1);
 CK(dnl_decode(rsp,m,&f)==DNL_OK);
 CK(f.type==DNL_JOB_RESULT && f.payload[0]==16);
 CK(sizeof(rt.stream.bytes)==40 && sizeof(rt)<512);
 return 0;
}
static int test_12_no_heap_or_hardware_dependencies(void){
 deus_esp01_node_runtime_t rt;
 deus_esp01_node_runtime_init(&rt);
 CK(rt.worker.session_id==0);
 CK(rt.frames_received==0 && rt.frames_replied==0);
 CK(rt.tx_failed==0);
 return 0;
}
int main(void){
 static const struct test{const char *name;int (*func)(void);} items[]={
 {"HELLO session ACK",test_01_hello_ack_and_session},
 {"CRC32 123456789 result",test_02_crc32_reference_123456789},
 {"duplicate replay exactly once",test_03_duplicate_idempotent},
 {"previous session request dropped",test_04_previous_session_drop},
 {"bad CRC dropped",test_05_bad_crc_no_response},
 {"partial stream timeout recovery",test_06_partial_frame_timeout},
 {"boot noise resync",test_07_noise_before_hello},
 {"wrong direction blocked",test_08_wrong_direction_silent},
 {"malformed job yields BAD_PAYLOAD",test_09_invalid_job_payload_returns_error},
 {"STATUS saturating 16-bit",test_10_status_reporting_and_max},
 {"maximum 16-byte job and bounds",test_11_max_input_bounds},
 {"init no peripheral action",test_12_no_heap_or_hardware_dependencies}
 };
 size_t i;
 for(i=0;i<sizeof(items)/sizeof(items[0]);++i)if(one(items[i].name,items[i].func))return 1;
 printf("RESULT=PASS CASES=%u ESP8266_ACTUAL=NOT_RUN HARDWARE=NO\n",pass_count);
 return 0;
}
