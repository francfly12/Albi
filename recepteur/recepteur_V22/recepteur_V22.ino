/*
  ALBI — RECEPTEUR v2.2 (multi-pages, croquis de François)
  P1 jauge (net conso-prod) / P2 jour conso+prod / P3 semaine conso+prod / P4 texte.
  Bouton bleu : page suivante ; bouton blanc (P4) : reset productions.
  Veille ecran 5 min ; reveil sur bouton -> page 1.
*/
#define MODE_TEST 0   // 1 = courbes synthetiques pour test graphique, 0 = utilisation reelle

#if MODE_TEST
void remplirTest();        // declaration (prototype) : fonctions dans #if non
void remplirTestSemaine(); // vues par le generateur automatique de l'IDE
#endif

#include <Arduino.h>
#include <UTFTGLUE.h>
#include <SoftwareSerial.h>
#define PZEM_DEFAULT_DEBUG_PORT Serial
#include <PZEM004Tv30.h>

UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0);
PZEM004Tv30 pzem(&Serial1);

const uint8_t PIN_BOUTON_BLEU   = 44;
const uint8_t PIN_BOUTON_BLANC  = 26;
const uint8_t PIN_MASSE_BOUTONS = 48;

const float PROD_MAX_GRAPH  = 800.0;    // vers le haut
const float CONSO_MAX_GRAPH = 9000.0;   // vers le bas

// --- valeurs instantanees ---
float consoW = 0, importKwh = 0, exportKwh = 0, prodW = 0, tensionV = 0;
unsigned long dernierRadioMs = 0;

// --- stockage ---
const int NB_JOUR    = 288;
const int NB_SEMAINE = 336;
uint16_t tabJour[NB_JOUR],       tabJourC[NB_JOUR];
uint16_t tabSemaine[NB_SEMAINE], tabSemaineC[NB_SEMAINE];
int idxJour = 0, idxSemaine = 0;
float prodJourKwh = 0, prodTotKwh = 0;

// --- nuit ---
const float SEUIL_NUIT_W = 5.0;
const unsigned long DUREE_NUIT_MS = 7200000UL;
unsigned long nuitDepuisMs = 0;
bool nuitEnCours = false;

// --- pages / veille ---
uint8_t page = 1;
const unsigned long VEILLE_MS = 300000UL;
bool ecranActif = true;
unsigned long dernierEveilMs = 0;

// --- timers ---
unsigned long dernierPzemMs = 0, dernierAffMs = 0;
unsigned long dernierEchJourMs = 0, dernierEchSemaineMs = 0;

// ============ RADIO ============
bool analyserTrame(const char* t) {
  const char* pP = strchr(t, 'P');
  const char* pI = strchr(t, 'I');
  const char* pE = strchr(t, 'E');
  if (!pP || !pI || !pE || pI < pP || pE < pI) return false;
  consoW = atof(pP + 1); importKwh = atof(pI + 1); exportKwh = atof(pE + 1);
  dernierRadioMs = millis();
  return true;
}

void lireRadio() {
  static char buf[48]; static uint8_t idx = 0;
  static unsigned long dernierCarMs = 0;
  while (Serial3.available()) {
    char c = Serial3.read(); dernierCarMs = millis();
    if (c == '\n' || c == '\r') { if (idx > 0) { buf[idx] = '\0'; analyserTrame(buf); idx = 0; } }
    else if (idx < sizeof(buf) - 1) buf[idx++] = c;
  }
  if (idx > 0 && millis() - dernierCarMs > 100) { buf[idx] = '\0'; analyserTrame(buf); idx = 0; }
}

// ============ NUIT ============
void verifierNuit() {
  unsigned long m = millis();
  if (prodW < SEUIL_NUIT_W) {
    if (!nuitEnCours) { nuitEnCours = true; nuitDepuisMs = m; }
    else if (m - nuitDepuisMs >= DUREE_NUIT_MS) {
      prodJourKwh = 0; nuitDepuisMs = m;
      Serial.println("[NUIT] nouveau jour");
    }
  } else nuitEnCours = false;
}

