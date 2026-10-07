
#include <stdint.h>
#include <stddef.h>

#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define REG16(a) (*(volatile uint16_t *)(uintptr_t)(a))

#define BOOT_VID 0x1209u
#define BOOT_PID 0x000Du
#define APP_BASE 0x08002000u
#define APP_END  0x0800F000u
#define APP_MAX_BYTES 53248u
#define META_A 0x0800F000u
#define META_B 0x0800F400u
#define META_PAGE_BYTES 1024u
#define PERSIST_A 0x0800F800u
#define PERSIST_B 0x0800FC00u
#define PHYS_FLASH_END 0x08010000u
#define RAM_BASE 0x20000000u
#define RAM_END  0x20005000u
#define PRODUCT_ID 0x534F4544u
#define TARGET_DEVICE_ID 0x0410u
#define IMAGE_FORMAT_VERSION 1u
#define META_MARKER_OFFSET 0x50u
#define META_MARKER_COMMITTED 0xA55Au

#define FRAME_TYPE_REQUEST 0x04u
#define FRAME_TYPE_RESPONSE 0x86u
#define FLAG_DESTRUCTIVE 0x01u
#define OP_INFO 0x02u
#define OP_BEGIN 0x03u
#define OP_AUTHORIZE 0x04u
#define OP_DATA 0x05u
#define OP_END 0x06u

#define ST_OK 0x00u
#define ST_INVALID_STATE 0x01u
#define ST_BAD_LENGTH 0x02u
#define ST_BAD_FLAGS 0x03u
#define ST_BAD_HEADER 0x04u
#define ST_AUTH_FAILED 0x05u
#define ST_TARGET_MISMATCH 0x06u
#define ST_VERSION_REJECTED 0x07u
#define ST_OUT_OF_SEQUENCE 0x08u
#define ST_FLASH_FAILED 0x09u
#define ST_DIGEST_FAILED 0x0Au
#define ST_VECTOR_INVALID 0x0Bu
#define ST_INTERNAL_ERROR 0x0Cu

#define STATE_RECOVERY_IDLE 0x00u
#define STATE_HEADER_STAGED 0x01u
#define STATE_AUTHORIZED 0x02u
#define STATE_RECEIVING 0x03u
#define STATE_VERIFYING 0x04u
#define STATE_COMMITTED 0x05u

#define RCC_CR REG32(0x40021000u)
#define RCC_CFGR REG32(0x40021004u)
#define RCC_APB1RSTR REG32(0x40021010u)
#define RCC_APB2ENR REG32(0x40021018u)
#define RCC_APB1ENR REG32(0x4002101Cu)
#define RCC_HSEON (1u << 16)
#define RCC_HSERDY (1u << 17)
#define RCC_PLLON (1u << 24)
#define RCC_PLLRDY (1u << 25)
#define RCC_SW_MASK 0x3u
#define RCC_SW_PLL 0x2u
#define RCC_SWS_MASK (0x3u << 2)
#define RCC_SWS_PLL (0x2u << 2)
#define RCC_PPRE1_DIV2 (0x4u << 8)
#define RCC_ADCPRE_DIV6 (0x2u << 14)
#define RCC_PLLSRC_HSE (1u << 16)
#define RCC_PLLMUL_X9 (0x7u << 18)
#define RCC_USBPRE (1u << 22)
#define RCC_APB1_USBEN (1u << 23)
#define RCC_APB1_USBRST (1u << 23)
#define RCC_APB1_BKPEN (1u << 27)
#define RCC_APB1_PWREN (1u << 28)
#define RCC_APB2_IOPAEN (1u << 2)

#define FLASH_ACR REG32(0x40022000u)
#define FLASH_KEYR REG32(0x40022004u)
#define FLASH_SR REG32(0x4002200Cu)
#define FLASH_CR REG32(0x40022010u)
#define FLASH_AR REG32(0x40022014u)
#define FLASH_LATENCY_2 0x2u
#define FLASH_PRFTBE (1u << 4)
#define FLASH_KEY1 0x45670123u
#define FLASH_KEY2 0xCDEF89ABu
#define FLASH_SR_BSY (1u << 0)
#define FLASH_SR_PGERR (1u << 2)
#define FLASH_SR_WRPRTERR (1u << 4)
#define FLASH_SR_EOP (1u << 5)
#define FLASH_CR_PG (1u << 0)
#define FLASH_CR_PER (1u << 1)
#define FLASH_CR_MER (1u << 2)
#define FLASH_CR_STRT (1u << 6)
#define FLASH_CR_LOCK (1u << 7)
#define FLASH_READY_SPIN_LIMIT 4000000u
#define CLOCK_READY_SPIN_LIMIT 1000000u

#define DBGMCU_IDCODE REG32(0xE0042000u)
#define SCB_VTOR REG32(0xE000ED08u)
#define SCB_AIRCR REG32(0xE000ED0Cu)
#define SCB_AIRCR_PRIGROUP (0x7u << 8)
#define SCB_AIRCR_SYSRESETREQ (1u << 2)
#define SCB_AIRCR_VECTKEY (0x5FAu << 16)

#define PWR_CR REG32(0x40007000u)
#define PWR_CR_DBP (1u << 8)
#define BKP_DR1 REG16(0x40006C04u)
#define BKP_DR2 REG16(0x40006C08u)
#define UPDATE_TOKEN1 0xD35Au
#define UPDATE_TOKEN2 0x2CA5u

#define GPIOA_CRH REG32(0x40010804u)
#define GPIOA_BRR REG32(0x40010814u)
#define GPIO_PIN_12 (1u << 12)
#define GPIO_CRH_PA12_MASK (0xFu << 16)
#define GPIO_CRH_PA12_OUTPUT_OD_2MHZ (0x6u << 16)

