/*
 * ESP32Encoder.cpp
 *
 * Migrated from legacy driver/pcnt.h to driver/pulse_cnt.h (ESP-IDF v5+).
 */

#include "ESP32Encoder.h"
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <rom/gpio.h>
#define delay(ms) vTaskDelay(pdMS_TO_TICKS(ms))
#endif

#include <soc/soc_caps.h>
#if SOC_PCNT_SUPPORTED

#include "esp_log.h"

static const char* TAG_ENCODER = "ESP32Encoder";

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
#define _ENTER_CRITICAL() portENTER_CRITICAL_SAFE(&spinlock)
#define _EXIT_CRITICAL()  portEXIT_CRITICAL_SAFE(&spinlock)

puType   ESP32Encoder::useInternalWeakPullResistors = puType::down;
uint32_t ESP32Encoder::isrServiceCpuCore = ISR_CORE_USE_DEFAULT;

ESP32Encoder::ESP32Encoder(bool always_interrupt_, enc_isr_cb_t enc_isr_cb, void* enc_isr_cb_data)
    : always_interrupt{always_interrupt_},
      aPinNumber{(gpio_num_t)0},
      bPinNumber{(gpio_num_t)0},
      count{0},
      _enc_isr_cb(enc_isr_cb),
      _enc_isr_cb_data(enc_isr_cb_data),
      attached{false},
      direction{false},
      working{false},
      _pcnt_unit{nullptr},
      _pcnt_chan{nullptr, nullptr}
{
    if (enc_isr_cb_data == nullptr) {
        _enc_isr_cb_data = this;
    }
}  ;

ESP32Encoder::~ESP32Encoder() {
    if (attached) detach();
}

// Called when the PCNT counter reaches a registered watchpoint.
// Handles counter overflow (high/low limits) by accumulating into count.
static bool IRAM_ATTR pcnt_on_reach(pcnt_unit_handle_t unit_handle,
                                     const pcnt_watch_event_data_t* edata,
                                     void* user_ctx)
{
    ESP32Encoder* enc = static_cast<ESP32Encoder*>(user_ctx);
    BaseType_t high_task_wakeup = pdFALSE;

    _ENTER_CRITICAL();
    int32_t wp = edata->watch_point_value;
    if (wp == _INT16_MAX || wp == _INT16_MIN) {
        // Hardware auto-wraps to 0 at the limits; accumulate overflow into count.
        enc->count += wp;
    }
    _EXIT_CRITICAL();

    return high_task_wakeup == pdTRUE;
}

void ESP32Encoder::detach() {
    if (!attached) return;
    pcnt_unit_stop(_pcnt_unit);
    pcnt_unit_disable(_pcnt_unit);
    for (int i = 0; i < 2; i++) {
        if (_pcnt_chan[i]) {
            pcnt_del_channel(_pcnt_chan[i]);
            _pcnt_chan[i] = nullptr;
        }
    }
    pcnt_unit_remove_watch_point(_pcnt_unit, _INT16_MAX);
    pcnt_unit_remove_watch_point(_pcnt_unit, _INT16_MIN);
    pcnt_del_unit(_pcnt_unit);
    _pcnt_unit = nullptr;
    attached = false;
}

void ESP32Encoder::detatch() {
    detach();
}

