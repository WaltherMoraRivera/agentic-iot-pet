#include <inttypes.h>

#include "bench.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "sdkconfig.h"

static const char *const TAG = "H01";

static void log_identity(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint32_t flash_size = 0;
    const esp_err_t flash_error = esp_flash_get_physical_size(NULL, &flash_size);

    ESP_LOGI(TAG, "ESP-IDF=%s chip=ESP32-S3 revision=v%u.%u cores=%u reset_reason=%d",
             esp_get_idf_version(), (unsigned)(chip.revision / 100),
             (unsigned)(chip.revision % 100), (unsigned)chip.cores, (int)esp_reset_reason());
    if (flash_error == ESP_OK) {
        ESP_LOGI(TAG, "Physical flash size=%" PRIu32 " B", flash_size);
    } else {
        ESP_LOGE(TAG, "Failed to read physical flash size: %s", esp_err_to_name(flash_error));
    }
}

#if CONFIG_H01_OLED_SH1106
#define OLED_NAME "SH1106"
#else
#define OLED_NAME "SSD1306"
#endif

static void log_pin_config(void)
{
    ESP_LOGI(TAG, "Pins: touch=%d led=%d sda=%d scl=%d oled_addr=0x%02x oled=%s",
             CONFIG_H01_TOUCH_GPIO, CONFIG_H01_LED_GPIO, CONFIG_H01_I2C_SDA_GPIO,
             CONFIG_H01_I2C_SCL_GPIO, CONFIG_H01_OLED_ADDR,
             OLED_NAME);
    ESP_LOGI(TAG, "Pins: mic bclk=%d ws=%d din=%d | amp bclk=%d lrc=%d din=%d volume=%d%%",
             CONFIG_H01_MIC_BCLK_GPIO, CONFIG_H01_MIC_WS_GPIO, CONFIG_H01_MIC_DIN_GPIO,
             CONFIG_H01_AMP_BCLK_GPIO, CONFIG_H01_AMP_LRC_GPIO, CONFIG_H01_AMP_DIN_GPIO,
             CONFIG_H01_AMP_VOLUME_PCT);
}

void app_main(void)
{
    log_identity();
    log_pin_config();

#if CONFIG_H01_TEST_TOUCH_LED
    ESP_LOGI(TAG, "Test: TTP223 touch and LED");
    test_touch_led_run();
#elif CONFIG_H01_TEST_I2C_SCAN
    ESP_LOGI(TAG, "Test: I2C bus scan");
    test_i2c_scan_run();
#elif CONFIG_H01_TEST_OLED
    ESP_LOGI(TAG, "Test: OLED border and two eyes");
    test_oled_run();
#elif CONFIG_H01_TEST_MIC
    ESP_LOGI(TAG, "Test: INMP441 relative level");
    test_mic_run();
#elif CONFIG_H01_TEST_AMP
    ESP_LOGI(TAG, "Test: MAX98357A test tone");
    test_amp_run();
#endif
}
