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
static const char *TAG="APP";

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
    brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100.0);
    brushed_motor_forward(MCPWM_UNIT_1, MCPWM_TIMER_0, 100.0);

    return ESP_OK;
}

httpd_uri_t forward = {
    .uri       = "/forward",
    .method    = HTTP_GET,
    .handler   = forward_get_handler,
    .user_ctx  = NULL
};

// Fast forward
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
    brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 100.0);
    brushed_motor_backward(MCPWM_UNIT_1, MCPWM_TIMER_0, 100.0);

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
    // Send response
    const char* resp_str = (const char*) "whizzer.bu.edu/team-quests/primary/test0";
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
        httpd_register_uri_handler(server, &fast_forward);
        httpd_register_uri_handler(server, &stop);
        httpd_register_uri_handler(server, &back);
        httpd_register_uri_handler(server, &left);
        httpd_register_uri_handler(server, &right);
        httpd_register_uri_handler(server, &message);
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

    printf("%dcm\n",*distance);

    if (*distance < 20){
        return true;
    }
    else return false;
    
  }
}

//************************** MAIN **************************

void app_main()
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

    // Httpd Sever and WiFi
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi(&server);

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

