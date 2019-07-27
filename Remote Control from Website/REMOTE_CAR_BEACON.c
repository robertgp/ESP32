//quest 6 beacon

/* Infrared IR/UART for car beacons for escaping the course demo!
   December 2018 -- Emily Lam
   RMT Pulse          -- pin 26 -- A0
   UART Transmitter   -- pin 25 -- A1
   Receiver           -- pin 34 -- A2
   Hardware interrupt -- pin 4 - A5
   Red LED            -- pin 33
   Green LED          -- pin 32
   Blue LED           -- Pin 14
*/

/* 
Move the car with a remote control in a website
*/
// Server includes
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <http_server.h>
#include <string.h>
// MCPWM includes
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
// Ultrasonic for front collision avoidance
#include <stdlib.h>
#include "ultrasonic.h"


#define EXAMPLE_WIFI_SSID "Group_8"
#define EXAMPLE_WIFI_PASS "smart-systems"
//#define LEDPIN 13
static const char *TAG="APP";

//Beacon
#include <stdio.h>
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"


// RMT definitions
#define RMT_TX_CHANNEL    1     // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25    // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV       100   // RMT counter clock divider
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US   9500     // RMT receiver timeout value(us)

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 39 // A2
#define BUF_SIZE (1024)

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1

// LED Output pins definitions
//#define BLUEPIN   14
//#define GREENPIN  32
#define REDPIN    15

// Default ID
#define ID 1

// Variables for my ID, minVal and status plus string fragments
char start = 0x1B;
char myID = (char) ID;
char rxID;
int len_out = 15;

char* frag1 = "whizzer.bu";
char* frag2 = ".edu/team-";
char* frag3 = "quests/pri";
char* frag4 = "mary/test0";
char data_collected[1024];
//char *data_collected = "testing";
//char* fake_variable;
//strcat(fake_variable, frag1);
//const char* data_collected = (const char*) fake_variable;

//data_collected = (char*) "new_testing";
//data_collected = (char*) malloc(1024*sizeof(char)); 
//*data_collected = (char*)"testing";

char* fr1 = "f1________";
char* fr2 = "f2________";
char* fr3 = "f3________";
char* fr4 = "f4________";

bool check1 = false;
bool check2 = false;
bool check3 = false;
bool check4 = false;

// Mutex (for resources), and Queues (for button)
SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;

// Button interrupt handler -- add to queue
static void IRAM_ATTR gpio_isr_handler(void* arg){
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}


//****************************************Beacon*************************************************

