#pragma once

#include "driver/uart.h"
#include "esp_err.h"

class MockTempSensor {
public:
    // Varsayılan olarak UART1, TX: GPIO 4, RX: GPIO 5 ve 115200 baud
    MockTempSensor(uart_port_t uart_num = UART_NUM_1, 
                   int tx_pin = 4, 
                   int rx_pin = 5, 
                   int baud_rate = 115200);

    // Sensörü ve UART donanımını başlatan fonksiyon
    esp_err_t init();

    // Dış dünyanın kullanacağı fonksiyon:
    // Başarılı olursa ESP_OK döner ve out_temp içine sıcaklığı yazar
    esp_err_t readTemperature(float &out_temp);

private:
    uart_port_t uart_num_;
    int tx_pin_;
    int rx_pin_;
    int baud_rate_;

    // Dışarıya kapalı (private) yardımcı haberleşme metotları:
    esp_err_t sendCommand(char cmd);
    int readLine(char *buffer, size_t max_len, uint32_t timeout_ms);
};