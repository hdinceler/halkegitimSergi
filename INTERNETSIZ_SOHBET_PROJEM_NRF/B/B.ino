#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

RF24 radio(9, 10); // CE, CSN pinleri

const uint64_t pipe = 0xE8E8F0F0E1LL; // Kanal adresi

LiquidCrystal_I2C lcd(0x3F, 16, 2); // LCD tanımı (adres, sütun sayısı, satır sayısı)

void setup() {
  Serial.begin(9600);
  lcd.init(); // LCD ekranını başlat
  lcd.backlight(); // Arka ışığı aç
  radio.begin();
  radio.openReadingPipe(1, pipe); // Okuma kanalını ayarla
  radio.startListening(); // Dinlemeyi başlat
}

void loop() {
  if (radio.available()) {
    char buf[32];
    radio.read(buf, sizeof(buf)); // Mesajı al
    String receivedMessage = String(buf);
    String controlString = receivedMessage.substring(0, 3); // Kontrol stringini al
    String message = receivedMessage.substring(4); // Geri kalan mesajı al (kontrol stringi hariç)

    // Kontrol stringini A'ya geri gönder
    radio.stopListening(); // Dinlemeyi durdur
    radio.write(controlString.c_str(), controlString.length() + 1); // Kontrol stringini gönder
    radio.startListening(); // Dinlemeyi tekrar başlat

    // Mesajı seri porta ve LCD'ye yaz
    Serial.println("B'den A'ya gelen mesaj: " + message);
    lcd.clear(); // LCD ekranını temizle

    // Eğer mesaj 16 karakterden fazla ise, iki satıra böl
    if (message.length() > 16) {
      String line1 = message.substring(0, 16);
      String line2 = message.substring(16);
      lcd.setCursor(0, 0); // İlk satıra yaz
      lcd.print(line1);
      lcd.setCursor(0, 1); // İkinci satıra yaz
      lcd.print(line2);
    } else {
      lcd.setCursor(0, 0); // Tek satırı yaz
      lcd.print(message);
    }
    delay(2000); // LCD ekranın okunması için kısa bir süre bekle
  }
}
