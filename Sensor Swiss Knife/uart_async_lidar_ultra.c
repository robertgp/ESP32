/* UART asynchronous example, that uses separate RX and TX tasks
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "ultrasonic.h"

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_17) //17
#define RXD_PIN (GPIO_NUM_16) //16

void init() {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

/* Ultrasonic definitions */
ultrasonic_sensor_t ultra_sensor = {.trigger_pin = 17, .echo_pin = 16};
ultrasonic_sensor_t *ultra_s = &ultra_sensor;


void app_main()
{
    // Lidar
    init();
    printf("Got past init");
    //xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
//    xTaskCreate(convertt, "convert_tag", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    
    /* Ultrasonic Sensor */
    ultrasonic_init(ultra_s);
    uint32_t dis;
    uint32_t *distance = &dis;
    

    while(1){
    printf("Inside the while loop");
    ultrasonic_measure_cm(ultra_s, 400, distance); // Ultrasonic measure
    printf("%d\n", dis);//Ultrasonic printing
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}