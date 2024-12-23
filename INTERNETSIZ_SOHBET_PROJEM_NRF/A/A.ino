
  #include <SPI.h>
  #include <nRF24L01.h>
  #include <RF24.h>
  #include <LiquidCrystal_I2C.h>

  #include <Wire.h>

LiquidCrystal_I2C lcd(0x3F,16,2);
#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN pinleri

const uint64_t pipe = 0xE8E8F0F0E1LL; // Kanal adresi

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(pipe); // Yazma kanalını ayarla
  randomSeed(analogRead(0)); // Rastgele sayı üretimi için seed belirle
}

void loop() {
  if (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    String controlString = generateRandomString(3); // 3 haneli rastgele string oluştur
    String finalMessage = controlString + ":" + message; // Mesaj başlığına kontrol stringini ekle
    char buf[32];
    finalMessage.toCharArray(buf, 32);
    radio.write(buf, sizeof(buf)); // Mesajı gönder
    Serial.println("A'dan B'ye gönderilen mesaj: " + finalMessage);
    delay(1000); // Bir sonraki mesajı göndermeden önce bekle
  }
}

String generateRandomString(int length) {
  String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; // Kullanılacak karakterler
  String randomString = "";
  for (int i = 0; i < length; i++) {
    randomString += characters[random(0, characters.length())]; // Rastgele karakterler oluştur
  }
  return randomString;
}