#define USB_BASE 0x40005C00u
#define USB_PMA_BASE 0x40006000u
#define USB_EP_REG(ep) REG16(USB_BASE + ((uint32_t)(ep) * 4u))
#define USB_CNTR REG16(USB_BASE + 0x40u)
#define USB_ISTR REG16(USB_BASE + 0x44u)
#define USB_DADDR REG16(USB_BASE + 0x4Cu)
#define USB_BTABLE REG16(USB_BASE + 0x50u)
#define USB_CNTR_FRES (1u << 0)
#define USB_CNTR_PDWN (1u << 1)
#define USB_ISTR_RESET (1u << 10)
#define USB_ISTR_ERR (1u << 13)
#define USB_ISTR_PMAOVR (1u << 14)
#define USB_ISTR_CTR (1u << 15)
#define USB_ISTR_EP_ID_MASK 0x000Fu
#define USB_DADDR_EF (1u << 7)
#define USB_DADDR_ADD_MASK 0x007Fu
#define USB_EP_CTR_RX (1u << 15)
#define USB_EP_DTOG_RX (1u << 14)
#define USB_EP_STAT_RX_MASK (0x3u << 12)
#define USB_EP_SETUP (1u << 11)
#define USB_EP_TYPE_MASK (0x3u << 9)
#define USB_EP_TYPE_BULK (0x0u << 9)
#define USB_EP_TYPE_CONTROL (0x1u << 9)
#define USB_EP_KIND (1u << 8)
#define USB_EP_CTR_TX (1u << 7)
#define USB_EP_DTOG_TX (1u << 6)
#define USB_EP_STAT_TX_MASK (0x3u << 4)
#define USB_EP_EA_MASK 0x000Fu
#define USB_EP_STAT_TX_DISABLED (0x0u << 4)
#define USB_EP_STAT_TX_STALL (0x1u << 4)
#define USB_EP_STAT_TX_NAK (0x2u << 4)
#define USB_EP_STAT_TX_VALID (0x3u << 4)
#define USB_EP_STAT_RX_DISABLED (0x0u << 12)
#define USB_EP_STAT_RX_STALL (0x1u << 12)
#define USB_EP_STAT_RX_NAK (0x2u << 12)
#define USB_EP_STAT_RX_VALID (0x3u << 12)
#define USB_EP0 0u
#define USB_EP1 1u
#define USB_EP0_MAX_PACKET 64u
#define USB_BULK_MAX_PACKET 64u
#define USB_BTABLE_LOCAL 0x000u
#define USB_EP0_TX_LOCAL 0x040u
#define USB_EP0_RX_LOCAL 0x080u
#define USB_EP1_RX_LOCAL 0x0C0u
#define USB_EP1_TX_LOCAL 0x100u
#define USB_RX_COUNT_64 0x8400u
#define USB_COUNT_MASK 0x03FFu
#define USB_BTABLE_EP_TX_ADDR(ep) (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 0u)
#define USB_BTABLE_EP_TX_COUNT(ep) (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 2u)
#define USB_BTABLE_EP_RX_ADDR(ep) (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 4u)
#define USB_BTABLE_EP_RX_COUNT(ep) (USB_BTABLE_LOCAL + ((uint16_t)(ep) * 8u) + 6u)
#define USB_DESC_DEVICE 1u
#define USB_DESC_CONFIGURATION 2u
#define USB_DESC_STRING 3u
#define USB_DESC_BOS 15u
#define USB_MS_OS_20_VENDOR_CODE 0x20u
#define USB_MS_OS_20_DESCRIPTOR_INDEX 0x0007u
#define USB_MS_OS_20_SET_TOTAL_LENGTH 178u
#define USB_MS_OS_20_CONFIG_SUBSET_LENGTH 168u
#define USB_MS_OS_20_FUNCTION_SUBSET_LENGTH 160u
#define USB_MS_OS_20_REG_PROPERTY_LENGTH 132u

typedef struct {
    uint32_t h[8];
    uint32_t total_bytes;
    uint32_t buffer_len;
    uint8_t buffer[64];
} sha256_ctx_t;

static uint32_t sha_schedule[64];
static uint8_t rx_packet[64];
static uint8_t tx_packet[64];
static uint8_t staged_header[48];
static uint8_t staged_tag[32];
static uint8_t previous_data[48];
static sha256_ctx_t stream_sha;
static uint16_t expected_offset;
static uint16_t previous_offset;
static uint8_t previous_length;
static uint8_t previous_valid;
static uint8_t update_state;
static uint8_t usb_configuration;
static uint8_t usb_tx_active;
static uint8_t reset_after_tx;
static uint32_t version_floor;
static uint32_t committed_version;
static uint32_t highest_floor_slot;
static uint32_t target_metadata_slot;

extern const uint8_t deus_update_key[32];
static const uint8_t hmac_domain[8] = {'D','E','U','S','H','D','R','1'};
static const uint8_t origin_le[4] = {0x00u,0x20u,0x00u,0x08u};

