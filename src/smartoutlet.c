
/* Very basic example that just demonstrates we can run at all!
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ssid_config.h"
#include "easyq_config.h"

#include "easyq.h"
#include "common.h"


#define STATUS_QUEUE "status"
#define COMMAND_QUEUE "cmd"
#define OUTLET1 2

EQSession * eq;



void on_command(const char * command) {
    printf("COMMAND: %s\n", command);
}


void on_status(const char * status) {
    printf("STATUS: %s\n", status);
}


void sender(void* args) {
    err_t err;
    int c = 0;
    char buff[16];
    Queue * queue = Queue_new(STATUS_QUEUE);
    while (1) {
        delay(1000);
    	// Wait for wifi conection
        wait_for_wifi_connection();
        
        printf("Inititlizing easyq\n");
        err = easyq_init(&eq);
        if (err != ERR_OK) {
            printf("Cannot Inititalize the easyq\n");
            continue;
        }
        printf("Session ID: %s\n", eq->id);

        while (1) {
            sprintf(buff, "%08d", c);
            easyq_push(eq, queue, buff, -1);
            delay(5000);
            c++;
        }
    }
}


void listener(void* args) {
    err_t err;
    Queue * command_queue = Queue_new(COMMAND_QUEUE);
    Queue * status_queue = Queue_new(STATUS_QUEUE);
    
    command_queue->callback = on_command;
    status_queue->callback = on_status;
    Queue * queues[2] = {
        command_queue,
        status_queue
    };

    while (1) {
        delay(1000);
        if (eq == NULL || !eq->ready) {
            continue;
        }

        err = easyq_loop(eq, queues, 2);
        if (err != ERR_OK) {
            printf("Cannot start easyq loop\n");
            continue;
        }
    }
}


void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    // Wifi
    struct sdk_station_config config = {
        .ssid     = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(sender, "sender", 512, NULL, 2, NULL);
    xTaskCreate(listener, "listener", 384, NULL, 2, NULL);
}

