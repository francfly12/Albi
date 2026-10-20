/*
  ALBI — EMETTEUR v5
  Identique à la v4. Correction radio : la trame se termine
  désormais par un vrai '\n' (fin de ligne), indispensable au récepteur.
  Format : P<puissance>;I<import>;E<export>\n
*/

#include <Arduino.h>
#include <EEPROM.h>         // mémorisation du sens en cas de coupure
#include <ApJSY163T.h>      // bibliothèque pour compteur JSY-MK-163T

ApJSY163T MyJSY163T;        // objet pour le compteur JSY-MK-163T

const uint8_t PIN_MASSE  = 48;   // D48 : masse commune bouton + LED
const uint8_t PIN_BOUTON = 50;   // D50 : bouton poussoir
const uint8_t PIN_LED    = 52;   // D52 : LED bleue (résistance 390 ohms)

const int ADRESSE_EEPROM_SENS = 0;

bool sensInverse = false;
bool boutonAvant = false;

// Flash LED : simple si sens normal, double si sens inversé
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
  pinMode(PIN_BOUTON, INPUT_PULLUP);   // bouton appuyé = LOW

  // relit le sens mémorisé (EEPROM vierge = 0xFF => sens normal)
  sensInverse = (EEPROM.read(ADRESSE_EEPROM_SENS) == 1);

  Serial.begin(9600);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}   // 3 s max, puis démarrage

  Serial3.begin(9600);      // HC12
  Serial2.begin(4800);       // JSY-MK-163T (4800 bauds, 8N1, adresse 1)
  MyJSY163T.UseSerial(&Serial2);
  delay(200);

  boutonAvant = (digitalRead(PIN_BOUTON) == LOW);

  Serial.print("Sens inverse = ");
  Serial.println(sensInverse ? "OUI" : "NON");
}

void envoyerTrame(float puissance, float importKwh, float exportKwh) {
  Serial3.print("P");
  Serial3.print(puissance);
  Serial3.print(";I");
  Serial3.print(importKwh);
  Serial3.print(";E");
  Serial3.print(exportKwh);
  Serial3.print('\n');        // fin de trame : INDISPENSABLE au récepteur
}

void loop() {
  // Détection du bouton (une fois par cycle)
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
      float temp   = importKwh;      // échange import <-> export
      importKwh = exportKwh;
      exportKwh = temp;
    }

    Serial.print("Power W = ");   Serial.println(puissance);
    Serial.print("Kwh import = "); Serial.println(importKwh);
    Serial.print("Kwh export = "); Serial.println(exportKwh);

    envoyerTrame(puissance, importKwh, exportKwh);
  } else {
    Serial.println("Error reading meter");
  }
  delay(995);  // presque une mesure par seconde
}