static const uint32_t sha_k[64] = {
0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

static const uint8_t usb_device_descriptor[] = {
    18u, USB_DESC_DEVICE, 0x10u,0x02u, 0x00u,0x00u,0x00u, 64u,
    0x09u,0x12u, 0x0Du,0x00u, 0x00u,0x01u, 0u,1u,0u,1u
};
static const uint8_t usb_configuration_descriptor[] = {
    9u,USB_DESC_CONFIGURATION, 32u,0u, 1u,1u,0u,0x80u,50u,
    9u,4u, 0u,0u,2u, 0xFFu,0u,0u,0u,
    7u,5u, 0x01u,0x02u, 64u,0u,0u,
    7u,5u, 0x81u,0x02u, 64u,0u,0u
};
static const uint8_t usb_bos_descriptor[] = {
    5u,USB_DESC_BOS,33u,0u,1u,
    28u,16u,5u,0u,
    0xDFu,0x60u,0xDDu,0xD8u,0x89u,0x45u,0xC7u,0x4Cu,
    0x9Cu,0xD2u,0x65u,0x9Du,0x9Eu,0x64u,0x8Au,0x9Fu,
    0x00u,0x00u,0x00u,0x0Au,
    (uint8_t)(USB_MS_OS_20_SET_TOTAL_LENGTH & 0xFFu),
    (uint8_t)(USB_MS_OS_20_SET_TOTAL_LENGTH >> 8),
    USB_MS_OS_20_VENDOR_CODE,0u
};
static const uint8_t usb_ms_os_20_descriptor_set[] = {
    0x0Au,0x00u,0x00u,0x00u,0x00u,0x00u,0x00u,0x0Au,
    (uint8_t)(USB_MS_OS_20_SET_TOTAL_LENGTH & 0xFFu),
    (uint8_t)(USB_MS_OS_20_SET_TOTAL_LENGTH >> 8),
    0x08u,0x00u,0x01u,0x00u,0x00u,0x00u,
    (uint8_t)(USB_MS_OS_20_CONFIG_SUBSET_LENGTH & 0xFFu),
    (uint8_t)(USB_MS_OS_20_CONFIG_SUBSET_LENGTH >> 8),
    0x08u,0x00u,0x02u,0x00u,0x00u,0x00u,
    (uint8_t)(USB_MS_OS_20_FUNCTION_SUBSET_LENGTH & 0xFFu),
    (uint8_t)(USB_MS_OS_20_FUNCTION_SUBSET_LENGTH >> 8),
    0x14u,0x00u,0x03u,0x00u,
    'W','I','N','U','S','B',0u,0u, 0u,0u,0u,0u,0u,0u,0u,0u,
    (uint8_t)(USB_MS_OS_20_REG_PROPERTY_LENGTH & 0xFFu),
    (uint8_t)(USB_MS_OS_20_REG_PROPERTY_LENGTH >> 8),
    0x04u,0x00u,
    0x07u,0x00u,
    0x2Au,0x00u,
    'D',0u,'e',0u,'v',0u,'i',0u,'c',0u,'e',0u,
    'I',0u,'n',0u,'t',0u,'e',0u,'r',0u,'f',0u,
    'a',0u,'c',0u,'e',0u,'G',0u,'U',0u,'I',0u,
    'D',0u,'s',0u,0u,0u,
    0x50u,0x00u,
    '{',0u,'F',0u,'0',0u,'8',0u,'9',0u,'0',0u,'7',0u,
    'B',0u,'7',0u,'-',0u,'B',0u,'E',0u,'C',0u,'4',0u,
    '-',0u,'5',0u,'F',0u,'C',0u,'F',0u,'-',0u,'B',0u,'C',0u,
    '4',0u,'C',0u,'-',0u,'B',0u,'4',0u,'4',0u,'6',0u,
    'E',0u,'D',0u,'3',0u,'4',0u,'5',0u,'D',0u,'8',0u,'7',0u,
    '}',0u,0u,0u,0u,0u
};
static const uint8_t usb_string_language[] = {4u,USB_DESC_STRING,0x09u,0x04u};
static const uint8_t usb_string_product[] = {
    38u,USB_DESC_STRING,
    'D',0u,'e',0u,'u',0u,'s',0u,' ',0u,'O',0u,'S',0u,' ',0u,
    'B',0u,'o',0u,'o',0u,'t',0u,'l',0u,'o',0u,'a',0u,'d',0u,'e',0u,'r',0u
};

_Static_assert(sizeof(usb_device_descriptor)==18u,"dev desc");
_Static_assert(sizeof(usb_configuration_descriptor)==32u,"cfg desc");
_Static_assert(sizeof(usb_bos_descriptor)==33u,"bos desc");
_Static_assert(sizeof(usb_ms_os_20_descriptor_set)==USB_MS_OS_20_SET_TOTAL_LENGTH,"ms os desc");
_Static_assert((10u+USB_MS_OS_20_CONFIG_SUBSET_LENGTH)==USB_MS_OS_20_SET_TOTAL_LENGTH,"ms os cfg len");
_Static_assert((8u+USB_MS_OS_20_FUNCTION_SUBSET_LENGTH)==USB_MS_OS_20_CONFIG_SUBSET_LENGTH,"ms os fn len");
_Static_assert((8u+20u+USB_MS_OS_20_REG_PROPERTY_LENGTH)==USB_MS_OS_20_FUNCTION_SUBSET_LENGTH,"ms os feature len");
_Static_assert(sizeof(usb_string_product)==38u,"product desc");

static uint32_t rotr32(uint32_t x, uint32_t n) { return (x >> n) | (x << (32u - n)); }
static uint32_t load_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
}
static uint16_t load_le16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1]<<8)); }
static void store_le16(uint8_t *p,uint16_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);}
static void store_le32(uint8_t *p,uint32_t v){p[0]=(uint8_t)v;p[1]=(uint8_t)(v>>8);p[2]=(uint8_t)(v>>16);p[3]=(uint8_t)(v>>24);}
static int bytes_equal(const uint8_t *a,const uint8_t *b,uint32_t n) {
    uint8_t d=0u; uint32_t i; for(i=0u;i<n;++i){d|=(uint8_t)(a[i]^b[i]);} return d==0u;
}
static void bytes_copy(uint8_t *d,const uint8_t *s,uint32_t n){uint32_t i;for(i=0u;i<n;++i)d[i]=s[i];}
static void bytes_fill(uint8_t *d,uint8_t v,uint32_t n){uint32_t i;for(i=0u;i<n;++i)d[i]=v;}

static void sha256_transform(sha256_ctx_t *c,const uint8_t block[64]) {
    uint32_t a,b,cc,d,e,f,g,h,t1,t2,s0,s1,ch,maj; uint32_t i;
    for(i=0u;i<16u;++i){
        uint32_t j=i*4u;
        sha_schedule[i]=((uint32_t)block[j]<<24)|((uint32_t)block[j+1u]<<16)|((uint32_t)block[j+2u]<<8)|block[j+3u];
    }
    for(i=16u;i<64u;++i){
        s0=rotr32(sha_schedule[i-15u],7)^rotr32(sha_schedule[i-15u],18)^(sha_schedule[i-15u]>>3);
        s1=rotr32(sha_schedule[i-2u],17)^rotr32(sha_schedule[i-2u],19)^(sha_schedule[i-2u]>>10);
        sha_schedule[i]=sha_schedule[i-16u]+s0+sha_schedule[i-7u]+s1;
    }
    a=c->h[0];b=c->h[1];cc=c->h[2];d=c->h[3];e=c->h[4];f=c->h[5];g=c->h[6];h=c->h[7];
    for(i=0u;i<64u;++i){
        s1=rotr32(e,6)^rotr32(e,11)^rotr32(e,25);
        ch=(e&f)^((~e)&g);
        t1=h+s1+ch+sha_k[i]+sha_schedule[i];
        s0=rotr32(a,2)^rotr32(a,13)^rotr32(a,22);
        maj=(a&b)^(a&cc)^(b&cc);
        t2=s0+maj;
        h=g;g=f;f=e;e=d+t1;d=cc;cc=b;b=a;a=t1+t2;
    }
    c->h[0]+=a;c->h[1]+=b;c->h[2]+=cc;c->h[3]+=d;
    c->h[4]+=e;c->h[5]+=f;c->h[6]+=g;c->h[7]+=h;
}
static void sha256_init(sha256_ctx_t *c){
    c->h[0]=0x6a09e667u;c->h[1]=0xbb67ae85u;c->h[2]=0x3c6ef372u;c->h[3]=0xa54ff53au;
    c->h[4]=0x510e527fu;c->h[5]=0x9b05688cu;c->h[6]=0x1f83d9abu;c->h[7]=0x5be0cd19u;
    c->total_bytes=0u;c->buffer_len=0u;
}
static void sha256_update(sha256_ctx_t *c,const uint8_t *data,uint32_t n){
    uint32_t i;
    for(i=0u;i<n;++i){
        c->buffer[c->buffer_len++]=data[i];
        ++c->total_bytes;
        if(c->buffer_len==64u){sha256_transform(c,c->buffer);c->buffer_len=0u;}
    }
}
static void sha256_final(sha256_ctx_t *c,uint8_t out[32]){
    uint32_t i,bitlen=c->total_bytes*8u;
    c->buffer[c->buffer_len++]=0x80u;
    if(c->buffer_len>56u){while(c->buffer_len<64u)c->buffer[c->buffer_len++]=0u;sha256_transform(c,c->buffer);c->buffer_len=0u;}
    while(c->buffer_len<56u)c->buffer[c->buffer_len++]=0u;
    for(i=0u;i<4u;++i)c->buffer[56u+i]=0u;
    c->buffer[60]=(uint8_t)(bitlen>>24);c->buffer[61]=(uint8_t)(bitlen>>16);c->buffer[62]=(uint8_t)(bitlen>>8);c->buffer[63]=(uint8_t)bitlen;
    sha256_transform(c,c->buffer);
    for(i=0u;i<8u;++i){out[i*4u]=(uint8_t)(c->h[i]>>24);out[i*4u+1u]=(uint8_t)(c->h[i]>>16);out[i*4u+2u]=(uint8_t)(c->h[i]>>8);out[i*4u+3u]=(uint8_t)c->h[i];}
}
static void sha256_memory(uint32_t address,uint32_t n,uint8_t out[32]){
    sha256_ctx_t c;sha256_init(&c);sha256_update(&c,(const uint8_t *)(uintptr_t)address,n);sha256_final(&c,out);
}
static void hmac_header(const uint8_t *header,uint8_t out[32]){
    uint8_t block[64];uint8_t inner[32];sha256_ctx_t c;uint32_t i;
    bytes_fill(block,0x36u,64u);for(i=0u;i<32u;++i)block[i]^=deus_update_key[i];
    sha256_init(&c);sha256_update(&c,block,64u);sha256_update(&c,hmac_domain,8u);sha256_update(&c,origin_le,4u);sha256_update(&c,header,48u);sha256_final(&c,inner);
    bytes_fill(block,0x5Cu,64u);for(i=0u;i<32u;++i)block[i]^=deus_update_key[i];
    sha256_init(&c);sha256_update(&c,block,64u);sha256_update(&c,inner,32u);sha256_final(&c,out);
}

