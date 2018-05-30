
#include "common.h"


void wait_for_wifi_connection()
{
    uint8_t status = sdk_wifi_station_get_connect_status();
    while (status != STATION_GOT_IP)
    {
        delay(1000);
        status = sdk_wifi_station_get_connect_status();
        switch (status)
        {
        case STATION_WRONG_PASSWORD:
            printf("WiFi: wrong password\n\r");
            break;
        case STATION_NO_AP_FOUND:
            printf("WiFi: AP not found\n\r");
            break;
        case STATION_CONNECT_FAIL:
            printf("WiFi: connection failed\r\n");
            break;
        case STATION_GOT_IP:
            break;
        default:
            printf("%s: status = %d\n\r", __func__, status);
            break;
        }
    }
    delay(1000);
}


