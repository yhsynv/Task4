#pragma once

#include "driver/uart.h"
#include "esp_err.h"

class MockTempSensor {
public:
    // Derleme zamanı (Compile-time) sabitleri - Sıfır çalışma zamanı/RAM maliyeti
    static constexpr int DEFAULT_TX_PIN       = 4;
    static constexpr int DEFAULT_RX_PIN       = 5;
    static constexpr int DEFAULT_BAUD_RATE    = 115200;
    static constexpr uint32_t TIMEOUT_MS      = 500;
    static constexpr char CMD_REQ_TEMP        = 'T';
    static constexpr float INVALID_TEMP       = -999.0f;

    // constexpr constructor: Nesne oluşturma derleme zamanında çözülür
    constexpr explicit MockTempSensor(uart_port_t uart_num = UART_NUM_1)
        : uart_num_(uart_num) {}

    esp_err_t init(int tx_pin = DEFAULT_TX_PIN, 
                   int rx_pin = DEFAULT_RX_PIN, 
                   int baud_rate = DEFAULT_BAUD_RATE);

    esp_err_t readTemperature(float &out_temp);

    float getTemperature() {
        float temp = 0.0f;
        return (readTemperature(temp) == ESP_OK) ? temp : INVALID_TEMP;
    }

private:
    uart_port_t uart_num_;
};