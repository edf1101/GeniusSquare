/*
 * ESP32Encoder.h
 *
 * Migrated from legacy driver/pcnt.h to driver/pulse_cnt.h (ESP-IDF v5+).
 */
#pragma once
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#ifndef ARDUINO
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/portable.h>
#include <freertos/semphr.h>
#endif

#define MAX_ESP32_ENCODERS 8
#define _INT16_MAX  32766
#define _INT16_MIN -32766
#define ISR_CORE_USE_DEFAULT (0xffffffff)

enum class encType { single, half, full };
enum class puType  { up, down, none };

class ESP32Encoder;
typedef void (*enc_isr_cb_t)(void*);

class ESP32Encoder {
public:
    /**
     * @brief Construct a new ESP32Encoder object.
     *
     * @param always_interrupt Fire callback on every encoder pulse (not just overflow).
     * @param enc_isr_cb       Optional callback fired on every pulse when always_interrupt is set.
     * @param enc_isr_cb_data  Argument passed to enc_isr_cb (defaults to this).
     */
    ESP32Encoder(bool always_interrupt = false,
                 enc_isr_cb_t enc_isr_cb = nullptr,
                 void* enc_isr_cb_data = nullptr);
    ~ESP32Encoder();

    void attachHalfQuad(int aPinNumber, int bPinNumber);
    void attachFullQuad(int aPinNumber, int bPinNumber);
    void attachSingleEdge(int aPinNumber, int bPinNumber);

    int64_t getCount();
    int64_t clearCount();
    int64_t pauseCount();
    int64_t resumeCount();
    void    detach();
    [[deprecated("Replaced by detach")]] void detatch();
    bool    isAttached() { return attached; }
    void    setCount(int64_t value);
    void    setFilter(uint16_t value);

    bool           always_interrupt;
    gpio_num_t     aPinNumber;
    gpio_num_t     bPinNumber;
    volatile int64_t count = 0;
    static puType  useInternalWeakPullResistors;
    static uint32_t isrServiceCpuCore;
    enc_isr_cb_t   _enc_isr_cb;
    void*          _enc_isr_cb_data;

private:
    void    attach(int aPinNumber, int bPinNumber, encType et);
    int64_t getCountRaw();

    bool attached;
    bool direction;
    bool working;

    pcnt_unit_handle_t    _pcnt_unit;
    pcnt_channel_handle_t _pcnt_chan[2];
};

// Added by Sloeber
#pragma once
