#include <stdint.h>
#include <string.h>

#include "bench.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *const TAG = "H01_I2C";

#define I2C_TIMEOUT_MS 50
#define OLED_WIDTH 128
#define OLED_PAGES 8

#if CONFIG_H01_OLED_SH1106
#define OLED_COL_OFFSET 2
#else
#define OLED_COL_OFFSET 0
#endif

static i2c_master_bus_handle_t open_bus(void)
{
    const int pins[] = {CONFIG_H01_I2C_SDA_GPIO, CONFIG_H01_I2C_SCL_GPIO};
    const char *const labels[] = {"I2C SDA", "I2C SCL"};
    if (!bench_pins_ok(pins, labels, 2)) {
        bench_idle_forever("I2C pins not usable");
    }

    // Internal pull-ups are weak; most OLED modules carry their own.
    const i2c_master_bus_config_t config = {
        .i2c_port = -1,
        .sda_io_num = CONFIG_H01_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_H01_I2C_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus = NULL;
    ESP_ERROR_CHECK(i2c_new_master_bus(&config, &bus));
    return bus;
}

static int scan(i2c_master_bus_handle_t bus)
{
    int found = 0;
    for (uint16_t addr = 0x08; addr <= 0x77; ++addr) {
        if (i2c_master_probe(bus, addr, I2C_TIMEOUT_MS) == ESP_OK) {
            ESP_LOGI(TAG, "ACK at 0x%02x", addr);
            ++found;
        }
    }
    ESP_LOGI(TAG, "Scan done: %d device(s)", found);
    return found;
}

void test_i2c_scan_run(void)
{
    i2c_master_bus_handle_t bus = open_bus();
    for (;;) {
        if (scan(bus) == 0) {
            ESP_LOGW(TAG, "No device answered: check power, SDA/SCL order and pull-ups.");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// --- OLED -----------------------------------------------------------------

static uint8_t framebuffer[OLED_PAGES][OLED_WIDTH];

static void set_pixel(int x, int y)
{
    if (x >= 0 && x < OLED_WIDTH && y >= 0 && y < OLED_PAGES * 8) {
        framebuffer[y / 8][x] |= (uint8_t)(1u << (y % 8));
    }
}

static void draw_border(void)
{
    for (int x = 0; x < OLED_WIDTH; ++x) {
        set_pixel(x, 0);
        set_pixel(x, OLED_PAGES * 8 - 1);
    }
    for (int y = 0; y < OLED_PAGES * 8; ++y) {
        set_pixel(0, y);
        set_pixel(OLED_WIDTH - 1, y);
    }
}

static void fill_circle(int cx, int cy, int r)
{
    for (int y = -r; y <= r; ++y) {
        for (int x = -r; x <= r; ++x) {
            if (x * x + y * y <= r * r) {
                set_pixel(cx + x, cy + y);
            }
        }
    }
}

static esp_err_t send_commands(i2c_master_dev_handle_t dev, const uint8_t *cmds, size_t len)
{
    uint8_t buf[32];
    if (len + 1 > sizeof(buf)) {
        return ESP_ERR_INVALID_SIZE;
    }
    buf[0] = 0x00; // control byte: command stream
    memcpy(&buf[1], cmds, len);
    return i2c_master_transmit(dev, buf, len + 1, I2C_TIMEOUT_MS);
}

static esp_err_t oled_init(i2c_master_dev_handle_t dev)
{
    static const uint8_t init[] = {
        0xAE,       // display off
        0xD5, 0x80, // clock divide
        0xA8, 0x3F, // multiplex 64
        0xD3, 0x00, // display offset 0
        0x40,       // start line 0
#if CONFIG_H01_OLED_SH1106
        0xAD, 0x8B, // SH1106: DC-DC on
#else
        0x8D, 0x14, // SSD1306: charge pump on
        0x20, 0x02, // SSD1306: page addressing mode
#endif
        0xA1,       // segment remap
        0xC8,       // COM scan descending
        0xDA, 0x12, // COM pins
        0x81, 0x80, // contrast
        0xD9, 0x22, // pre-charge
        0xDB, 0x35, // VCOM detect
        0xA4,       // resume from RAM
        0xA6,       // normal (not inverted)
        0xAF,       // display on
    };
    return send_commands(dev, init, sizeof(init));
}

static esp_err_t oled_flush(i2c_master_dev_handle_t dev)
{
    uint8_t data[OLED_WIDTH + 1];
    data[0] = 0x40; // control byte: data stream
    for (int page = 0; page < OLED_PAGES; ++page) {
        const uint8_t col = OLED_COL_OFFSET;
        const uint8_t addr[] = {(uint8_t)(0xB0 | page), (uint8_t)(0x00 | (col & 0x0F)),
                                (uint8_t)(0x10 | (col >> 4))};
        esp_err_t err = send_commands(dev, addr, sizeof(addr));
        if (err != ESP_OK) {
            return err;
        }
        memcpy(&data[1], framebuffer[page], OLED_WIDTH);
        err = i2c_master_transmit(dev, data, sizeof(data), I2C_TIMEOUT_MS);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

void test_oled_run(void)
{
    i2c_master_bus_handle_t bus = open_bus();
    const uint16_t addr = CONFIG_H01_OLED_ADDR;
    if (addr < 0x08 || addr > 0x77) {
        ESP_LOGE(TAG, "OLED address not configured. Run the I2C scan and set it in menuconfig.");
        scan(bus);
        bench_idle_forever("OLED address not configured");
    }
    if (i2c_master_probe(bus, addr, I2C_TIMEOUT_MS) != ESP_OK) {
        ESP_LOGE(TAG, "No ACK at 0x%02x.", addr);
        scan(bus);
        bench_idle_forever("OLED did not answer");
    }

    const i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = 400000,
    };
    i2c_master_dev_handle_t dev = NULL;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_config, &dev));

    memset(framebuffer, 0, sizeof(framebuffer));
    draw_border();
    fill_circle(40, 30, 10);
    fill_circle(88, 30, 10);

    esp_err_t err = oled_init(dev);
    if (err == ESP_OK) {
        err = oled_flush(dev);
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "OLED write failed: %s", esp_err_to_name(err));
        bench_idle_forever("OLED write failed");
    }

    ESP_LOGI(TAG, "Drawn: 1 px border on all four edges and two eyes at x=40 and x=88.");
    ESP_LOGI(TAG, "Check that all four border lines are visible and none is cut or doubled.");
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "OLED test still running");
    }
}
