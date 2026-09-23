#include "MockTempSensor.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN_APP";

extern "C" void app_main() {
    MockTempSensor sensor;
    if (sensor.init() != ESP_OK) {
        ESP_LOGE(TAG, "Sensor baslatilamadi!");
        return;
    }

    ESP_LOGI(TAG, "Sensor baslatildi. Periyodik okuma basliyor...");

    while (true) {
        float temp = sensor.getTemperature();
        if (temp > -900.0f) {
            ESP_LOGI(TAG, "Okunan Sicaklik: %.2f °C", temp);
        } else {
            ESP_LOGW(TAG, "Sensor okuma basarisiz!");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}