// ============ PAGES / VEILLE ============
void changerPage() {
  page = page % 4 + 1;
  dernierEveilMs = millis();
  dessinerPage(page);
}

void reveillerEcran() {
  ecranActif = true; dernierEveilMs = millis();
  page = 1;
  // myGLCD.lcdOn(); // supprime, n'existe pas dans utftglue
  dessinerPage(page);
  Serial.println("[VEILLE] rallumage, page 1");
}

void setup() {
  pinMode(PIN_MASSE_BOUTONS, OUTPUT); digitalWrite(PIN_MASSE_BOUTONS, LOW);
  pinMode(PIN_BOUTON_BLEU, INPUT_PULLUP);
  pinMode(PIN_BOUTON_BLANC, INPUT_PULLUP);

  pinMode(30, OUTPUT); digitalWrite(30, LOW);   // masse du bouton BLANC (D26/D30)

  Serial.begin(9600);
  Serial3.begin(9600);

  for (int i = 0; i < NB_JOUR; i++)    { tabJour[i] = 65535;    tabJourC[i] = 65535; }
  for (int i = 0; i < NB_SEMAINE; i++) { tabSemaine[i] = 65535; tabSemaineC[i] = 65535; }

  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);

  Serial.println("=== ALBI recepteur v2.2 ===");
  randomSeed(analogRead(A5));   // suite vraiment aleatoire pour le mode test

#if MODE_TEST
  remplirTest();          // courbes synthetiques
  remplirTestSemaine();   // ATTENTION : desactiver (MODE_TEST 0) pour la vraie utilisation
#endif

  dessinerPage(page);     // 1er affichage APRES le remplissage test
}

void loop() {
  lireRadio();
  unsigned long m = millis();

  if (m - dernierPzemMs >= 2000) {
    dernierPzemMs = m;
#if MODE_TEST            // #if (pas #ifdef) : coupe vraiment quand MODE_TEST = 0
    testValeurs();
#else
    prodW    = pzem.power();
    tensionV = pzem.voltage();
#endif
    prodJourKwh += prodW / 1800.0;   // energie = prodW W x 2 s, en kWh (2 s d'echantillonnage)
    prodTotKwh  += prodW / 1800.0;
  }
  verifierNuit();

  if (m - dernierEchJourMs >= 300000UL) {
    dernierEchJourMs = m;
    tabJour[idxJour] = prodW; tabJourC[idxJour] = consoW;
    idxJour = (idxJour + 1) % NB_JOUR;
  }
  if (m - dernierEchSemaineMs >= 1800000UL) {
    dernierEchSemaineMs = m;
    tabSemaine[idxSemaine] = prodW; tabSemaineC[idxSemaine] = consoW;
    idxSemaine = (idxSemaine + 1) % NB_SEMAINE;
  }

  static bool bleuAvant = false;
  bool bleu = (digitalRead(PIN_BOUTON_BLEU) == LOW);
  if (bleu && !bleuAvant) { if (!ecranActif) reveillerEcran(); else changerPage(); }
  bleuAvant = bleu;

  static bool blancAvant = false;
  bool blanc = (digitalRead(PIN_BOUTON_BLANC) == LOW);
  if (blanc && !blancAvant) {
    if (!ecranActif) reveillerEcran();
    else if (page == 4) {
      prodJourKwh = 0; prodTotKwh = 0;
      Serial.println("[RESET] productions a zero");
    }
  }
  blancAvant = blanc;

  if (ecranActif && m - dernierEveilMs >= VEILLE_MS) {
    ecranActif = false;
    myGLCD.fillScr(0, 0, 0);   // ecran vide = veille
    Serial.println("[VEILLE] extinction");
  }

  if (ecranActif && m - dernierAffMs >= 200) {
    dernierAffMs = m;
    rafraichirPage(page);
  }
}