static uint16_t crc16_ccitt(const uint8_t *data,uint32_t n){
    uint16_t crc=0xFFFFu;uint32_t i;uint8_t bit;
    for(i=0u;i<n;++i){
        crc^=(uint16_t)((uint16_t)data[i]<<8);
        for(bit=0u;bit<8u;++bit){
            if((crc&0x8000u)!=0u)crc=(uint16_t)(((uint32_t)crc<<1)^0x1021u);
            else crc=(uint16_t)((uint32_t)crc<<1);
        }
    }
    return crc;
}

static uint16_t device_id(void){return (uint16_t)(DBGMCU_IDCODE & 0x0FFFu);}
static int header_structural_valid(const uint8_t *h){
    uint32_t len=load_le32(h+8),ver=load_le32(h+12);
    return h[4]==IMAGE_FORMAT_VERSION && h[5]==0u &&
           len>=8u && len<=APP_MAX_BYTES && (len&3u)==0u && ver!=0u;
}
static int header_target_valid(const uint8_t *h){
    return load_le32(h)==PRODUCT_ID && load_le16(h+6)==TARGET_DEVICE_ID &&
           device_id()==TARGET_DEVICE_ID;
}
static int vector_values_valid(uint32_t msp,uint32_t reset,uint32_t image_len){
    uint32_t handler=reset&~1u,image_end;
    if(image_len<8u || image_len>APP_MAX_BYTES)return 0;
    image_end=APP_BASE+image_len;
    if(image_end<APP_BASE || image_end>APP_END)return 0;
    if((msp&7u)!=0u || msp<RAM_BASE || msp>RAM_END)return 0;
    if((reset&1u)==0u || handler<APP_BASE || handler>=image_end)return 0;
    return 1;
}
static int vectors_valid(uint32_t image_len){
    return vector_values_valid(REG32(APP_BASE),REG32(APP_BASE+4u),image_len);
}
static int app_matches_header(const uint8_t *header){
    uint8_t digest[32];uint32_t len=load_le32(header+8);
    sha256_memory(APP_BASE,len,digest);
    return bytes_equal(digest,header+16,32u) && vectors_valid(len);
}
static int metadata_authenticated(uint32_t slot,uint32_t *version_out){
    uint8_t calc[32];const uint8_t *header=(const uint8_t *)(uintptr_t)slot;
    const uint8_t *tag=(const uint8_t *)(uintptr_t)(slot+48u);
    if(REG16(slot+META_MARKER_OFFSET)!=META_MARKER_COMMITTED)return 0;
    if(!header_structural_valid(header))return 0;
    hmac_header(header,calc);if(!bytes_equal(calc,tag,32u))return 0;
    if(!header_target_valid(header))return 0;
    *version_out=load_le32(header+12);return 1;
}
static void scan_metadata(void){
    uint32_t va=0u,vb=0u;int aa=metadata_authenticated(META_A,&va);int ab=metadata_authenticated(META_B,&vb);
    uint32_t best_boot=0u;committed_version=0u;version_floor=0u;highest_floor_slot=0u;
    if(aa){version_floor=va;highest_floor_slot=META_A;}
    if(ab && vb>=version_floor){version_floor=vb;highest_floor_slot=META_B;}
    if(aa && app_matches_header((const uint8_t *)(uintptr_t)META_A)){best_boot=va;committed_version=va;}
    if(ab && app_matches_header((const uint8_t *)(uintptr_t)META_B) && vb>=best_boot){best_boot=vb;committed_version=vb;}
}

static int flash_wait_ready(void){uint32_t spins=FLASH_READY_SPIN_LIMIT;while((FLASH_SR&FLASH_SR_BSY)!=0u){if(spins--==0u)return 0;}return 1;}
static int flash_unlock(void){
    if(!flash_wait_ready())return 0;
    if((FLASH_CR&FLASH_CR_LOCK)!=0u){FLASH_KEYR=FLASH_KEY1;FLASH_KEYR=FLASH_KEY2;if((FLASH_CR&FLASH_CR_LOCK)!=0u)return 0;}
    return 1;
}
static void flash_lock(void){FLASH_CR=(FLASH_CR&~(FLASH_CR_PG|FLASH_CR_PER|FLASH_CR_MER))|FLASH_CR_LOCK;}
static int flash_owned_address(uint32_t a){
    return ((a>=APP_BASE && a<APP_END)||(a>=META_A && a<(META_B+META_PAGE_BYTES))) && ((a&1u)==0u);
}
static int flash_erase_page(uint32_t page){
    uint32_t off,status;
    if((page&0x3FFu)!=0u)return 0;
    if(!((page>=APP_BASE && page<APP_END)||page==META_A||page==META_B))return 0;
    if((RCC_CR&(1u<<1))==0u || !flash_unlock())return 0;
    FLASH_SR=FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR;FLASH_CR&=~(FLASH_CR_PG|FLASH_CR_PER|FLASH_CR_MER);
    FLASH_CR|=FLASH_CR_PER;FLASH_AR=page;FLASH_CR|=FLASH_CR_STRT;
    if(!flash_wait_ready()){flash_lock();return 0;}
    status=FLASH_SR;flash_lock();FLASH_SR=FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR;
    if((status&(FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR))!=FLASH_SR_EOP)return 0;
    for(off=0u;off<1024u;off+=2u)if(REG16(page+off)!=0xFFFFu)return 0;
    return 1;
}
static int flash_program_halfword(uint32_t a,uint16_t v){
    uint32_t status;
    if(!flash_owned_address(a))return 0;
    if(v==0xFFFFu)return REG16(a)==0xFFFFu;
    if(REG16(a)!=0xFFFFu)return 0;
    if((RCC_CR&(1u<<1))==0u || !flash_unlock())return 0;
    FLASH_SR=FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR;FLASH_CR&=~(FLASH_CR_PG|FLASH_CR_PER|FLASH_CR_MER);FLASH_CR|=FLASH_CR_PG;
    REG16(a)=v;
    if(!flash_wait_ready()){flash_lock();return 0;}
    status=FLASH_SR;flash_lock();FLASH_SR=FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR;
    if((status&(FLASH_SR_EOP|FLASH_SR_PGERR|FLASH_SR_WRPRTERR))!=FLASH_SR_EOP)return 0;
    return REG16(a)==v;
}
static int flash_program_even_bytes(uint32_t a,const uint8_t *p,uint32_t n){
    uint32_t i;if((n&1u)!=0u)return 0;for(i=0u;i<n;i+=2u)if(!flash_program_halfword(a+i,(uint16_t)(p[i]|((uint16_t)p[i+1u]<<8))))return 0;return 1;
}

