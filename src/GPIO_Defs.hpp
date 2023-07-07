#pragma once
/*
 * GPIO_Defs.hpp - Output Management class
 *
 * Project: JurasicParkGate - 
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

#include "JurasicParkGate.h"

#ifdef ARDUINO_ARCH_ESP32
#include <driver/gpio.h>
#include <hal/uart_types.h>
#endif // ifdef ARDUINO_ARCH_ESP32

#ifdef ARDUINO_ARCH_ESP8266
    typedef enum
    {
        GPIO_NUM_0 = 0,
        GPIO_NUM_1,
        GPIO_NUM_2,
        GPIO_NUM_3,
        GPIO_NUM_4,
        GPIO_NUM_5,
        GPIO_NUM_6,
        GPIO_NUM_7,
        GPIO_NUM_8,
        GPIO_NUM_9,
        GPIO_NUM_10,
        GPIO_NUM_11,
        GPIO_NUM_12,
        GPIO_NUM_13,
        GPIO_NUM_14,
        GPIO_NUM_15,
        GPIO_NUM_16,
        GPIO_NUM_17,
        GPIO_NUM_18,
        GPIO_NUM_19,
        GPIO_NUM_20,
        GPIO_NUM_21,
        GPIO_NUM_22,
        GPIO_NUM_23,
        GPIO_NUM_24,
        GPIO_NUM_25,
        GPIO_NUM_26,
        GPIO_NUM_27,
        GPIO_NUM_28,
        GPIO_NUM_29,
        GPIO_NUM_30,
        GPIO_NUM_31,
        GPIO_NUM_32,
        GPIO_NUM_33,
        GPIO_NUM_34
    } gpio_num_t;

    typedef enum
    {
        UART_NUM_0 = 0,
        UART_NUM_1,
        UART_NUM_2
    } uart_port_t;
#endif // def ARDUINO_ARCH_ESP8266

// Platform specific GPIO definitions

#if defined (BOARD_ESP32_TTGO_T8)
#include "platformDefinitions/GPIO_Defs_ESP32_TTGO_T8.hpp"
#else // if defined (BOARD_ESP32_TTGO_T8)
#error "No valid platform definition"
#endif // ndef platform specific GPIO definitions

#if defined (SUPPORT_SD) || defined (SUPPORT_SD_MMC)
    #define SUPPORT_FPP
#endif // defined(SUPPORT_SD) || defined(SUPPORT_SD_MMC)
