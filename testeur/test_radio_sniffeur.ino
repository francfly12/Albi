/*
  ALBI — TESTEUR RADIO v1 (sniffeur HC12)
  Arduino UNO R4 Minima : ecoute les trames radio du transmetteur ALBI
  et les affiche horodatees sur le moniteur serie PC.

  Cablage :
  - HC12 sur Serial1 : HC12 RXD <- D1 (TX1), HC12 TXD -> D0 (RX1), 9600 bauds
  - Alim HC12 : GND / +5V. Broche SET en l'air (mode communication).
  - Attention R4 Minima : D0/D1 partages avec l'USB, conflit possible si PC branche.

  Ne write jamais sur Serial1 (pur ecoute).
  Purge des trames incompletes apres 200 ms sans caractere.
*/

const unsigned long PURGE_MS = 200;

char buf[48];
uint8_t idx = 0;
unsigned long dernierCarMs = 0;
unsigned long numTrame = 0;
unsigned long tDepart = 0;

void setup() {
  Serial.begin(115200);   // moniteur serie PC
  Serial1.begin(9600);    // HC12
  tDepart = millis();
  Serial.println("=== Sniffeur HC12 ALBI ===");
}

void loop() {
  // purge des trames incompletes
  if (idx > 0 && millis() - dernierCarMs > PURGE_MS) {
    idx = 0;   // trame abandonnee
  }

  while (Serial1.available()) {
    char c = Serial1.read();
    dernierCarMs = millis();
    if (c == '\n') {
      buf[idx] = '\0';
      numTrame++;
      float debit = (float)numTrame / ((millis() - tDepart) / 1000.0);
      Serial.print(millis() / 1000.0, 1);
      Serial.print("s  #");
      Serial.print(numTrame);
      Serial.print("  ");
      Serial.print(buf);
      Serial.print("   (");
      Serial.print(debit, 2);
      Serial.println(" trames/s)");
      idx = 0;
    } else if (idx < sizeof(buf) - 1) {
      buf[idx++] = c;
    }
  }
}
