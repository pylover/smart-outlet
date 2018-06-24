
#include "espressif/esp_common.h"
#include "espressif/phy_info.h"
#include "FreeRTOS.h"
#include "task.h"


void wait_for_wifi_connection();
void ensure_phy_info();

#define delay(s) vTaskDelay(s / portTICK_PERIOD_MS)


