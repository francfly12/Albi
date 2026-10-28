/* PAGE 3 v2 : conso + prod sur la semaine (336 points de 30 min).
   Identique a la page 2, sauf :
   - echelle horizontale = 7 jours (separateurs gris tous les 48 points)
   - legende "CONSO+PROD SEMAINE" (BigFont) en bas au milieu
   - production TOTALE en kWh, blanc, au-dessus de la legende (live)
   Depend de profondeurConso() definie dans page2.ino (meme dossier,
   concatene avant par l'IDE : ordre alphabetique page2 < page3). */

const int G3_X1 = 1,   G3_X2 = 319;   // cadre elargi comme la page 2
const int G3_HAUT = 1,  G3_BAS = 226;
const int G3_ZERO = 60;               // zero a la meme hauteur que page 1
const int G3_Y_KWH = 202;             // prod totale, blanc, centré
const int G3_Y_LEG = 222;             // legende BigFont en bas
float p3_affKwh = -1;

void dessinerPage3() {
  myGLCD.clrScr();
  myGLCD.setColor(100, 100, 100);
  myGLCD.drawRect(G3_X1, G3_HAUT, G3_X2, G3_BAS+12);       // cadre
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawLine(G3_X1, G3_ZERO, G3_X2, G3_ZERO);      // ligne du 0 (blanche)

  for (int x = G3_X1; x <= G3_X2; x++) {
    int i = ((x - G3_X1) * (NB_SEMAINE - 1)) / (G3_X2 - G3_X1);  // instant du pixel

    if (x % 2 == 0) {                       // --- pixel PAIR : production (bleu)
      if (tabSemaine[i] != 65535) {
        // base : sommet de la barre conso au meme instant (meme principe que page 1)
        uint16_t consoIci = (tabSemaineC[i] == 65535) ? 0 : tabSemaineC[i];
        int base = G3_ZERO + profondeurConso(consoIci);
        int h = (int)((uint32_t)tabSemaine[i] * 45UL / 1000);  // 45 px / 1000 W
        if (h < 1) h = 1;
        int yHaut = base - h;                // sommet = niveau du total
        if (yHaut < G3_HAUT) yHaut = G3_HAUT;                 // ecretage au cadre
        myGLCD.setColor(0, 130, 255);
        myGLCD.drawLine(x, yHaut, x, base);                  // barre bleue
        // marqueur blanc : un pixel a l'extremite, un pixel au-dessus
        if (yHaut > G3_HAUT) {
          myGLCD.setColor(255, 255, 255);
          myGLCD.drawPixel(x, yHaut);
          myGLCD.drawPixel(x, yHaut - 1);
        } else {
          myGLCD.setColor(255, 255, 255);                     // cadre atteint : 1 seul
          myGLCD.drawPixel(x, yHaut);
        }
      }
    } else {                                // --- pixel IMPAIR : conso (orange)
      if (tabSemaineC[i] != 65535) {
        int d = profondeurConso(tabSemaineC[i]);
        if (d < 1) d = 1;
        if (G3_ZERO + d > G3_BAS) d = G3_BAS - G3_ZERO;       // limite du cadre
        myGLCD.setColor(255, 165, 0);
        myGLCD.drawLine(x, G3_ZERO, x, G3_ZERO + d);
      }
    }
  }

  // separateurs de jours (gris) : 1 barre tous les 48 points (24 h)
  myGLCD.setColor(100, 100, 100);
  for (int j = 1; j < 7; j++) {
    int x = G3_X1 + (int)((j * 48UL) * (G3_X2 - G3_X1) / NB_SEMAINE);
    myGLCD.drawLine(x, G3_HAUT, x, G3_BAS);
  }

  ecrireLegendePage3();
  p3_affKwh = -1;
  rafraichirPage3();
}

void ecrireLegendePage3() {
  // "CONSO+PROD SEMAINE" : 18 caracteres BigFont (16 px) = 288 px
  myGLCD.setFont(BigFont);
  int x = 160 - 144;                  // centre de l'ecran (320 px)
  myGLCD.setColor(255, 165, 0);  myGLCD.print("CONSO", x, G3_Y_LEG);         x += 5 * 16;
  myGLCD.setColor(255, 255, 255);  myGLCD.print("+", x, G3_Y_LEG);          x += 1 * 16;
  myGLCD.setColor(0, 130, 255);  myGLCD.print("PROD", x, G3_Y_LEG);         x += 4 * 16;
  myGLCD.setColor(255, 255, 255);  myGLCD.print(" SEMAINE", x, G3_Y_LEG);
}

void rafraichirPage3() {
  // production TOTALE en blanc, centree au-dessus de la legende, live
  if (prodTotKwh != p3_affKwh) {
    p3_affKwh = prodTotKwh;
    char buf[16];
    dtostrf(prodTotKwh, 0, 2, buf);          // ex. "45.30"
    strcat(buf, " Kwh");
    myGLCD.setFont(BigFont);
    myGLCD.setColor(0, 0, 0);                // efface (fillRect, pas d'espaces)
    myGLCD.fillRect(88, G3_Y_KWH, 232, G3_Y_KWH + 16);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(buf, CENTER, G3_Y_KWH);
  }
}
