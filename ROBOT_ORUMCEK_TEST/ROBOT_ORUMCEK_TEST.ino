#include <Servo.h>

// Servo tanımlamaları
Servo servo1A, servo1B, servo1C;
Servo servo2A, servo2B, servo2C;
Servo servo3A, servo3B, servo3C;
Servo servo4A, servo4B, servo4C;

// Pin tanımlamaları
const int pin1A = 2;
const int pin1B = 3;
const int pin1C = 4;
const int pin2A = 5;
const int pin2B = 6;
const int pin2C = 7;
const int pin3A = 8;
const int pin3B = 9;
const int pin3C = 10;
const int pin4A = 11;
const int pin4B = 12;
const int pin4C = 13;

// Hız değişkeni (milisaniye)
int speed = 10;

// Yürüme adımı sayısı
int stepSize = 30; 

void setup() {
  // Servoları başlat ve pinlere bağla
  servo1A.attach(pin1A);
  servo1B.attach(pin1B);
  servo1C.attach(pin1C);
  
  servo2A.attach(pin2A);
  servo2B.attach(pin2B);
  servo2C.attach(pin2C);
  
  servo3A.attach(pin3A);
  servo3B.attach(pin3B);
  servo3C.attach(pin3C);
  
  servo4A.attach(pin4A);
  servo4B.attach(pin4B);
  servo4C.attach(pin4C);
  
  // Seri haberleşmeyi başlat
  Serial.begin(9600);
}

void loop() {
  // Yürüme döngüsü
  walkStep();
}

// Robotun adım atmasını sağlayan fonksiyon
void walkStep() {
  // Adım atma işlemi
  moveLeg(1, true);
  moveLeg(3, true);
  moveLeg(2, false);
  moveLeg(4, false);

  delay(200); // Adımlar arası bekleme süresi

  moveLeg(1, false);
  moveLeg(3, false);
  moveLeg(2, true);
  moveLeg(4, true);

  delay(200); // Adımlar arası bekleme süresi
}

// Bir bacağı hareket ettiren fonksiyon
void moveLeg(int leg, bool forward) {
  int angle = forward ? stepSize : -stepSize;

  switch (leg) {
    case 1:
      moveServoSlowly(servo1A, servo1A.read(), servo1A.read() + angle, "1A");
      moveServoSlowly(servo1B, servo1B.read(), servo1B.read() + angle, "1B");
      moveServoSlowly(servo1C, servo1C.read(), servo1C.read() + angle, "1C");
      break;
    case 2:
      moveServoSlowly(servo2A, servo2A.read(), servo2A.read() + angle, "2A");
      moveServoSlowly(servo2B, servo2B.read(), servo2B.read() + angle, "2B");
      moveServoSlowly(servo2C, servo2C.read(), servo2C.read() + angle, "2C");
      break;
    case 3:
      moveServoSlowly(servo3A, servo3A.read(), servo3A.read() + angle, "3A");
      moveServoSlowly(servo3B, servo3B.read(), servo3B.read() + angle, "3B");
      moveServoSlowly(servo3C, servo3C.read(), servo3C.read() + angle, "3C");
      break;
    case 4:
      moveServoSlowly(servo4A, servo4A.read(), servo4A.read() + angle, "4A");
      moveServoSlowly(servo4B, servo4B.read(), servo4B.read() + angle, "4B");
      moveServoSlowly(servo4C, servo4C.read(), servo4C.read() + angle, "4C");
      break;
  }
}

// Servo motorunu yavaşça hareket ettiren fonksiyon
void moveServoSlowly(Servo &servo, int startAngle, int endAngle, const char *servoName) {
  if (startAngle < endAngle) {
    for (int angle = startAngle; angle <= endAngle; angle++) {
      servo.write(angle);
      delay(speed);
    }
  } else {
    for (int angle = startAngle; angle >= endAngle; angle--) {
      servo.write(angle);
      delay(speed);
    }
  }
  Serial.print(servoName);
  Serial.println((startAngle < endAngle) ? " açıldı" : " kapandı");
}
