/* Transport neutral ESP-01 worker; never reads/writes UART itself. */
#include "node_runtime.h"
#include <string.h>
void deus_esp01_node_runtime_init(deus_esp01_node_runtime_t *rt){
 if(rt){
  memset(rt,0,sizeof(*rt));
  dnl_worker_init(&rt->worker);
  dnl_stream_init(&rt->stream);
 }
}
int deus_esp01_node_runtime_feed(deus_esp01_node_runtime_t *rt,
 uint8_t byte,uint32_t ms,uint8_t *reply,size_t cap,size_t *reply_len){
 dnl_frame_t frame;
 uint8_t raw[DNL_MAX_FRAME];size_t n=0;
 int status;
 if(!rt || !reply || !reply_len)return DNL_BAD_ARGUMENT;
 *reply_len=0u;
 status=dnl_stream_feed(&rt->stream,byte,ms,&frame);
 if(status!=DNL_OK){
  if(status==DNL_DROP){
   if(rt->invalid_or_partial_bytes!=UINT32_MAX)++rt->invalid_or_partial_bytes;
  }
  return status;
 }
 if(rt->frames_received!=UINT32_MAX)++rt->frames_received;
 /* Decode was already CRC/length validated; canonical frame re-encoding
  * does not bypass role/session/duplicate validation in worker_process. */
 status=dnl_encode(&frame,raw,sizeof(raw),&n);
 if(status!=DNL_OK)return status;
 status=dnl_worker_process(&rt->worker,raw,n,reply,cap,reply_len);
 if(status==DNL_OK && rt->frames_replied!=UINT32_MAX)++rt->frames_replied;
 return status;
}
