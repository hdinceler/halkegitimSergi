#include <Servo.h>

#define basParmak 2
#define isaretParmagi 3
#define ortaParmak 4
#define yuzukParmagi 5
#define serceParmak 6

#define gecikme 450  //  250 den küçük sayı girmeyin
#define baslamaAcisi 0  // Açı en az 0, en fazla 180 
#define bitisAcisi 180 // Açı en az 0, en fazla 180 
#define yavaslamaGecikmesi 10  // Servo motorun yavaş hareket etmesi için adım gecikmesi

Servo servo;

void parmakHareketi(int pin) {
  servo.attach(pin);
  
  // Başlama açısından bitiş açısına kadar yavaşça hareket ettir
  for (int aci = baslamaAcisi; aci <= bitisAcisi; aci += 1) {
    servo.write(aci);
    delay(yavaslamaGecikmesi);
  }
  delay(gecikme);
  
  // Bitiş açısından başlama açısına kadar yavaşça hareket ettir
  for (int aci = bitisAcisi; aci >= baslamaAcisi; aci -= 1) {
    servo.write(aci);
    delay(yavaslamaGecikmesi);
  }
  delay(gecikme);
  
  servo.detach();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Test Başlıyor...");
  delay(1000);
}

void loop() {
  parmakHareketi(basParmak);
  parmakHareketi(isaretParmagi);
  parmakHareketi(ortaParmak);
  parmakHareketi(yuzukParmagi);
  parmakHareketi(serceParmak);
  
  delay(gecikme);
  
  // Tersten hareket ettirmek için
  parmakHareketi(serceParmak);
  parmakHareketi(yuzukParmagi);
  parmakHareketi(ortaParmak);
  parmakHareketi(isaretParmagi);
  parmakHareketi(basParmak);
  
  delay(gecikme);
}