static int consume_update_token(void){
    uint16_t a,b;RCC_APB1ENR|=RCC_APB1_PWREN|RCC_APB1_BKPEN;PWR_CR|=PWR_CR_DBP;
    a=BKP_DR1;b=BKP_DR2;BKP_DR1=0u;BKP_DR2=0u;PWR_CR&=~PWR_CR_DBP;
    return a==UPDATE_TOKEN1 && b==UPDATE_TOKEN2;
}
__attribute__((noreturn)) static void system_reset(void){
    __asm volatile("cpsid i");
    SCB_AIRCR=(SCB_AIRCR&SCB_AIRCR_PRIGROUP)|SCB_AIRCR_VECTKEY|SCB_AIRCR_SYSRESETREQ;
    for(;;){}
}
__attribute__((noreturn)) static void handoff_app(void){
    uint32_t msp=REG32(APP_BASE),reset=REG32(APP_BASE+4u);
    /* Normal boot enters here directly from reset before bootloader IRQ setup.
     * Preserve reset-like PRIMASK=0 semantics for the application; masking
     * interrupts here would leak PRIMASK=1 across the MSP/VTOR handoff and
     * prevent the application's USB/SysTick/UART IRQs from ever running. */
    SCB_VTOR=APP_BASE;
    __asm volatile("dsb");
    __asm volatile("isb");
    __asm volatile("msr msp,%0\nbx %1"::"r"(msp),"r"(reset):"memory");
    __builtin_unreachable();
}
static uint32_t clock_init(void){
    uint32_t spins;
    FLASH_ACR=FLASH_PRFTBE|FLASH_LATENCY_2;
    RCC_CR|=RCC_HSEON;spins=CLOCK_READY_SPIN_LIMIT;
    while((RCC_CR&RCC_HSERDY)==0u){if(spins--==0u)return 0u;}
    RCC_CFGR=RCC_PPRE1_DIV2|RCC_ADCPRE_DIV6|RCC_PLLSRC_HSE|RCC_PLLMUL_X9;
    RCC_CR|=RCC_PLLON;spins=CLOCK_READY_SPIN_LIMIT;
    while((RCC_CR&RCC_PLLRDY)==0u){if(spins--==0u)return 0u;}
    RCC_CFGR=(RCC_CFGR&~RCC_SW_MASK)|RCC_SW_PLL;spins=CLOCK_READY_SPIN_LIMIT;
    while((RCC_CFGR&RCC_SWS_MASK)!=RCC_SWS_PLL){if(spins--==0u)return 0u;}
    RCC_CFGR&=~RCC_USBPRE;return 72000000u;
}

static void usb_pma_write16(uint16_t off,uint16_t v){REG16(USB_PMA_BASE+((uint32_t)off*2u))=v;}
static uint16_t usb_pma_read16(uint16_t off){return REG16(USB_PMA_BASE+((uint32_t)off*2u));}
static void usb_pma_write(uint16_t off,const uint8_t *src,uint16_t n){uint16_t i;for(i=0u;i<n;i=(uint16_t)(i+2u)){uint16_t w=src[i];if((uint16_t)(i+1u)<n)w|=(uint16_t)((uint16_t)src[i+1u]<<8);usb_pma_write16((uint16_t)(off+i),w);}}
static void usb_pma_read(uint16_t off,uint8_t *dst,uint16_t n){uint16_t i;for(i=0u;i<n;i=(uint16_t)(i+2u)){uint16_t w=usb_pma_read16((uint16_t)(off+i));dst[i]=(uint8_t)w;if((uint16_t)(i+1u)<n)dst[i+1u]=(uint8_t)(w>>8);}}
static uint16_t ep_invariant(uint16_t v){return (uint16_t)(v&(USB_EP_CTR_RX|USB_EP_TYPE_MASK|USB_EP_KIND|USB_EP_CTR_TX|USB_EP_EA_MASK));}
static void ep_set_tx(uint8_t ep,uint16_t st){uint16_t c=USB_EP_REG(ep),w=ep_invariant(c);w|=(uint16_t)((c^st)&USB_EP_STAT_TX_MASK);USB_EP_REG(ep)=w;}
static void ep_set_rx(uint8_t ep,uint16_t st){uint16_t c=USB_EP_REG(ep),w=ep_invariant(c);w|=(uint16_t)((c^st)&USB_EP_STAT_RX_MASK);USB_EP_REG(ep)=w;}
static void ep_clear_ctr_rx(uint8_t ep){uint16_t c=USB_EP_REG(ep),w=ep_invariant(c);w&=(uint16_t)~USB_EP_CTR_RX;USB_EP_REG(ep)=w;}
static void ep_clear_ctr_tx(uint8_t ep){uint16_t c=USB_EP_REG(ep),w=ep_invariant(c);w&=(uint16_t)~USB_EP_CTR_TX;USB_EP_REG(ep)=w;}
static void ep_clear_dtog_rx(uint8_t ep){uint16_t c=USB_EP_REG(ep);if(c&USB_EP_DTOG_RX)USB_EP_REG(ep)=(uint16_t)(ep_invariant(c)|USB_EP_DTOG_RX);}
static void ep_clear_dtog_tx(uint8_t ep){uint16_t c=USB_EP_REG(ep);if(c&USB_EP_DTOG_TX)USB_EP_REG(ep)=(uint16_t)(ep_invariant(c)|USB_EP_DTOG_TX);}
static void ep_reset(uint8_t ep,uint16_t type){ep_set_tx(ep,USB_EP_STAT_TX_DISABLED);ep_set_rx(ep,USB_EP_STAT_RX_DISABLED);ep_clear_ctr_rx(ep);ep_clear_ctr_tx(ep);ep_clear_dtog_rx(ep);ep_clear_dtog_tx(ep);USB_EP_REG(ep)=(uint16_t)(type|ep);}
static void istr_clear(uint16_t f){USB_ISTR=(uint16_t)~f;}
static uint16_t ep_rx_count(uint8_t ep){return (uint16_t)(usb_pma_read16(USB_BTABLE_EP_RX_COUNT(ep))&USB_COUNT_MASK);}

