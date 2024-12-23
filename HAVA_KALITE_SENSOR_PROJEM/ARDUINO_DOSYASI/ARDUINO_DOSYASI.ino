#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "ESP8266HTTPClient.h"
#include "ESP8266HTTPUpdate.h"
#include <WiFiManager.h> // WiFiManager kütüphanesi
#include <DHTesp.h> // DHT sensör kütüphanesi
#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif
bool serialGoster= false;

DHTesp dht;
WiFiManager wifiManager;
WiFiClient espClient;

char url_1[255] = "https://haysaf.com/sensor.php"; // Sunucu3 URL'si
char url_2[255] = "http://haysaf.com/sensor.php"; // Sunucu2 URL'si
char url_3[255] = "http://haysaf.com/sensor:1883"; // Sunucu2 URL'si
char url_4[255] = "https://haysaf.com/sensor:8883"; // Sunucu2 URL'si
char url_5[255] = "http://192.168.1.254/sensor.php"; // Sunucu1 URL'si
char url_6[255] = "http://192.168.1.254/sensor:1883"; // Sunucu2 URL'si

const char* api_key = "SDd53X92"; // API anahtarı
const char* update_url = "http://haysaf.com/update"; // Güncelleme URL'si
float wifiSignalPower = 20;  // esp wifi sinyal gücü min 0 dbm max 20.5 dbm

const int dhtPin = 5;
const int analogPin = 0;
const int led_wifi_connected = 18;  //d8
const int led_server_asked = 13;    // D7
const int led_server_answered = 12; // D6
const int led_findme = 4;           // D2
const int pin_relay = 0;            // D3

unsigned int delay_time = 2000;
bool relay_status = false;

/*
yapılamayanlar:
8,15,20
5a , 8b
güncellenenler
 2,  mudur odası,11,10,16,18,13   ,7,1,6
*/

void connectWifi() {

  //önce bir süre bağlanmayı bekle.
  uint8_t sayac=0;
  while (WiFi.status() != WL_CONNECTED ){
     
     if (WiFi.status() == WL_CONNECTED) {
        if(serialGoster) Serial.println("\nWiFi Bağlandı!");
        return;
      }
    sayac++;
    Serial.print(".");
    delay(20);
    if(sayac>10) break;
  }


  if (WiFi.status() != WL_CONNECTED) {
    //bağlanmadıysa aşağıda 30 saniye bağlantı bekle bağlanmazsa  ap aç şifre iste 180 de girilmezse resetle
    wifiManager.setTimeout(120);        // saniye cinsinden diyalog penceresinde bekle, giriş olmazsa sonraki işleme geç
    wifiManager.setConnectTimeout(10);  // 10 saniye boyunca bağlantı denenecek her denemede 10 saniye bekle
    if (wifiManager.autoConnect("Hava Kalite-Haysaf.com")) return;
    else ESP.restart();
  }

}

void findme(uint8_t durum) {
  if (durum == 1) {
    digitalWrite(led_server_asked, LOW);
    digitalWrite(led_server_answered, LOW);
    digitalWrite(led_wifi_connected, LOW);
    for (int i = 0; i < 50; i++) {
      digitalWrite(led_findme, HIGH);
      delay(50);
      digitalWrite(led_findme, LOW);
      delay(50);
    }
  } else {
    digitalWrite(led_wifi_connected, HIGH);
  }
}

String deviceID = "";
void setup_deviceID() {
  uint8_t mac_address[6]; 
  WiFi.macAddress(mac_address);
  for (int i = 0; i < 6; ++i) {
    deviceID += String(mac_address[i], 16);
  }
}


float readHumidity() {
  float humidity = dht.getHumidity();
  int retry = 5;
  while (isnan(humidity) && retry > 0) {
    delay(2000); // 2 saniye bekle ve tekrar dene
    humidity = dht.getHumidity();
    retry--;
  }
  return humidity;
}

float readTemperature() {
  float temperature = dht.getTemperature();
  int retry = 5;
  while (isnan(temperature) && retry > 0) {
    delay(2000); // 2 saniye bekle ve tekrar dene
    temperature = dht.getTemperature();
    retry--;
  }
  return temperature;
}

