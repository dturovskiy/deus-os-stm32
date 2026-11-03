/* Single SDK build bridge: reuse the one canonical portable source.
 * The relative include is anchored at OS/nodes/esp8266/firmware/main/.
 * No forked protocol source and no target HAL dependency in the codec.
 */
#include "../../../../shared/node_link/src/deus_node_link.c"
