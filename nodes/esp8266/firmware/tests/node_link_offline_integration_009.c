/* ESP01-009: real C controller<->worker bounded byte-wire emulator.
 * Only source-host simulated UART. No physical peripheral, heap or SDK.
 * Frozen REV2 wire format and ESP01-008 source are unmodified.
 */
#include "node_runtime.h"
#include "deus_node_link.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define CHECK(expr) do {if(!(expr)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#expr);return 1;}} while(0)
typedef struct { dnl_controller_t ctrl; deus_esp01_node_runtime_t node; } rig_t;
static unsigned count;
static void start(rig_t *r) {
 dnl_controller_init(&r->ctrl);
 deus_esp01_node_runtime_init(&r->node);
}
/* Emulate fragment-by-fragment UART, including untrusted ROM noise. */
static int wire(rig_t *r,const uint8_t *request,size_t length,
 uint32_t now,uint8_t *out,size_t *out_len,unsigned noise){
 static const uint8_t boot_noise[]={0x72,0xd5,0x00,0x4e,0xff,0x0d,0x0a};
 size_t i,n=0;unsigned responses=0;
 *out_len=0;
 if(noise){
  for(i=0;i<sizeof boot_noise;++i){
   int s=deus_esp01_node_runtime_feed(&r->node,boot_noise[i],now,out,40,&n);
   if(s!=DNL_DROP || n) return -10;
  }
 }
 for(i=0;i<length;++i){
  int s=deus_esp01_node_runtime_feed(&r->node,request[i],
    now+(uint32_t)(i/7u),out,40,&n);
  if(s==DNL_OK && n){++responses;*out_len=n;}
  else if(s!=DNL_DROP || n) return -11;
 }
 return (int)responses;
}
static int hello(rig_t *r,uint32_t sid,uint32_t now){
 uint8_t req[40],rsp[40];size_t n=0,m=0;
 CHECK(dnl_controller_hello(&r->ctrl,sid,req,40,&n)==DNL_OK);
 CHECK(wire(r,req,n,now,rsp,&m,1u)==1);
 CHECK(dnl_controller_receive(&r->ctrl,rsp,m)==DNL_OK);
 CHECK(r->ctrl.ready && !r->ctrl.pending && r->node.worker.session_id==sid);
 return 0;
}
static int roundtrip(rig_t *r,uint8_t type,const uint8_t *payload,
 uint8_t length,uint32_t now,dnl_frame_t *frame){
 uint8_t req[40],rsp[40];size_t n=0,m=0;
 CHECK(dnl_controller_request(&r->ctrl,type,payload,length,req,40,&n)==DNL_OK);
 CHECK(wire(r,req,n,now,rsp,&m,0u)==1);
 CHECK(dnl_decode(rsp,m,frame)==DNL_OK);
 CHECK(dnl_controller_receive(&r->ctrl,rsp,m)==DNL_OK);
 return 0;
}
static int t1_hello_crc32(void){
 rig_t r; dnl_frame_t f;uint8_t job[]={9,'1','2','3','4','5','6','7','8','9'};
 start(&r);CHECK(hello(&r,0x125634ab,0)==0);
 CHECK(roundtrip(&r,DNL_JOB_CRC32,job,sizeof job,10,&f)==0);
 CHECK(f.type==DNL_JOB_RESULT && f.payload_length==5 && f.payload[0]==9);
 CHECK(f.payload[1]==0x26 && f.payload[2]==0x39 &&
       f.payload[3]==0xf4 && f.payload[4]==0xcb);
 CHECK(r.node.worker.completed_jobs==1);
 return 0;
}
static int t2_ping_status(void){
 rig_t r;dnl_frame_t f;
 start(&r);CHECK(hello(&r,0x44556611,2)==0);
 CHECK(roundtrip(&r,DNL_PING,0,0,8,&f)==0);
 CHECK(f.type==DNL_PONG && f.payload_length==0);
 CHECK(roundtrip(&r,DNL_GET_STATUS,0,0,12,&f)==0);
 CHECK(f.type==DNL_STATUS && f.payload[0]==2);
 return 0;
}
static int t3_fragmented_noise_resync(void){
 rig_t r;uint8_t req[40],rsp[40];size_t n=0,m=0;
 start(&r);
 CHECK(dnl_controller_hello(&r.ctrl,0x10110203,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,10,rsp,&m,1)==1);
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_OK);
 CHECK(r.node.frames_received==1 && r.node.frames_replied==1);
 return 0;
}
static int t4_lost_ping_reply_replay_identical(void){
 rig_t r;uint8_t req[40],retry[40],rsp[40],rsp2[40];
 size_t n=0,n2=0,m=0,m2=0;
 start(&r);CHECK(hello(&r,0x10302040,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_PING,0,0,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,20,rsp,&m,0)==1);
 CHECK(dnl_controller_timeout(&r.ctrl,retry,40,&n2)==DNL_OK);
 CHECK(n2==n && memcmp(req,retry,n)==0);
 CHECK(wire(&r,retry,n2,30,rsp2,&m2,0)==1);
 CHECK(m==m2 && memcmp(rsp,rsp2,m)==0);
 CHECK(dnl_controller_receive(&r.ctrl,rsp2,m2)==DNL_OK);
 CHECK(!r.ctrl.pending && r.ctrl.ready);
 return 0;
}
static int t5_job_timeout_no_repeat(void){
 rig_t r;uint8_t req[40],retry[40],rsp[40];
 uint8_t job[]={1,0x42};size_t n=0,m=0;
 start(&r);CHECK(hello(&r,0x10203040,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_JOB_CRC32,job,sizeof job,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,10,rsp,&m,0)==1);
 CHECK(r.node.worker.completed_jobs==1);
 CHECK(dnl_controller_timeout(&r.ctrl,retry,40,&m)==DNL_DROP);
 CHECK(m==0 && r.ctrl.ready==0 && r.ctrl.pending==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_PING,0,0,retry,40,&m)==DNL_NOT_READY_OR_BUSY);
 return 0;
}
static int t6_corrupted_reply_preserves_pending(void){
 rig_t r;uint8_t req[40],rsp[40];size_t n=0,m=0;
 start(&r);CHECK(hello(&r,0x10203041,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_PING,0,0,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,15,rsp,&m,0)==1);
 rsp[m-1]^=0x40;
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_BAD_FRAME);
 CHECK(r.ctrl.pending==1);
 rsp[m-1]^=0x40;
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_OK);
 CHECK(!r.ctrl.pending);
 return 0;
}
static int t7_old_session_is_rejected(void){
 rig_t r;uint8_t req[40],rsp[40],old[40];size_t n=0,m=0,k=0;
 start(&r);CHECK(hello(&r,0x10101010,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_PING,0,0,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,5,old,&k,0)==1);
 CHECK(dnl_controller_hello(&r.ctrl,0x20202020,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,10,rsp,&m,0)==1);
 CHECK(dnl_controller_receive(&r.ctrl,old,k)==DNL_STALE_SESSION);
 CHECK(r.ctrl.pending==1);
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_OK);
 CHECK(r.ctrl.ready && !r.ctrl.pending);
 return 0;
}
static int t8_replay_job_no_double_compute(void){
 rig_t r;uint8_t req[40],rsp[40],rsp2[40];
 uint8_t job[]={1,0x91};size_t n=0,m=0,m2=0;
 start(&r);CHECK(hello(&r,0x30303030,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_JOB_CRC32,job,sizeof job,req,40,&n)==DNL_OK);
 CHECK(wire(&r,req,n,10,rsp,&m,0)==1);
 CHECK(wire(&r,req,n,20,rsp2,&m2,0)==1);
 CHECK(m2==m && memcmp(rsp,rsp2,m)==0);
 CHECK(r.node.worker.completed_jobs==1);
 CHECK(dnl_controller_receive(&r.ctrl,rsp2,m2)==DNL_OK);
 return 0;
}
static int t9_max_job_16_bytes(void){
 rig_t r;dnl_frame_t f;uint8_t job[17];size_t i;
 job[0]=16;
 for(i=1;i<sizeof job;++i)job[i]=(uint8_t)(i*13u);
 start(&r);CHECK(hello(&r,0x00445211,0)==0);
 CHECK(roundtrip(&r,DNL_JOB_CRC32,job,sizeof job,15,&f)==0);
 CHECK(f.type==DNL_JOB_RESULT && f.payload[0]==16);
 CHECK(r.node.worker.completed_jobs==1);
 return 0;
}
static int t10_partial_timeout_recovery(void){
 rig_t r;uint8_t req[40],rsp[40];size_t n=0,m=0,i;
 start(&r);
 CHECK(dnl_controller_hello(&r.ctrl,0x12345678,req,40,&n)==DNL_OK);
 for(i=0;i<9;++i){
  size_t out=0;
  CHECK(deus_esp01_node_runtime_feed(&r.node,req[i],1,rsp,40,&out)==DNL_DROP);
 }
 CHECK(wire(&r,req,n,75,rsp,&m,0)==1);
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_OK);
 return 0;
}
static int t11_controller_busy_and_retry_ceiling(void){
 rig_t r;uint8_t req[40],retry[40],rsp[40];size_t n=0,m=0,rn=0;
 start(&r);CHECK(hello(&r,0x00992334,0)==0);
 CHECK(dnl_controller_request(&r.ctrl,DNL_GET_STATUS,0,0,req,40,&n)==DNL_OK);
 CHECK(dnl_controller_request(&r.ctrl,DNL_PING,0,0,retry,40,&rn)==DNL_NOT_READY_OR_BUSY);
 CHECK(dnl_controller_timeout(&r.ctrl,retry,40,&rn)==DNL_OK);
 CHECK(rn==n && memcmp(retry,req,n)==0);
 CHECK(dnl_controller_timeout(&r.ctrl,retry,40,&rn)==DNL_DROP);
 CHECK(r.ctrl.ready==0 && r.ctrl.pending==0);
 CHECK(wire(&r,req,n,3,rsp,&m,0)==1);
 CHECK(dnl_controller_receive(&r.ctrl,rsp,m)==DNL_UNSOLICITED);
 return 0;
}
static int t12_wrap_requires_new_hello(void){
 rig_t r;uint8_t req[40];size_t n=0;
 start(&r);CHECK(hello(&r,0x20022002,0)==0);
 r.ctrl.last_sequence=UINT16_MAX;
 CHECK(dnl_controller_request(&r.ctrl,DNL_GET_STATUS,0,0,req,40,&n)==DNL_REHANDSHAKE_REQUIRED);
 CHECK(dnl_controller_hello(&r.ctrl,0x20022003,req,40,&n)==DNL_OK);
 return 0;
}
int main(void){
 static const struct {const char *name;int(*test)(void);} cases[]={
  {"hello CRC32 peer",t1_hello_crc32},{"ping status wire",t2_ping_status},
  {"UART fragments and ROM noise",t3_fragmented_noise_resync},
  {"lost PING idempotent retry",t4_lost_ping_reply_replay_identical},
  {"JOB timeout fail closed",t5_job_timeout_no_repeat},
  {"bad CRC reply leaves request pending",t6_corrupted_reply_preserves_pending},
  {"stale session reply rejected",t7_old_session_is_rejected},
  {"duplicate JOB exactly once",t8_replay_job_no_double_compute},
  {"16-byte CRC job",t9_max_job_16_bytes},
  {"partial UART timeout",t10_partial_timeout_recovery},
  {"controller busy and retry ceiling",t11_controller_busy_and_retry_ceiling},
  {"sequence wrap rehandshake",t12_wrap_requires_new_hello}
 };
 size_t i;
 for(i=0;i<sizeof cases/sizeof cases[0];++i){
  if(cases[i].test()!=0)return 1;
  ++count;printf("PASS %u %s\n",count,cases[i].name);
 }
 printf("RESULT=PASS CASES=%u PHYSICAL_UART=NO FLASH=NO\n",count);
 return 0;
}
