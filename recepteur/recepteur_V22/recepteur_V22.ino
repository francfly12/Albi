/*
  ALBI — RECEPTEUR v2.2 (multi-pages, croquis de François)
  P1 jauge (net conso-prod) / P2 jour conso+prod / P3 semaine conso+prod / P4 texte.
  Bouton bleu : page suivante ; bouton blanc (P4) : reset productions.
  Veille ecran 5 min (fillScr noir, UTFTGLUE n'a pas lcdOff) ; reveil -> page 1.
  Tableaux uint16_t (marqueur 65535 = pas de donnee) pour la RAM.
  Fichiers : recepteur_V22.ino (principal) + page1..4.ino, MEME dossier de sketch.
*/

#include <Arduino.h>
#include <UTFTGLUE.h>
#include <SoftwareSerial.h>            // requis par la bibliotheque PZEM
#define PZEM_DEFAULT_DEBUG_PORT Serial // debug USB, AVANT l'include
#include <PZEM004Tv30.h>

UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0);
PZEM004Tv30 pzem(&Serial1);            // PZEM : TX1=D18, RX1=D19

const uint8_t PIN_BOUTON_BLEU   = 44;  // D44, masse commune D48
const uint8_t PIN_BOUTON_BLANC  = 26;  // D26, masse commune D30 (ou D48)
const uint8_t PIN_MASSE_BOUTONS = 48;  // D48 maintenu a 0 V

const float PROD_MAX_GRAPH  = 800.0;    // echelle prod vers le haut
const float CONSO_MAX_GRAPH = 9000.0;   // echelle conso vers le bas

// --- valeurs instantanees ---
float consoW = 0, importKwh = 0, exportKwh = 0, prodW = 0, tensionV = 0;
unsigned long dernierRadioMs = 0;

// --- stockage (uint16_t : W entiers, 65535 = pas de donnee) ---
const int NB_JOUR    = 288;    // 24 h x 12 echantillons/h (5 min)
const int NB_SEMAINE = 336;    // 7 j x 48 echantillons/j (30 min)
uint16_t tabJour[NB_JOUR],       tabJourC[NB_JOUR];       // prod / conso
uint16_t tabSemaine[NB_SEMAINE], tabSemaineC[NB_SEMAINE];
int idxJour = 0, idxSemaine = 0;
float prodJourKwh = 0;          // production du jour (reset la nuit)
float prodTotKwh  = 0;          // production totale (reset bouton blanc)

// --- detection de nuit ---
const float SEUIL_NUIT_W = 5.0;
const unsigned long DUREE_NUIT_MS = 7200000UL;   // 2 h
unsigned long nuitDepuisMs = 0;
bool nuitEnCours = false;

// --- pages / veille ---
uint8_t page = 1;
const unsigned long VEILLE_MS = 300000UL;   // 5 minutes
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
      if (idx > 0) { buf[idx] = '\0'; analyserTrame(buf); idx = 0; }
    } else if (idx < sizeof(buf) - 1) {
      buf[idx++] = c;
    }
  }
  if (idx > 0 && millis() - dernierCarMs > 100) {   // fin implicite
    buf[idx] = '\0'; analyserTrame(buf); idx = 0;
  }
}

// ============ NUIT ============
void verifierNuit() {
  unsigned long m = millis();
  if (prodW < SEUIL_NUIT_W) {
    if (!nuitEnCours) { nuitEnCours = true; nuitDepuisMs = m; }
    else if (m - nuitDepuisMs >= DUREE_NUIT_MS) {
      prodJourKwh = 0;                    // nouveau jour
      nuitDepuisMs = m;                   // evite les resets repetes
      Serial.println("[NUIT] nouveau jour, prod journaliere remise a zero");
    }
  } else {
    nuitEnCours = false;                  // le soleil revient
  }
}

// ============ PAGES / VEILLE ============
void changerPage() {
  page = page % 4 + 1;            // 1->2->3->4->1
  dernierEveilMs = millis();
  dessinerPage(page);
}

void reveillerEcran() {
  ecranActif = true;
  dernierEveilMs = millis();
  page = 1;
  dessinerPage(page);
  Serial.println("[VEILLE] rallumage, page 1");
}

void setup() {
  pinMode(PIN_MASSE_BOUTONS, OUTPUT);
  digitalWrite(PIN_MASSE_BOUTONS, LOW);
  pinMode(PIN_BOUTON_BLEU,  INPUT_PULLUP);
  pinMode(PIN_BOUTON_BLANC, INPUT_PULLUP);

  Serial.begin(9600);
  Serial3.begin(9600);   // HC12

  for (int i = 0; i < NB_JOUR; i++)    { tabJour[i] = 65535; tabJourC[i] = 65535; }
  for (int i = 0; i < NB_SEMAINE; i++) { tabSemaine[i] = 65535; tabSemaineC[i] = 65535; }

  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  dessinerPage(page);
  Serial.println("=== ALBI recepteur v2.2 ===");
}

void loop() {
  lireRadio();

  unsigned long m = millis();

  // production PZEM toutes les secondes + cumuls d'energie
  if (m - dernierPzemMs >= 1000) {
    dernierPzemMs = m;
    prodW    = pzem.power();
    tensionV = pzem.voltage();
    prodJourKwh += prodW / 3600.0;   // W x 1 s -> kWh
    prodTotKwh  += prodW / 3600.0;
  }
  verifierNuit();

  // echantillon journalier toutes les 5 min
  if (m - dernierEchJourMs >= 300000UL) {
    dernierEchJourMs = m;
    tabJour[idxJour]   = (uint16_t)prodW;
    tabJourC[idxJour]  = (uint16_t)consoW;
    idxJour = (idxJour + 1) % NB_JOUR;
  }
  // echantillon hebdomadaire toutes les 30 min
  if (m - dernierEchSemaineMs >= 1800000UL) {
    dernierEchSemaineMs = m;
    tabSemaine[idxSemaine]   = (uint16_t)prodW;
    tabSemaineC[idxSemaine]  = (uint16_t)consoW;
    idxSemaine = (idxSemaine + 1) % NB_SEMAINE;
  }

  // bouton bleu : page suivante, ou reveil si ecran en veille
  static bool bleuAvant = false;
  bool bleu = (digitalRead(PIN_BOUTON_BLEU) == LOW);
  if (bleu && !bleuAvant) {
    if (!ecranActif) reveillerEcran();
    else changerPage();
  }
  bleuAvant = bleu;

  // bouton blanc : reset productions (page 4 uniquement), ou reveil
  static bool blancAvant = false;
  bool blanc = (digitalRead(PIN_BOUTON_BLANC) == LOW);
  if (blanc && !blancAvant) {
    if (!ecranActif) reveillerEcran();
    else if (page == 4) {
      prodJourKwh = 0;
      prodTotKwh  = 0;
      Serial.println("[RESET] productions remises a zero (bouton blanc)");
    }
  }
  blancAvant = blanc;

  // mise en veille apres 5 min sans appui (ecran noir, retroeclairage reste on)
  if (ecranActif && m - dernierEveilMs >= VEILLE_MS) {
    ecranActif = false;
    myGLCD.fillScr(0, 0, 0);
    Serial.println("[VEILLE] ecran eteint");
  }

  // rafraichissement de la page affichee
  if (ecranActif && m - dernierAffMs >= 200) {
    dernierAffMs = m;
    rafraichirPage(page);
  }
}
