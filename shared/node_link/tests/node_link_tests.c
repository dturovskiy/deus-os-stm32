/* A000017 E002 T007 G001 -- host-only portable C11 codec tests.
 * Exact expected frozen REV2 vectors, deliberately independent of Python model.
 */
#include "deus_node_link.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL:%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;} }while(0)
#define CHECK_OK(x) CHECK((x)==DNL_OK)
#define SESS 0x04030201u
static unsigned cases=0;
static int run_case(const char *name,int (*fn)(void)){
 int v=fn();if(v)return 1;
 ++cases;printf("PASS %02u %s\n",cases,name);return 0;
}
static size_t hex_bytes(const char *hex,uint8_t *p,size_t cap){
 size_t n=0; unsigned v=0;int consumed=0;
 while(*hex){
  if(*hex==' ' || *hex=='\n'){++hex;continue;}
  if(sscanf(hex,"%2x%n",&v,&consumed)!=1 || consumed!=2 || n>=cap)return 0;
  p[n++]=(uint8_t)v;hex+=2;
 }
 return n;
}
static const struct vector {
 uint8_t type;uint16_t seq;uint32_t session;uint8_t len;
 const char *payload;const char *wire;
} vectors[]={
 {1,1,0,4,"01 02 03 04","D5 4E 01 01 01 00 01 00 04 00 00 00 00 00 01 02 03 04 F0 B4"},
 {0x81,1,SESS,8,"01 02 03 04 01 03 10 02","D5 4E 01 81 01 00 01 00 08 00 01 02 03 04 01 02 03 04 01 03 10 02 46 DE"},
 {2,2,SESS,0,"","D5 4E 01 02 01 00 02 00 00 00 01 02 03 04 DF CA"},
 {0x82,2,SESS,0,"","D5 4E 01 82 01 00 02 00 00 00 01 02 03 04 75 43"},
 {3,3,SESS,10,"09 31 32 33 34 35 36 37 38 39","D5 4E 01 03 01 00 03 00 0A 00 01 02 03 04 09 31 32 33 34 35 36 37 38 39 C8 49"},
 {0x83,3,SESS,5,"09 26 39 F4 CB","D5 4E 01 83 01 00 03 00 05 00 01 02 03 04 09 26 39 F4 CB 7E 31"},
 {4,4,SESS,0,"","D5 4E 01 04 01 00 04 00 00 00 01 02 03 04 E0 BB"},
 {0x84,4,SESS,4,"02 00 01 00","D5 4E 01 84 01 00 04 00 04 00 01 02 03 04 02 00 01 00 81 68"},
 {0x55,5,SESS,0,"","D5 4E 01 55 01 00 05 00 00 00 01 02 03 04 92 35"},
 {0xff,5,SESS,2,"01 55","D5 4E 01 FF 01 00 05 00 02 00 01 02 03 04 01 55 08 F6"}
};
static int test_01_crc_algorithms(void){
 static const uint8_t check[]="123456789";
 CHECK(dnl_crc16_ccitt_false(check,9)==0x29b1u);
 CHECK(dnl_crc32_ieee(check,9)==0xcbf43926u);
 CHECK(dnl_crc32_ieee(NULL,0)==0);
 return 0;
}
static int test_02_vectors(void){
 size_t i;
 for(i=0;i<sizeof(vectors)/sizeof(vectors[0]);++i){
  uint8_t payload[24],actual[40],wire[40];size_t n=0,wn;dnl_frame_t f,g;
  memset(&f,0,sizeof(f));
  f.type=vectors[i].type;f.sequence=vectors[i].seq;
  f.session_id=vectors[i].session;f.payload_length=vectors[i].len;
  CHECK(hex_bytes(vectors[i].payload,payload,sizeof(payload))==f.payload_length);
  memcpy(f.payload,payload,f.payload_length);
  CHECK_OK(dnl_encode(&f,actual,sizeof(actual),&n));
  wn=hex_bytes(vectors[i].wire,wire,sizeof(wire));
  CHECK(wn==n);CHECK(memcmp(wire,actual,n)==0);
  CHECK_OK(dnl_decode(actual,n,&g));
  CHECK(g.type==f.type && g.sequence==f.sequence && g.session_id==f.session_id);
  CHECK(g.payload_length==f.payload_length);
  CHECK(memcmp(g.payload,f.payload,f.payload_length)==0);
  CHECK_OK(dnl_validate_payload(&g));
 }
 return 0;
}
static int test_03_enc_boundaries(void){
 dnl_frame_t f;uint8_t b[40];size_t n=333;
 memset(&f,0,sizeof(f));f.type=DNL_PING;f.sequence=1;f.session_id=SESS;
 CHECK(dnl_encode(&f,b,15,&n)==DNL_OUTPUT_TOO_SMALL);
 CHECK(n==0);CHECK_OK(dnl_encode(&f,b,16,&n));CHECK(n==16);
 f.payload_length=24;CHECK_OK(dnl_encode(&f,b,40,&n));CHECK(n==40);
 f.payload_length=25;CHECK(dnl_encode(&f,b,40,&n)==DNL_BAD_MESSAGE);
 f.payload_length=0;f.sequence=0;
 CHECK(dnl_encode(&f,b,40,&n)==DNL_BAD_MESSAGE);
 return 0;
}
static int test_04_corruption(void){
 dnl_frame_t f,g;uint8_t b[40],original[40];size_t n=0;
 memset(&f,0,sizeof(f));f.type=DNL_PING;f.sequence=2;f.session_id=SESS;
 CHECK_OK(dnl_encode(&f,b,40,&n));memcpy(original,b,n);
 b[0]=0;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[2]=2;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[4]=2;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[5]=1;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[9]=1;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[6]=0;b[7]=0;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[8]=25;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);memcpy(b,original,n);
 b[n-1]^=1;CHECK(dnl_decode(b,n,&g)==DNL_BAD_FRAME);
 CHECK(dnl_decode(original,n-1,&g)==DNL_BAD_FRAME);
 CHECK(dnl_decode(original,n+1,&g)==DNL_BAD_FRAME);
 return 0;
}
static int test_05_payload_negative(void){
 dnl_frame_t f;memset(&f,0,sizeof(f));
 f.type=DNL_HELLO;f.payload_length=4;
 CHECK(dnl_validate_payload(&f)==DNL_BAD_MESSAGE);
 f.payload[0]=1;CHECK_OK(dnl_validate_payload(&f));
 f.type=DNL_JOB_CRC32;f.payload_length=2;f.payload[0]=3;
 CHECK(dnl_validate_payload(&f)==DNL_BAD_MESSAGE);
 f.type=DNL_STATUS;f.payload_length=4;f.payload[0]=0;
 CHECK(dnl_validate_payload(&f)==DNL_BAD_MESSAGE);
 f.type=DNL_HELLO_ACK;f.payload_length=8;memset(f.payload,0,24);
 CHECK(dnl_validate_payload(&f)==DNL_BAD_MESSAGE);
 return 0;
}
static int test_06_stream_splits(void){
 size_t cut,i;uint8_t bytes[40];dnl_stream_t s;dnl_frame_t out;
 size_t n=hex_bytes(vectors[4].wire,bytes,40);
 CHECK(n==26);
 for(cut=0;cut<=n;++cut){
  unsigned received=0;dnl_stream_init(&s);
  for(i=0;i<n;++i){
   int rc=dnl_stream_feed(&s,bytes[i],i<cut?0u:20u,&out);
   if(rc==DNL_OK){++received;CHECK(out.type==DNL_JOB_CRC32);}
   else CHECK(rc==DNL_DROP);
   CHECK(s.used<=40);
  }
  CHECK(received==1);
 }
 return 0;
}
static int test_07_stream_noise_crc_and_timeout(void){
 uint8_t bytes[40],noise[]={0x65,0x73,0xd5,0x01,0xd5};
 size_t i,n=hex_bytes(vectors[2].wire,bytes,40);
 dnl_stream_t s;dnl_frame_t out;unsigned pass=0;
 dnl_stream_init(&s);
 for(i=0;i<sizeof(noise);++i)CHECK(dnl_stream_feed(&s,noise[i],0,&out)==DNL_DROP);
 for(i=0;i<n;++i)if(dnl_stream_feed(&s,bytes[i],10,&out)==DNL_OK)++pass;
 CHECK(pass==1);
 bytes[n-1]^=1;
 for(i=0;i<n;++i)CHECK(dnl_stream_feed(&s,bytes[i],20,&out)==DNL_DROP);
 bytes[n-1]^=1;
 for(i=0;i<6;++i)CHECK(dnl_stream_feed(&s,bytes[i],30,&out)==DNL_DROP);
 pass=0;
 for(i=0;i<n;++i)if(dnl_stream_feed(&s,bytes[i],90,&out)==DNL_OK)++pass;
 CHECK(pass==1);return 0;
}
static int hello(dnl_worker_t *w,dnl_controller_t *c,uint32_t challenge){
 uint8_t req[40],reply[40];size_t rn=0,pn=0;
 CHECK_OK(dnl_controller_hello(c,challenge,req,40,&rn));
 CHECK_OK(dnl_worker_process(w,req,rn,reply,40,&pn));
 CHECK_OK(dnl_controller_receive(c,reply,pn));
 CHECK(c->ready==1u);return 0;
}
static int job(dnl_worker_t *w,dnl_controller_t *c,const uint8_t *data,size_t len){
 uint8_t p[17],req[40],rep[40];size_t rn=0,pn=0;dnl_frame_t f;
 CHECK(len<=16);p[0]=(uint8_t)len;if(len)memcpy(p+1,data,len);
 CHECK_OK(dnl_controller_request(c,DNL_JOB_CRC32,p,(uint8_t)(len+1),req,40,&rn));
 CHECK_OK(dnl_worker_process(w,req,rn,rep,40,&pn));CHECK_OK(dnl_decode(rep,pn,&f));
 CHECK(f.type==DNL_JOB_RESULT && f.payload[0]==len);
 CHECK(f.payload[1]==(uint8_t)dnl_crc32_ieee(data,len));
 CHECK_OK(dnl_controller_receive(c,rep,pn));return 0;
}
static int test_08_hello_job_and_status(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t b[40],r[40];
 size_t n,m;dnl_frame_t f;
 dnl_worker_init(&w);dnl_controller_init(&c);
 CHECK(hello(&w,&c,SESS)==0);
 CHECK(job(&w,&c,(const uint8_t *)"123456789",9)==0);
 CHECK(w.completed_jobs==1);
 CHECK_OK(dnl_controller_request(&c,DNL_GET_STATUS,NULL,0,b,40,&n));
 CHECK_OK(dnl_worker_process(&w,b,n,r,40,&m));CHECK_OK(dnl_decode(r,m,&f));
 CHECK(f.type==DNL_STATUS && f.payload[0]==2 && f.payload[2]==1);
 CHECK_OK(dnl_controller_receive(&c,r,m));return 0;
}
static int test_09_duplicate_job_and_changed_bytes(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t req[40],rep[40],again[40];
 size_t n,m,k;dnl_frame_t f;uint8_t payload[]={1,'a'};
 dnl_worker_init(&w);dnl_controller_init(&c);
 CHECK(hello(&w,&c,SESS)==0);
 CHECK_OK(dnl_controller_request(&c,DNL_JOB_CRC32,payload,2,req,40,&n));
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_worker_process(&w,req,n,again,40,&k));
 CHECK(k==m && memcmp(rep,again,m)==0 && w.completed_jobs==1);
 f.type=3;f.sequence=2;f.session_id=SESS;f.payload_length=2;
 f.payload[0]=1;f.payload[1]='b';
 CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK_OK(dnl_worker_process(&w,req,n,again,40,&k));CHECK_OK(dnl_decode(again,k,&f));
 CHECK(f.type==DNL_ERROR && f.payload[0]==DNL_ERR_BAD_SEQUENCE && w.completed_jobs==1);
 return 0;
}
static int test_10_hello_replay_and_new_session(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t h[40],reply[40],cache[40];
 size_t hn,rn,cn;dnl_frame_t f;
 dnl_worker_init(&w);dnl_controller_init(&c);
 CHECK_OK(dnl_controller_hello(&c,SESS,h,40,&hn));
 CHECK_OK(dnl_worker_process(&w,h,hn,reply,40,&rn));
 CHECK_OK(dnl_controller_receive(&c,reply,rn));
 CHECK(job(&w,&c,(const uint8_t *)"x",1)==0);
 CHECK_OK(dnl_worker_process(&w,h,hn,cache,40,&cn));
 CHECK(cn==rn && memcmp(cache,reply,rn)==0 && w.completed_jobs==1);
 CHECK(hello(&w,&c,0x12345678u)==0);
 CHECK(w.completed_jobs==0 && w.session_id==0x12345678u);
 CHECK_OK(dnl_decode(reply,rn,&f));
 CHECK(dnl_controller_receive(&c,reply,rn)==DNL_UNSOLICITED);
 return 0;
}
static int test_11_wrong_reply_type_length_and_session(void){
 dnl_worker_t w;dnl_controller_t c;dnl_frame_t f;
 uint8_t req[40],rep[40],p[]={9,'1','2','3','4','5','6','7','8','9'};
 size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 CHECK_OK(dnl_controller_request(&c,DNL_JOB_CRC32,p,10,req,40,&n));
 memset(&f,0,sizeof(f));f.type=DNL_PONG;f.sequence=2;f.session_id=SESS;
 CHECK_OK(dnl_encode(&f,rep,40,&m));
 CHECK(dnl_controller_receive(&c,rep,m)==DNL_WRONG_REPLY_TYPE);
 f.type=DNL_JOB_RESULT;f.payload_length=5;f.payload[0]=8;
 CHECK_OK(dnl_encode(&f,rep,40,&m));
 CHECK(dnl_controller_receive(&c,rep,m)==DNL_BAD_RESULT_LENGTH);
 f.session_id=0x12345678u;f.payload[0]=9;
 CHECK_OK(dnl_encode(&f,rep,40,&m));
 CHECK(dnl_controller_receive(&c,rep,m)==DNL_STALE_SESSION);
 f.session_id=SESS;f.sequence=3;
 CHECK_OK(dnl_encode(&f,rep,40,&m));
 CHECK(dnl_controller_receive(&c,rep,m)==DNL_WRONG_SEQUENCE);
 CHECK(c.pending==1);
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_controller_receive(&c,rep,m));return 0;
}
static int test_12_direction_unknown_request(void){
 dnl_worker_t w;dnl_controller_t c;dnl_frame_t f;uint8_t req[40],rep[40];
 size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 memset(&f,0,sizeof(f));f.sequence=2;f.session_id=SESS;
 f.type=DNL_PONG;CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK(dnl_worker_process(&w,req,n,rep,40,&m)==DNL_DROP);
 f.type=0x55;CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_decode(rep,m,&f));CHECK(f.type==DNL_ERROR &&
 f.payload[0]==DNL_ERR_UNSUPPORTED && f.payload[1]==0x55);
 return 0;
}
static int test_13_bad_sequence(void){
 dnl_worker_t w;dnl_controller_t c;dnl_frame_t f;uint8_t req[40],rep[40];
 size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 memset(&f,0,sizeof(f));f.type=DNL_PING;f.sequence=4;f.session_id=SESS;
 CHECK_OK(dnl_encode(&f,req,40,&n));CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_decode(rep,m,&f));CHECK(f.type==DNL_ERROR && f.payload[0]==DNL_ERR_BAD_SEQUENCE);
 f.type=DNL_PING;f.sequence=1;f.payload_length=0;
 CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_decode(rep,m,&f));
 CHECK(f.type==DNL_ERROR && f.payload[0]==DNL_ERR_BAD_SEQUENCE);
 return 0;
}
static int test_14_controller_timeout(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t req[40],retry[40];
 size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);
 CHECK_OK(dnl_controller_hello(&c,SESS,req,40,&n));
 CHECK_OK(dnl_controller_timeout(&c,retry,40,&m));
 CHECK(n==m && memcmp(req,retry,n)==0);
 CHECK(dnl_controller_timeout(&c,retry,40,&m)==DNL_DROP && !c.ready && !c.pending);
 dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 CHECK_OK(dnl_controller_request(&c,DNL_PING,NULL,0,req,40,&n));
 CHECK_OK(dnl_controller_timeout(&c,retry,40,&m));
 CHECK(dnl_controller_timeout(&c,retry,40,&m)==DNL_DROP);
 dnl_controller_init(&c);CHECK(hello(&w,&c,SESS+1)==0);
 {uint8_t p[]={1,'x'};
  CHECK_OK(dnl_controller_request(&c,DNL_JOB_CRC32,p,2,req,40,&n));
  CHECK(dnl_controller_timeout(&c,retry,40,&m)==DNL_DROP && !c.ready);
 }
 return 0;
}
static int test_15_job_zero_and_max(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t payload[16];size_t i;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 CHECK(job(&w,&c,NULL,0)==0);
 for(i=0;i<16;++i)payload[i]=(uint8_t)i;
 CHECK(job(&w,&c,payload,16)==0);
 CHECK(w.completed_jobs==2);return 0;
}
static int test_16_saturating_counter(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t x=5;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 w.completed_jobs=65535u;
 CHECK(job(&w,&c,&x,1)==0);
 CHECK(w.completed_jobs==65535u);return 0;
}
static int test_17_out_capacity_is_atomic(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t req[40],rep[40],p[]={1,'x'};
 size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 CHECK_OK(dnl_controller_request(&c,DNL_JOB_CRC32,p,2,req,40,&n));
 CHECK(dnl_worker_process(&w,req,n,rep,4,&m)==DNL_OUTPUT_TOO_SMALL);
 CHECK(w.completed_jobs==0 && w.highest_sequence==1);
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK(w.completed_jobs==1);return 0;
}
static int test_18_no_wrap_or_heap_state(void){
 dnl_worker_t w;dnl_controller_t c;uint8_t req[40];size_t n;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 c.last_sequence=65535;
 CHECK(dnl_controller_request(&c,DNL_PING,NULL,0,req,40,&n)==DNL_REHANDSHAKE_REQUIRED);
 CHECK_OK(dnl_controller_hello(&c,SESS+1,req,40,&n));
 CHECK(sizeof(dnl_frame_t)<=40);
 CHECK(sizeof(dnl_stream_t)<=48);
 CHECK(sizeof(dnl_worker_t)<200);
 CHECK(sizeof(dnl_controller_t)<64);
 return 0;
}
static int test_19_bad_hello(void){
 dnl_worker_t w;dnl_controller_t c;dnl_frame_t f;
 uint8_t req[40],rep[40];size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);
 CHECK(dnl_controller_hello(&c,0,req,40,&n)==DNL_BAD_MESSAGE);
 memset(&f,0,sizeof(f));f.type=DNL_HELLO;f.sequence=1;f.payload_length=4;
 CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK(dnl_worker_process(&w,req,n,rep,40,&m)==DNL_DROP);
 f.payload[0]=1;f.session_id=SESS;
 CHECK_OK(dnl_encode(&f,req,40,&n));
 CHECK(dnl_worker_process(&w,req,n,rep,40,&m)==DNL_DROP);
 return 0;
}
static int test_20_rejected_response_not_consumed(void){
 dnl_controller_t c;dnl_worker_t w;uint8_t req[40],rep[40];size_t n,m;
 dnl_worker_init(&w);dnl_controller_init(&c);CHECK(hello(&w,&c,SESS)==0);
 CHECK_OK(dnl_controller_request(&c,DNL_PING,NULL,0,req,40,&n));
 CHECK(dnl_controller_receive(&c,req,n)==DNL_WRONG_DIRECTION);
 CHECK(c.pending==1);
 CHECK_OK(dnl_worker_process(&w,req,n,rep,40,&m));
 CHECK_OK(dnl_controller_receive(&c,rep,m));
 CHECK(c.pending==0);return 0;
}
int main(void){
 static const struct test {const char *name;int(*fn)(void);} suite[]={
 {"CRC16/CRC32 check",test_01_crc_algorithms},
 {"ten frozen REV2 vectors",test_02_vectors},
 {"encode capacity/header",test_03_enc_boundaries},
 {"malformed magic/version/node/CRC",test_04_corruption},
 {"typed payload negatives",test_05_payload_negative},
 {"byte stream all split positions",test_06_stream_splits},
 {"noise, corrupt CRC, 50ms timeout",test_07_stream_noise_crc_and_timeout},
 {"HELLO, CRC32 job, status",test_08_hello_job_and_status},
 {"idempotent duplicate and altered payload",test_09_duplicate_job_and_changed_bytes},
 {"HELLO replay and session recovery",test_10_hello_replay_and_new_session},
 {"strict wrong type/n/session/seq reply",test_11_wrong_reply_type_length_and_session},
 {"direction and unsupported request",test_12_direction_unknown_request},
 {"out-of-order and stale requests",test_13_bad_sequence},
 {"bounded idempotent retry/no JOB retry",test_14_controller_timeout},
 {"zero- and max-16-byte CRC jobs",test_15_job_zero_and_max},
 {"saturating jobs counter",test_16_saturating_counter},
 {"small output buffer leaves worker intact",test_17_out_capacity_is_atomic},
 {"no sequence wrap and bounded structs",test_18_no_wrap_or_heap_state},
 {"bad HELLO zero challenge/session",test_19_bad_hello},
 {"wrong-direction response not accepted",test_20_rejected_response_not_consumed}
 };
 size_t i;
 for(i=0;i<sizeof(suite)/sizeof(suite[0]);++i)
  if(run_case(suite[i].name,suite[i].fn))return 1;
 printf("RESULT=PASS CASES=%u DEVICE=NO FLASH=NO\n",cases);
 return 0;
}