void performHTTPReq(const char* url) {
  HTTPClient http;
  float humidity = roundf(readHumidity() * 10) / 10;
  float temperature = roundf(readTemperature() * 10) / 10;
  float felt = roundf(dht.computeHeatIndex(temperature, humidity, false) * 10) / 10;
  relay_status = digitalRead(pin_relay);
  String getUrl = String(url) +
                  "?id=" + deviceID +
                   "&tmp=" + String(temperature) +
                   "&felt=" + String(felt) +
                   "&hum=" + String(humidity) +
                   "&relay=" + String(relay_status) +
                   "&airQ=" + String(analogRead(analogPin)) +
                   "&api_key=" + api_key +
                   "&volt=" + String(analogRead(analogPin) * (5.0 / 1023.0));
  if(serialGoster) Serial.println(getUrl);
  if (http.begin(espClient, getUrl)) {
    digitalWrite(led_wifi_connected, HIGH);
    int httpCode = http.GET();
    digitalWrite(led_server_asked, HIGH); delay(100); digitalWrite(led_server_asked, LOW);
    if (httpCode > 0) {
      String payload = http.getString();
      if(serialGoster) Serial.println("HTTP isteği başarılı. Sonuç: " + payload);
      digitalWrite(led_server_answered, HIGH); delay(100); digitalWrite(led_server_answered, LOW);

      // GET ile sunucudan alınan "relay" değerini kontrol et
      int relay_index = payload.indexOf("relay=");
      if (relay_index != -1) { // Eğer "relay" değeri bulunursa
        relay_index += 6; // "relay" kelimesinden sonraki değeri almak için index'i güncelle
        char relay_char = payload.charAt(relay_index); // "relay" değerini al
        relay_status = relay_char - '0'; // ASCII'den uint8_t'ye dönüştür
        if (relay_status == 1) {
          digitalWrite(pin_relay, HIGH); // röle durumunu güncelle
        } else {
          digitalWrite(pin_relay, LOW); // röleyi kapat (LOW yap)
        }
      } else {
        // Eğer "relay" değeri bulunamazsa röleyi kapat (LOW yap)
        digitalWrite(pin_relay, LOW);
      }

      // Diğer kontroller ve işlemler (findme ve period)

    } else {
      if(serialGoster) Serial.println("HTTP isteği başarısız");
      digitalWrite(led_server_asked, LOW);
      digitalWrite(led_server_answered, LOW);
      digitalWrite(pin_relay, LOW); // HTTP isteği başarısızsa röleyi kapat (LOW yap)
    }
    http.end();
  } else {
    if(serialGoster) Serial.println("HTTP bağlantısı başlatılamadı");
    digitalWrite(led_server_asked, LOW);
    digitalWrite(led_server_answered, LOW);
    digitalWrite(pin_relay, LOW); // HTTP bağlantısı başlatılamadıysa röleyi kapat (LOW yap)
  }
}



void setup() {
  WiFi.setOutputPower(wifiSignalPower);  // Güç seviyesini wifiSignalPower değişkeninde ki sayı kadar  dBm olarak ayarlar
  setup_deviceID();
  if(serialGoster) Serial.begin(115200);
  pinMode(pin_relay, OUTPUT);
  pinMode(led_findme, OUTPUT);
  pinMode(led_wifi_connected, OUTPUT);
  pinMode(led_server_asked, OUTPUT);
  pinMode(led_server_answered, OUTPUT);
  digitalWrite(led_wifi_connected, LOW);
  digitalWrite(led_server_asked, LOW);
  digitalWrite(led_server_answered, LOW);
  digitalWrite(led_findme, LOW);
  digitalWrite(pin_relay, LOW);

 
  // Arayüz üzerinde URL girişi için input alanı oluştur
  WiFiManagerParameter custom_url("server", "Server URL", url_1, 255);
  // WiFiManager'a input alanını ekleyin
  wifiManager.addParameter(&custom_url);
  wifiManager.setTimeout(120); // 2 dakika timeout
  wifiManager.autoConnect("Hava Kalite-Haysaf.com"); // Bağlantıyı başlat ve erişim noktası adını ayarla
  // Bağlantı başarılı olduğunda URL'yi oku
  strncpy(const_cast<char*>(url_1), custom_url.getValue(), 255);
  // Serial Monitöre URL'yi yazdır
  if(serialGoster) Serial.println("Entered URL: " + String(url_1));

  // Eğer bağlantı başarılıysa
  if (WiFi.status() == WL_CONNECTED) {
    if(serialGoster) Serial.println("WiFi bağlı");
    digitalWrite(led_wifi_connected, HIGH);
    delay(2000);

    // DHT sensörünü başlat
    dht.setup(dhtPin, DHTesp::DHT11);

  } else {
    // Bağlantı başarısızsa cihazı yeniden başlat
    if(serialGoster) Serial.println("WiFi Bağlanamadı. Cihaz yeniden başlatılıyor...");
    delay(1000);
    ESP.restart();
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led_wifi_connected, LOW);
    digitalWrite(pin_relay, LOW); // WiFi bağlı değilse röleyi kapat (LOW yap)
    connectWifi();
     
  }else {
    performHTTPReq(url_1); delay(delay_time);
    performHTTPReq(url_2); delay(delay_time);
    performHTTPReq(url_3); delay(delay_time);
    performHTTPReq(url_4); delay(delay_time);
    performHTTPReq(url_5); delay(delay_time);
    performHTTPReq(url_6); delay(delay_time);

  }  
}



// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
// #include <DNSServer.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiManager.h> // WiFiManager kütüphanesi

// #include <DHTesp.h> // DHT sensör kütüphanesi
// #ifdef ESP32
// #pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
// #error Select ESP8266 board.
// #endif

