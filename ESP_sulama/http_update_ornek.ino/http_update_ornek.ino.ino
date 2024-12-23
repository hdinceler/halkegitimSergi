#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "ESP8266HTTPClient.h"
#include "ESP8266httpUpdate.h"
#include <WiFiManager.h>
#include <LittleFS.h>

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif
//DTH SAMPLING
DHTesp dht;

// ID  & SERVER API PARAMS
String deviceID = ""; //sunucuda cihazı tanımlayan benzersiz id
uint8_t firmvareVersionNumber=100;   // firmware versiyon numaraası mevcut değilse default 100 e ayarlanıyor
const char* api_key = "SDd53X92"; // API anahtarı
char url_1[255] = "https://haysaf.com/sensor.php"; // Sunucu3 URL'si
char url_2[255] = "http://haysaf.com/sensor.php"; // Sunucu2 URL'si
char url_3[255] = "http://haysaf.com/sensor"; // Sunucu2 URL'si
char url_4[255] = "https://haysaf.com/sensor"; // Sunucu2 URL'si
char url_5[255] = "http://192.168.1.254/sensor.php"; // Sunucu1 URL'si
char url_6[255] = "http://192.168.1.254/sensor"; // Sunucu2 URL'si

//FIRMWARE UPDATE PARAMS
const char* firmwareBaseURL = "http://192.168.1.254/firmwares/okul_sensor/sadece_sicaklik_nem/";
const char* mcuVersionFile = "/version.txt";
int mcuFirmvareVersionNumber=100;

//WORKING PARAMS
unsigned int delay_time = 2000;
bool relay_status = false;
//PIN NUMBERS
const int dhtPin = 5;
const int analogPin = 0;
const int led_wifi_connected = 18;  //d8
const int led_server_asked = 13;    // D7
const int led_server_answered = 12; // D6
const int led_findme = 4;           // D2
const int pin_relay = 0;            // D3


void setup() {
  Serial.begin(115200);
  WiFi.setOutputPower(0);  // sinyal gücü min 0 max  20.5 dBm
  WiFiManager wifiManager;

}
void loop() {
  
   

}

void setup_deviceID() {
  // cihazın benzersiz id sini mac adresindeki : ları kaldırıp elde et
  uint8_t mac_address[6]; 
  WiFi.macAddress(mac_address);
  for (int i = 0; i < 6; ++i) {
    deviceID += String(mac_address[i], 16);
  }
}

bool firmwareUpdateControl(){
  if( checkVersionFile )
}

bool checkVersionFile(){
    //eğer mcuVersionFile mevcut değilse  yani mcuHaveVersionFile()==0 ise updteMcuVersionFile(mcuFirmvareVersionNumber) yap  sonucu seriale de yaz
    // eğer mcuVersionFile mevcut ise yani mcuHaveVersionFile()==1 ise   
}

bool mcuHaveVersionFile(){
   //eğer mcuVersionFile mevcut değilse  yani mcuHaveVersionFile()==0 ise updteMcuVersionFile(mcuFirmvareVersionNumber) yap  sonucu seriale de yaz
}

bool writeFirstMcuVersionFile(){
  //mcu ya mcuVersionFile değişkeninde tanımlı dosya ismiyle bir versiyon dosyası oluştur ve içine 100 yaz. bunu başarırsan geriye 1 , başaramazsan 0 döndür.sonucu seriale de yaz
}
bool updteMcuVersionFile(int mcuVersionFile){
  // versionNumber değişkeniyle verilen versiyon numarasını mcuVersionFile 
  //değişkeniyle tanımmlanmış dosyaya eğer dosya yoksa yeniden oluşturuğ içine yaz .eğer önceden varsa eskisini sil mcuVersionFile değişkenini üzerine  yaz. sonucu seriale de yaz
}
bool mcuHaveOldVersion(){
  // mcu da mcuVersionFile  dosyasının içini okut içinde 100 , 101 gibi bir sayı kayıtlı ise 1 dönder yoksa sıfır dönder.sonucu seriale de yaz
}

bool newVersionFound(){
  // firmwareBaseURL  ye gir dönen String i sayıya çevir. mesela 103 döndü. bunu 103 sayısına çevir.sonucu seriale de yaz
}