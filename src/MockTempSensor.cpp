#include "MockTempSensor.hpp"
#include <cstdlib>

MockTempSensor::MockTempSensor(uart_port_t uart_num, int tx_pin, int rx_pin, int baud_rate)
    : uart_num_(uart_num), tx_pin_(tx_pin), rx_pin_(rx_pin), baud_rate_(baud_rate) {}

esp_err_t MockTempSensor::init() {
    uart_config_t config = {};
    config.baud_rate = baud_rate_;
    config.data_bits = UART_DATA_8_BITS;
    config.parity    = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_DEFAULT;

    esp_err_t err = uart_param_config(uart_num_, &config);
    if (err != ESP_OK) return err;

    err = uart_set_pin(uart_num_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) return err;

    return uart_driver_install(uart_num_, 256, 0, 0, NULL, 0);
}

esp_err_t MockTempSensor::readTemperature(float &out_temp) {
    uart_flush_input(uart_num_); // Önceki kalıntı verileri temizle

    // 1. Python mock sensörüne 'T' istek komutunu gönder
    if (uart_write_bytes(uart_num_, "T", 1) != 1) {
        return ESP_FAIL;
    }

    // 2. Python'dan gelen cevabı bekle (500 ms zaman aşımı)
    char buffer[32];
    int len = uart_read_bytes(uart_num_, buffer, sizeof(buffer) - 1, pdMS_TO_TICKS(500));
    if (len <= 0) {
        return ESP_ERR_TIMEOUT;
    }

    buffer[len] = '\0';
    out_temp = strtof(buffer, nullptr);
    return ESP_OK;
}