// DHTesp dht;
// char url_1[255] = "https://haysaf.com/sensor.php"; // Sunucu3 URL'si
// char url_2[255] = "http://haysaf.com/sensor.php"; // Sunucu2 URL'si
// char url_3[255] = "http://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_4[255] = "https://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_5[255] = "http://192.168.1.254/sensor.php"; // Sunucu1 URL'si
// char url_6[255] = "http://192.168.1.254/sensor"; // Sunucu2 URL'si

// const char* api_key = "SDd53X92"; // API anahtarı

// const int dhtPin = 5;
// const int analogPin = 0;
// const int led_wifi_connected = 18;  //d8
// const int led_server_asked = 13;    // D7
// const int led_server_answered = 12; // D6
// const int led_findme = 4;           // D2
// const int pin_relay = 0;            // D3

// unsigned int delay_time = 2000;
// bool relay_status = false;

// void findme(uint8_t durum) {
//   if (durum == 1) {
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//     digitalWrite(led_wifi_connected, LOW);
//     for (int i = 0; i < 50; i++) {
//       digitalWrite(led_findme, HIGH);
//       delay(50);
//       digitalWrite(led_findme, LOW);
//       delay(50);
//     }
//   } else {
//     digitalWrite(led_wifi_connected, HIGH);
//   }
// }

// String deviceID = "";
// void setup_deviceID() {
//   uint8_t mac_address[6]; 
//   WiFi.macAddress(mac_address);
//   for (int i = 0; i < 6; ++i) {
//     deviceID += String(mac_address[i], 16);
//   }
// }

// WiFiClient espClient;

// void performHTTPReq(const char* url) {
//   HTTPClient http;
//   float humidity = roundf(dht.getHumidity() * 10) / 10;
//   float temperature = roundf(dht.getTemperature() * 10) / 10;
//   float felt = roundf(dht.computeHeatIndex(temperature, humidity, false) * 10) / 10;
//   relay_status = digitalRead(pin_relay);
//   String getUrl = String(url) +
//                   "?id=" + deviceID +
//                    "&tmp=" + String(temperature) +
//                    "&felt=" + String(felt) +
//                    "&hum=" + String(humidity) +
//                    "&relay=" + String(relay_status) +
//                    "&airQ=" + String(analogRead(analogPin)) +
//                    "&api_key=" + api_key +
//                    "&volt=" + String(analogRead(analogPin) * (5.0 / 1023.0));
//   if(serialGoster) Serial.println(getUrl);
//   if (http.begin(espClient, getUrl)) {
//     digitalWrite(led_wifi_connected, HIGH);
//     int httpCode = http.GET();
//     digitalWrite(led_server_asked, HIGH); delay(100); digitalWrite(led_server_asked, LOW);
//     if (httpCode > 0) {
//       String payload = http.getString();
//       if(serialGoster) Serial.println("HTTP isteği başarılı. Sonuç: " + payload);
//       digitalWrite(led_server_answered, HIGH); delay(100); digitalWrite(led_server_answered, LOW);

//       // GET ile sunucudan alınan "relay" değerini kontrol et
//       int relay_index = payload.indexOf("relay=");
//       if (relay_index != -1) { // Eğer "relay" değeri bulunursa
//         relay_index += 6; // "relay" kelimesinden sonraki değeri almak için index'i güncelle
//         char relay_char = payload.charAt(relay_index); // "relay" değerini al
//         relay_status = relay_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         if (relay_status == 1) {
//           digitalWrite(pin_relay, HIGH); // röle durumunu güncelle
//         } else {
//           digitalWrite(pin_relay, LOW); // röleyi kapat (LOW yap)
//         }
//       } else {
//         // Eğer "relay" değeri bulunamazsa röleyi kapat (LOW yap)
//         digitalWrite(pin_relay, LOW);
//       }

//       // Diğer kontroller ve işlemler (findme ve period)

//     } else {
//       if(serialGoster) Serial.println("HTTP isteği başarısız");
//       digitalWrite(led_server_asked, LOW);
//       digitalWrite(led_server_answered, LOW);
//       digitalWrite(pin_relay, LOW); // HTTP isteği başarısızsa röleyi kapat (LOW yap)
//     }
//     http.end();
//   } else {
//     if(serialGoster) Serial.println("HTTP bağlantısı başlatılamadı");
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//     digitalWrite(pin_relay, LOW); // HTTP bağlantısı başlatılamadıysa röleyi kapat (LOW yap)
//   }
// }

// void setup() {
//   setup_deviceID();
//   Serial.begin(115200);
//   pinMode(pin_relay, OUTPUT);
//   pinMode(led_findme, OUTPUT);
//   pinMode(led_wifi_connected, OUTPUT);
//   pinMode(led_server_asked, OUTPUT);
//   pinMode(led_server_answered, OUTPUT);
//   digitalWrite(led_wifi_connected, LOW);
//   digitalWrite(led_server_asked, LOW);
//   digitalWrite(led_server_answered, LOW);
//   digitalWrite(led_findme, LOW);
//   digitalWrite(pin_relay, LOW);

//   WiFiManager wifiManager; // WiFiManager nesnesi oluştur
//   // Arayüz üzerinde URL girişi için input alanı oluştur

