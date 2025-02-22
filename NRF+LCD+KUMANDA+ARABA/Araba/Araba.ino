#include "nRF24L01.h"
#include "RF24.h"
#include <SPI.h>
#define serialDebug 1  //seriali aç kapat
#define yakinlik_siniri  5   // araba bu cm cinsinden degerden  daha yakın cisim algılarsa dursun
long zaman=0;
long mesafe_on=0;
long mesafe_arka=0;
bool on_far_durumu=false;
unsigned long baslangic_zamani_millis=0;
unsigned long baslangic_zamani_micros=0;
char buff[32];

unsigned int x_degeri=0;
unsigned int y_degeri=0;
unsigned int joy_buton=0;
unsigned int yukari_buton=0;
unsigned int asagi_buton=0;
unsigned int sag_buton=0;
unsigned int sol_buton=0;

#define pin_on_far 2
#define pin_sag_teker_ileri 3 //~PWM
#define pin_korna 4
#define pin_sag_teker_geri 5 //~PWM
#define pin_sol_teker_ileri 6 //~PWM
#define CSN_PIN 7 // NRF için
#define CE_PIN 8 // NRF için
#define pin_sol_teker_geri 9 //~PWM
#define pin_sis_fari 10

uint8_t pin_mesafe_echo_on = A0;
uint8_t pin_mesafe_trig_on = A1;
uint8_t pin_mesafe_echo_arka = A2;
uint8_t pin_mesafe_trig_arka = A3;
#include "araba_kutuphane.h"

RF24 radio(CE_PIN , CSN_PIN); // CE, CSN pimleri
char gelen_komut[32] = "";
uint8_t top_speed=255;
 
void setup() {
  pinMode( pin_on_far, OUTPUT);
  pinMode( pin_sag_teker_ileri , OUTPUT);
  pinMode( pin_korna , OUTPUT);
  pinMode( pin_sag_teker_geri , OUTPUT);
  pinMode( pin_sol_teker_ileri , OUTPUT);
  pinMode( pin_sol_teker_geri , OUTPUT);
  pinMode( pin_sis_fari, OUTPUT);

  pinMode(pin_mesafe_trig_on, OUTPUT);
  pinMode(pin_mesafe_echo_on, INPUT);
  pinMode(pin_mesafe_trig_arka, OUTPUT);
  pinMode(pin_mesafe_echo_arka, INPUT);
  if(serialDebug) Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0F0E12E); // Vericinin yazma hattı adresi
  radio.setPALevel(RF24_PA_MIN); 
  radio.startListening();
    Serial.println("araba");
 
}

void loop() {
 
//RADYO İLETİŞİM
  if (radio.available()) {
    radio.read(&gelen_komut, sizeof(gelen_komut));   
    Serial.println(gelen_komut);
 
    strcpy( buff , strtok( gelen_komut,  "," ) ); x_degeri= atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); y_degeri= atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); joy_buton= atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); yukari_buton= atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); asagi_buton= atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); sag_buton = atoi(buff);
    strcpy( buff , strtok(NULL,  "," ) ); sol_buton= atoi(buff);
    digitalWrite(pin_sis_fari,sol_buton );
    on_far_durumu=!yukari_buton;
    digitalWrite(pin_on_far , on_far_durumu );
    digitalWrite(pin_korna, sag_buton);
 
        if (x_degeri >= 0 && x_degeri < 470) {sola_don(map(x_degeri, 470, 0, 0, top_speed)); }
        else if (x_degeri > 555 && x_degeri < 1024) {saga_don(map(x_degeri, 555, 1023, 0, top_speed)); }
        else if (y_degeri >=550 && y_degeri < 1024){   ileri_git( map(y_degeri, 550,1023, 0,top_speed) ); } 
        else if (y_degeri >= 0 && y_degeri < 470) {geri_git(map(y_degeri, 470, 0, 0, top_speed)); }
        else dur();
 

  }else dur();
}
