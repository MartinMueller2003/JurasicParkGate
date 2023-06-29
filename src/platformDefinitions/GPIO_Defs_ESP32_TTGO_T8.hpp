#pragma once
/*
* GPIO_Defs_ESP32_TTGO_T8.hpp - Output Management class
*
* Project: JurasicParkGate - An ESP8266 / ESP32 and E1.31 based pixel driver
* Copyright (c) 2023 Martin Mueller
* http://www.MartnMueller2003.com
*
*  This program is provided free for you to use in any way that you wish,
*  subject to the laws and regulations where you are using it.  Due diligence
*  is strongly suggested before using this code.  Please give credit where due.
*
*  The Author makes no warranty of any kind, express or implied, with regard
*  to this program or the documentation contained in this document.  The
*  Author shall not be liable in any event for incidental or consequential
*  damages in connection with, or arising out of, the furnishing, performance
*  or use of these programs.
*
*/

//Output Manager
#define DEFAULT_RMT_0_GPIO     gpio_num_t::GPIO_NUM_0
#define DEFAULT_RMT_1_GPIO     gpio_num_t::GPIO_NUM_4

#define DEFAULT_RMT_2_GPIO      gpio_num_t::GPIO_NUM_25
#define DEFAULT_RMT_3_GPIO      gpio_num_t::GPIO_NUM_26
#define DEFAULT_RMT_4_GPIO      gpio_num_t::GPIO_NUM_27
#define DEFAULT_RMT_5_GPIO      gpio_num_t::GPIO_NUM_14

#define LED_SDA                 gpio_num_t::GPIO_NUM_21  // Green LED and SDA. Will light-up if PCA9865 is used.

#define DEFAULT_I2C_SDA         gpio_num_t::GPIO_NUM_21
#define DEFAULT_I2C_SCL         gpio_num_t::GPIO_NUM_22

// File Manager
#define SUPPORT_SD
#define SD_CARD_MISO_PIN        gpio_num_t::GPIO_NUM_2
#define SD_CARD_MOSI_PIN        gpio_num_t::GPIO_NUM_15
#define SD_CARD_CLK_PIN         gpio_num_t::GPIO_NUM_14
#define SD_CARD_CS_PIN          gpio_num_t::GPIO_NUM_13
#define USE_MISO_PULLUP

// Output Types
#define SUPPORT_OutputType_Relay            // GPIO
#define SUPPORT_OutputType_Servo_PCA9685    // I2C (default pins)