//   WiFiManagerParameter custom_url("server", "Server URL", url_1, 255);
//   // WiFiManager'a input alanını ekleyin
//   wifiManager.addParameter(&custom_url);
//   wifiManager.setTimeout(120); // 2 dakika timeout
//   wifiManager.autoConnect("Hava Kalite-Haysaf.com"); // Bağlantıyı başlat ve erişim noktası adını ayarla
//   // Bağlantı başarılı olduğunda URL'yi oku
//   strncpy(const_cast<char*>(url_1), custom_url.getValue(), 255);
//   // Serial Monitöre URL'yi yazdır
//   if(serialGoster) Serial.println("Entered URL: " + String(url_1));

//   // Eğer bağlantı başarılıysa
//   if (WiFi.status() == WL_CONNECTED) {
//     if(serialGoster) Serial.println("WiFi bağlı");
//     digitalWrite(led_wifi_connected, HIGH);
//     dht.setup(dhtPin, DHTesp::DHT11); // DHT sensörünü başlat
//   } else {
//     if(serialGoster) Serial.println("WiFi bağlı değil");
//     digitalWrite(led_wifi_connected, LOW);
//   }
// }

// void loop() {
//   if (WiFi.status() == WL_CONNECTED) {
//     digitalWrite(led_wifi_connected, HIGH);

//     // Sırasıyla 6 farklı sunucu kombinasyonu deniyor
//     performHTTPReq(url_1); delay(delay_time);
//     performHTTPReq(url_2); delay(delay_time);
//     performHTTPReq(url_3); delay(delay_time);
//     performHTTPReq(url_4); delay(delay_time);
//     performHTTPReq(url_5); delay(delay_time);
//     performHTTPReq(url_6); delay(delay_time);

//   } else {
//     digitalWrite(led_wifi_connected, LOW);
//     digitalWrite(pin_relay, LOW); // WiFi bağlı değilse röleyi kapat (LOW yap)

//     WiFiManager wifiManager;
//     wifiManager.setTimeout(120); // 2 dakika timeout
//     if (!wifiManager.autoConnect("Hava Kalite-Haysaf.com")) { 
//       if(serialGoster) Serial.println("WiFi Bağlanamadı. Cihaz yeniden başlatılıyor...");
//       delay(1000);
//       ESP.restart();
//     }
//   }
// }



// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
// #include <DNSServer.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiManager.h> // WiFiManager kütüphanesi

// #include <DHTesp.h> // DHT sensör kütüphanesi
// #ifdef ESP32
// #pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
// #error Select ESP8266 board.
// #endif

// DHTesp dht;
// char url_1[255] = "https://haysaf.com/sensor.php"; // Sunucu3 URL'si
// char url_2[255] = "http://haysaf.com/sensor.php"; // Sunucu2 URL'si
// char url_3[255] = "http://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_4[255] = "https://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_5[255] = "http://192.168.1.254/sensor.php"; // Sunucu1 1URL'si
// char url_6[255] = "http://192.168.1.254/sensor"; // Sunucu2 URL'si

// const char* api_key = "SDd53X92"; // API anahtarı

// const int dhtPin = 5;
// const int analogPin = 0;
// const int led_wifi_connected = 18;  //d8
// const int led_server_asked =13;// D7;
// const int led_server_answered =12;// D6;
// const int led_findme =4;// D2;
// const int pin_relay =0;// D3;  

// unsigned int delay_time = 2000;
// bool relay_status = false;

// void findme(uint8_t durum) {
//   if (durum == 1) {
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//     digitalWrite(led_wifi_connected, LOW);
//     for (int i = 0; i < 50; i++) {
//       digitalWrite(led_findme, HIGH);
//       delay(50);
//       digitalWrite(led_findme, LOW);
//       delay(50);
//     }
//   } else {
//     digitalWrite(led_wifi_connected, HIGH);
//   }
// }

// String deviceID = "";
// void setup_deviceID() {
//   uint8_t mac_address[6]; 
//   WiFi.macAddress(mac_address);
//   for (int i = 0; i < 6; ++i) {
//     deviceID += String(mac_address[i], 16);
//   }
// }

// WiFiClient espClient;

