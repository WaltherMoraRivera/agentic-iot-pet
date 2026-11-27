#include "bench.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *const TAG = "H01";

// GPIO19 and GPIO20 are USB D-/D+ on the ESP32-S3 and carry the console.
static bool is_usb_pin(int gpio)
{
    return gpio == 19 || gpio == 20;
}

// Strapping pins can change the boot mode if a module pulls them.
static bool is_strapping_pin(int gpio)
{
    return gpio == 0 || gpio == 3 || gpio == 45 || gpio == 46;
}

bool bench_pin_ok(int gpio, const char *label)
{
    if (gpio < 0) {
        ESP_LOGE(TAG, "%s: pin not configured. Set the verified GPIO in menuconfig > H01 bench.",
                 label);
        return false;
    }
    if (!GPIO_IS_VALID_GPIO(gpio)) {
        ESP_LOGE(TAG, "%s: GPIO%d does not exist on this chip.", label, gpio);
        return false;
    }
    if (is_usb_pin(gpio)) {
        ESP_LOGE(TAG, "%s: GPIO%d is used by the USB console.", label, gpio);
        return false;
    }
    if (is_strapping_pin(gpio)) {
        ESP_LOGW(TAG, "%s: GPIO%d is a strapping pin; check boot with the module connected.",
                 label, gpio);
    }
    return true;
}

bool bench_pins_ok(const int *gpios, const char *const *labels, int count)
{
    bool ok = true;
    for (int i = 0; i < count; ++i) {
        if (!bench_pin_ok(gpios[i], labels[i])) {
            ok = false;
            continue;
        }
        for (int j = 0; j < i; ++j) {
            if (gpios[j] == gpios[i]) {
                ESP_LOGE(TAG, "%s and %s share GPIO%d.", labels[j], labels[i], gpios[i]);
                ok = false;
            }
        }
    }
    return ok;
}

void bench_idle_forever(const char *reason)
{
    for (;;) {
        ESP_LOGW(TAG, "Test not running: %s", reason);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
