
#include "espressif/esp_common.h"
#include "FreeRTOS.h"
#include "task.h"


void wait_for_wifi_connection();

#define delay(s) vTaskDelay(s / portTICK_PERIOD_MS)


