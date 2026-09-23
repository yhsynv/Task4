#pragma once

#include "driver/uart.h"
#include "esp_err.h"

class MockTempSensor {
public:
    explicit MockTempSensor(uart_port_t uart_num = UART_NUM_1)
        : uart_num_(uart_num) {}

    // Sensör ve UART donanımını başlat (varsayılan: TX 4, RX 5, 115200 baud)
    esp_err_t init(int tx_pin = 4, int rx_pin = 5, int baud_rate = 115200);

    // Hata kontrollü sıcaklık okuma
    esp_err_t readTemperature(float &out_temp);

    // Doğrudan sıcaklık dönen sade arayüz (Hata durumunda -999.0f)
    float getTemperature() {
        float temp = 0.0f;
        return (readTemperature(temp) == ESP_OK) ? temp : -999.0f;
    }

private:
    uart_port_t uart_num_;
};