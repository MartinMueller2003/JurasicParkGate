#pragma once
/*
 * GPIO_Defs_ESP32_TTGO_T8.hpp - Output Management class
 *
 * Project: JurasicParkGate
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

// Output Manager
#define LED_SDA gpio_num_t::GPIO_NUM_21  // Green LED.

#define DEFAULT_I2C_SDA gpio_num_t::GPIO_NUM_22
#define DEFAULT_I2C_SCL gpio_num_t::GPIO_NUM_23

#define DEFAULT_UART_TX gpio_num_t::GPIO_NUM_19
#define DEFAULT_UART_RX gpio_num_t::GPIO_NUM_18

// File Manager
#define SUPPORT_SD
#define SD_CARD_MISO_PIN    gpio_num_t::GPIO_NUM_2
#define SD_CARD_MOSI_PIN    gpio_num_t::GPIO_NUM_15
#define SD_CARD_CLK_PIN     gpio_num_t::GPIO_NUM_14
#define SD_CARD_CS_PIN      gpio_num_t::GPIO_NUM_13
#define USE_MISO_PULLUP
