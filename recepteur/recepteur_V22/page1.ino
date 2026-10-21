/* PAGE 1 : jauge demi-cercle (net = conso - prod), aiguille blanche,
   graduations gris. Echelle -800 W (a gauche) a +9000 W (a droite). */

const int JX = 160, JY = 150, JR = 95;   // centre et rayon de la jauge
float p1_netAffiche = -99999;

float angleJauge(float net) {             // degres, 180 = gauche, 0 = droite
  if (net < -800) net = -800;
  if (net > 9000) net = 9000;
  return 180.0 - (net + 800.0) * 180.0 / 9800.0;
}

void dessinerPage1() {
  myGLCD.clrScr();

  // arc de la jauge (gris)
  myGLCD.setColor(100, 100, 100);
  for (int a = 0; a <= 180; a += 2) {
    float r = a * 3.14159 / 180.0;
    myGLCD.drawPixel(JX - JR * cos(r), JY - JR * sin(r));
  }
  // graduations tous les 1000 W (gris)
  for (int w = 0; w <= 9000; w += 1000) {
    float r = angleJauge(w) * 3.14159 / 180.0;
    myGLCD.setColor(100, 100, 100);
    myGLCD.drawLine(JX - (JR - 8) * cos(r), JY - (JR - 8) * sin(r),
                    JX - JR * cos(r),        JY - JR * sin(r));
  }
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(100, 100, 100);
  myGLCD.printNumI(0,    JX - JR - 12, JY + 4);
  myGLCD.printNumI(9000, JX + JR - 28, JY + 4);

  // ligne de conso / prod en bas (ambre / bleu)
  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 165, 0);
  myGLCD.print("CONSO", LEFT, 200);
  myGLCD.setColor(0, 130, 255);
  myGLCD.print("PROD.", 170, 200);

  p1_netAffiche = -99999;
  rafraichirPage1();
}

void rafraichirPage1() {
  float net = consoW - prodW;
  if (net != p1_netAffiche) {
    // effacer l'ancienne aiguille (noir) puis tracer la nouvelle (blanc)
    float rOld = angleJauge(p1_netAffiche) * 3.14159 / 180.0;
    myGLCD.setColor(0, 0, 0);
    myGLCD.drawLine(JX, JY, JX - (JR - 12) * cos(rOld), JY - (JR - 12) * sin(rOld));
    float rNew = angleJauge(net) * 3.14159 / 180.0;
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawLine(JX, JY, JX - (JR - 12) * cos(rNew), JY - (JR - 12) * sin(rNew));
    p1_netAffiche = net;
  }
  // valeurs numeriques
  myGLCD.setColor(255, 165, 0);
  myGLCD.printNumF(consoW, 0, LEFT, 216);
  myGLCD.setColor(0, 130, 255);
  myGLCD.printNumF(prodW, 0, 170, 216);
}
