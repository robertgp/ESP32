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

static const int RX_BUF_SIZE = 1024;

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

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

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    //ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task()
{
    static const char *TX_TASK_TAG = "TX_TASK";
    //esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task()
{
    //auto int convert(int hexf);
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            //printf(rxBytes);
            data[rxBytes] = 0;
           // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            //changed to LOG_BUFFER_HEX for just hex values--> want the output of this macro somehow
            //ESP_LOG_BUFFER_HEX(RX_TASK_TAG, data, rxBytes);
            //printf("size of data: %d \n ", sizeof(data));
            int i;
            for (i = 0; i < (RX_BUF_SIZE+1); i++){
                //printf("%d ", data[i]);
                if (data[i] == 89 && data[i-1] == 89){
                    printf("\n");
                    printf("The distance is: %dmm ~ ", data[i+1]-data[i+2]);
                    printf("The strength is: %d ~ ", data[i+3]-data[i+4]);
                    printf("Reserved: %d ~ ", data[i+5]);
                    printf("Signal quality is: %d", data[i+6]);
                    printf("\n");
                }
                /*
                if (data[i] == 89 && data[i+1] == 89) {
                    printf("OJ ");
                    printf("%d   ~ ",data[i]);

                } 
                */
            }
            //int mine = ESP_LOG_BUFFER_HEX(RX_TASK_TAG, data, rxBytes);
           /* int convert(ESP_LOG_BUFFER_HEX(RX_TASK_TAG, data, rxBytes)) {
                printf("yes");
                printf(hexf);
                return 1;

            }
            */
            //printf("%d", mine);

            //modified
            //uart_write_bytes(UART_NUM_1, rxBytes, RX_BUF_SIZE);
        }
    }
    free(data);
}
/*
static void convertt()
{
    while(1){
        printf("booya");
    }
    
}

*/
void app_main()
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
//    xTaskCreate(convertt, "convert_tag", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
    
}