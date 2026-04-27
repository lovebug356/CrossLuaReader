#pragma once

#define portTICK_PERIOD_MS 1

#ifdef __cplusplus
extern "C" {
#endif

void vTaskDelay(uint32_t ticks);

#ifdef __cplusplus
}
#endif
