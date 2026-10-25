/* PAGE 2 v2 : conso + prod sur 24 h (288 points de 5 min).
   Echelle verticale identique a la page 1 : zero a y=60,
   prod vers le haut (45 px / 1000 W, bleu), conso vers le bas
   echelle non lineaire (1000->70, 2000->105, 4000->125,
   6000->145, 8000->165 px, orange).
   Pixels PAIRS = production, IMPAIRS = consommation.
   Legende "CONSO + PROD JOUR" (BigFont) en bas au milieu,
   prod du jour en blanc, centree au-dessus de la legende.
   Barre bleue assise sur le niveau conso, sommet = total (meme
   principe que la page 1). [TEST] : diagnostic temporaire serie. */

const int G2_X1 = 1,  G2_X2 = 319;   // horizontal : garde l'actuel
const int G2_HAUT = 1;               // sommet du cadre
const int G2_ZERO = 60;               // zero a la meme hauteur que la page 1
const int G2_BAS  = 226;              // fond du cadre
const int G2_Y_KWH = 202;             // prod du jour, blanc, centré
const int G2_Y_LEG = 222;             // legende BigFont en bas
float p2_affKwh = -1;

// profondeur en px sous le zero : echelle NON LINEAIRE de la page 1
int profondeurConso(uint16_t w) {
  if (w <= 1000) return       (int)(( w        * 70UL) / 1000);
  if (w <= 2000) return 70  + (int)(((w - 1000) * 35UL) / 1000);
  if (w <= 4000) return 105 + (int)(((w - 2000) * 20UL) / 2000);
  if (w <= 6000) return 125 + (int)(((w - 4000) * 20UL) / 2000);
  return             145 + (int)(((w - 6000) * 20UL) / 2000);
}

void dessinerPage2() {
  myGLCD.clrScr();
  myGLCD.setColor(100, 100, 100);
  myGLCD.drawRect(G2_X1, G2_HAUT, G2_X2, G2_BAS+12);       // cadre
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawLine(G2_X1, G2_ZERO, G2_X2, G2_ZERO);      // ligne du 0 (blanche)

  for (int x = G2_X1; x <= G2_X2; x++) {
    int i = ((x - G2_X1) * (NB_JOUR - 1)) / (G2_X2 - G2_X1);  // instant du pixel

    // --- [TEST] diagnostic serie sur quelques instants temoins
    if (i == 100 || i == 150) {
      Serial.print("[P2] x=");    Serial.print(x);
      Serial.print(" i=");       Serial.print(i);
      Serial.print(" pair=");    Serial.print((x % 2 == 0) ? "OUI" : "NON");
      Serial.print(" prod=");    Serial.print(tabJour[i]);
      Serial.print(" conso=");   Serial.print(tabJourC[i]);
      Serial.print(" base=");    Serial.println(G2_ZERO + profondeurConso(tabJourC[i] == 65535 ? 0 : tabJourC[i]));
    }
    if (x % 2 == 0) {                       // --- pixel PAIR : production (bleu)
      if (tabJour[i] != 65535) {
        // base : sommet de la barre conso au meme instant (meme principe que page 1)
        uint16_t consoIci = (tabJourC[i] == 65535) ? 0 : tabJourC[i];
        int base = G2_ZERO + profondeurConso(consoIci);
        int h = (int)((uint32_t)tabJour[i] * 45UL / 1000);   // 45 px / 1000 W
        if (h < 1) h = 1;
        int yHaut = base - h;                // sommet = niveau du total
        if (yHaut < G2_HAUT) yHaut = G2_HAUT;                // ecretage au cadre
        myGLCD.setColor(0, 130, 255);
        myGLCD.drawLine(x, yHaut, x, base);                  // barre bleue
        // --- marqueur blanc : un pixel a l'extremite, un pixel au-dessus
        if (yHaut > G2_HAUT) {                               // place pour le pixel du dessus
          myGLCD.setColor(255, 255, 255);
          myGLCD.drawPixel(x, yHaut);
          myGLCD.drawPixel(x, yHaut - 1);
        } else {
          myGLCD.setColor(255, 255, 255);                    // cadre atteint : un seul pixel
          myGLCD.drawPixel(x, yHaut);
        }
      }

    } else {                                // --- pixel IMPAIR : conso (orange)
      if (tabJourC[i] != 65535) {
        int d = profondeurConso(tabJourC[i]);
        if (d < 1) d = 1;
        if (G2_ZERO + d > G2_BAS) d = G2_BAS - G2_ZERO;      // limite du cadre
        myGLCD.setColor(255, 165, 0);
        myGLCD.drawLine(x, G2_ZERO, x, G2_ZERO + d);
      }
    }
  }

  ecrireLegendePage2();
  p2_affKwh = -1;
  rafraichirPage2();
}

void ecrireLegendePage2() {
  // "CONSO + PROD JOUR" : 17 caracteres BigFont (16 px) = 272 px
  myGLCD.setFont(BigFont);
  int x = 160 - 136;                  // centre de l'ecran (320 px)
  myGLCD.setColor(255, 165, 0);  myGLCD.print("CONSO", x, G2_Y_LEG);        x += 5 * 16;
  myGLCD.setColor(255, 255, 255);  myGLCD.print(" + ", x, G2_Y_LEG);       x += 3 * 16;
  myGLCD.setColor(0, 130, 255);  myGLCD.print("PROD", x, G2_Y_LEG);        x += 4 * 16;
  myGLCD.setColor(255, 255, 255);  myGLCD.print(" JOUR", x, G2_Y_LEG);
}

void rafraichirPage2() {
  // prod du jour en blanc, centree au-dessus de la legende, live
  if (prodJourKwh != p2_affKwh) {
    p2_affKwh = prodJourKwh;
    char buf[16];
    dtostrf(prodJourKwh, 0, 2, buf);          // ex. "1.70" (pas de sprintf float sur AVR)
    strcat(buf, " Kwh");                      // -> "1.70 kWh"
    myGLCD.setFont(BigFont);
    myGLCD.setColor(0, 0, 0);                // efface (lecon UTFT : fillRect, pas espaces)
    myGLCD.fillRect(96, G2_Y_KWH, 224, G2_Y_KWH + 16);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(buf, CENTER, G2_Y_KWH);
  }
}