#define EP0_IDLE 0u
#define EP0_DATA_IN 1u
#define EP0_STATUS_OUT 2u
#define EP0_STATUS_IN 3u
static const uint8_t *ep0_ptr;
static uint16_t ep0_remaining;
static uint8_t ep0_state;
static uint8_t ep0_pending_address;
static uint8_t ep0_address_valid;
static uint8_t ep0_pending_configuration;
static uint8_t ep0_configuration_valid;
static uint8_t ep0_one_byte;
static uint8_t ep0_two_bytes[2];

static void ep0_arm_rx(void){ep_set_rx(USB_EP0,USB_EP_STAT_RX_VALID);}
static void ep0_send_packet(const uint8_t *d,uint16_t n){if(n)usb_pma_write(USB_EP0_TX_LOCAL,d,n);usb_pma_write16(USB_BTABLE_EP_TX_COUNT(USB_EP0),n);ep_set_tx(USB_EP0,USB_EP_STAT_TX_VALID);}
static void ep0_send_next(void){uint16_t n=ep0_remaining>64u?64u:ep0_remaining;ep0_send_packet(ep0_ptr,n);ep0_ptr+=n;ep0_remaining=(uint16_t)(ep0_remaining-n);}
static void ep0_start_in(const uint8_t *d,uint16_t n,uint16_t requested){if(n>requested)n=requested;ep0_ptr=d;ep0_remaining=n;ep0_state=EP0_DATA_IN;ep0_send_next();}
static void ep0_status_in(void){ep0_state=EP0_STATUS_IN;ep0_send_packet((const uint8_t*)0,0u);}
static void ep0_stall(void){ep0_state=EP0_IDLE;ep_set_tx(USB_EP0,USB_EP_STAT_TX_STALL);ep_set_rx(USB_EP0,USB_EP_STAT_RX_STALL);}

static void bulk_disable(void){ep_reset(USB_EP1,USB_EP_TYPE_BULK);usb_tx_active=0u;}
static void bulk_enable(void){
    ep_reset(USB_EP1,USB_EP_TYPE_BULK);
    usb_pma_write16(USB_BTABLE_EP_TX_ADDR(USB_EP1),USB_EP1_TX_LOCAL);
    usb_pma_write16(USB_BTABLE_EP_TX_COUNT(USB_EP1),0u);
    usb_pma_write16(USB_BTABLE_EP_RX_ADDR(USB_EP1),USB_EP1_RX_LOCAL);
    usb_pma_write16(USB_BTABLE_EP_RX_COUNT(USB_EP1),USB_RX_COUNT_64);
    ep_set_tx(USB_EP1,USB_EP_STAT_TX_NAK);ep_set_rx(USB_EP1,USB_EP_STAT_RX_VALID);
}
static void apply_configuration(uint8_t c){usb_configuration=c;if(c==1u)bulk_enable();else bulk_disable();}

static void handle_setup(void){
    uint8_t s[8];uint8_t bm,breq,desc_type,desc_index;uint16_t wv,wi,wl;const uint8_t *d=(const uint8_t*)0;uint16_t n=0u;
    usb_pma_read(USB_EP0_RX_LOCAL,s,8u);bm=s[0];breq=s[1];wv=load_le16(s+2);wi=load_le16(s+4);wl=load_le16(s+6);
    ep0_address_valid=0u;ep0_configuration_valid=0u;
    if(bm==0x80u && breq==6u){
        desc_type=(uint8_t)(wv>>8);desc_index=(uint8_t)wv;
        if(desc_type==USB_DESC_DEVICE){d=usb_device_descriptor;n=sizeof(usb_device_descriptor);}
        else if(desc_type==USB_DESC_CONFIGURATION){d=usb_configuration_descriptor;n=sizeof(usb_configuration_descriptor);}
        else if(desc_type==USB_DESC_BOS){d=usb_bos_descriptor;n=sizeof(usb_bos_descriptor);}
        else if(desc_type==USB_DESC_STRING && desc_index==0u){d=usb_string_language;n=sizeof(usb_string_language);}
        else if(desc_type==USB_DESC_STRING && desc_index==1u){d=usb_string_product;n=sizeof(usb_string_product);}
        else {ep0_stall();return;}
        ep0_start_in(d,n,wl);return;
    }
    if(bm==0xC0u && breq==USB_MS_OS_20_VENDOR_CODE && wi==USB_MS_OS_20_DESCRIPTOR_INDEX && wv==0u){
        ep0_start_in(usb_ms_os_20_descriptor_set,sizeof(usb_ms_os_20_descriptor_set),wl);return;
    }
    if(bm==0x00u && breq==5u && wi==0u && wl==0u && wv<=127u){ep0_pending_address=(uint8_t)wv;ep0_address_valid=1u;ep0_status_in();return;}
    if(bm==0x00u && breq==9u && wi==0u && wl==0u && (wv==0u||wv==1u)){ep0_pending_configuration=(uint8_t)wv;ep0_configuration_valid=1u;ep0_status_in();return;}
    if(bm==0x80u && breq==8u && wv==0u && wi==0u && wl>=1u){ep0_one_byte=usb_configuration;ep0_start_in(&ep0_one_byte,1u,wl);return;}
    if((bm==0x80u||bm==0x81u||bm==0x82u) && breq==0u && wv==0u && wl>=2u){ep0_two_bytes[0]=0u;ep0_two_bytes[1]=0u;ep0_start_in(ep0_two_bytes,2u,wl);return;}
    if(bm==0x81u && breq==10u && wv==0u && wi==0u && wl>=1u){ep0_one_byte=0u;ep0_start_in(&ep0_one_byte,1u,wl);return;}
    if(bm==0x01u && breq==11u && wv==0u && wi==0u && wl==0u){ep0_status_in();return;}
    ep0_stall();
}
static void handle_ep0_rx(uint16_t reg){
    uint16_t n=ep_rx_count(USB_EP0);ep_clear_ctr_rx(USB_EP0);
    if(reg&USB_EP_SETUP){if(n==8u)handle_setup();else ep0_stall();return;}
    if(ep0_state==EP0_STATUS_OUT && n==0u){ep0_state=EP0_IDLE;ep0_arm_rx();return;}
    ep0_stall();
}
static void handle_ep0_tx(void){
    ep_clear_ctr_tx(USB_EP0);
    if(ep0_state==EP0_DATA_IN){
        if(ep0_remaining!=0u){ep0_send_next();return;}
        ep0_state=EP0_STATUS_OUT;ep0_arm_rx();return;
    }
    if(ep0_state==EP0_STATUS_IN){
        if(ep0_address_valid){USB_DADDR=(uint16_t)(USB_DADDR_EF|(ep0_pending_address&USB_DADDR_ADD_MASK));ep0_address_valid=0u;}
        if(ep0_configuration_valid){apply_configuration(ep0_pending_configuration);ep0_configuration_valid=0u;}
        ep0_state=EP0_IDLE;ep0_arm_rx();return;
    }
    ep0_arm_rx();
}
static void usb_bus_reset(void){
    USB_BTABLE=USB_BTABLE_LOCAL;usb_pma_write16(USB_BTABLE_EP_TX_ADDR(USB_EP0),USB_EP0_TX_LOCAL);
    usb_pma_write16(USB_BTABLE_EP_TX_COUNT(USB_EP0),0u);usb_pma_write16(USB_BTABLE_EP_RX_ADDR(USB_EP0),USB_EP0_RX_LOCAL);
    usb_pma_write16(USB_BTABLE_EP_RX_COUNT(USB_EP0),USB_RX_COUNT_64);ep_reset(USB_EP0,USB_EP_TYPE_CONTROL);bulk_disable();
    usb_configuration=0u;ep0_state=EP0_IDLE;USB_DADDR=USB_DADDR_EF;ep_set_tx(USB_EP0,USB_EP_STAT_TX_NAK);ep0_arm_rx();
}
static void force_disconnect(uint32_t hz){
    volatile uint32_t spin;uint32_t saved,iterations=hz/50u;RCC_APB1ENR&=~RCC_APB1_USBEN;RCC_APB2ENR|=RCC_APB2_IOPAEN;
    saved=GPIOA_CRH&GPIO_CRH_PA12_MASK;GPIOA_BRR=GPIO_PIN_12;GPIOA_CRH=(GPIOA_CRH&~GPIO_CRH_PA12_MASK)|GPIO_CRH_PA12_OUTPUT_OD_2MHZ;
    for(spin=0u;spin<iterations;++spin)__asm volatile("nop");
    GPIOA_CRH=(GPIOA_CRH&~GPIO_CRH_PA12_MASK)|saved;
}
static void usb_init(uint32_t hz){
    volatile uint32_t spin;force_disconnect(hz);RCC_APB1ENR|=RCC_APB1_USBEN;RCC_APB1RSTR|=RCC_APB1_USBRST;RCC_APB1RSTR&=~RCC_APB1_USBRST;
    USB_CNTR=(uint16_t)(USB_CNTR_FRES|USB_CNTR_PDWN);USB_CNTR=USB_CNTR_FRES;
    for(spin=0u;spin<((hz/1000000u)*4u);++spin)__asm volatile("nop");
    USB_CNTR=0u;USB_ISTR=0u;USB_BTABLE=USB_BTABLE_LOCAL;USB_DADDR=0u;usb_bus_reset();
}
static int bulk_send(const uint8_t *d,uint16_t n){
    if(usb_configuration!=1u||usb_tx_active||n>64u)return 0;
    usb_pma_write(USB_EP1_TX_LOCAL,d,n);
    usb_pma_write16(USB_BTABLE_EP_TX_COUNT(USB_EP1),n);
    usb_tx_active=1u;
    ep_set_tx(USB_EP1,USB_EP_STAT_TX_VALID);
    return 1;
}

