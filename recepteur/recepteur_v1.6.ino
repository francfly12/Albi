/*
  ALBI — RECEPTEUR v1.6
  Mega 2560 : consommation reçue par HC12 (Serial3), production mesurée
  par PZEM-004T v3.0 (Serial1), affichage ILI9341 via UTFTGLUE.

  v1.6 : version de production, nettoyée (retrait des statistiques de test).
  v1.5 : fin de trame par '\n' ou silence 100 ms (correction radio).
*/

#include <Arduino.h>
#include <UTFTGLUE.h>
#include <SoftwareSerial.h>          // requis par la bibliotheque PZEM
#define PZEM_DEFAULT_DEBUG_PORT Serial // debug sur l'USB, AVANT l'include
#include <PZEM004Tv30.h>

UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0);
PZEM004Tv30 pzem(&Serial1);          // PZEM : TX1=D18, RX1=D19

// valeurs radio
float consoW = 0, importKwh = 0, exportKwh = 0;
unsigned long dernierRadioMs = 0;

// affichage
float affConso = -1, affProd = -1;
bool affRadioOk = false, affPzOk = false;
unsigned long dernierAffMs = 0;
unsigned long dernierPzemMs = 0;
float prodW = 0;

bool analyserTrame(const char* t) {
  const char* pP = strchr(t, 'P');
  const char* pI = strchr(t, 'I');
  const char* pE = strchr(t, 'E');
  if (!pP || !pI || !pE || pI < pP || pE < pI) return false;
  consoW    = atof(pP + 1);
  importKwh = atof(pI + 1);
  exportKwh = atof(pE + 1);
  dernierRadioMs = millis();
  return true;
}

void lireRadio() {
  static char buf[48];
  static uint8_t idx = 0;
  static unsigned long dernierCarMs = 0;

  while (Serial3.available()) {
    char c = Serial3.read();
    dernierCarMs = millis();
    if (c == '\n' || c == '\r') {
      if (idx > 0) {
        buf[idx] = '\0';
        analyserTrame(buf);
        idx = 0;
      }
    } else if (idx < sizeof(buf) - 1) {
      buf[idx++] = c;
    }
  }
  // fin de trame implicite : 100 ms sans caractere
  if (idx > 0 && millis() - dernierCarMs > 100) {
    buf[idx] = '\0';
    analyserTrame(buf);
    idx = 0;
  }
}

void dessinerValeur(float v, bool ok, const char* msgKo, int y) {
  myGLCD.print("            ", LEFT, y);
  if (ok) myGLCD.printNumF(v, 1, LEFT, y);
  else    myGLCD.print(msgKo, LEFT, y);
}

void rafraichirAffichage() {
  bool radioOk = (millis() - dernierRadioMs < 10000);
  bool pzOk = !isnan(prodW);

  if (consoW != affConso || prodW != affProd || radioOk != affRadioOk || pzOk != affPzOk) {
    affConso = consoW; affProd = prodW;
    affRadioOk = radioOk; affPzOk = pzOk;
    dessinerValeur(consoW, radioOk, "--- (pas de radio)", 90);
    dessinerValeur(prodW,  pzOk,   "--- (PZEM ?)",      180);
  }
}

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);   // HC12

  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);

  myGLCD.setColor(255, 165, 0);
  myGLCD.print("Consommation", LEFT, 60);
  myGLCD.setColor(0, 130, 255);
  myGLCD.print("Production PV", LEFT, 150);
  myGLCD.setColor(255, 255, 255);
  affConso = -1;
  rafraichirAffichage();
}

void loop() {
  lireRadio();

  unsigned long maintenant = millis();
  if (maintenant - dernierPzemMs >= 1000) {
    dernierPzemMs = maintenant;
    prodW = pzem.power();
  }
  if (maintenant - dernierAffMs >= 200) {
    dernierAffMs = maintenant;
    rafraichirAffichage();
  }
}