// void performHTTPReq(const char* url) {
//   HTTPClient http;
//   float humidity = roundf(dht.getHumidity() * 10) / 10;
//   float temperature = roundf(dht.getTemperature() * 10) / 10;
//   float felt = roundf(dht.computeHeatIndex(temperature, humidity, false) * 10) / 10;
//   relay_status = digitalRead(pin_relay);
//   String getUrl = String(url) +
//                   "?id=" + deviceID +
//                    "&tmp=" + String(temperature) +
//                    "&felt=" + String(felt) +
//                    "&hum=" + String(humidity) +
//                    "&relay=" + String(relay_status) +
//                    "&airQ=" + String(analogRead(analogPin)) +
//                    "&api_key=" + api_key +
//                    "&volt=" + String(analogRead(analogPin) * (5.0 / 1023.0));
//   if(serialGoster) Serial.println(getUrl);
//   if (http.begin(espClient, getUrl)) {
//     digitalWrite(led_wifi_connected, HIGH);
//     int httpCode = http.GET();
//       digitalWrite(led_server_asked, HIGH); delay(100); digitalWrite(led_server_asked, LOW);
//     if (httpCode > 0) {
//       String payload = http.getString();
//       if(serialGoster) Serial.println("HTTP isteği başarılı. Sonuç: " + payload);
//       digitalWrite(led_server_answered, HIGH); delay(100); digitalWrite(led_server_answered, LOW);
//       //GET ile sunucudan alınan payload içinde "findme" değerini kontrol et
//       int findme_index = payload.indexOf("findme=");
//       if (findme_index != -1) { // Eğer "findme" değeri bulunursa
//         findme_index += 7; // "findme" kelimesinden sonraki değeri almak için index'i güncelle
//         char findme_char = payload.charAt(findme_index); // "findme" değerini al
//         uint8_t findme_value = findme_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         findme(findme_value); // findme fonksiyonunu çağır
//       }

//       //GET ile sunucudan alınan payload içinde "period" değerini kontrol et
//       int period_index = payload.indexOf("period=");
//       if (period_index != -1) { // Eğer "period" değeri bulunursa
//         period_index += 7; // "period" kelimesinden sonraki değeri almak için index'i güncelle
//         int comma_index = payload.indexOf(",", period_index); // "," işaretini bul
//         String period_str = payload.substring(period_index, comma_index); // period değerini al
//         delay_time = period_str.toInt(); // period değerini integer'a dönüştür ve delay_time değişkenine ata
//       }

//       //GET ile sunucudan alınan "relay" değerini kontrol et
//       int relay_index = payload.indexOf("relay=");
//       if (relay_index != -1) { // Eğer "relay" değeri bulunursa
//         relay_index += 6; // "relay" kelimesinden sonraki değeri almak için index'i günc
//         char relay_char = payload.charAt(relay_index); // "relay" değerini al
//         relay_status = relay_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         digitalWrite(pin_relay, relay_status); // röle durumunu güncelle
//       }

//     } else {
//       if(serialGoster) Serial.println("HTTP isteği başarısız");
//       digitalWrite(led_server_asked, LOW);
//       digitalWrite(led_server_answered, LOW);
//     }
//     http.end();
//   } else {
//     if(serialGoster) Serial.println("HTTP bağlantısı başlatılamadı");
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//   }
// }

// void setup() {
//   setup_deviceID();
//   Serial.begin(115200);
//   pinMode(pin_relay, OUTPUT);
//   pinMode(led_findme, OUTPUT);
//   pinMode(led_wifi_connected, OUTPUT);
//   pinMode(led_server_asked, OUTPUT);
//   pinMode(led_server_answered, OUTPUT);
//   digitalWrite(led_wifi_connected, LOW);
//   digitalWrite(led_server_asked, LOW);
//   digitalWrite(led_server_answered, LOW);
//   digitalWrite(led_findme, LOW);
//   digitalWrite(pin_relay, LOW);

//   WiFiManager wifiManager; // WiFiManager nesnesi oluştur

//   // WiFiManagerParameter custom_url("server", "Server URL", url_1, 255);
//   // wifiManager.addParameter(&custom_url);

//   wifiManager.setTimeout(300); // 5 dakika =300timeout bekle arayüzden şifre girilmezse resetlenip yeniden bağlanmaya çalış
//   if (!wifiManager.autoConnect("Hava Kalite-Haysaf.com")) { 
//     if(serialGoster) Serial.println("WiFi Bağlanamadı. Cihaz yeniden başlatılıyor...");
//     delay(1000);
//     ESP.restart();
//   }

//   if (WiFi.status() == WL_CONNECTED) {
//     if(serialGoster) Serial.println("WiFi bağlı");
//     digitalWrite(led_wifi_connected, HIGH);
//     dht.setup(dhtPin, DHTesp::DHT11); // DHT sensörünü başlat
//   } else {
//     if(serialGoster) Serial.println("WiFi bağlı değil");
//     digitalWrite(led_wifi_connected, LOW);
//   }
// }

// void loop() {
//   if (WiFi.status() == WL_CONNECTED) {
//     digitalWrite(led_wifi_connected, HIGH);

//     // Sırasıyla 6 farklı sunucu kombinasyonu deniyor
//     performHTTPReq(url_1);  delay(delay_time); 
//     performHTTPReq(url_2); delay(delay_time);  
//     performHTTPReq(url_3); delay(delay_time);   
//     performHTTPReq(url_4);  delay(delay_time);   
//     performHTTPReq(url_5);  delay(delay_time);   
//     performHTTPReq(url_6);  delay(delay_time);   

//   } else {
//     digitalWrite(led_wifi_connected, LOW);