// RMT tx init
static void rmt_tx_init() {
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    // Carrier Frequency of the IR receiver
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    // Never idle -> aka ontinuous TX of 38kHz pulses
    rmt_tx.tx_config.idle_level = 1;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

// Configure UART
static void uart_init() {
  // Basic configs
  uart_config_t uart_config = {
      .baud_rate = 1200, // Slow BAUD rate
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1,UART_INVERSE_RXD);

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs
static void led_init() {
    gpio_pad_select_gpio(REDPIN);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
}

// Send task -- sends payload | Start | myID | Start | myID
void send_task(){
  while(1) {

    char *data_out = (char *) malloc(len_out);
    xSemaphoreTake(mux, portMAX_DELAY);
    data_out[0] = start;
    data_out[1] = (char) myID;
    data_out[2] = '\0';
    if ((int) myID == 1) {
      strcat(data_out, frag1);
    }
    else if ((int) myID == 2) {
      strcat(data_out, frag2);
    }
    else if ((int) myID == 3) {
      strcat(data_out, frag3);
    }
    else if ((int) myID == 4) {
      strcat(data_out, frag4);
    }
    // Debug logging
    // printf("Sent -- START (HEX): 0x%02X, ID (HEX): 0x%02X",data_out[0],myID);
    // char fragment[11];
    // strncpy(fragment,data_out + 2,11);
    // printf(", Fragment (STRING) -- %s\n",fragment);
    uart_write_bytes(UART_NUM_1, data_out, len_out+1);
    xSemaphoreGive(mux);

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

// Receives task -- looks for Start byte then stores received values
void recv_task(){
  // Buffer for input data
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);
  while (1) {
    int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
    if (len_in >0) {
      for (int i=0; i < 24; i++) {
        if (data_in[i] == start) {

            rxID = data_in[i+1];
            char rxFragment[11];
            for (int j = 0; j < 11; j++){
                rxFragment[j] = (char) data_in[i+j+1];
            }
            if ((int) rxID == 1 && check1 == false) {
            //strcat(data_collected, rxFragment);
            fr1 = rxFragment;
            //console.log(fr1);
            //ESP_LOGI(TAG, fr1);
            printf(fr1);
            //*data_collected = *fr1;
            check1 = true;
            }
            else if ((int) rxID == 2 && check2 == false) {
            //strcat(data_collected, rxFragment);
            fr2 = rxFragment;
            //console.log(fr2);
            //ESP_LOGI(TAG, fr2);
            printf(fr2);
            //*data_collected = *fr2;
            check2 = true;
            }
            else if ((int) rxID == 3 && check3 == false) {
            //strcat(data_collected, rxFragment);
            fr3 = rxFragment;
            printf(fr3);
            //console.log(fr3);
            //ESP_LOGI(TAG, fr3);
            //*data_collected = *fr3;
            check3 = true;
            }
            else if ((int) rxID == 4 && check4 == false) {
            //strcat(data_collected, rxFragment);
            fr4 = rxFragment;
            printf(fr4);
            //ESP_LOGI(TAG, fr4);
            //console.log(fr4);
            //*data_collected = *fr4;
            check4 = true;
            }
            
            
        printf("Received from device ID 0x%02X fragment: %s\n", rxID, rxFragment);
        //snprintf(data_collected, sizeof(data_collected), "fob=%d&hub=0&code=%d ", rxID, rcode);
        printf(data_collected);
        gpio_set_level(REDPIN, 1);
        gpio_set_level(REDPIN, 0);
        break;
        }
      }
    }
    else{
      // printf("Nothing received.\n");
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
  free(data_in);
}


//************************** MOVE THE CAR **************************

//motor one (right)
#define GPIO_PWM0A_OUT 22   //Set GPIO 15 as PWM0A 22
#define GPIO_PWM0B_OUT 23   //Set GPIO 16 as PWM0B 23

//motor two (left)
#define GPIO_PWM0A_OUT2 14   //Set GPIO 15 as PWM0A2 //15 & 26 at cons 1
#define GPIO_PWM0B_OUT2 32   //Set GPIO 16 as PWM0B2

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

static void brushed_motor_turn_left(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle) // big turn left 
{
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 55.0);
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0); 
    vTaskDelay(2000/portTICK_RATE_MS);
}

static void brushed_motor_turn_right(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle) // big turn right 
{
    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 55.0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0); 
    vTaskDelay(2000/portTICK_RATE_MS);
}

// Move the car forward
esp_err_t forward_get_handler(httpd_req_t *req)
{
    // Move forward code goes here
    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100.0); //150
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 90.0);  //55

    return ESP_OK;
}

httpd_uri_t forward = {
    .uri       = "/forward",
    .method    = HTTP_GET,
    .handler   = forward_get_handler,
    .user_ctx  = NULL
};

esp_err_t fast_forward_get_handler(httpd_req_t *req)
{
    // Move forward code goes here
    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 200.0);
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 200.0);

    return ESP_OK;
}

httpd_uri_t fast_forward = {
    .uri       = "/fast_forward",
    .method    = HTTP_GET,
    .handler   = fast_forward_get_handler,
    .user_ctx  = NULL
};

//  Stop the car
esp_err_t stop_get_handler(httpd_req_t *req)
{
    // Stop code goes here
    brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
    brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);

    return ESP_OK;
}

