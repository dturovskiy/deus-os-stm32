/* Exact source-linked firmware command service tests, host only.
 * No simulation of ARM flash bytes or hardware effects.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "kernel/command_service.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAILED line %u: %s\n",(unsigned)__LINE__,#x); return 1; } } while (0)
static char output[4096];
static uint32_t written;
static int output_byte(void *ctx,uint8_t byte)
{
    (void)ctx;
    if (written >= sizeof(output)-1u) return 0;
    output[written++] = (char)byte;
    output[written] = '\0';
    return 1;
}
static int output_fail(void *ctx,uint8_t b)
{
    (void)ctx;(void)b;
    return 0;
}
static const command_service_descriptor_t *legacy_find(uint16_t id)
{
    uint32_t i;
    if (id == 0u) return 0;
    for (i=0u;i<command_service_registry_count();++i) {
        const command_service_descriptor_t *d=command_service_registry_at(i);
        if (d != 0 && d->rpc_id == id) return d;
    }
    return 0;
}
static command_service_status_t echo_handler(
    const command_service_request_t *r,command_service_context_t *c,void *p)
{
    (void)p;
    if (r == 0 || r->descriptor == 0) return COMMAND_SERVICE_STATUS_INTERNAL_ERROR;
    return command_service_write(c,r->descriptor->name);
}
int main(void)
{
    const uint32_t n=command_service_registry_count();
    command_service_context_t c = {0};
    command_service_request_t req;
    uint32_t j;
    CHECK(n == (uint32_t)COMMAND_SERVICE_METHOD_COUNT);
    CHECK(n == 39u);
    CHECK(command_service_registry_at(n)==0);
    CHECK(command_service_find(0)==0);
    CHECK(command_service_find_rpc_id(0u)==0);
    for (j=0u;j<n;++j) {
        const command_service_descriptor_t *d=command_service_registry_at(j);
        CHECK(d != 0);
        CHECK(d->name != 0);
        CHECK(d->rpc_id == j+1u);
        CHECK((uint32_t)d->method_id == j);
        CHECK(command_service_find(d->name)==d);
        CHECK(command_service_find_rpc_id(d->rpc_id)==d);
        CHECK(legacy_find(d->rpc_id)==d);
        CHECK(d->min_args <= d->max_args);
        CHECK(d->max_args <= COMMAND_SERVICE_MAX_ARGS);
    }
    /* Exhaustive RPC identifier equivalence to previous linear lookup,
     * including invalid ID 0 and the entire out-of-range 16-bit space. */
    for (j=0u;j<=65535u;++j)
        CHECK(command_service_find_rpc_id((uint16_t)j)==legacy_find((uint16_t)j));
    CHECK(command_service_find("invalid_name_") == 0);
    CHECK(strcmp(command_service_class_name(COMMAND_SERVICE_CLASS_SAFE),"SAFE")==0);
    CHECK(strcmp(command_service_class_name(COMMAND_SERVICE_CLASS_DIAGNOSTIC),"DIAGNOSTIC")==0);
    CHECK(strcmp(command_service_class_name(COMMAND_SERVICE_CLASS_DESTRUCTIVE),"DESTRUCTIVE")==0);
    CHECK(strcmp(command_service_class_name((command_service_class_t)99),"UNKNOWN")==0);
    {
        char line[]=" \t nodecrc \tDEUS-V8-ESP \t";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_OK);
        CHECK(req.descriptor != 0);
        CHECK(req.descriptor->rpc_id == COMMAND_SERVICE_RPC_NODECRC);
        CHECK(req.argc == 1u);
        CHECK(strcmp(req.argv[0],"DEUS-V8-ESP")==0);
        CHECK(req.argv[1]==0);
    }
    {
        char line[]="nonexistent";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_NOT_FOUND);
    }
    {
        char line[]="nodecrc";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_BAD_ARGS);
    }
    {
        char line[]="nodecrc a b";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_BAD_ARGS);
    }
    {
        char line[]="   ";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_BAD_ARGS);
    }
    {
        char line[]="help ping";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_OK);
        c.write_byte=output_byte;
        c.write_context=0;
        written=0; output[0]=0;
        CHECK(command_service_execute_help(&req,&c)==COMMAND_SERVICE_STATUS_OK);
        CHECK(strcmp(output,"HELP_METHOD=ping CLASS=SAFE MIN_ARGS=0x00000000 MAX_ARGS=0x00000000\r\n")==0);
    }
    {
        char line[]="help";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_OK);
        written=0; output[0]=0;
        CHECK(command_service_execute_help(&req,&c)==COMMAND_SERVICE_STATUS_OK);
        CHECK(strstr(output,"HELP_COUNT=0x00000027\r\nHELP_METHODS=ping uptime")!=0);
        CHECK(strstr(output," nodecrc nodestat\r\n")!=0);
    }
    {
        char line[]="ping";
        CHECK(command_service_parse_line(line,&req)==COMMAND_SERVICE_STATUS_OK);
        written=0; output[0]=0;
        CHECK(command_service_execute(&req,&c,echo_handler,0)==COMMAND_SERVICE_STATUS_OK);
        CHECK(strcmp(output,"ping")==0);
    }
    {
        written=0; output[0]=0;
        CHECK(command_service_write_hex32(&c,0x1A2B3C4Du)==COMMAND_SERVICE_STATUS_OK);
        CHECK(strcmp(output,"0x1A2B3C4D")==0);
        CHECK(command_service_write_line(&c,"X")==COMMAND_SERVICE_STATUS_OK);
        CHECK(strcmp(output,"0x1A2B3C4DX\r\n")==0);
    }
    {
        c.write_byte=output_fail;
        c.write_failed=0u;
        CHECK(command_service_write(&c,"z")==COMMAND_SERVICE_STATUS_INTERNAL_ERROR);
        CHECK(c.write_failed!=0u);
    }
    puts("COMMAND_SERVICE_LOOKUP=PASS ALL_65536_IDS_39_METHODS_HELP_PARSE_OUTPUT_FAILCLOSED");
    return 0;
}
