/* SDK integration for optional wifi_lab only. Not part of signed images.
 * SDK calls are made synchronously only in the ESP application task,
 * never by event callback or ISR. Scan itself is nonblocking.
 */
#include "wifi_node_sdk_bridge_v0.h"
#include "wifi_scan_v0.h"
#include <string.h>

static int do_start(void *ctx, uint32_t now_ms)
{
    deus_wifi_scan_v0_t snapshot;
    (void)ctx;
    deus_wifi_scan_v0_snapshot(&snapshot);
    if (snapshot.state==DEUS_WIFI_OFF &&
        !deus_wifi_scan_v0_radio_start())
        return 0;
    /* Failed SDK radio setup remains ERROR and cannot be auto retried. */
    return deus_wifi_scan_v0_start(now_ms);
}
static void do_poll(void *ctx, uint32_t now_ms)
{
    (void)ctx;
    deus_wifi_scan_v0_poll(now_ms);
}
static void do_snapshot(void *ctx, deus_wifi_scan_v0_t *out)
{
    (void)ctx;
    if (out) deus_wifi_scan_v0_snapshot(out);
}
void deus_wifi_node_sdk_bridge_v0_init(void)
{
    /* No Wi-Fi SDK API runs here; radio starts only at first explicit
     * application-approved SCAN_START (once per power-on init).
     */
    deus_wifi_scan_v0_init();
}
deus_wifi_node_ops_v0_t deus_wifi_node_sdk_bridge_v0_ops(void)
{
    deus_wifi_node_ops_v0_t ops;
    memset(&ops,0,sizeof(ops));
    ops.start_scan=do_start;
    ops.poll=do_poll;
    ops.snapshot=do_snapshot;
    return ops;
}
