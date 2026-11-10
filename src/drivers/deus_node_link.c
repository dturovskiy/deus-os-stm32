/* Deus Node Link v0 REV2: transport-neutral, allocation-free C11 codec/FSM. */
#include "deus_node_link.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p) {
 return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static uint32_t rd32(const uint8_t *p) {
 return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void wr16(uint8_t *p, uint16_t n) {
 p[0]=(uint8_t)n; p[1]=(uint8_t)(n >> 8);
}
static void wr32(uint8_t *p, uint32_t n) {
 p[0]=(uint8_t)n; p[1]=(uint8_t)(n >> 8);
 p[2]=(uint8_t)(n >> 16); p[3]=(uint8_t)(n >> 24);
}
uint16_t dnl_crc16_ccitt_false(const uint8_t *bytes, size_t n) {
 uint16_t crc=0xffffu;
 size_t i; unsigned j;
 if (!bytes && n) return 0;
 for(i=0;i<n;++i) {
  crc=(uint16_t)(crc ^ ((uint16_t)bytes[i] << 8));
  for(j=0;j<8;++j) crc=(uint16_t)((crc&0x8000u)?(((uint32_t)crc<<1)^0x1021u):((uint32_t)crc<<1));
 }
 return crc;
}
uint32_t dnl_crc32_ieee(const uint8_t *bytes,size_t n) {
 uint32_t crc=0xffffffffu;
 size_t i; unsigned j;
 if (!bytes && n) return 0;
 for(i=0;i<n;++i){
  crc^=bytes[i];
  for(j=0;j<8;++j) crc=(crc&1u)?((crc>>1)^0xedb88320u):(crc>>1);
 }
 return crc^0xffffffffu;
}
int dnl_is_request(uint8_t type){return type>=1u && type<=4u;}
int dnl_is_reply(uint8_t type){return type==0x81u || type==0x82u ||
 type==0x83u || type==0x84u || type==0xffu;}
uint32_t dnl_response_timeout_ms(uint8_t type){
 if(type==DNL_JOB_CRC32)return 500u;
 if(type==DNL_HELLO || type==DNL_PING || type==DNL_GET_STATUS)return 200u;
 return 0;
}
int dnl_encode(const dnl_frame_t *f,uint8_t *out,size_t cap,size_t *len){
 size_t n;
 uint16_t checksum;
 if(!f || !out || !len)return DNL_BAD_ARGUMENT;
 *len=0;
 if(f->sequence==0 || f->payload_length>DNL_MAX_PAYLOAD)return DNL_BAD_MESSAGE;
 n=DNL_FRAME_OVERHEAD+(size_t)f->payload_length;
 if(cap<n)return DNL_OUTPUT_TOO_SMALL;
 out[0]=0xd5u;out[1]=0x4eu;out[2]=DNL_VERSION;out[3]=f->type;
 out[4]=DNL_NODE_ID;out[5]=0;
 wr16(out+6,f->sequence);out[8]=f->payload_length;out[9]=0;
 wr32(out+10,f->session_id);
 if(f->payload_length)memcpy(out+14,f->payload,f->payload_length);
 checksum=dnl_crc16_ccitt_false(out+2,n-4u);
 wr16(out+14+f->payload_length,checksum);
 *len=n;return DNL_OK;
}
int dnl_decode(const uint8_t *buf,size_t n,dnl_frame_t *f){
 uint8_t length;
 if(!buf || !f)return DNL_BAD_ARGUMENT;
 if(n<16u || n>40u)return DNL_BAD_FRAME;
 if(buf[0]!=0xd5u || buf[1]!=0x4eu || buf[2]!=DNL_VERSION ||
    buf[4]!=DNL_NODE_ID || buf[5]!=0 || buf[9]!=0)return DNL_BAD_FRAME;
 length=buf[8];
 if(length>DNL_MAX_PAYLOAD || n!=(size_t)length+16u ||
    rd16(buf+6)==0)return DNL_BAD_FRAME;
 if(rd16(buf+14+length)!=dnl_crc16_ccitt_false(buf+2,n-4u))
    return DNL_BAD_FRAME;
 f->type=buf[3];f->sequence=rd16(buf+6);f->session_id=rd32(buf+10);
 f->payload_length=length;
 memset(f->payload,0,sizeof(f->payload));
 if(length)memcpy(f->payload,buf+14,length);
 return DNL_OK;
}
int dnl_validate_payload(const dnl_frame_t *f){
 if(!f)return DNL_BAD_ARGUMENT;
 switch(f->type){
 case DNL_HELLO:
  return (f->payload_length==4u && rd32(f->payload)!=0u)?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_HELLO_ACK:
  return (f->payload_length==8u && rd32(f->payload)!=0u &&
   f->payload[4]==1u && f->payload[5]==3u &&
   f->payload[6]==16u && f->payload[7]==2u)?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_PING:case DNL_PONG:case DNL_GET_STATUS:
  return f->payload_length==0u?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_JOB_CRC32:
  return (f->payload_length>=1u && f->payload[0]<=DNL_MAX_JOB_BYTES &&
   f->payload_length==(uint8_t)(1u+f->payload[0]))?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_JOB_RESULT:
  return (f->payload_length==5u && f->payload[0]<=DNL_MAX_JOB_BYTES)?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_STATUS:
  return (f->payload_length==4u && f->payload[0]>=1u && f->payload[0]<=4u &&
    f->payload[1]<=5u)?DNL_OK:DNL_BAD_MESSAGE;
 case DNL_ERROR:
  return (f->payload_length==2u && f->payload[0]>=1u &&
    f->payload[0]<=5u)?DNL_OK:DNL_BAD_MESSAGE;
 default:
  return DNL_OK;  /* Unknown requests are handled with UNSUPPORTED. */
 }
}
void dnl_stream_init(dnl_stream_t *s){if(s)memset(s,0,sizeof(*s));}
static void stream_shift(dnl_stream_t *s){
 if(s->used){--s->used;memmove(s->bytes,s->bytes+1u,s->used);}
 if(!s->used)s->has_started=0u;
}
int dnl_stream_feed(dnl_stream_t *s,uint8_t byte,uint32_t time_ms,dnl_frame_t *out){
 size_t expected;
 if(!s || !out)return DNL_BAD_ARGUMENT;
 if(s->has_started && (uint32_t)(time_ms-s->started_at_ms)>DNL_PARTIAL_TIMEOUT_MS)
   dnl_stream_init(s);
 if(!s->has_started){s->started_at_ms=time_ms;s->has_started=1u;}
 if(s->used>=DNL_MAX_FRAME) dnl_stream_init(s);
 s->bytes[s->used++]=byte;
 for(;;){
  if(s->used==0u)return DNL_DROP;
  if(s->bytes[0]!=0xd5u){stream_shift(s);continue;}
  if(s->used==1u)return DNL_DROP;
  if(s->bytes[1]!=0x4eu){stream_shift(s);continue;}
  if(s->used<14u)return DNL_DROP;
  if(s->bytes[2]!=DNL_VERSION || s->bytes[4]!=DNL_NODE_ID ||
     s->bytes[5]!=0u || s->bytes[9]!=0u ||
     (s->bytes[6]==0u && s->bytes[7]==0u) ||
     s->bytes[8]>DNL_MAX_PAYLOAD){stream_shift(s);continue;}
  expected=(size_t)s->bytes[8]+DNL_FRAME_OVERHEAD;
  if(s->used<expected)return DNL_DROP;
  if(dnl_decode(s->bytes,expected,out)!=DNL_OK){stream_shift(s);continue;}
  dnl_stream_init(s);
  return DNL_OK;
 }
}
void dnl_worker_init(dnl_worker_t *w){if(w)memset(w,0,sizeof(*w));}
static int output_reply(dnl_frame_t *f,uint8_t *buf,size_t cap,size_t *n){
 return dnl_encode(f,buf,cap,n);
}
static int worker_error(uint8_t type,uint16_t seq,uint32_t sess,uint8_t error,
                        uint8_t *out,size_t cap,size_t *outlen){
 dnl_frame_t f;
 memset(&f,0,sizeof(f));f.type=DNL_ERROR;f.sequence=seq;f.session_id=sess;
 f.payload_length=2u;f.payload[0]=error;f.payload[1]=type;
 return output_reply(&f,out,cap,outlen);
}
int dnl_worker_process(dnl_worker_t *w,const uint8_t *raw,size_t n,
                       uint8_t *out,size_t cap,size_t *outlen){
 dnl_frame_t req,resp;size_t reply_len=0;
 uint32_t challenge;
 int status;
 if(!w || !raw || !out || !outlen)return DNL_BAD_ARGUMENT;
 *outlen=0u;
 if(dnl_decode(raw,n,&req)!=DNL_OK)return DNL_DROP;
 if(req.type>=0x80u)return DNL_DROP;
 if(req.type==DNL_HELLO){
  if(req.session_id!=0u || req.sequence!=1u ||
      dnl_validate_payload(&req)!=DNL_OK)return DNL_DROP;
  challenge=rd32(req.payload);
  if(challenge==w->session_id && w->hello_reply_length){
   if(cap<w->hello_reply_length)return DNL_OUTPUT_TOO_SMALL;
   memcpy(out,w->hello_reply,w->hello_reply_length);
   *outlen=w->hello_reply_length;return DNL_OK;
  }
  /* Commit new state only after output capacity is verified. */
  memset(&resp,0,sizeof(resp));resp.type=DNL_HELLO_ACK;
  resp.sequence=1u;resp.session_id=challenge;resp.payload_length=8u;
  wr32(resp.payload,challenge);resp.payload[4]=1u;resp.payload[5]=3u;
  resp.payload[6]=16u;resp.payload[7]=2u;
  status=output_reply(&resp,out,cap,&reply_len);
  if(status!=DNL_OK)return status;
  dnl_worker_init(w);
  w->session_id=challenge;w->highest_sequence=1u;
  memcpy(w->hello_reply,out,reply_len);w->hello_reply_length=(uint8_t)reply_len;
  *outlen=reply_len;return DNL_OK;
 }
 if(w->session_id==0u || req.session_id!=w->session_id)return DNL_DROP;
 if(req.sequence==w->highest_sequence && w->has_last){
  if(n==w->last_request_length && memcmp(raw,w->last_request,n)==0){
   if(cap<w->last_reply_length)return DNL_OUTPUT_TOO_SMALL;
   memcpy(out,w->last_reply,w->last_reply_length);
   *outlen=w->last_reply_length;return DNL_OK;
  }
  return worker_error(req.type,req.sequence,w->session_id,DNL_ERR_BAD_SEQUENCE,
                      out,cap,outlen);
 }
 if(req.sequence<w->highest_sequence)return DNL_DROP;
 if((uint32_t)req.sequence != (uint32_t)w->highest_sequence+1u)
  return worker_error(req.type,req.sequence,w->session_id,DNL_ERR_BAD_SEQUENCE,
                      out,cap,outlen);
 memset(&resp,0,sizeof(resp));resp.sequence=req.sequence;resp.session_id=w->session_id;
 if(dnl_validate_payload(&req)!=DNL_OK){
  resp.type=DNL_ERROR;resp.payload_length=2u;
  resp.payload[0]=DNL_ERR_BAD_PAYLOAD;resp.payload[1]=req.type;
 }else if(req.type==DNL_PING){resp.type=DNL_PONG;
 }else if(req.type==DNL_JOB_CRC32){
  resp.type=DNL_JOB_RESULT;resp.payload_length=5u;
  resp.payload[0]=req.payload[0];
  wr32(resp.payload+1,dnl_crc32_ieee(req.payload+1,req.payload[0]));
 }else if(req.type==DNL_GET_STATUS){
  resp.type=DNL_STATUS;resp.payload_length=4u;resp.payload[0]=2u;
  resp.payload[1]=0u;wr16(resp.payload+2,w->completed_jobs);
 }else{
  resp.type=DNL_ERROR;resp.payload_length=2u;
  resp.payload[0]=DNL_ERR_UNSUPPORTED;resp.payload[1]=req.type;
 }
 status=output_reply(&resp,out,cap,&reply_len);
 if(status!=DNL_OK)return status;
 /* At most 40 B retained; preserve request/response for idempotent replay. */
 memcpy(w->last_request,raw,n);w->last_request_length=(uint8_t)n;
 memcpy(w->last_reply,out,reply_len);w->last_reply_length=(uint8_t)reply_len;
 w->has_last=1u;w->highest_sequence=req.sequence;
 if(req.type==DNL_JOB_CRC32 && resp.type==DNL_JOB_RESULT &&
    w->completed_jobs<0xffffu)++w->completed_jobs;
 *outlen=reply_len;return DNL_OK;
}
void dnl_controller_init(dnl_controller_t *c){if(c)memset(c,0,sizeof(*c));}
static int controller_emit(dnl_controller_t *c,const dnl_frame_t *f,
                           uint8_t *out,size_t cap,size_t *n){
 int status=dnl_encode(f,out,cap,n);
 if(status!=DNL_OK)return status;
 memcpy(c->request_bytes,out,*n);
 c->request_length=(uint8_t)*n;c->pending=1u;c->pending_type=f->type;
 c->retry_count=0u;
 return DNL_OK;
}
int dnl_controller_hello(dnl_controller_t *c,uint32_t challenge,
                         uint8_t *out,size_t cap,size_t *n){
 dnl_frame_t f;int s;
 if(!c || !out || !n)return DNL_BAD_ARGUMENT;
 *n=0;
 if(challenge==0u || challenge==c->session_id)return DNL_BAD_MESSAGE;
 memset(&f,0,sizeof(f));f.type=DNL_HELLO;f.sequence=1u;
 f.session_id=0u;f.payload_length=4u;wr32(f.payload,challenge);
 s=controller_emit(c,&f,out,cap,n);
 if(s==DNL_OK){
  c->session_id=challenge;c->last_sequence=1u;c->ready=0u;
  c->requested_job_length=0u;
 }
 return s;
}
int dnl_controller_request(dnl_controller_t *c,uint8_t type,
                           const uint8_t *payload,uint8_t length,
                           uint8_t *out,size_t cap,size_t *n){
 dnl_frame_t f;int s;
 if(!c || !out || !n)return DNL_BAD_ARGUMENT;
 *n=0;
 if(!c->ready || c->pending)return DNL_NOT_READY_OR_BUSY;
 if(type!=DNL_PING && type!=DNL_JOB_CRC32 &&
    type!=DNL_GET_STATUS)return DNL_BAD_MESSAGE;
 if(length>DNL_MAX_PAYLOAD || (length && !payload))return DNL_BAD_ARGUMENT;
 if(c->last_sequence==0xffffu)return DNL_REHANDSHAKE_REQUIRED;
 memset(&f,0,sizeof(f));f.type=type;f.sequence=(uint16_t)(c->last_sequence+1u);
 f.session_id=c->session_id;f.payload_length=length;
 if(length)memcpy(f.payload,payload,length);
 if(dnl_validate_payload(&f)!=DNL_OK)return DNL_BAD_MESSAGE;
 s=controller_emit(c,&f,out,cap,n);
 if(s==DNL_OK){
  c->last_sequence=f.sequence;
  c->requested_job_length=(type==DNL_JOB_CRC32)?f.payload[0]:0u;
 }
 return s;
}
int dnl_controller_receive(dnl_controller_t *c,const uint8_t *raw,size_t n){
 dnl_frame_t f;uint8_t expected;
 if(!c || !raw)return DNL_BAD_ARGUMENT;
 if(dnl_decode(raw,n,&f)!=DNL_OK)return DNL_BAD_FRAME;
 if(!dnl_is_reply(f.type))return DNL_WRONG_DIRECTION;
 if(!c->pending)return DNL_UNSOLICITED;
 if(f.session_id!=c->session_id)return DNL_STALE_SESSION;
 if(f.sequence!=c->last_sequence)return DNL_WRONG_SEQUENCE;
 expected=(uint8_t)(c->pending_type | 0x80u);
 if(f.type!=expected && f.type!=DNL_ERROR)return DNL_WRONG_REPLY_TYPE;
 if(dnl_validate_payload(&f)!=DNL_OK)return DNL_BAD_MESSAGE;
 if(f.type==DNL_ERROR){
  if(f.payload[1]!=c->pending_type)return DNL_WRONG_REPLY_TYPE;
  c->pending=0u;
  return DNL_REMOTE_ERROR;
 }
 if(c->pending_type==DNL_HELLO &&
    (rd32(f.payload)!=c->session_id || f.session_id!=rd32(f.payload)))
  return DNL_BAD_HELLO_ECHO;
 if(c->pending_type==DNL_JOB_CRC32 &&
    f.payload[0]!=c->requested_job_length)return DNL_BAD_RESULT_LENGTH;
 c->pending=0u;
 if(c->pending_type==DNL_HELLO)c->ready=1u;
 return DNL_OK;
}
int dnl_controller_timeout(dnl_controller_t *c,uint8_t *out,size_t cap,size_t *n){
 if(!c || !out || !n)return DNL_BAD_ARGUMENT;
 *n=0;
 if(!c->pending)return DNL_DROP;
 if((c->pending_type==DNL_HELLO || c->pending_type==DNL_PING ||
     c->pending_type==DNL_GET_STATUS) && c->retry_count==0u){
  if(cap<c->request_length)return DNL_OUTPUT_TOO_SMALL;
  memcpy(out,c->request_bytes,c->request_length);
  *n=c->request_length;c->retry_count=1u;return DNL_OK;
 }
 c->pending=0u;c->ready=0u;
 return DNL_DROP;
}
