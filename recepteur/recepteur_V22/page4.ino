/* PAGE 4 : production en chiffres (labels verts, valeurs bleues) + tension PZEM.
   Bouton blanc (page 4 uniquement) : reset prod jour + totale. */

float p4_aff[4] = {-1, -1, -1, -1};

void dessinerPage4() {
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 200, 0);
  myGLCD.print("Prod instantanee", LEFT, 40);
  myGLCD.print("Prod jour kWh",   LEFT, 90);
  myGLCD.print("Prod totale kWh", LEFT, 140);
  myGLCD.print("Tension V",       LEFT, 190);
  for (byte i = 0; i < 4; i++) p4_aff[i] = -1;
  rafraichirPage4();
}

void p4_affChiffre(byte slot, int y, float v) {
  if (v == p4_aff[slot]) return;
  p4_aff[slot] = v;
  myGLCD.print("          ", 180, y);
  myGLCD.setColor(0, 130, 255);
  myGLCD.printNumF(v, 2, 180, y);
  myGLCD.setColor(0, 200, 0);
}

void rafraichirPage4() {
  p4_affChiffre(0, 40,  prodW);
  p4_affChiffre(1, 90,  prodJourKwh);
  p4_affChiffre(2, 140, prodTotKwh);
  p4_affChiffre(3, 190, tensionV);
}

// ---------- aiguillage des pages (commun) ----------
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
