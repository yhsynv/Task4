#include "MockTempSensor.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN_APP";

extern "C" void app_main() {
    ESP_LOGI(TAG, "Sistem baslatiliyor...");

    // C++ Sensör sınıfımızın nesnesini oluşturuyoruz.
    // Varsayılan: UART_NUM_1, TX: GPIO 4, RX: GPIO 5, 115200 Baud
    MockTempSensor sensor;

    // Sensörü ve alt seviye donanımı başlat
    esp_err_t err = sensor.init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Sensor baslatilamadi! Hata: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Sensor basariyla baslatildi. Periyodik okuma dongusu basliyor...");

    // Ana döngü: Saniyede bir kez sıcaklık oku ve logla
    while (true) {
        float temperature = 0.0f;
        
        // app_main içinde HİÇBİR UART veya donanım register detayı yok!
        // Yalnızca sade ve temiz sensor.readTemperature() arayüzü çağrılır.
        if (sensor.readTemperature(temperature) == ESP_OK) {
            ESP_LOGI(TAG, "Okunan Sicaklik: %.2f °C", temperature);
        } else {
            ESP_LOGW(TAG, "Sensor okuma basarisiz oldu veya zaman asimina ugradi.");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}