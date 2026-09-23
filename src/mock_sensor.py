import serial
import time
import random

# Harici USB-TTL modülünün portu ve baudrate ayarı
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200

def main():
    print(f"[MOCK SENSOR] Port açılıyor: {SERIAL_PORT} @ {BAUD_RATE} baud...")
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1.0)
    except serial.SerialException as e:
        print(f"[HATA] Seri porta erişilemedi: {e}")
        print("İpucu: Port izinleri için 'sudo chmod 666 /dev/ttyACM0' gerekebilir.")
        return

    print("[MOCK SENSOR] Sensör hazır! ESP32'den istek bekleniyor (Komut: 'T')...")
    
    base_temp = 24.5  # Başlangıç sıcaklığı (°C)

    try:
        while True:
            # ESP32'den komut byte'ı bekle
            if ser.in_waiting > 0:
                cmd = ser.read(1).decode('utf-8', errors='ignore')
                
                if cmd == 'T':  # 'T' -> Temperature oku komutu
                    # Gerçekçi görünmesi için küçük bir gürültü/dalgalanma ekle (+-0.3 °C)
                    current_temp = base_temp + random.uniform(-0.3, 0.3)
                    
                    # Veriyi ESP32'ye string formatında satır sonu ile gönder (Örn: "24.62\n")
                    response = f"{current_temp:.2f}\n"
                    ser.write(response.encode('utf-8'))
                    
                    print(f"[MOCK SENSOR] 'T' isteği alındı -> Gönderilen Sıcaklık: {current_temp:.2f} °C")
            
            time.sleep(0.01)  # CPU'yu %100 yapmamak için minik bekleme

    except KeyboardInterrupt:
        print("\n[MOCK SENSOR] Simülasyon durduruldu.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()