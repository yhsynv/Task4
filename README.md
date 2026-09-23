# ESP32-C6 Sensör Sürücüsü & Donanım Soyutlama (HAL) Mock Projesi

Bu proje, fiziksel sensör donanımının bulunmadığı senaryolarda **Donanım Soyutlama Katmanı (HAL)** ve **Mocking** mimarisi kullanılarak geliştirilmiş ESP-IDF tabanlı C++ sensör sürücüsüdür. ESP32-C6, harici bir USB-TTL dönüştürücü üzerinden PC'de çalışan Python simülasyonu ile UART protokolüyle haberleşir.

## Mimari ve Kapsülleme (Encapsulation)

```
[ app_main ] 
     │ (Sadece sensor.getTemperature() çağırır)
     ▼
[ MockTempSensor ] (HAL / C++ Sınıfı)
     │ (Arka planda UART paketleme, timeout, baudrate gizlidir)
     ▼
[ UART1 (GPIO 4 TX / GPIO 5 RX) ] 
     │
     ▼ (Harici USB-TTL Modülü)
[ PC: mock_sensor.py ]
```

* `app_main` içinde hiçbir UART fonksiyonu, donanım register adresi veya seri port okuma mantığı yer almaz.
* `MockTempSensor` sınıfı dış dünyaya yalnızca `init()` ve `getTemperature()` arayüzlerini sunar.
* Tüm sabitler ve yapıcı fonksiyon (constructor) `constexpr` olarak tanımlanarak derleme zamanında (compile-time) çözülmüş, çalışma zamanı RAM maliyeti sıfırlanmıştır.

## Donanım Bağlantı Tablosu

| ESP32-C6 Pin | USB-TTL Modülü | Açıklama |
|---|---|---|
| **GPIO 4 (TX)** | **RX** | ESP32 veri gönderim pini |
| **GPIO 5 (RX)** | **TX** | ESP32 veri alım pini |
| **GND** | **GND** | Ortak voltaj referansı |
| *VCC* | *Bağlanmaz* | Kartlar enerjiyi kendi USB portlarından alır |

* **Port 1:** ESP32-C6 dahili USB portu (Kod yükleme ve `ESP_LOGI` konsolu).
* **Port 2:** Harici USB-TTL modülü (Python mock sensör haberleşme kanalı).

## Çalıştırma Adımları

1. **Python Mock Sensörünü Başlatın (PC):**
   ```bash
   python3 mock_sensor.py
   ```
   *(Gereksinim: `pip install pyserial`)*

2. **Firmware'i Derleyin, Yükleyin ve Logları İzleyin:**
   ```bash
   pio run -t upload -t monitor
   ```

---

## Araştırma Soruları ve Teknik Analiz

### 1. Donanım (I2C Fiziği): Pull-up Dirençleri ve Open-Drain Mimarisi
I2C hatları (SDA ve SCL) **Open-Drain (Açık Kollektör)** yapısındadır. Veri yoluna bağlı cihazlar hattı sadece **LOW (0V / GND)** seviyesine çekebilir; aktif olarak HIGH (3.3V) basamazlar. Hattın HIGH seviyesine çıkması harici **Pull-up dirençleri** ile sağlanır.
* **Dirençler Olmazsa:** Hat serbest bırakıldığında elektriksel olarak havada asılı kalır (floating / high-impedance). Parazitik kapasitanslar hattın lojik-1 seviyesine hızla yükselmesini engeller (aşırı rise-time), sinyal formu bozulur ve haberleşme kilitlenir.

### 2. Mimari (HAL Konsepti): Donanım Soyutlama Katmanı
HAL, uygulama katmanını (`app_main`) doğrudan donanım kütüklerinden (register) ve fiziksel arayüzlerden (I2C, SPI, UART) izole eden yazılım katmanıdır.
* **Mocking:** Fiziksel donanım mevcut değilken ya da CI/CD otomatik testlerinde sanal veri kaynaklarıyla test yapılmasını sağlar.
* **Taşınabilirlik:** Sensör veya haberleşme protokolü değiştiğinde ana iş mantığına dokunulmaz; yalnızca HAL sürücüsü güncellenir.

### 3. İleri Seviye C++ (Kapsülleme / Encapsulation)
Alt seviye fonksiyonların (register erişimi, buffer yönetimi) `private` yapılması **Bilgi Gizleme (Information Hiding)** ilkesidir.
* Dış uygulamanın donanım fonksiyonlarına doğrudan erişmesi engellenmezse; donanım durum makinesi bozulabilir, thread-safety riskleri doğar ve yazılım donanıma bağımlı (tightly-coupled) hale gelir.

### 4. Sürüm Kontrol (Git): Binary Dosyalarının Dışlanması
Derleme çıktıları (`.elf`, `.bin`, `.o` veya `build/`, `.pio/` klasörleri) repoya atılmamalıdır:
* **Repo Şişmesi:** Binary dosyalar her derlemede tamamen değişir. Git metin tabanlı fark (diff) algoritmalarını binary üzerinde çalıştıramaz ve repo boyutu kontrolsüzce büyür.
* **Çakışmalar (Merge Conflicts):** Farklı geliştirici ortamlarında (Linux/Windows, farklı derleyici sürümleri) derlenen binary'ler birbirini ezer ve çözülemez çakışmalara yol açar.
* Kaynak kod ve konfigürasyon dosyaları projenin **Tek Gerçek Kaynağıdır (Single Source of Truth)**; binary'ler her zaman kaynak koddan yeniden üretilebilir.
