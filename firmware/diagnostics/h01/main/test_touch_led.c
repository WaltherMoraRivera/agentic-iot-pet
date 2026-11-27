#include <inttypes.h>
#include <stdbool.h>

#include "bench.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *const TAG = "H01_TOUCH";

// Polling every 5 ms is enough to measure contact durations for the
// gesture windows that T04 must calibrate; no debounce is applied so the
// log shows what the TTP223 really outputs.
#define POLL_MS 5

static void configure_led(int gpio)
{
    const gpio_config_t led = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&led));
    gpio_set_level(gpio, 0);
}

static void blink_only(int led_gpio)
{
    ESP_LOGW(TAG, "Touch pin not configured: blinking LED at 1 Hz only.");
    configure_led(led_gpio);
    for (bool on = true;; on = !on) {
        gpio_set_level(led_gpio, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void test_touch_led_run(void)
{
    const int touch_gpio = CONFIG_H01_TOUCH_GPIO;
    const int led_gpio = CONFIG_H01_LED_GPIO;

    if (touch_gpio < 0 && bench_pin_ok(led_gpio, "LED")) {
        blink_only(led_gpio);
    }
    if (!bench_pin_ok(touch_gpio, "TTP223 I/O")) {
        bench_idle_forever("touch pin not usable");
    }

    // The TTP223 drives its output, so the pull-down only defines the level
    // when the module is disconnected.
    const gpio_config_t touch = {
        .pin_bit_mask = 1ULL << touch_gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&touch));

    const bool has_led = led_gpio >= 0 && bench_pin_ok(led_gpio, "LED") && led_gpio != touch_gpio;
    if (has_led) {
        configure_led(led_gpio);
    } else {
        ESP_LOGW(TAG, "LED not usable: logging touch only.");
    }

    ESP_LOGI(TAG, "Touch the sensor. The LED follows the contact; each release logs its duration.");
    int last = gpio_get_level(touch_gpio);
    ESP_LOGI(TAG, "Initial level=%d", last);
    int64_t pressed_at_us = last ? esp_timer_get_time() : 0;
    uint32_t count = 0;

    for (;;) {
        const int level = gpio_get_level(touch_gpio);
        if (level != last) {
            const int64_t now_us = esp_timer_get_time();
            if (level) {
                pressed_at_us = now_us;
                ESP_LOGI(TAG, "press seq=%" PRIu32 " t=%" PRIi64 " ms", count, now_us / 1000);
            } else {
                ESP_LOGI(TAG, "release seq=%" PRIu32 " t=%" PRIi64 " ms duration=%" PRIi64 " ms",
                         count, now_us / 1000, (now_us - pressed_at_us) / 1000);
                ++count;
            }
            if (has_led) {
                gpio_set_level(led_gpio, level);
            }
            last = level;
        }
        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}
