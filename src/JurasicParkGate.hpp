#pragma once
/*
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

#include <Arduino.h>

#include <ArduinoJson.h>

#include "Common.hpp"
#include "CfgMgr.hpp"
// #include "Logging.hpp"
// #include "ConstNames.hpp"

#define REBOOT_DELAYms  100     ///< Delay for rebooting once reboot flag is set
#define LOG_PORT        Serial  ///< Serial port for console logging

#define I2CClockPin     gpio_num_t::GPIO_NUM_5
#define I2CDataPin      gpio_num_t::GPIO_NUM_15
#define I2CResetPin     gpio_num_t::GPIO_NUM_4
#define TracePulseGpio  gpio_num_t::GPIO_NUM_16

#define SD_CARD_MISO_PIN    gpio_num_t::GPIO_NUM_35
#define SD_CARD_MOSI_PIN    gpio_num_t::GPIO_NUM_19
#define SD_CARD_CLK_PIN     gpio_num_t::GPIO_NUM_18
#define SD_CARD_CLK_MHZ     SD_SCK_MHZ (50)  // 50 MHz SPI clock
#define SD_CARD_CS_PIN      gpio_num_t::GPIO_NUM_21

#define PWM1_PIN    gpio_num_t::GPIO_NUM_27
#define PWM2_PIN    gpio_num_t::GPIO_NUM_13

extern bool reboot;
