/* 
Lidar, IR and Ultrasonic sensors working together
*/
// Required for IR
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

// Required for Lidar
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"

// Required for Ultrasonic
#include "ultrasonic.h"


#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

// Next three lines are for lidar
static const int RX_BUF_SIZE = 1024;
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

// Next two lines are for ultrasonic
ultrasonic_sensor_t ultra_sensor = {.trigger_pin = 14, .echo_pin = 32};
ultrasonic_sensor_t *ultra_s = &ultra_sensor;

/* IR Sensor Functions*/

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_6;
static const adc_unit_t unit = ADC_UNIT_1;

static void check_efuse() // IR Sensor
{
    //Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type) //IR Sensor
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

/* Lidar Sensor Functions */
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
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            int i;
            int count_measures = 0;
            for (i = 0; i < (RX_BUF_SIZE+1); i++){
                if (data[i] == 89 && data[i-1] == 89){
                    count_measures++;
                    /* // This is all the info that you can get from the lidar
                    printf("\n");
                    printf("The distance is: %dmm ~ ", data[i+1]-data[i+2]);
                    printf("The strength is: %d ~ ", data[i+3]-data[i+4]);
                    printf("Reserved: %d ~ ", data[i+5]);
                    printf("Signal quality is: %d", data[i+6]);
                    printf("\n");
                    */
                   // Print for chart plotting
                   if (count_measures == 30){
                   printf("L%d\n", ((data[i+1] - data[i+2])/100));
                   count_measures = 0;
                   }
                }
            }
        }
        vTaskDelay(100);
    }
    free(data);
}

void app_main()
{
    /******** Ultrasonic Sensor *********/
    ultrasonic_init(ultra_s);
    uint32_t dis;
    uint32_t *distance = &dis;

    /******** Lidar Sensor *********/

    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);


    /******** IR Sensor *********/
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1
    while (1) {
        /* Lidar Sensor */

        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        // Convert voltage to distance
        
        uint16_t con_table[16];
        con_table[0] = 0;
        con_table[1] = 2000;
        con_table[2] = 1800;
        con_table[3] = 1600;
        con_table[4] = 1400;
        con_table[5] = 1200;
        con_table[6] = 1050;
        con_table[7] = 900;
        con_table[8] = 800;
        con_table[9] = 750;
        con_table[10] = 675;
        con_table[11] = 600;
        con_table[12] = 550;
        con_table[13] = 500;
        con_table[14] = 450;
        con_table[15] = 400;

        
        uint8_t tolerance = 50;
        bool hit = false;
        uint32_t dif = 0;
        for (int i = 1; i < 16; i++){
            dif = (int)voltage - (int)con_table[i];
            if(dif < tolerance && dif > 0) {
                printf("IR%d\n", i); // times 10 to cm
                hit = true;
                break;
            }
            else hit = false;
            if(i == 15 && hit == false){
                tolerance += 25;
                i = 1;
            }
        }

        /* Ultrasonic Sensor */
        ultrasonic_measure_cm(ultra_s, 400, distance); // Ultrasonic measure
        printf("U%d\n",(dis/1000)); //Ultrasonic printing, /100 cause meters
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



