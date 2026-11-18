/* Candidate STM32 command-service Wi-Fi RPC no-device regression.
 * Firmware normal9 current registry-39 remains historical/installed.
 * Candidate adds exactly one diagnostic method at 0x0028/registry40.
 */
#include "kernel/command_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)) { \
  fprintf(stderr,"NODE_WIFI_REGISTRY_FAIL line=%d: %s\n",__LINE__,#x); \
  exit(2); } } while(0)
int main(void)
{
    const command_service_descriptor_t *found;
    command_service_request_t req;
    char caps[]="nodewifi caps";
    char scan[]="nodewifi scan";
    char stat[]="nodewifi status";
    char result[]="nodewifi result";
    char page[]="nodewifi page 0x1234ABCD 3 32";
    char wrong[]="nodewifi scan unexpected";
    CHECK(command_service_registry_count()==41u);
    found=command_service_find_rpc_id(COMMAND_SERVICE_RPC_NODEWIFI);
    CHECK(found!=0);
    CHECK(strcmp(found->name,"nodewifi")==0);
    CHECK(found->rpc_id==0x28u && found->command_class==COMMAND_SERVICE_CLASS_DIAGNOSTIC);
    CHECK(found->min_args==1u&&found->max_args==4u);
    CHECK(found->method_id==COMMAND_SERVICE_METHOD_NODEWIFI);
    found=command_service_find_rpc_id(COMMAND_SERVICE_RPC_ESPAPPRESET);
    CHECK(found!=0 && found->rpc_id==0x29u);
    CHECK(strcmp(found->name,"espappreset")==0);
    CHECK(found->command_class==COMMAND_SERVICE_CLASS_DESTRUCTIVE);
    CHECK(found->min_args==0u && found->max_args==0u);
    CHECK(found->method_id==COMMAND_SERVICE_METHOD_ESPAPPRESET);
    CHECK(command_service_find_rpc_id(COMMAND_SERVICE_RPC_NODESTAT)!=0);
    CHECK(command_service_find_rpc_id(COMMAND_SERVICE_RPC_NODESTART)!=0);
    CHECK(command_service_find_rpc_id(COMMAND_SERVICE_RPC_NODECRC)!=0);
    CHECK(command_service_parse_line(caps,&req)==COMMAND_SERVICE_STATUS_OK);
    CHECK(req.argc==1u&&strcmp(req.argv[0],"caps")==0);
    CHECK(command_service_parse_line(scan,&req)==COMMAND_SERVICE_STATUS_OK);
    CHECK(command_service_parse_line(stat,&req)==COMMAND_SERVICE_STATUS_OK);
    CHECK(command_service_parse_line(result,&req)==COMMAND_SERVICE_STATUS_OK);
    CHECK(command_service_parse_line(page,&req)==COMMAND_SERVICE_STATUS_OK);
    CHECK(req.argc==4u);
    CHECK(strcmp(req.argv[1],"0x1234ABCD")==0);
    CHECK(command_service_parse_line(wrong,&req)==COMMAND_SERVICE_STATUS_OK);
    /* Semantic verb/arg validation belongs to the task0 handler. */
    puts("STM32_WIFI_COMMAND_REGISTRY_41_WITH_EXPLICIT_DESTRUCTIVE_APP_RESET=PASS SOURCE_ONLY");
    return 0;
}
