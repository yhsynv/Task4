#include "MockTempSensor.hpp"
#include <cstdlib>

esp_err_t MockTempSensor::init(int tx_pin, int rx_pin, int baud_rate) {
    uart_config_t config = {};
    config.baud_rate = baud_rate;
    config.data_bits = UART_DATA_8_BITS;
    config.parity    = UART_PARITY_DISABLE;
    config.stop_bits = UART_STOP_BITS_1;
    config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    config.source_clk = UART_SCLK_DEFAULT;

    esp_err_t err = uart_param_config(uart_num_, &config);
    if (err != ESP_OK) return err;

    err = uart_set_pin(uart_num_, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) return err;

    return uart_driver_install(uart_num_, 256, 0, 0, NULL, 0);
}

esp_err_t MockTempSensor::readTemperature(float &out_temp) {
    uart_flush_input(uart_num_);

    // 1. Python'a compile-time sabit komutunu gönder ('T')
    if (uart_write_bytes(uart_num_, &CMD_REQ_TEMP, 1) != 1) {
        return ESP_FAIL;
    }

    // 2. Python'dan gelen cevabı bekle (TIMEOUT_MS compile-time sabitidir)
    char buffer[32];
    int len = uart_read_bytes(uart_num_, buffer, sizeof(buffer) - 1, pdMS_TO_TICKS(TIMEOUT_MS));
    if (len <= 0) {
        return ESP_ERR_TIMEOUT;
    }

    buffer[len] = '\0';
    out_temp = strtof(buffer, nullptr);
    return ESP_OK;
}
