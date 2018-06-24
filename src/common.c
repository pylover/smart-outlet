
#include "common.h"

#ifndef VDD33
#define VDD33 33
#endif


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


void ensure_phy_info() {
    sdk_phy_info_t * info = malloc(sizeof(sdk_phy_info_t));;

    read_saved_phy_info(info);
    printf("VERSION: %d\n", info->version);
    printf("VDD33_CONST: %d\n", info->pa_vdd);
    if (info->pa_vdd != VDD33) {
        printf("vdd33_const is not set properly, trying to set it to %d\n", VDD33);
        info->pa_vdd = VDD33;
        write_saved_phy_info(info);
        printf("setting vdd33_const to %d done. So, restarting...\n", VDD33);
        sdk_system_restart();
    }
}

