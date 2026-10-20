/*
  ALBI — EMETTEUR v4
  Arduino Mega 2560 : mesure la puissance au compteur électrique
  (JSY-MK-163T v1.2 sur Serial2) et transmet par radio HC12 (Serial3).

  Cablage :
  - HC12 (Serial3) : TX3 = D14 (jaune) -> RXD HC12 ; RX3 = D15 (vert) -> TXD HC12
  - JSY-MK-163T (Serial2) : TX2 = D16 (orange) -> RX JSY ; RX2 = D17 (rouge) -> TX JSY, 4800 bauds
  - Bouton poussoir : D50 vers D48 (masse commune)
  - LED bleue : D52 via resistance 390 ohms, masse D48
  - D48 maintenu en permanence a 0 V (masse commune bouton + LED)

  VERSION 4 : booleen sensInverse (sens de montage du conducteur de mesure)
  - sensInverse = false : valeurs normales, LED 1 flash de 2 ms
  - sensInverse = true : valeurs inversees (x -1, echange import/export), LED double flash
  - appui bouton : bascule, memorise en EEPROM (adresse 0)
*/

#include <Arduino.h>
#include <EEPROM.h>         // memorisation du sens en cas de coupure
#include <ApJSY163T.h>      // bibliothèque pour compteur JSY-MK-163T
                          // (installation manuelle en ZIP : https://github.com/antonioprevitali/ApJSY163T)

ApJSY163T MyJSY163T;        // objet pour le compteur JSY-MK-163T

const uint8_t PIN_MASSE  = 48;   // D48 : masse commune bouton + LED
const uint8_t PIN_BOUTON = 50;   // D50 : bouton poussoir
const uint8_t PIN_LED    = 52;   // D52 : LED bleue (resistance 390 ohms)

const int ADRESSE_EEPROM_SENS = 0;

bool sensInverse = false;
bool boutonAvant = false;

// Flash LED : simple si sens normal, double si sens inverse
void flashLed() {
  digitalWrite(PIN_LED, HIGH);
  delay(2);
  digitalWrite(PIN_LED, LOW);
  if (sensInverse) {
    delay(50);
    digitalWrite(PIN_LED, HIGH);
    delay(2);
    digitalWrite(PIN_LED, LOW);
  }
}

void setup() {
  pinMode(PIN_MASSE, OUTPUT);
  digitalWrite(PIN_MASSE, LOW);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  pinMode(PIN_BOUTON, INPUT_PULLUP);   // bouton appuye = LOW

  // relit le sens memorise (EEPROM vierge = 0xFF => sens normal)
  sensInverse = (EEPROM.read(ADRESSE_EEPROM_SENS) == 1);

  Serial.begin(9600);       // moniteur serie. Fonctionne meme sans PC connecte en USB
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}   // 3 s max, puis demarrage

  Serial3.begin(9600);      // HC12
  Serial2.begin(4800);                // JSY-MK-163T (4800 bauds, 8N1, adresse 1)
  MyJSY163T.UseSerial(&Serial2);
  delay(200);

  boutonAvant = (digitalRead(PIN_BOUTON) == LOW);

  Serial.print("Sens inverse = ");
  Serial.println(sensInverse ? "OUI" : "NON");
}

void loop() {
  // Detection du bouton (une fois par cycle) : bascule au moment de l'appui seulement
  bool boutonAppuye = (digitalRead(PIN_BOUTON) == LOW);
  if (boutonAppuye && !boutonAvant) {
    sensInverse = !sensInverse;
    EEPROM.update(ADRESSE_EEPROM_SENS, sensInverse ? 1 : 0);
    Serial.print("Sens inverse = ");
    Serial.println(sensInverse ? "OUI" : "NON");
  }
  boutonAvant = boutonAppuye;

  flashLed();

  if (MyJSY163T.okReadMeter()) {
    float puissance = MyJSY163T.Power;
    float importKwh = MyJSY163T.Import;
    float exportKwh = MyJSY163T.Export;

    if (sensInverse) {
      puissance = -puissance;
      float temp   = importKwh;      // echange import <-> export
      importKwh = exportKwh;
      exportKwh = temp;
    }

    Serial.print("Power W = ");
    Serial.println(puissance);
    Serial3.print("P");               // envoi radio
    Serial3.print(puissance);
    Serial.print("Kwh import = ");
    Serial.println(importKwh);
    Serial3.print("I");
    Serial3.print(importKwh);
    Serial.print("Kwh export = ");
    Serial.println(exportKwh);
    Serial3.print("E");
    Serial3.print(exportKwh);
    Serial3.println();                // fin de trame : le recepteur lit jusqu'au '\n'
  } else {
    Serial.println("Error reading meter");
  }
  delay(995);  // presque une mesure par seconde
}
