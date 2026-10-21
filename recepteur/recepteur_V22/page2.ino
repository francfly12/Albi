/* PAGE 2 : conso + prod sur 24 h (288 points de 5 min).
   Prod vers le haut (0..800 W, bleu), conso vers le bas (0..9000 W, ambre),
   ligne 0 au milieu (gris). Prod du jour en haut a droite (bleu). */

const int G2_HAUT = 30, G2_ZERO = 120, G2_BAS = 220;
float p2_affKwh = -1;

void dessinerPage2() {
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Conso+Prod Jour", CENTER, 5);
  myGLCD.setColor(100, 100, 100);
  myGLCD.drawRect(10, G2_HAUT, 310, G2_BAS);
  myGLCD.drawLine(10, G2_ZERO, 310, G2_ZERO);      // ligne du 0

  // production vers le haut (bleu)
  myGLCD.setColor(0, 130, 255);
  for (int i = 0; i < NB_JOUR; i++) {
    if (tabJour[i] == 65535) continue;             // pas de donnee
    int x = 10 + (i * 300UL) / NB_JOUR;
    int h = (int)((G2_ZERO - G2_HAUT) * tabJour[i] / PROD_MAX_GRAPH);
    if (h < 1) h = 1;
    myGLCD.drawLine(x, G2_ZERO - h, x, G2_ZERO);
  }
  // consommation vers le bas (ambre)
  myGLCD.setColor(255, 165, 0);
  for (int i = 0; i < NB_JOUR; i++) {
    if (tabJourC[i] == 65535) continue;
    int x = 10 + (i * 300UL) / NB_JOUR;
    int h = (int)((G2_BAS - G2_ZERO) * tabJourC[i] / CONSO_MAX_GRAPH);
    if (h < 1) h = 1;
    myGLCD.drawLine(x, G2_ZERO, x, G2_ZERO + h);
  }

  p2_affKwh = -1;
  rafraichirPage2();
}

void rafraichirPage2() {
  // seule la valeur kWh du jour se met a jour en live
  if (prodJourKwh != p2_affKwh) {
    p2_affKwh = prodJourKwh;
    myGLCD.setColor(0, 0, 0);
    myGLCD.print("        ", 240, 5);
    myGLCD.setColor(0, 130, 255);
    myGLCD.printNumF(prodJourKwh, 2, 240, 5);
  }
}
