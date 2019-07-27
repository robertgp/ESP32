/* i2c - Example MODIFIED for the ESP32/ADAFRUIT Alpha display
 Our edits look like this ***OUR EDITS***
   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//***************************Clock**************************************************

#include <stdio.h>
#include "driver/i2c.h"

/* ***NEED TO CHANGE PINS TO OURS -- WHAT PINS?*** */
// *** Change pins to match the Huzzah SDA and SCL ***
#define I2C_EXAMPLE_MASTER_SCL_IO          22               /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO          23               /*!< gpio number for I2C master data  */

// *** There are two i2c ports I2C_NUM_0 or I2C_NUM_1  -- don't need to change ***
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */

// *** This is not needed for Master write, disable -- don't need to change  ***
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */

// *** Communication frequency -- don't need to change
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000           /*!< I2C master clock frequency */


// *** These need to be changed to the address of the matirx driver (in datasheet for HT16K33), Rename defines ***
#define BH1750_SENSOR_ADDR                 0x70             /*!< slave address for BH1750 sensor */
// *** This applies to a different device, our device will have different CMDs
// #define BH1750_CMD_START                   0x23             /*!< Command to set measure mode */

// *** Not needed, we're setup as a master, not slave ***
// #define ESP_SLAVE_ADDR                     0x28             /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */


static esp_err_t i2c_example_master_write_slave(i2c_port_t i2c_num, uint8_t* data_wr, size_t size)
{
    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( BH1750_SENSOR_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
  }
  
  // Function to send one byte as a command
  static esp_err_t i2c_master_one_byte(i2c_port_t i2c_num, uint8_t data_wr, size_t size){
    // *** This creates a structure (class) called cmd
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // *** adds the i2c start bit into cmd
    i2c_master_start(cmd);
    // *** This adds the alpha i2c driver address to cmd
    i2c_master_write_byte(cmd, ( BH1750_SENSOR_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    // *** Add the commmand payload you want to send to device
    i2c_master_write_byte(cmd, data_wr, ACK_CHECK_EN);
    // *** adds the i2c stop bit to cmd
    i2c_master_stop(cmd);
    // *** This command is what puts the cmd payload onto the i2c bus
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
  }

/**
 * @brief i2c master initialization
 */
static void i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}

// *** This code won't work until you set the defines correctly !! ***
static void test(int l, int k, int j, int i){
  // All 10 digits
  uint8_t* numbers = (uint8_t*) malloc(16);
  //uint8_t numbers[10];
  numbers[0] = 0x3f;
  numbers[1] = 0x06;
  numbers[2] = 0xdb;
  numbers[3] = 0xcf;
  numbers[4] = 0xe6;
  numbers[5] = 0xed;
  numbers[6] = 0xfd;
  numbers[7] = 0x07;
  numbers[8] = 0xff;
  numbers[9] = 0xef;

  size_t size = 16;
  uint8_t* activate = (uint8_t*) malloc(64);
  int ret;
  ret = i2c_example_master_write_slave(I2C_EXAMPLE_MASTER_NUM, activate, size);

  if(ret == ESP_OK) {
    printf("\n i2c worked! \n\n"); 
  }
  else {
    printf("\n i2c did not work! \n\n"); }
  // Now send real commands to the display
  // 1. Send a command to turn on the display oscillator
  i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0x21, size);
  // 2. Send a command to set the blink rate or to not blink
  i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0x81, size);
  // 3. Send a command to turn the brightness
  i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0xEF, size);
  // 4. Finally, send a command with bitmap to display a character
  //Array
  uint8_t* bitmap = (uint8_t*) malloc(16);
  bitmap[0] = 0x00;
  //bitmap[1] = 0x06;
  bitmap[2] = 0x00;
  //bitmap[3] = 0x5b;
  bitmap[4] = 0x00;
  //bitmap[5] = 0x3f;
  bitmap[6] = 0x00;
  //bitmap[7] = 0x3f;

/* This is just a clock that shows time */
/*
int i = 0; // minutes unit
int j = 0; // minutes tenths
int k = 9; // hour units
int l = 0; // hours tenths
*/

// Set the alarm
uint8_t* alarm_time = (uint8_t*) malloc(16);
alarm_time[0] = 1;
alarm_time[1] = 0;
alarm_time[2] = 6;
alarm_time[3] = 0;
bool alarm_hit = false;

for (; l < 2;){ // hours dec
    bitmap[7] = numbers[i];
    bitmap[5] = numbers[j];
    bitmap[3] = numbers[k];
    bitmap[1] = numbers[l];

// Alarm function
    if (alarm_time[0] == i && alarm_time[1] == j && alarm_time[2] == k && alarm_time[3] == l) alarm_hit = true;
    if (alarm_hit == true){
      i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0x83, size);
      for (int counter = 0; counter < 3; counter++){
        i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0x80, size);
        vTaskDelay(25);
        i2c_master_one_byte(I2C_EXAMPLE_MASTER_NUM, 0x81, size);
        vTaskDelay(25);
      }
      alarm_hit = false;
    }

     if (i == 9) {
      i = -1;
      if (j == 5) {
        j = 0;
        if (k != 9 && l != 1) {
          k++;
        }
        else if (k == 9 && l != 1){
         k = 0;
         l++;
        }  
        else if (k == 2 && l == 1){
         k = 1;
         l = 0;
        }  
        else k++;
      }
      else j++;
    }
    else i++;

    i2c_example_master_write_slave(I2C_EXAMPLE_MASTER_NUM, bitmap, size);
    vTaskDelay(6000);
}

}

//***************************End of Clock**************************************************
//***************************Servo**************************************************
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 500 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2400 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 180 //Maximum angle in degree upto which servo can rotate
//#define SERVO_HOUR_MAX 180 //Hour Servo

static void mcpwm_example_gpio_initialize()
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    //Set GPIO 18 as PWM0A, to which servo is connected
    mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, 19); // SECOND SERVO
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}
/*
static uint32_t servo_per_degree_init_2(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_HOUR_MAX)));
    return cal_pulsewidth;
}
*/
/**
 * @brief Configure MCPWM module
 */
void mcpwm_example_servo_control(void *arg)
{
    uint32_t angle, count, angle_hour; // angle_hour is for Second Servo
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config); // SECOND SERVO
    while (1) {
        for (count = 0; count < SERVO_MAX_DEGREE; count+=3) {
            printf("Angle of rotation: %d\n", count);
            angle = servo_per_degree_init(count);
            printf("pulse width: %dus\n", angle);
            
            uint32_t count2;
            for (count2 = 0; count2 < SERVO_MAX_DEGREE; count2+=3){
                angle_hour = servo_per_degree_init(count2);
                mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, angle_hour); // SECOND SERVO
                vTaskDelay(100); // usually commented out
            }
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
            //vTaskDelay(33); // 33 works
            //vTaskDelay(2040);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
            
        }
    }
}

//***************************End of Servo**************************************************

void app_main()
{
    // Servo
    xTaskCreate(mcpwm_example_servo_control, "mcpwm_example_servo_control", 4096, NULL, 5, NULL);
    // Clock
    i2c_example_master_init();
    test(0,6,0,0);
}