//     WiFiManager wifiManager;
//     wifiManager.setTimeout(120); // 2 dakika timeout
//     /*wifiManager.setTimeout(120); kodu, WiFiManager'ın WiFi bağlantısı kurmak için ne kadar süre bekleyeceğini (timeout süresini) belirler. Bu belirli süre (bu örnekte 120 saniye veya 2 dakika) içinde WiFi bağlantısı sağlanamazsa, WiFiManager bağlantı sürecini sonlandırır.*/
//     if (!wifiManager.autoConnect("Hava Kalite-Haysaf.com")) { 
//       if(serialGoster) Serial.println("WiFi Bağlanamadı. Cihaz yeniden başlatılıyor...");
//       delay(1000);
//       ESP.restart();
//     }
//   }
// }



// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
// #include <DNSServer.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiManager.h> // WiFiManager kütüphanesi

// #include <DHTesp.h> // DHT sensör kütüphanesi
// #ifdef ESP32
// #pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
// #error Select ESP8266 board.
// #endif

// DHTesp dht;
// char url_1[255] = "https://haysaf.com/sensor.php"; // Sunucu3 URL'si
// char url_2[255] = "http://haysaf.com/sensor.php"; // Sunucu2 URL'si
// char url_3[255] = "http://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_4[255] = "https://haysaf.com/sensor"; // Sunucu2 URL'si
// char url_5[255] = "http://192.168.1.254/sensor.php"; // Sunucu1 1URL'si
// char url_6[255] = "http://192.168.1.254/sensor"; // Sunucu2 URL'si

// const char* api_key = "SDd53X92"; // API anahtarı

// const int dhtPin = 5;
// const int analogPin = 0;
// const int led_wifi_connected = 18;  //d8
// const int led_server_asked =13;// D7;
// const int led_server_answered =12;// D6;
// const int led_findme =4;// D2;
// const int pin_relay =0;// D3;  

// unsigned int delay_time = 2000;
// bool relay_status = false;

// void findme(uint8_t durum) {
//   if (durum == 1) {
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//     digitalWrite(led_wifi_connected, LOW);
//     for (int i = 0; i < 50; i++) {
//       digitalWrite(led_findme, HIGH);
//       delay(50);
//       digitalWrite(led_findme, LOW);
//       delay(50);
//     }
//   }else{
//     digitalWrite(led_wifi_connected, HIGH);
    
//   }
// }

// String deviceID = "";
// void setup_deviceID() {
//   uint8_t mac_address[6]; 
//   WiFi.macAddress(mac_address);
//   for (int i = 0; i < 6; ++i) {
//     deviceID += String(mac_address[i], 16);
//   }
// }

// WiFiClient espClient;


// void performHTTPReq(const char* url) {
//   HTTPClient http;
//   float humidity = roundf(dht.getHumidity() * 10) / 10;
//   float temperature = roundf(dht.getTemperature() * 10) / 10;
//   float felt = roundf(dht.computeHeatIndex(temperature, humidity, false) * 10) / 10;
//   relay_status = digitalRead(pin_relay);
//   String getUrl = String(url) +
//                   "?id=" + deviceID +
//                    "&tmp=" + String(temperature) +
//                    "&felt=" + String(felt) +
//                    "&hum=" + String(humidity) +
//                    "&relay=" + String(relay_status) +
//                    "&airQ=" + String(analogRead(analogPin)) +
//                    "&api_key=" + api_key +
//                    "&volt=" + String(analogRead(analogPin) * (5.0 / 1023.0));
//   if(serialGoster) Serial.println(getUrl);
//   if (http.begin(espClient, getUrl)) {
//     digitalWrite(led_wifi_connected, HIGH);
//     int httpCode = http.GET();
//       digitalWrite(led_server_asked, HIGH); delay(100); digitalWrite(led_server_asked, LOW);
//     if (httpCode > 0) {
//       String payload = http.getString();
//       if(serialGoster) Serial.println("HTTP isteği başarılı. Sonuç: " + payload);
//       digitalWrite(led_server_answered, HIGH); delay(100); digitalWrite(led_server_answered, LOW);
//       //GET ile sunucudan alınan payload içinde "findme" değerini kontrol et
//       int findme_index = payload.indexOf("findme=");
//       if (findme_index != -1) { // Eğer "findme" değeri bulunursa
//         findme_index += 7; // "findme" kelimesinden sonraki değeri almak için index'i güncelle
//         char findme_char = payload.charAt(findme_index); // "findme" değerini al
//         uint8_t findme_value = findme_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         findme(findme_value); // findme fonksiyonunu çağır
//       }

//       //GET ile sunucudan alınan payload içinde "period" değerini kontrol et
//       int period_index = payload.indexOf("period=");
//       if (period_index != -1) { // Eğer "period" değeri bulunursa
//         period_index += 7; // "period" kelimesinden sonraki değeri almak için index'i güncelle
//         int comma_index = payload.indexOf(",", period_index); // "," işaretini bul
//         String period_str = payload.substring(period_index, comma_index); // period değerini al
//         delay_time = period_str.toInt(); // period değerini integer'a dönüştür ve delay_time değişkenine ata
//       }