static uint8_t handle_begin(const uint8_t *p,uint16_t n){
    if(n!=49u)return ST_BAD_LENGTH;
    if(update_state!=STATE_RECOVERY_IDLE&&update_state!=STATE_HEADER_STAGED)return ST_INVALID_STATE;
    bytes_copy(staged_header,p+1,48u);previous_valid=0u;expected_offset=0u;target_metadata_slot=0u;
    if(!header_structural_valid(staged_header)){update_state=STATE_RECOVERY_IDLE;return ST_BAD_HEADER;}
    update_state=STATE_HEADER_STAGED;return ST_OK;
}
static uint8_t handle_authorize(const uint8_t *p,uint16_t n){
    uint8_t calc[32];uint32_t cand;
    if(n!=33u)return ST_BAD_LENGTH;
    if(update_state!=STATE_HEADER_STAGED)return ST_INVALID_STATE;
    bytes_copy(staged_tag,p+1,32u);hmac_header(staged_header,calc);if(!bytes_equal(calc,staged_tag,32u))return ST_AUTH_FAILED;
    if(!header_target_valid(staged_header))return ST_TARGET_MISMATCH;
    scan_metadata();cand=load_le32(staged_header+12);
    if(committed_version!=0u){if(cand<=version_floor)return ST_VERSION_REJECTED;}else{if(cand<version_floor)return ST_VERSION_REJECTED;}
    target_metadata_slot=(highest_floor_slot==META_A)?META_B:META_A;
    if(!flash_erase_page(target_metadata_slot)){update_state=STATE_RECOVERY_IDLE;return ST_FLASH_FAILED;}
    sha256_init(&stream_sha);expected_offset=0u;previous_valid=0u;update_state=STATE_AUTHORIZED;return ST_OK;
}
static uint8_t handle_data(const uint8_t *p,uint16_t n){
    uint16_t off;uint8_t len;uint32_t image_len,addr;uint8_t i;
    if(n<5u||n>51u)return ST_BAD_LENGTH;
    if(update_state!=STATE_AUTHORIZED&&update_state!=STATE_RECEIVING)return ST_INVALID_STATE;
    off=load_le16(p+1);len=(uint8_t)(n-3u);image_len=load_le32(staged_header+8);
    if((off&1u)||(len&1u)||len<2u||len>48u||(uint32_t)off+len>image_len)return ST_BAD_LENGTH;
    if((((uint32_t)off&0x3FFu)+(uint32_t)len)>1024u)return ST_BAD_LENGTH;
    if(previous_valid&&off==previous_offset&&len==previous_length&&bytes_equal(p+3,previous_data,len))return ST_OK;
    if(off!=expected_offset)return ST_OUT_OF_SEQUENCE;
    addr=APP_BASE+(uint32_t)off;
    if(((uint32_t)off&0x3FFu)==0u){if(!flash_erase_page(addr)){update_state=STATE_RECOVERY_IDLE;return ST_FLASH_FAILED;}}
    if(!flash_program_even_bytes(addr,p+3,len)){update_state=STATE_RECOVERY_IDLE;return ST_FLASH_FAILED;}
    for(i=0u;i<len;++i)if(*(const volatile uint8_t *)(uintptr_t)(addr+i)!=p[3u+i]){update_state=STATE_RECOVERY_IDLE;return ST_FLASH_FAILED;}
    sha256_update(&stream_sha,p+3,len);bytes_copy(previous_data,p+3,len);previous_offset=off;previous_length=len;previous_valid=1u;
    expected_offset=(uint16_t)(expected_offset+len);update_state=STATE_RECEIVING;return ST_OK;
}
static uint8_t handle_end(uint16_t n){
    uint8_t streamed[32],readback[32];uint32_t image_len,cand;
    if(n!=1u)return ST_BAD_LENGTH;
    if(update_state!=STATE_AUTHORIZED&&update_state!=STATE_RECEIVING)return ST_INVALID_STATE;
    image_len=load_le32(staged_header+8);if(expected_offset!=image_len)return ST_OUT_OF_SEQUENCE;update_state=STATE_VERIFYING;
    sha256_final(&stream_sha,streamed);if(!bytes_equal(streamed,staged_header+16,32u)){update_state=STATE_RECOVERY_IDLE;return ST_DIGEST_FAILED;}
    sha256_memory(APP_BASE,image_len,readback);if(!bytes_equal(readback,staged_header+16,32u)){update_state=STATE_RECOVERY_IDLE;return ST_DIGEST_FAILED;}
    if(!vectors_valid(image_len)){update_state=STATE_RECOVERY_IDLE;return ST_VECTOR_INVALID;}
    if(target_metadata_slot!=META_A&&target_metadata_slot!=META_B){update_state=STATE_RECOVERY_IDLE;return ST_INTERNAL_ERROR;}
    if(!flash_program_even_bytes(target_metadata_slot,staged_header,48u)||!flash_program_even_bytes(target_metadata_slot+48u,staged_tag,32u)||!flash_program_halfword(target_metadata_slot+META_MARKER_OFFSET,META_MARKER_COMMITTED)){update_state=STATE_RECOVERY_IDLE;return ST_FLASH_FAILED;}
    cand=load_le32(staged_header+12);if(cand>version_floor)version_floor=cand;committed_version=cand;update_state=STATE_COMMITTED;reset_after_tx=1u;return ST_OK;
}

