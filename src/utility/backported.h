#pragma once
#include <Arduino.h>


// ESP32 defines various types and functions that are missing on ESP8266.
// Backport missing types / macros to allow simpler source management.

#if defined (ARDUINO_ARCH_ESP32)
#include "esp_err.h"
#elif defined (ARDUINO_ARCH_ESP8266)

#ifdef __cplusplus
        extern "C"
        {
#endif // ifdef __cplusplus

    typedef int esp_err_t;  // currently only need two defined status: OK & FAIL
    #define ESP_OK      0
    #define ESP_FAIL    -1

#ifdef DEBUG_ESP_PORT
        #define LOG_ERROR_MSG(...) DEBUG_ESP_PORT.printf (__VA_ARGS__)
#else // ifdef DEBUG_ESP_PORT
        #define LOG_ERROR_MSG(...)
#endif // ifdef DEBUG_ESP_PORT

#ifndef likely
        #define likely(x) __builtin_expect (!!(x), 1)
#endif // ifndef likely
#ifndef unlikely
        #define unlikely(x) __builtin_expect (!!(x), 0)
#endif // ifndef unlikely

#if defined (NDEBUG)
    #define ESP_ERROR_CHECK(x)           \
            do                           \
            {                            \
                esp_err_t err_rc_ = (x); \
                (void) sizeof(err_rc_);  \
            } while(0)
#elif defined (CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT)
    #define ESP_ERROR_CHECK(x)                     \
            do                                     \
            {                                      \
                esp_err_t err_rc_ = (x);           \
                if ( unlikely(err_rc_ != ESP_OK) ) \
                {                                  \
                    abort();                       \
                }                                  \
            } while(0)
#else // if defined (NDEBUG)
    #define ESP_ERROR_CHECK(x)                                \
            do                                                \
            {                                                 \
                esp_err_t err_rc_ = (x);                      \
                if ( unlikely(err_rc_ != ESP_OK) )            \
                {                                             \
                    LOG_ERROR_MSG("err: esp_err_t = %d", rc); \
                    assert(0 && #x);                          \
                }                                             \
            } while(0);

#endif // if defined (NDEBUG)



#ifdef __cplusplus
        }
#endif // ifdef __cplusplus

#else // if defined (ARDUINO_ARCH_ESP32)
#error "Unsupported CPU type"
#endif // if defined (ARDUINO_ARCH_ESP32)
