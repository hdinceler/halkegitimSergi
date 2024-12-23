#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,20,4); 
#include <Servo.h>
#include <DHT.h>
 
#define DHTTYPE DHT11
#define servoPinSicaklik 3
#define servoPinNem 4
#define servoPinIsik 5
#define servoPinHavaKalite 6
#define sensorPinDHT11 7
#define sensorPinMQ9 A0
#define sensorPinLDR A1
#define buzzerPin 8

DHT dht(sensorPinDHT11, DHTTYPE);

// Formatlı sicaklik, nem, gaz ve isik değerlerini tutacak karakter dizilerini tanımlayın
char formatli_sicaklik[3];
char formatli_nem[4];
char formatli_gaz[4];
char formatli_isik[4];

Servo servoSicaklik;
Servo servoNem;
Servo servoIsik;
Servo servoHavaKalite;
int currentPosition; // Başlangıçta mevcut pozisyon 0 derece olarak ayarlanır
int targetPosition;  // Başlangıçta mevcut pozisyon 0 derece olarak ayarlanır

void setup()
{
  lcd.init();           
  lcd.backlight();
    
  pinMode(buzzerPin,OUTPUT);
  Serial.begin(9600);
  servoSicaklik.attach(servoPinSicaklik);     // Sıcaklık servo pini
  servoNem.attach(servoPinNem);               // Nem servo pini
  servoIsik.attach(servoPinIsik);             // Işık servo pini
  servoHavaKalite.attach(servoPinHavaKalite); // Hava kalite servo pini

  dht.begin();

  // delay(500);
}

void goServo(uint8_t targetPosition, Servo targetServo)
{
  uint8_t currentPosition = targetServo.read();

  if (targetPosition > currentPosition)
  {
    for (uint8_t i = currentPosition; i < targetPosition; i++)
    {
      targetServo.write(i);
      delay(50);
    }
  }
  if (targetPosition < currentPosition)
  {
    for (uint8_t i = currentPosition; i > targetPosition; i--)
    {
      targetServo.write(i);
      delay(50);
    }
  }
}

void loop()
{

 
  // if (Serial.available() > 0)
  // {
  //   currentPosition = servoSicaklik.read();
  //   String buff1 = Serial.readStringUntil('\n');
  //   targetPosition = buff1.toInt();

  //   int nemServoPozisyon = map( targetPosition , 30 , 100  , 180,0  );    
  //   int sicaklikServoPozisyon = map(targetPosition, 0, 45, 180, 0);
  //   int servoMQ9position= map( targetPosition, 0 , 1023 , 180 , 0 );
  //   int isikServoPozisyon = map(targetPosition, 20, 600, 180, 0);

  //   Serial.println(targetPosition);
  //   goServo(nemServoPozisyon, servoNem);
  //   goServo(sicaklikServoPozisyon, servoSicaklik);
  //   goServo(servoMQ9position, servoHavaKalite);
  //   goServo(isikServoPozisyon, servoIsik);
  //   return; // Seri porttan veri alındığında döngüyü sonlandır
  // }

  float nem = dht.readHumidity();
  int yuvarlanmisNem=  (int)nem;
  int nemServoPozisyon = map( yuvarlanmisNem , 30 , 100  , 180,0  );
  goServo( nemServoPozisyon , servoNem );

  float sicaklik = dht.readTemperature();
  int yuvarlanmisSicaklik = constrain((int)sicaklik, 0, 45);
  int sicaklikServoPozisyon = map(yuvarlanmisSicaklik, 0, 45, 180, 0);
  goServo(sicaklikServoPozisyon, servoSicaklik);

  int mq9Value = analogRead(sensorPinMQ9);
  int servoMQ9position= map( mq9Value, 0 , 1023 , 180 , 0 );
  if(mq9Value>500){digitalWrite(buzzerPin, HIGH);} else{digitalWrite(buzzerPin, LOW);}
  goServo( servoMQ9position,  servoHavaKalite );

  int isik = analogRead(sensorPinLDR);
  int isikServoPozisyon;
  isikServoPozisyon = map(isik, 20, 600, 180, 0);
  goServo(isikServoPozisyon, servoIsik); // Servo pozisyonunu ayarla

  sprintf( formatli_sicaklik,"%2d",  yuvarlanmisSicaklik);
  sprintf( formatli_nem,"%3d", yuvarlanmisNem);
  sprintf( formatli_gaz,"%3d",  mq9Value);
  sprintf( formatli_isik,"%3d",  isik);

  lcd.setCursor(0, 0); lcd.print("SCK:");
  lcd.setCursor(5, 0); lcd.print(formatli_sicaklik);
  lcd.setCursor(8, 0); lcd.print("NEM : %");
  lcd.setCursor(13,0 ); lcd.print(formatli_nem);

  lcd.setCursor(0, 1); lcd.print("GAZ:");
  lcd.setCursor(4, 1); lcd.print(formatli_gaz);
  lcd.setCursor(8, 1); lcd.print("ISIK:");
  lcd.setCursor(13,1 ); lcd.print(formatli_isik);

 Serial.print("Nem:");Serial.print(yuvarlanmisNem);Serial.print(", Sicaklik:");Serial.print(yuvarlanmisSicaklik);
 Serial.print(", Hava Kalite:");Serial.print(mq9Value);Serial.print(", Isik:");Serial.print( isik );
 Serial.print("\n");
 
  delay(400);

 

}
