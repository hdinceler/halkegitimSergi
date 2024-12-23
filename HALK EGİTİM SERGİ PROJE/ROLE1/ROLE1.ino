#define a_role_1 4
#define a_role_2 3
#define a_role_3 A4
#define a_role_4 13
#define a_role_5 6
#define gecikme 0

void setup() {
  Serial.begin(9600);

  pinMode(a_role_1, OUTPUT); 
  pinMode(a_role_2, OUTPUT); 
  pinMode(a_role_3, OUTPUT);  
  pinMode(a_role_4, OUTPUT); 
  pinMode(a_role_5, OUTPUT);

  // Tüm röleleri başlangıçta kapalı yap
  digitalWrite(a_role_1, LOW);
  digitalWrite(a_role_2, LOW);
  digitalWrite(a_role_3, LOW);
  digitalWrite(a_role_4, LOW);
  digitalWrite(a_role_5, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    String komut = Serial.readStringUntil('\n');
    komut.trim();
    Serial.println(komut);
    if (komut == "a_role_1-on") {
      digitalWrite(a_role_1, HIGH);
      Serial.println("a_role_1 acildi");
      delay(gecikme);
    } else if (komut == "a_role_1-off") {
      digitalWrite(a_role_1, LOW);
      Serial.println("a_role_1 kapandi");
      delay(gecikme);
    } else if (komut == "a_role_2-on") {
      digitalWrite(a_role_2, HIGH);
      delay(gecikme);
    } else if (komut == "a_role_2-off") {
      digitalWrite(a_role_2, LOW);
      delay(gecikme);
    } else if (komut == "a_role_3-on") {
      digitalWrite(a_role_3, HIGH);
      delay(gecikme);
    } else if (komut == "a_role_3-off") {
      digitalWrite(a_role_3, LOW);
      delay(gecikme);
    } else if (komut == "a_role_4-on") {
      digitalWrite(a_role_4, HIGH);
      delay(gecikme);
    } else if (komut == "a_role_4-off") {
      digitalWrite(a_role_4, LOW);
      delay(gecikme);
    } else if (komut == "a_role_5-on") {
      digitalWrite(a_role_5, HIGH);
      delay(gecikme);
    } else if (komut == "a_role_5-off") {
      digitalWrite(a_role_5, LOW);
      delay(gecikme);
    }
  }
}


  // digitalWrite(a_role_1,0 );delay(gecikme);  digitalWrite(a_role_1, 1);delay(gecikme);
  // digitalWrite(a_role_2,0 );delay(gecikme);  digitalWrite(a_role_2, 1);delay(gecikme);
  // digitalWrite(a_role_3,0 );delay(gecikme);  digitalWrite(a_role_3, 1);delay(gecikme);
  // digitalWrite(a_role_4,0 );delay(gecikme);  digitalWrite(a_role_4, 1);delay(gecikme);
  // digitalWrite(a_role_5,0 );delay(gecikme);  digitalWrite(a_role_5, 1);delay(gecikme);