static void build_response(uint16_t request_id,uint8_t opcode,uint8_t status){
    uint16_t crc;uint16_t eo=(update_state==STATE_AUTHORIZED||update_state==STATE_RECEIVING)?expected_offset:0u;
    tx_packet[0]=0xA5u;tx_packet[1]=0x5Au;tx_packet[2]=1u;tx_packet[3]=FRAME_TYPE_RESPONSE;tx_packet[4]=0u;tx_packet[5]=0u;
    store_le16(tx_packet+6,request_id);store_le16(tx_packet+8,16u);
    tx_packet[10]=opcode;tx_packet[11]=status;tx_packet[12]=update_state;tx_packet[13]=0u;store_le16(tx_packet+14,eo);tx_packet[16]=0u;tx_packet[17]=0u;
    store_le32(tx_packet+18,version_floor);store_le32(tx_packet+22,committed_version);
    crc=crc16_ccitt(tx_packet+2,24u);store_le16(tx_packet+26,crc);
}
static int process_bulk_packet(uint16_t n){
    uint16_t payload,request_id,wire_crc;uint8_t flags,opcode,status=ST_INTERNAL_ERROR;
    if(n<12u||n>64u)return 0;
    if(rx_packet[0]!=0xA5u||rx_packet[1]!=0x5Au||rx_packet[2]!=1u||rx_packet[3]!=FRAME_TYPE_REQUEST||rx_packet[5]!=0u)return 0;
    request_id=load_le16(rx_packet+6);payload=load_le16(rx_packet+8);if(request_id==0u||payload>52u||n!=(uint16_t)(12u+payload)||payload==0u)return 0;
    wire_crc=load_le16(rx_packet+10u+payload);if(crc16_ccitt(rx_packet+2,8u+payload)!=wire_crc)return 0;
    flags=rx_packet[4];opcode=rx_packet[10];
    if(opcode==OP_INFO){if(flags!=0u||payload!=1u)status=(flags!=0u)?ST_BAD_FLAGS:ST_BAD_LENGTH;else{scan_metadata();status=ST_OK;}}
    else{
        if(flags!=FLAG_DESTRUCTIVE)status=ST_BAD_FLAGS;
        else if(opcode==OP_BEGIN)status=handle_begin(rx_packet+10,payload);
        else if(opcode==OP_AUTHORIZE)status=handle_authorize(rx_packet+10,payload);
        else if(opcode==OP_DATA)status=handle_data(rx_packet+10,payload);
        else if(opcode==OP_END)status=handle_end(payload);
        else status=ST_INVALID_STATE;
    }
    build_response(request_id,opcode,status);if(!bulk_send(tx_packet,28u))ep_set_rx(USB_EP1,USB_EP_STAT_RX_VALID);return 1;
}
static void handle_ep1_rx(void){
    uint16_t n=ep_rx_count(USB_EP1);ep_clear_ctr_rx(USB_EP1);ep_set_rx(USB_EP1,USB_EP_STAT_RX_NAK);
    if(n<=64u){usb_pma_read(USB_EP1_RX_LOCAL,rx_packet,n);if(!process_bulk_packet(n))ep_set_rx(USB_EP1,USB_EP_STAT_RX_VALID);}else ep_set_rx(USB_EP1,USB_EP_STAT_RX_VALID);
}
static void handle_ep1_tx(void){
    ep_clear_ctr_tx(USB_EP1);usb_tx_active=0u;if(reset_after_tx)system_reset();ep_set_tx(USB_EP1,USB_EP_STAT_TX_NAK);ep_set_rx(USB_EP1,USB_EP_STAT_RX_VALID);
}
static void usb_poll(void){
    uint16_t istr=USB_ISTR;
    if(istr&USB_ISTR_RESET){istr_clear(USB_ISTR_RESET);usb_bus_reset();}
    if(istr&USB_ISTR_ERR)istr_clear(USB_ISTR_ERR);
    if(istr&USB_ISTR_PMAOVR)istr_clear(USB_ISTR_PMAOVR);
    while(USB_ISTR&USB_ISTR_CTR){
        uint8_t ep=(uint8_t)(USB_ISTR&USB_ISTR_EP_ID_MASK);uint16_t reg=USB_EP_REG(ep);
        if(reg&USB_EP_CTR_RX){if(ep==USB_EP0)handle_ep0_rx(reg);else if(ep==USB_EP1)handle_ep1_rx();else ep_clear_ctr_rx(ep);}
        reg=USB_EP_REG(ep);
        if(reg&USB_EP_CTR_TX){if(ep==USB_EP0)handle_ep0_tx();else if(ep==USB_EP1)handle_ep1_tx();else ep_clear_ctr_tx(ep);}
    }
}

void boot_main(void){
    uint32_t clock_hz;int explicit_update;
    explicit_update=consume_update_token();scan_metadata();
    if(!explicit_update && committed_version!=0u)handoff_app();
    update_state=STATE_RECOVERY_IDLE;expected_offset=0u;previous_valid=0u;reset_after_tx=0u;
    clock_hz=clock_init();
    if(clock_hz==0u){for(;;){__asm volatile("wfi");}}
    usb_init(clock_hz);
    for(;;)usb_poll();
}
