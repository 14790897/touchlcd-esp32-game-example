extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
}

static const char *TAG = "SPI_TEST";

// XPT2046 SPI娴嬭瘯
#define TOUCH_CS_PIN 7
#define SPI_SCLK_PIN 42
#define SPI_MOSI_PIN 45
#define SPI_MISO_PIN 46

void test_xpt2046_spi()
{
    ESP_LOGI(TAG, "Starting XPT2046 SPI hardware test");

    // 配置SPI总线
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = SPI_MOSI_PIN;
    buscfg.miso_io_num = SPI_MISO_PIN;
    buscfg.sclk_io_num = SPI_SCLK_PIN;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 64;

    // 给 MISO 下拉，避免浮空导致读满 0xFFF 的假象（需在 bus 初始化前设置）
    gpio_set_pull_mode((gpio_num_t)SPI_MISO_PIN, GPIO_PULLDOWN_ONLY);

    // 先尝试 SPI2，失败回退 SPI3，并记录最终 host
    spi_host_device_t host = SPI2_HOST;
    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "SPI2 init failed: %s, trying SPI3", esp_err_to_name(ret));
        host = SPI3_HOST;
        ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "SPI3 init failed: %s", esp_err_to_name(ret));
            return;
        }
    }
    ESP_LOGI(TAG, "Using %s", host == SPI2_HOST ? "SPI2_HOST" : "SPI3_HOST");

    // 配置SPI设备（全双工，低速，适当CS保持时间）
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 1000000; // 1MHz
    devcfg.mode = 0;
    devcfg.spics_io_num = TOUCH_CS_PIN;
    devcfg.queue_size = 1;
    devcfg.cs_ena_pretrans = 2;
    devcfg.cs_ena_posttrans = 2;

    spi_device_handle_t spi;
    ret = spi_bus_add_device(host, &devcfg, &spi);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        spi_bus_free(host);
        return;
    }

    ESP_LOGI(TAG, "SPI device initialized successfully");

    // 读取X / Y / Z1：单笔24bit，保持一次CS有效，避免中间抬CS
    const uint8_t CMD_X = 0xD0;  // X position
    const uint8_t CMD_Y = 0x90;  // Y position
    const uint8_t CMD_Z1 = 0xB0; // Z1 pressure

    auto xfer_24b = [&](uint8_t cmd, uint16_t &val)
    {
        spi_transaction_t t = {};
        uint8_t tx[3] = {cmd, 0x00, 0x00};
        uint8_t rx[3] = {0};
        t.length = 24; // 发送24bit，同时接收24bit
        t.tx_buffer = tx;
        t.rx_buffer = rx;
        esp_err_t e = spi_device_transmit(spi, &t);
        if (e != ESP_OK)
            return e;
        uint16_t raw = ((uint16_t)rx[1] << 8) | rx[2];
        val = (raw >> 3) & 0x0FFF; // 12bit 有效
        return ESP_OK;
    };

    for (int i = 0; i < 10; i++)
    {
        uint16_t x_raw = 0, y_raw = 0, z1_raw = 0;
        ret = xfer_24b(CMD_X, x_raw);
        if (ret == ESP_OK)
            ret = xfer_24b(CMD_Y, y_raw);
        if (ret == ESP_OK)
            ret = xfer_24b(CMD_Z1, z1_raw);

        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Test %d: X=%u Y=%u Z1=%u", i + 1, x_raw, y_raw, z1_raw);
        }
        else
        {
            ESP_LOGE(TAG, "SPI read failed: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 娓呯悊
    spi_bus_remove_device(spi);
    spi_bus_free(host);

    ESP_LOGI(TAG, "XPT2046 SPI test completed");
}

extern "C" void app_main(void)
{
    test_xpt2046_spi();
}
