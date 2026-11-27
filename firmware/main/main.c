#include <inttypes.h>

#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pet_app.h"

static const char *const TAG = "PRUEBA_USB";

void app_main(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    const unsigned revision_major = chip.revision / 100;
    const unsigned revision_minor = chip.revision % 100;
    const int reset_reason = esp_reset_reason();

    ESP_LOGI(TAG, "ESP-IDF=%s chip=ESP32-S3 revision=v%u.%u cores=%u "
                  "free_heap=%" PRIu32 " B reset_reason=%d",
             esp_get_idf_version(), revision_major, revision_minor,
             (unsigned)chip.cores, esp_get_free_heap_size(), reset_reason);

    // La capacidad física puede diferir del tamaño configurado para la imagen.
    uint32_t flash_size;
    const esp_err_t flash_error = esp_flash_get_physical_size(NULL, &flash_size);
    if (flash_error == ESP_OK) {
        ESP_LOGI(TAG, "Physical flash size=%" PRIu32 " B", flash_size);
    } else {
        ESP_LOGE(TAG, "Failed to read physical flash size: %s",
                 esp_err_to_name(flash_error));
    }

    // Comportamiento local de la mascota (T04) en su propia tarea.
    pet_app_start();

    // Repite la identidad para quien abra el monitor después del arranque.
    TickType_t last_wake = xTaskGetTickCount();
    for (uint32_t sequence = 0;; ++sequence) {
        ESP_LOGI(TAG, "Heartbeat seq=%" PRIu32 " uptime=%" PRIi64 " ms "
                      "free_heap=%" PRIu32 " B reset_reason=%d "
                      "chip=ESP32-S3 revision=v%u.%u cores=%u",
                 sequence, esp_timer_get_time() / 1000, esp_get_free_heap_size(),
                 reset_reason, revision_major, revision_minor, (unsigned)chip.cores);
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
    }
}
