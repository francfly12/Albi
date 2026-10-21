/* PAGE 3 : conso + prod sur la semaine (336 points de 30 min).
   Prod vers le haut (bleu), conso vers le bas (ambre), ligne 0 au milieu,
   separateurs de jours (1 barre grise tous les 48 points). */

const int G3_HAUT = 30, G3_ZERO = 120, G3_BAS = 220;

void dessinerPage3() {
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Conso+Prod Semaine", CENTER, 5);
  myGLCD.setColor(100, 100, 100);
  myGLCD.drawRect(10, G3_HAUT, 310, G3_BAS);
  myGLCD.drawLine(10, G3_ZERO, 310, G3_ZERO);

  // production vers le haut (bleu)
  myGLCD.setColor(0, 130, 255);
  for (int i = 0; i < NB_SEMAINE; i++) {
    if (tabSemaine[i] == 65535) continue;
    int x = 10 + (i * 300UL) / NB_SEMAINE;
    int h = (int)((G3_ZERO - G3_HAUT) * tabSemaine[i] / PROD_MAX_GRAPH);
    if (h < 1) h = 1;
    myGLCD.drawLine(x, G3_ZERO - h, x, G3_ZERO);
  }
  // consommation vers le bas (ambre)
  myGLCD.setColor(255, 165, 0);
  for (int i = 0; i < NB_SEMAINE; i++) {
    if (tabSemaineC[i] == 65535) continue;
    int x = 10 + (i * 300UL) / NB_SEMAINE;
    int h = (int)((G3_BAS - G3_ZERO) * tabSemaineC[i] / CONSO_MAX_GRAPH);
    if (h < 1) h = 1;
    myGLCD.drawLine(x, G3_ZERO, x, G3_ZERO + h);
  }
  // separateurs de jours (gris)
  myGLCD.setColor(100, 100, 100);
  for (int j = 1; j < 7; j++) {
    int x = 10 + (j * 48 * 300UL) / NB_SEMAINE;
    myGLCD.drawLine(x, G3_HAUT, x, G3_BAS);
  }
}

void rafraichirPage3() { /* graphe statique : rien a faire */ }
