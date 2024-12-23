#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>

const char* serverAddress = "http://192.168.1.2454/getDeviceParams.php?firmwareName=okul_sensor_100&firmwareVersion=100";

WiFiManager wifiManager;
HTTPClient http;

void setup() {
  Serial.begin(115200);

  wifiManager.autoConnect(); // WiFi bağlantısını sağla

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi bağlantısı başarılı!");
    checkAndUpdateFirmware();
  } else {
    Serial.println("WiFi bağlantısı sağlanamadı!");
  }
}

void loop() {
  // Diğer işlemleri buraya ekleyebilirsiniz
}

void checkAndUpdateFirmware() {
  if (http.begin(serverAddress)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Sunucudan gelen JSON verisi: " + payload);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.println("JSON verisi işlenirken hata oluştu!");
          return;
        }

        bool updateRequired = doc["updateRequired"];
        String fileURL = doc["fileURL"];
        bool restart = doc["restart"];

        if (updateRequired) {
          Serial.println("Firmware güncellemesi gerekiyor.");
          updateFirmwareFromURL(fileURL);
        } else {
          Serial.println("Firmware güncellemesi gerekmeyiyor.");
        }

        if (restart) {
          Serial.println("MCU yeniden başlatılıyor...");
          ESP.restart();
        }
      } else {
        Serial.println("HTTP isteği başarısız, HTTP kodu: " + String(httpCode));
      }
    } else {
      Serial.println("HTTP isteği yapılamadı!");
    }
    http.end();
  } else {
    Serial.println("HTTP bağlantısı başlatılamadı!");
  }
}

void updateFirmwareFromURL(String fileURL) {
  // Firmware güncelleme işlemini buraya ekleyebilirsiniz
}
