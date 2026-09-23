#include "MockTempSensor.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdlib>
#include <cstring>

static const char *TAG = "MockTempSensor";

MockTempSensor::MockTempSensor(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate)
    : uart_num_(uart_num), tx_pin_(tx_pin), rx_pin_(rx_pin), baud_rate_(baud_rate) {}

esp_err_t MockTempSensor::init() {
    uart_config_t uart_config = {};
    uart_config.baud_rate = baud_rate_;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity    = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    esp_err_t err = uart_param_config(uart_num_, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config failed: %s", esp_err_to_name(err));
        return err;
    }

    err = uart_set_pin(uart_num_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin failed: %s", esp_err_to_name(err));
        return err;
    }

    // RX buffer 256 bytes, TX buffer 0 (blocking send)
    const int rx_buffer_size = 256;
    err = uart_driver_install(uart_num_, rx_buffer_size, 0, 0, NULL, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Initialized on UART %d (TX: %d, RX: %d @ %d baud)", 
             uart_num_, tx_pin_, rx_pin_, baud_rate_);
    return ESP_OK;
}

esp_err_t MockTempSensor::sendCommand(char cmd) {
    int bytes_sent = uart_write_bytes(uart_num_, &cmd, 1);
    if (bytes_sent != 1) {
        ESP_LOGE(TAG, "Failed to send command '%c'", cmd);
        return ESP_FAIL;
    }
    return ESP_OK;
}

int MockTempSensor::readLine(char *buffer, size_t max_len, uint32_t timeout_ms) {
    if (buffer == nullptr || max_len == 0) {
        return -1;
    }

    size_t idx = 0;
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (idx < max_len - 1) {
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            break;
        }

        uint8_t byte = 0;
        int len = uart_read_bytes(uart_num_, &byte, 1, pdMS_TO_TICKS(20));
        if (len > 0) {
            if (byte == '\n') {
                break;
            }
            if (byte != '\r') {
                buffer[idx++] = static_cast<char>(byte);
            }
        }
    }

    buffer[idx] = '\0';
    return static_cast<int>(idx);
}

esp_err_t MockTempSensor::readTemperature(float &out_temp) {
    // 1. Önceki kalıntı verileri temizle
    uart_flush_input(uart_num_);

    // 2. Python'a sıcaklık isteği gönder ('T')
    esp_err_t err = sendCommand('T');
    if (err != ESP_OK) {
        return err;
    }

    // 3. Python'dan gelen cevabı bekle (500 ms zaman aşımı)
    char rx_buffer[32];
    int len = readLine(rx_buffer, sizeof(rx_buffer), 500);
    if (len <= 0) {
        ESP_LOGW(TAG, "Response timeout or empty data from mock sensor");
        return ESP_ERR_TIMEOUT;
    }

    // 4. String veriyi float değere dönüştür
    char *endptr = nullptr;
    float val = strtof(rx_buffer, &endptr);
    if (endptr == rx_buffer) {
        ESP_LOGE(TAG, "Failed to parse float from: '%s'", rx_buffer);
        return ESP_ERR_INVALID_RESPONSE;
    }

    out_temp = val;
    return ESP_OK;
}