httpd_uri_t stop = {
    .uri       = "/stop",
    .method    = HTTP_GET,
    .handler   = stop_get_handler,
    .user_ctx  = NULL
};

// Back the car
esp_err_t back_get_handler(httpd_req_t *req)
{
   // Backward move code goes here
    brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100.0); //150
    brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_0, 100.0);  //55

    return ESP_OK;
}

httpd_uri_t back = {
    .uri       = "/back",
    .method    = HTTP_GET,
    .handler   = back_get_handler,
    .user_ctx  = NULL
};

// Turn to the left
esp_err_t left_get_handler(httpd_req_t *req)
{
   // Backward move code goes here
    xTaskCreate(brushed_motor_turn_left, "brushed_motor_turn_left", 4096, NULL, 5, NULL);

    return ESP_OK;
}

httpd_uri_t left = {
    .uri       = "/left",
    .method    = HTTP_GET,
    .handler   = left_get_handler,
    .user_ctx  = NULL
};

// Turn to the right
esp_err_t right_get_handler(httpd_req_t *req)
{
   // Backward move code goes here
   xTaskCreate(brushed_motor_turn_right, "brushed_motor_turn_right", 4096, NULL, 5, NULL);

    return ESP_OK;
}

httpd_uri_t right = {
    .uri       = "/right",
    .method    = HTTP_GET,
    .handler   = right_get_handler,
    .user_ctx  = NULL
};

esp_err_t message_get_handler(httpd_req_t *req)
{
  
    if (check1 == true && check2 == true && check3 == true && check4 == true){
                strcat(data_collected, fr1);
                strcat(data_collected, fr2);
                strcat(data_collected, fr3);
                strcat(data_collected, fr4);
    }
    // Send response
    const char* resp_str = (const char*) data_collected;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

httpd_uri_t message = {
    .uri       = "/message",
    .method    = HTTP_GET,
    .handler   = message_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

//************************** SERVER **************************

// Code for the httpd server
httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &forward);
        httpd_register_uri_handler(server, &stop);
        httpd_register_uri_handler(server, &back);
        httpd_register_uri_handler(server, &left);
        httpd_register_uri_handler(server, &right);
        httpd_register_uri_handler(server, &message);
        httpd_register_uri_handler(server, &fast_forward);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    httpd_handle_t *server = (httpd_handle_t *) ctx;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "Got IP: '%s'",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        /* Start the web server */
        if (*server == NULL) {
            *server = start_webserver();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());

        /* Stop the web server */
        if (*server) {
            stop_webserver(*server);
            *server = NULL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

// wifi init code
static void initialise_wifi(void *arg)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, arg));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

//************************** ULTRASONIC **************************

uint32_t dis;
uint32_t *distance = &dis;
uint32_t max = 400;
ultrasonic_sensor_t ultra_sensor = {.trigger_pin = 17, .echo_pin = 16};
ultrasonic_sensor_t *ultra_s = &ultra_sensor;

static bool ultra_task(){
  while (1){
    ultrasonic_measure_cm(ultra_s, max, distance); // Ultrasonic measure

    //printf("%dcm\n",*distance);

    if (*distance < 20){
        return true;
    }
    else return false;
    
  }
}


void app_main() {
  
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

    // Httpd Sever and WiFi
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi(&server);


//*************BEACON***************************
    // Mutex for current values when sending and during election
    mux = xSemaphoreCreateMutex();

    // Initialize transmitt and button interrupt
    rmt_tx_init();
    led_init();
    uart_init();
    
    // Create tasks for receive, send, elect, set gpio, and button
    xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(send_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

//****************************ULTRASONIC*****************

    ultrasonic_init(ultra_s);
    bool stops = false;
    while(1){
        stops = ultra_task();
        if (stops == true) {
            //printf("%d true\n",1);
            brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
            brushed_motor_stop(MCPWM_UNIT_1, MCPWM_TIMER_0);
        }
        else {
            //printf("%d false\n",0);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}