void ESP32Encoder::attach(int a, int b, encType et) {
    if (attached) {
        ESP_LOGE(TAG_ENCODER, "attach: already attached");
        return;
    }

    aPinNumber = (gpio_num_t)a;
    bPinNumber = (gpio_num_t)b;

    // Create PCNT unit with counter range ±32766.
    pcnt_unit_config_t unit_config = {};
    unit_config.low_limit  = _INT16_MIN;
    unit_config.high_limit = _INT16_MAX;
    if (pcnt_new_unit(&unit_config, &_pcnt_unit) != ESP_OK) {
        ESP_LOGE(TAG_ENCODER, "Failed to create PCNT unit");
        return;
    }

    // Channel 0: A=edge, B=level.
    pcnt_chan_config_t chan0_cfg = {};
    chan0_cfg.edge_gpio_num  = a;
    chan0_cfg.level_gpio_num = b;
    if (pcnt_new_channel(_pcnt_unit, &chan0_cfg, &_pcnt_chan[0]) != ESP_OK) {
        ESP_LOGE(TAG_ENCODER, "Failed to create PCNT channel 0");
        return;
    }
    // Rising A → decrement, Falling A → increment; B HIGH reverses direction.
    pcnt_channel_edge_action_t pos0 = (et != encType::single)
        ? PCNT_CHANNEL_EDGE_ACTION_DECREASE
        : PCNT_CHANNEL_EDGE_ACTION_HOLD;
    pcnt_channel_set_edge_action(_pcnt_chan[0], pos0, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(_pcnt_chan[0],
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE,  // B HIGH → reverse
        PCNT_CHANNEL_LEVEL_ACTION_KEEP);    // B LOW  → keep

    // Channel 1: B=edge, A=level (used only for full-quad).
    pcnt_chan_config_t chan1_cfg = {};
    chan1_cfg.edge_gpio_num  = b;
    chan1_cfg.level_gpio_num = a;
    if (pcnt_new_channel(_pcnt_unit, &chan1_cfg, &_pcnt_chan[1]) != ESP_OK) {
        ESP_LOGE(TAG_ENCODER, "Failed to create PCNT channel 1");
        return;
    }
    if (et == encType::full) {
        pcnt_channel_set_edge_action(_pcnt_chan[1],
            PCNT_CHANNEL_EDGE_ACTION_DECREASE,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE);
        pcnt_channel_set_level_action(_pcnt_chan[1],
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,     // A HIGH → keep
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE);  // A LOW  → reverse
    } else {
        // Disable channel 1 for single/half modes.
        pcnt_channel_set_edge_action(_pcnt_chan[1],
            PCNT_CHANNEL_EDGE_ACTION_HOLD,
            PCNT_CHANNEL_EDGE_ACTION_HOLD);
        pcnt_channel_set_level_action(_pcnt_chan[1],
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP);
    }

    // pcnt_new_channel() resets GPIO pull resistors to pull-up by default.
    // Re-apply the requested pull configuration after channel creation.
    if (useInternalWeakPullResistors == puType::down) {
        gpio_pulldown_en(aPinNumber);
        gpio_pulldown_en(bPinNumber);
    } else if (useInternalWeakPullResistors == puType::up) {
        gpio_pullup_en(aPinNumber);
        gpio_pullup_en(bPinNumber);
    } else {
        gpio_pulldown_dis(aPinNumber);
        gpio_pullup_dis(aPinNumber);
        gpio_pulldown_dis(bPinNumber);
        gpio_pullup_dis(bPinNumber);
    }

    // Glitch filter: 250 APB cycles ≈ 3125 ns at 80 MHz.
    setFilter(250);

    // Register watchpoints at the counter limits so overflows are tracked.
    pcnt_unit_add_watch_point(_pcnt_unit, _INT16_MAX);
    pcnt_unit_add_watch_point(_pcnt_unit, _INT16_MIN);

    pcnt_event_callbacks_t cbs = {};
    cbs.on_reach = pcnt_on_reach;
    if (pcnt_unit_register_event_callbacks(_pcnt_unit, &cbs, this) != ESP_OK) {
        ESP_LOGE(TAG_ENCODER, "Failed to register PCNT callbacks");
    }

    pcnt_unit_enable(_pcnt_unit);
    pcnt_unit_clear_count(_pcnt_unit);
    pcnt_unit_start(_pcnt_unit);

    attached = true;
}

void ESP32Encoder::attachHalfQuad(int aPinNumber, int bPinNumber) {
    attach(aPinNumber, bPinNumber, encType::half);
}
void ESP32Encoder::attachSingleEdge(int aPinNumber, int bPinNumber) {
    attach(aPinNumber, bPinNumber, encType::single);
}
void ESP32Encoder::attachFullQuad(int aPinNumber, int bPinNumber) {
    attach(aPinNumber, bPinNumber, encType::full);
}

void ESP32Encoder::setCount(int64_t value) {
    _ENTER_CRITICAL();
    count = value - getCountRaw();
    _EXIT_CRITICAL();
}

int64_t ESP32Encoder::getCountRaw() {
    int raw = 0;
    pcnt_unit_get_count(_pcnt_unit, &raw);
    return (int64_t)raw;
}

int64_t ESP32Encoder::getCount() {
    _ENTER_CRITICAL();
    int64_t result = count + getCountRaw();
    _EXIT_CRITICAL();
    return result;
}

int64_t ESP32Encoder::clearCount() {
    _ENTER_CRITICAL();
    count = 0;
    _EXIT_CRITICAL();
    return (int64_t)pcnt_unit_clear_count(_pcnt_unit);
}

int64_t ESP32Encoder::pauseCount() {
    return (int64_t)pcnt_unit_stop(_pcnt_unit);
}

int64_t ESP32Encoder::resumeCount() {
    return (int64_t)pcnt_unit_start(_pcnt_unit);
}

void ESP32Encoder::setFilter(uint16_t value) {
    if (value == 0) {
        pcnt_unit_set_glitch_filter(_pcnt_unit, nullptr);
    } else {
        // Convert APB clock cycles (80 MHz) to nanoseconds.
        pcnt_glitch_filter_config_t filter_cfg = {};
        filter_cfg.max_glitch_ns = (uint32_t)value * 1000 / 80;
        pcnt_unit_set_glitch_filter(_pcnt_unit, &filter_cfg);
    }
}

#else
#warning PCNT not supported on this SoC, this will likely lead to linker errors when using ESP32Encoder
#endif // SOC_PCNT_SUPPORTED
