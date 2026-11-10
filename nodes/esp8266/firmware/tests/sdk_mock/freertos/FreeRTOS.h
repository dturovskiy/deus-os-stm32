#ifndef MOCK_FREERTOS_H
#define MOCK_FREERTOS_H
#include <stdint.h>
typedef uint32_t TickType_t;
#define portTICK_PERIOD_MS 10u
#define pdMS_TO_TICKS(ms) ((TickType_t)((ms)/portTICK_PERIOD_MS))
#endif
