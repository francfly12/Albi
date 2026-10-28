/* PAGE 4 : texte — prod inst. (bleu), conso (orange), prod jour (blanc),
   prod totale (blanc), tension (vert). Valeur + unite, couleurs du label.
   Reset bouton blanc gere dans loop() (fichier principal). */

const int P4_X_VAL = 184;               // x de depart des valeurs (recule d'un cadrat)

float p4_aff[5] = { -1, -1, -1, -1, -1 };   // memoire des dernieres valeurs affichees

// slot : 0=prodW 1=consoW 2=prodJourKwh 3=prodTotKwh 4=tensionV
// dec = nombre de decimales ; unite = texte colle apres la valeur
void p4_affChiffre(byte slot, int y, float v, byte dec,
                   const char* unite, byte r, byte g, byte b) {
  if (isnan(v)) v = 0;                     // garde NaN (PZEM muet en mode reel)
  if (v == p4_aff[slot]) return;
  p4_aff[slot] = v;
  myGLCD.setColor(0, 0, 0);                // efface la zone valeur + unite
  myGLCD.fillRect(P4_X_VAL, y, 319, y + 16);
  myGLCD.setColor(r, g, b);
  char buf[12];
  dtostrf(v, 0, dec, buf);                 // pas de sprintf float sur AVR
  myGLCD.print(buf, P4_X_VAL, y);
  myGLCD.print(unite, P4_X_VAL + strlen(buf) * 16, y);   // BigFont = 16 px/cadrat
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
