/* PAGE 4 : texte — prod inst. (bleu), conso (orange), prod jour (blanc),
   prod totale (blanc), tension (vert). Valeur + unite, couleurs du label.
   Reset bouton blanc gere dans loop() (fichier principal).

   v2.3 corr. débordement : quand la valeur est large ("4530.15" + "Kwh"),
   l'unité sortait de l'écran (le "w" orphelin). Désormais :
   - on retire les décimales si valeur+unité ne tiennent pas,
   - sinon on décale la valeur vers la gauche (alignée à droite sur le bord),
   - la zone effaçée va de x=172 à 319 (couvre aussi les valeurs décalées). */

const int P4_X_VAL = 184;               // x habituel des valeurs (cadrat après libellé)
const int P4_X_MIN = 172;               // bord gauche mini : "Prod.total" finit à x=165
const int P4_X_MAX = 319;               // bord droit de l'écran
const int P4_CADRAT = 16;               // BigFont = 16 px/cadrat

float p4_aff[5] = { -1, -1, -1, -1, -1 };   // memoire des dernieres valeurs affichees

// slot : 0=prodW 1=consoW 2=prodJourKwh 3=prodTotKwh 4=tensionV
// dec = nombre de decimales ; unite = texte colle apres la valeur
void p4_affChiffre(byte slot, int y, float v, byte dec,
                   const char* unite, byte r, byte g, byte b) {
  if (isnan(v)) v = 0;                     // garde NaN (PZEM muet en mode reel)
  if (v == p4_aff[slot]) return;
  p4_aff[slot] = v;
  char buf[12];
  dtostrf(v, 0, dec, buf);                 // pas de sprintf float sur AVR

  // --- anti-debordement (v2.3) ---
  int total = (strlen(buf) + strlen(unite)) * P4_CADRAT;   // largeur valeur+unite
  if (total > (P4_X_MAX - P4_X_MIN + 1) && dec > 0) {
    dec = 0;                               // "4530.15Kwh" (10 cadrats) -> "4530Kwh" (8)
    dtostrf(v, 0, dec, buf);
    total = (strlen(buf) + strlen(unite)) * P4_CADRAT;
  }
  int xVal = P4_X_MAX + 1 - total;         // aligne a droite si ca deborde...
  if (xVal > P4_X_VAL) xVal = P4_X_VAL;    // ...sinon position habituelle
  if (xVal < P4_X_MIN) xVal = P4_X_MIN;    // jamais sous le libelle (cas extreme)

  myGLCD.setColor(0, 0, 0);                // efface TOUTE la zone valeur (meme si
  myGLCD.fillRect(P4_X_MIN, y, P4_X_MAX, y + 15);  // la valeur precedente etait large)
  myGLCD.setColor(r, g, b);
  myGLCD.print(buf, xVal, y);
  myGLCD.print(unite, xVal + strlen(buf) * P4_CADRAT, y);
}

void dessinerPage4() {
  myGLCD.clrScr();
  myGLCD.setColor(100, 100, 100);
  myGLCD.drawRect(G3_X1, G3_HAUT, G3_X2, G3_BAS+12);       // cadre (constantes page 3)
  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);   // BLANC
  myGLCD.print("RESET >", 200, 10);
  myGLCD.setColor(0, 0, 255);       // BLEU
  myGLCD.print("Prod. inst.", 5, 40);
  myGLCD.setColor(255, 165, 0);     // ORANGE
  myGLCD.print("Conso.", 5, 80);
  myGLCD.setColor(255, 255, 255);   // BLANC
  myGLCD.print("Prod.jour", 5, 120);
  myGLCD.print("Prod.total", 5, 160);
  myGLCD.setColor(0, 255, 0);       // VERT
  myGLCD.print("Tension", 5, 200);

  for (byte s = 0; s < 5; s++) p4_aff[s] = -1;     // force le reaffichage
  rafraichirPage4();
}

void rafraichirPage4() {
  p4_affChiffre(0, 40,  prodW,       0, "W",     0,   0, 255);  // bleu, entier
  p4_affChiffre(1, 80,  consoW,      0, "W",   255, 165,   0);  // orange, entier
  p4_affChiffre(2, 120, prodJourKwh, 2, "Kwh", 255, 255, 255);  // blanc
  p4_affChiffre(3, 160, prodTotKwh,  2, "Kwh", 255, 255, 255);  // blanc
  p4_affChiffre(4, 200, tensionV,    2, "V",     0, 255,   0);  // vert
}

// ============ AIGUILLAGE DES PAGES ============
void dessinerPage(uint8_t p) {
  if      (p == 1) dessinerPage1();
  else if (p == 2) dessinerPage2();
  else if (p == 3) dessinerPage3();
  else             dessinerPage4();
}

void rafraichirPage(uint8_t p) {
  if      (p == 1) rafraichirPage1();
  else if (p == 2) rafraichirPage2();
  else if (p == 3) rafraichirPage3();
  else             rafraichirPage4();
}
