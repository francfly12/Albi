/* Test brut PZEM-004T v3.0 — requete Modbus manuelle
   Mega 2560 : TX1 = D18 vers PZEM RX, RX1 = D19 depuis PZEM TX */

uint8_t requete[8] = {0xF8, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x64, 0x64};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("=== Test brut PZEM ===");
}

void loop() {
  Serial1.write(requete, 8);          // lecture des 10 registres
  Serial.println("Requete envoyee, attente reponse...");
  delay(500);

  if (!Serial1.available()) {
    Serial.println("AUCUNE reponse recue sur D19");
  }
  while (Serial1.available()) {
    Serial.print(Serial1.read(), HEX);
    Serial.print(' ');
  }
  Serial.println();
  delay(3000);
}