//       //GET ile sunucudan alınan "relay" değerini kontrol et
//       int relay_index = payload.indexOf("relay=");
//       if (relay_index != -1) { // Eğer "relay" değeri bulunursa
//         relay_index += 6; // "relay" kelimesinden sonraki değeri almak için index'i günc
//         char relay_char = payload.charAt(relay_index); // "relay" değerini al
//         relay_status = relay_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         digitalWrite(pin_relay, relay_status); // röle durumunu güncelle
//       }

//     } else {
//       if(serialGoster) Serial.println("HTTP isteği başarısız");
//       digitalWrite(led_server_asked, LOW);
//       digitalWrite(led_server_answered, LOW);
//     }
//     http.end();
//   } else {
//     if(serialGoster) Serial.println("HTTP bağlantısı başlatılamadı");
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//   }
// }
// void setup() {
   
//   setup_deviceID();
//   Serial.begin(115200);
//   pinMode(pin_relay, OUTPUT);
//   pinMode(led_findme, OUTPUT);
//   pinMode(led_wifi_connected, OUTPUT);
//   pinMode(led_server_asked, OUTPUT);
//   pinMode(led_server_answered, OUTPUT);
//   digitalWrite(led_wifi_connected, LOW);
//   digitalWrite(led_server_asked, LOW);
//   digitalWrite(led_server_answered, LOW);
//   digitalWrite(led_findme, LOW);
//   digitalWrite(pin_relay, LOW);

//   WiFiManager wifiManager; // WiFiManager nesnesi oluştur
//   // Arayüz üzerinde URL girişi için input alanı oluştur

//   // WiFi.disconnect(true); eski ssid ve wifi parolalarını sil
//   // wifiManager.resetSettings();

//   WiFiManagerParameter custom_url("server", "Server URL", url_1, 255);
//   // WiFiManager'a input alanını ekleyin
//   wifiManager.addParameter(&custom_url);
//   wifiManager.autoConnect("Hava Kalite-Haysaf.com"); // Bağlantıyı başlat ve erişim noktası adını ayarla
//  // Bağlantı başarılı olduğunda URL'yi oku
//   strncpy(const_cast<char*>(url_1), custom_url.getValue(), 255);
//   // Serial Monitöre URL'yi yazdır
//   if(serialGoster) Serial.println("Entered URL: " + String(url_1));

  
//   // Eğer bağlantı başarılıysa
//   if (WiFi.status() == WL_CONNECTED) {
//     if(serialGoster) Serial.println("WiFi bağlı");
//     digitalWrite(led_wifi_connected, HIGH);
//     dht.setup(dhtPin, DHTesp::DHT11); // DHT sensörünü başlat
//   } else {
//     if(serialGoster) Serial.println("WiFi bağlı değil");
//     digitalWrite(led_wifi_connected, LOW);
//   }
// }

// void loop() {
//   if (WiFi.status() == WL_CONNECTED) {
//     digitalWrite(led_wifi_connected, HIGH);

// //sırasıyla 6 farklı sunucu kombinasyonu deniyor
//     performHTTPReq(url_1);  delay(delay_time); 
//     performHTTPReq(url_2); delay(delay_time);  
//     performHTTPReq(url_3); delay(delay_time);   
//     performHTTPReq(url_4);  delay(delay_time);   
//     performHTTPReq(url_5);  delay(delay_time);   
//     performHTTPReq(url_6);  delay(delay_time);   

//   } else {
//     digitalWrite(led_wifi_connected, LOW);
//   }
// }







// ***********************eski kod aşağıda çalışıyor wifimanager yok sadece/////

// #include<Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>
// #include <DNSServer.h>
// #include <ESP8266HTTPClient.h>
// #include <WiFiManager.h> // ortam wifi şifresi için

// #include <DHTesp.h> // Click here to get the library: http://librarymanager/All#DHTesp
// #ifdef ESP32
// #pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
// #error Select ESP8266 board.
// #endif

// DHTesp dht;
// const char* ssid = "cxvnj4w35uR4354ERWR";
// const char* password = "cxvnj4w35uR4354ERWR.+/#!RoS6Qk8C:|";
// const char* url = "http://192.168.1.3/sensor.php";

// const char*  api_key= "SDd53X92";

// const int dhtPin = 5;
// const int analogPin = A0;
// const int led_wifi_connected = D8;
// const int led_server_asked = D7;
// const int led_server_answered = D6;
// const int led_findme= D2;
// const int pin_relay=D3;

// unsigned int delay_time = 2000;
// bool  relay_status=false;

// void findme(uint8_t  durum){
//   if(durum==1){
//     digitalWrite(led_server_asked,0);
//     digitalWrite(led_server_answered,0);
//     // digitalWrite(led_wifi_connected,0);
//     for(int i=0 ; i<50;i++){
//       digitalWrite( led_findme , 1 );delay(50);
//       digitalWrite( led_findme , 0 );delay(50);
//     }
//   }
// }

// String deviceID = "";
// uint8_t mac_address[6];

