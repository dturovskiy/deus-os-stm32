/* SDK v3.4 service adapter for SOURCE-ONLY ESP Wi-Fi candidate.
 * Called only from ESP application task, no credentials/persistence.
 * Not linked into production ESP v2 (firmware/main/component.mk).
 */
#ifndef DEUS_WIFI_NODE_SDK_BRIDGE_V0_H
#define DEUS_WIFI_NODE_SDK_BRIDGE_V0_H
#include "wifi_node_dispatch_v0.h"

/* Initializes in-memory scan state; no SDK radio operations on init. */
void deus_wifi_node_sdk_bridge_v0_init(void);
/* Installs callbacks from existing SDK Wi-Fi scan v0. */
deus_wifi_node_ops_v0_t deus_wifi_node_sdk_bridge_v0_ops(void);
#endif
