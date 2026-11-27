#include <inttypes.h>
#include <stdio.h>

#include "esp_attr.h"
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *const TAG = "PRUEBA_USB";

// La RAM RTC sin inicializar conserva el contador entre reinicios que no cortan
// la alimentación; el valor mágico descarta el contenido aleatorio del encendido.
#define BOOT_COUNT_MAGIC 0x50455431u
static RTC_NOINIT_ATTR uint32_t boot_count_magic;
static RTC_NOINIT_ATTR uint32_t boot_count;

static const char *reset_reason_name(esp_reset_reason_t reason)
{
    switch (reason) {
    case ESP_RST_UNKNOWN:    return "UNKNOWN";
    case ESP_RST_POWERON:    return "POWERON";
    case ESP_RST_EXT:        return "EXT";
    case ESP_RST_SW:         return "SW";
    case ESP_RST_PANIC:      return "PANIC";
    case ESP_RST_INT_WDT:    return "INT_WDT";
    case ESP_RST_TASK_WDT:   return "TASK_WDT";
    case ESP_RST_WDT:        return "WDT";
    case ESP_RST_DEEPSLEEP:  return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:   return "BROWNOUT";
    case ESP_RST_SDIO:       return "SDIO";
    case ESP_RST_USB:        return "USB";
    case ESP_RST_JTAG:       return "JTAG";
    case ESP_RST_EFUSE:      return "EFUSE";
    case ESP_RST_PWR_GLITCH: return "PWR_GLITCH";
    case ESP_RST_CPU_LOCKUP: return "CPU_LOCKUP";
    default:                 return "UNLISTED";
    }
}

void app_main(void)
{
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    const unsigned revision_major = chip.revision / 100;
    const unsigned revision_minor = chip.revision % 100;
    const esp_reset_reason_t reset_reason = esp_reset_reason();
    const char *const reset_name = reset_reason_name(reset_reason);

    char chip_name[24];
    if (chip.model == CHIP_ESP32S3) {
        snprintf(chip_name, sizeof chip_name, "ESP32-S3");
    } else {
        snprintf(chip_name, sizeof chip_name, "unknown(%d)", (int)chip.model);
    }

    if (boot_count_magic != BOOT_COUNT_MAGIC || reset_reason == ESP_RST_POWERON) {
        boot_count_magic = BOOT_COUNT_MAGIC;
        boot_count = 0;
    }
    ++boot_count;

    ESP_LOGI(TAG, "ESP-IDF=%s chip=%s revision=v%u.%u cores=%u "
                  "free_heap=%" PRIu32 " B reset_reason=%d reset_name=%s "
                  "boot_count=%" PRIu32,
             esp_get_idf_version(), chip_name, revision_major, revision_minor,
             (unsigned)chip.cores, esp_get_free_heap_size(), (int)reset_reason,
             reset_name, boot_count);

    // La capacidad física puede diferir del tamaño configurado para la imagen.
    uint32_t flash_size;
    const esp_err_t flash_error = esp_flash_get_physical_size(NULL, &flash_size);
    if (flash_error == ESP_OK) {
        ESP_LOGI(TAG, "Physical flash size=%" PRIu32 " B", flash_size);
    } else {
        ESP_LOGE(TAG, "Failed to read physical flash size: %s",
                 esp_err_to_name(flash_error));
    }

    // Repite la identidad para quien abra el monitor después del arranque.
    TickType_t last_wake = xTaskGetTickCount();
    for (uint32_t sequence = 0;; ++sequence) {
        ESP_LOGI(TAG, "Heartbeat seq=%" PRIu32 " uptime=%" PRIi64 " ms "
                      "free_heap=%" PRIu32 " B reset_reason=%d reset_name=%s "
                      "boot_count=%" PRIu32 " chip=%s revision=v%u.%u cores=%u",
                 sequence, esp_timer_get_time() / 1000, esp_get_free_heap_size(),
                 (int)reset_reason, reset_name, boot_count, chip_name,
                 revision_major, revision_minor, (unsigned)chip.cores);
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(2000));
    }
}