// WiFiClient espClient;
// void performHTTPReq() {
//   HTTPClient http;
//   float humidity = roundf(dht.getHumidity() * 10) / 10;
//   float temperature = roundf(dht.getTemperature() * 10) / 10;
//   float felt = roundf(dht.computeHeatIndex(temperature, humidity, false) * 10) / 10;
//   relay_status = digitalRead(pin_relay);
//   String getUrl = String(url_1) +
//                   "?id=" + deviceID +
//                    "&tmp=" + String(temperature) +
//                    "&felt=" + String(felt) +
//                    "&hum=" + String(humidity) +
//                    "&relay=" + String(relay_status) +
//                    "&airQ=" + String(analogRead(analogPin)) +
//                    "&api_key=" + api_key +
//                    "&volt=" + String(analogRead(analogPin) * (5.0 / 1023.0));
//   if(serialGoster) Serial.println(getUrl);
//   if (http.begin(espClient, getUrl)) {
//     digitalWrite(led_wifi_connected, HIGH);
//     int httpCode = http.GET();
//       digitalWrite(led_server_asked, HIGH); delay(100); digitalWrite(led_server_asked, LOW);
//     if (httpCode > 0) {
//       String payload = http.getString();
//       if(serialGoster) Serial.println("HTTP isteği başarılı. Sonuç: " + payload);
//       digitalWrite(led_server_answered, HIGH); delay(100); digitalWrite(led_server_answered, LOW);
//       //GET ile sunucudan alınan payload içinde "findme" değerini kontrol et
//       int findme_index = payload.indexOf("findme=");
//       if (findme_index != -1) { // Eğer "findme" değeri bulunursa
//         findme_index += 7; // "findme" kelimesinden sonraki değeri almak için index'i güncelle
//         char findme_char = payload.charAt(findme_index); // "findme" değerini al
//         uint8_t findme_value = findme_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         findme(findme_value); // findme fonksiyonunu çağır
//       }

//       //GET ile sunucudan alınan payload içinde "period" değerini kontrol et
//       int period_index = payload.indexOf("period=");
//       if (period_index != -1) { // Eğer "period" değeri bulunursa
//         period_index += 7; // "period" kelimesinden sonraki değeri almak için index'i güncelle
//         int comma_index = payload.indexOf(",", period_index); // "," işaretini bul
//         String period_str = payload.substring(period_index, comma_index); // period değerini al
//         delay_time = period_str.toInt(); // period değerini integer'a dönüştür ve delay_time değişkenine ata
//       }

//       //GET ile sunucudan alınan "relay" değerini kontrol et
//       int relay_index = payload.indexOf("relay=");
//       if (relay_index != -1) { // Eğer "relay" değeri bulunursa
//         relay_index += 6; // "relay" kelimesinden sonraki değeri almak için index'i günc
//         char relay_char = payload.charAt(relay_index); // "relay" değerini al
//         relay_status = relay_char - '0'; // ASCII'den uint8_t'ye dönüştür
//         digitalWrite(pin_relay, relay_status); // röle durumunu güncelle
//       }

//     } else {
//       if(serialGoster) Serial.println("HTTP isteği başarısız");
//       digitalWrite(led_server_asked, LOW);
//       digitalWrite(led_server_answered, LOW);
//     }
//     http.end();
//   } else {
//     if(serialGoster) Serial.println("HTTP bağlantısı başlatılamadı");
//     digitalWrite(led_server_asked, LOW);
//     digitalWrite(led_server_answered, LOW);
//   }
// }
// void setup() {
//   Serial.begin(9600);
//   pinMode(pin_relay, OUTPUT);
//   pinMode(led_findme, OUTPUT);
//   pinMode(led_wifi_connected, OUTPUT);
//   pinMode(led_server_asked, OUTPUT);
//   pinMode(led_server_answered, OUTPUT);
//   digitalWrite(led_wifi_connected, LOW);
//   digitalWrite(led_server_asked, LOW);
//   digitalWrite(led_server_answered, LOW);
//   digitalWrite(led_findme, LOW);
//   digitalWrite(pin_relay, LOW);
//   setup_deviceID();


//   Serial.begin(115200);
//   dht.setup(dhtPin, DHTesp::DHT11);

// }

// void loop() {
//  // findme();//findme ledini yakıp söndürür

//   // digitalWrite(led_wifi_connected, HIGH);
//   // digitalWrite(led_server_answered, LOW);
//   // digitalWrite(led_server_asked, LOW);

//   // float humidity = dht.getHumidity();
//   // float temperature = dht.getTemperature();
//   // float felt=roundf( dht.computeHeatIndex(temperature, humidity, false) *10 )/10 ;

//     if (WiFi.status() == WL_CONNECTED) {
//       if(serialGoster) Serial.println("WiFi bağlı");
//       digitalWrite(led_wifi_connected, HIGH);
//       performHTTPReq();
//     } else {
//       if(serialGoster) Serial.println("WiFi bağlı değil");
//       digitalWrite(led_wifi_connected, LOW);
//       digitalWrite(led_server_asked, LOW);
//       digitalWrite(led_server_answered, LOW);
//     }
//   delay(delay_time);
// }
