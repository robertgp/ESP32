/* brushed dc motor control example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"

//motor one (right)
#define GPIO_PWM0A_OUT 22   //Set GPIO 15 as PWM0A 22
#define GPIO_PWM0B_OUT 23   //Set GPIO 16 as PWM0B 23

//motor two (left)
#define GPIO_PWM0A_OUT2 14   //Set GPIO 15 as PWM0A2 //15 & 26 at cons 1
#define GPIO_PWM0B_OUT2 32   //Set GPIO 16 as PWM0B2

//ir
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   5          //Multisampling

//PID
#define dt 0.1     //100ms - 0.01
#define Kp 0.1 //0.1
#define Kd 0 //0.01
#define Ki 0 //0.005
#define SETPOINT 40 //in mm

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;

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


static void check_efuse()
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

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm gpio...\n");
    //right
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);

    //left
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, GPIO_PWM0A_OUT2);
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0B, GPIO_PWM0B_OUT2);
    
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
{
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);
    
    
    while (1) {
        // Get lidar measurements
        static const char *RX_TASK_TAG = "RX_TASK";
        esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
        uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
        int lidar_dis = 0;
        //while (1) {
            const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
            if (rxBytes > 0) {
                data[rxBytes] = 0;
                int i;
                for (i = 0; i < (RX_BUF_SIZE+1); i++){
                    if (data[i] == 89 && data[i-1] == 89){
                    lidar_dis = data[i+1] - data[i+2];
                    //printf("%dmm\n", lidar_dis); // Print distance
                    }
                }
            }
        //}
        free(data);
        // End of lidar measurements

        //ir rangefinder
        uint32_t adc_reading = 0;
        //Multisampling
        //for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
            adc_reading += raw;
        }
        //}
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        // PID calculations
        static float previous_error = 0;
        static float integral = 0;
        float error;
        float derivative;
        float output;

        //calculate error
        error = SETPOINT - lidar_dis;
        //printf("\n SETPOINT(%d) - VOLTAGE(%d) = ERROR (%f) \n", SETPOINT, voltage, error);
        integral = integral + error * dt;
        derivative = (error - previous_error) / dt;
        output = Kp * error + Ki * integral + Kd * derivative;
        previous_error = error;

        //printf("Lidar distance: %dmm\n", lidar_dis);
        //printf("output: %f , error: %f \n", output, error);
        vTaskDelay(dt);

        //control via ir rangefinder:
        if (voltage > 250){
            brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
            brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);
            vTaskDelay(3000 / portTICK_RATE_MS);
        }
        else {
                //pid control via side lidar:
                if (output > 1.25){ //left nudge -- too close
                    
                    printf("\n IF IF IF IF IF IF IF IF \n");
                    //brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 20.0);
                    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);

                    vTaskDelay(250/portTICK_RATE_MS);

                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 60.0);
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 50.0);
                }
                else if (output < -1){ //right nudge -- too far
                    
                    printf("\n ELSE IF ELSE IF ELSE IF ELSE IF \n");
                    //brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 20.0);
                    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);

                    vTaskDelay(500/portTICK_RATE_MS);

                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 50.0);
                }
                else {
                    printf("\n ELSE ELSE ELSE ELSE ELSE ELSE ELSE \n");
                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 50.0);
                    //vTaskDelay(1000 / portTICK_RATE_MS);
                    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
                    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 50.0);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                }
        }

    }
}

void app_main()
{
    init();
    check_efuse();

    //Configure ADC for IR
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

    printf("Testing brushed motor...\n");
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_examlpe_brushed_motor_control", 4096, NULL, 5, NULL);
}
