#pragma once

#include "driver/uart.h"
#include "esp_err.h"

class MockTempSensor {
public:
    // Varsayılan: UART1, TX: GPIO 4, RX: GPIO 5, 115200 Baud
    MockTempSensor(uart_port_t uart_num = UART_NUM_1, 
                   int tx_pin = 4, 
                   int rx_pin = 5, 
                   int baud_rate = 115200);

    // Sensörü ve UART donanımını başlatan fonksiyon
    esp_err_t init();

    // Hata kontrollü sıcaklık okuma (Başarılıysa ESP_OK döner)
    esp_err_t readTemperature(float &out_temp);

    // Sadeleştirilmiş doğrudan sıcaklık değeri döndüren arayüz
    float getTemperature() {
        float temp = 0.0f;
        return (readTemperature(temp) == ESP_OK) ? temp : -999.0f;
    }

private:
    uart_port_t uart_num_;
    int tx_pin_;
    int rx_pin_;
    int baud_rate_;
};