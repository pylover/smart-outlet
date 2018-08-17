
/* Very basic example that just demonstrates we can run at all!
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "esp/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "ssid_config.h"
#include "easyq_config.h"

#include "easyq.h"
#include "common.h"


#define STATUS_QUEUE  EASYQ_LOGIN":status"
#define OUTLET1_QUEUE EASYQ_LOGIN":heater"
#define OUTLET2_QUEUE EASYQ_LOGIN":powersupply"
#define OUTLET3_QUEUE EASYQ_LOGIN":signalgenerator"
#define OUTLET4_QUEUE EASYQ_LOGIN":oscilloscope"

#define OUTLET1 3
#define OUTLET2 12
#define OUTLET3 13
#define OUTLET4 14

#define CURRENT_MEASURE_INTERVAL 2000
#define HALL_OFFSET 307  // 1/1024 V
#define HALL_UNIT 0.050 // mV/A



EQSession * eq;


void outlet_command(int gpio, const char * name, const char * command) {
    printf("OUTLET: %s GPIO: %d COMMAND: %s\n", name, gpio, command);
    gpio_write(gpio, strcasecmp("on", command) != 0);
}


void on_outlet1_command(const char * command) {
    outlet_command(OUTLET1, OUTLET1_QUEUE, command);
}

void on_outlet2_command(const char * command) {
    outlet_command(OUTLET2, OUTLET2_QUEUE, command);
}

void on_outlet3_command(const char * command) {
    outlet_command(OUTLET3, OUTLET3_QUEUE, command);
}

void on_outlet4_command(const char * command) {
    outlet_command(OUTLET4, OUTLET4_QUEUE, command);
}


void on_status(const char * status) {
    printf("STATUS: %s\n", status);
}


float measure_current() {
    uint16_t hall = sdk_system_adc_read();
    printf("Hall Value: %05d\n", hall);
    float voltage = (hall - HALL_OFFSET) / 1024.0;
    return voltage / HALL_UNIT;
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

        while (eq->ready) {
            sprintf(buff, "%08d:%.3f", c, measure_current());
            err = easyq_push(eq, queue, buff, -1);
            if (err != ERR_OK) {
                printf("Write error: %d", err);
                break;
            }
            delay(CURRENT_MEASURE_INTERVAL);
            c++;
        }
        easyq_close(eq);
    }
}


void listener(void* args) {
    err_t err;
    Queue * outlet1_queue = Queue_new(OUTLET1_QUEUE);
    Queue * outlet2_queue = Queue_new(OUTLET2_QUEUE);
    Queue * outlet3_queue = Queue_new(OUTLET3_QUEUE);
    Queue * outlet4_queue = Queue_new(OUTLET4_QUEUE);
    Queue * status_queue = Queue_new(STATUS_QUEUE);
    
    outlet1_queue->callback = on_outlet1_command;
    outlet2_queue->callback = on_outlet2_command;
    outlet3_queue->callback = on_outlet3_command;
    outlet4_queue->callback = on_outlet4_command;

    status_queue->callback = on_status;
    Queue * queues[5] = {
        outlet1_queue,
        outlet2_queue,
        outlet3_queue,
        outlet4_queue,
        status_queue
    };

    while (1) {
        printf("Waiting 1s");
        delay(1000);
        if (eq == NULL || !eq->ready) {
            printf("EasyQ is not READY, waiting ...\n");
            continue;
        }

        err = easyq_loop(eq, queues, 5);
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

    // Configuring GPIOs
    gpio_enable(OUTLET1, GPIO_OUT_OPEN_DRAIN);
    gpio_enable(OUTLET2, GPIO_OUT_OPEN_DRAIN);
    gpio_enable(OUTLET3, GPIO_OUT_OPEN_DRAIN);
    gpio_enable(OUTLET4, GPIO_OUT_OPEN_DRAIN);

    gpio_write(OUTLET1, 1);
    gpio_write(OUTLET2, 1);
    gpio_write(OUTLET3, 1);
    gpio_write(OUTLET4, 1);

    // Wifi
    struct sdk_station_config config = {
        .ssid     = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    // Phy_info
    ensure_phy_info();

    xTaskCreate(sender, "sender", 512, NULL, 2, NULL);
    xTaskCreate(listener, "listener", 384, NULL, 2, NULL);
}

