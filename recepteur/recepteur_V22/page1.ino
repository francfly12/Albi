/* PAGE 1 (v2 finale) : barres verticales — pas d'arcs, affichage rapide.
   Index gauche non lineaire : 0->1000 grand, puis 2000..8000 dilates
   pour que 8000 soit en bas de l'echelle.
   Boite orange = conso (du 0 vers le bas), boite bleue = prod
   (assise sur le niveau conso, barre blanche au niveau du TOTAL).
   "EXCEDENT" en bleu en bas de page si prod >= conso. */

// ---- geometrie (ajustable apres test ecran) ----
const int P1_Y0   = 60;    // ligne du zero
const int P1_KY   = 45;    // px pour 0->1000 W au-dessus du zero (prod)
const int P1_XOR  = 60;    // bord gauche boite orange (largeur 40)
const int P1_XBL  = 105;   // bord gauche boite bleue  (largeur 40)

float p1_consoAvant = -1, p1_prodAvant = -1;
float p1_affP = -1, p1_affC = -1, p1_affT = -1;

// echelle non lineaire, DIRECTEMENT EN PIXELS sous le zero :
// 1000 -> 70 ; 2000 -> 105 ; 4000 -> 125 ; 6000 -> 145 ; 8000 -> 165 (bas)
float p1_echelle(float w) {
  const float pts[6][2] = {
    {0, 0}, {1000, 70}, {2000, 105}, {4000, 125}, {6000, 145}, {8000, 165}
  };
  if (w <= 0) return 0;
  if (w >= 8000) return 165;
  for (int i = 0; i < 5; i++) {
    if (w >= pts[i][0] && w <= pts[i + 1][0]) {
      float t = (w - pts[i][0]) / (pts[i + 1][0] - pts[i][0]);
      return pts[i][1] + t * (pts[i + 1][1] - pts[i][1]);
    }
  }
  return 165;
}

// y d'une valeur : conso positive sous le zero (echelle non lineaire),
// total negatif (excendent) au-dessus du zero (lineaire, echelle prod)
int p1_yDe(float v) {
  if (v >= 0) return P1_Y0 + (int)p1_echelle(v);   // plus de facteur KY/90 !
  return P1_Y0 + (int)(v * P1_KY / 1000.0);        // negatif -> au-dessus du zero
}

// affiche une valeur sur fond propre : efface TOUJOURS le meme rectangle
// noir (6 chiffres BigFont = 96 px), puis ecrit la valeur dedans
void p1_affValeur(int x, int y, float val, byte r, byte v, byte b) {
  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(x, y, x + 95, y + 15);     // zone fixe, 6 chiffres de large
  myGLCD.setColor(r, v, b);
  myGLCD.printNumF(val, 0, x, y);
}

void dessinerPage1() {
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);

  // ---- symbole 1 : index gris clair ----
  myGLCD.setColor(254, 254, 254);              // blanc
  myGLCD.fillRect(45, P1_Y0, P1_XOR - 3, P1_Y0 + 2);   // ligne du zero
  myGLCD.setColor(200, 200, 200);              // gris clair
  myGLCD.fillRect(42, 1, 44, 240);            // ligne verticale
  myGLCD.setColor(0, 130, 255);
  myGLCD.print("1000", 5, P1_Y0 - P1_KY - 15);        // haut, bleu
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("0", 25, P1_Y0 - 8);
  myGLCD.setColor(255, 165, 0);
  myGLCD.print("1000", 5, p1_yDe(1000) - 8);
  myGLCD.print("2000", 5, p1_yDe(2000) - 8);
  myGLCD.print("4000", 5, p1_yDe(4000) - 8);
  myGLCD.setColor(255, 0, 0);
  myGLCD.print("6000", 5, p1_yDe(6000) - 8);
  myGLCD.print("8000", 5, p1_yDe(8000) - 8);

  // ---- symboles 5, 6, 7 : colonne de droite ----
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 130, 255);
  myGLCD.print("PROD.", 220, 30);
  myGLCD.setColor(255, 165, 0);
  myGLCD.print("CONSO", 220, 95);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("TOTAL", 220, 165);

  p1_consoAvant = -1; p1_prodAvant = -1;
  p1_affP = -1; p1_affC = -1; p1_affT = -1;
  rafraichirPage1();
}

void rafraichirPage1() {
  float total = consoW - prodW;

  // ---- symboles 2 et 3 : boites orange (conso) et bleue (prod) ----
  if (consoW != p1_consoAvant || prodW != p1_prodAvant) {
    // effacer l'ancienne zone des barres (noir)
    int yMax = p1_yDe(max(p1_consoAvant, consoW));
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(P1_XOR - 2, P1_Y0 - P1_KY - 2, P1_XBL + 72, yMax + 2);

    // boite orange : du zero jusqu'a la conso (largeur 40)
    int yC = p1_yDe(consoW);
    myGLCD.setColor(255, 165, 0);
    myGLCD.fillRect(P1_XOR, P1_Y0, P1_XOR + 40, yC);

    // boite bleue : du total (sommet) jusqu'a la conso (base, largeur 40)
    int yT = p1_yDe(total);
    myGLCD.setColor(0, 130, 255);
    myGLCD.fillRect(P1_XBL, yT, P1_XBL + 40, yC);

    // barre blanche horizontale au niveau du total (haut de la boite bleue)
    myGLCD.setColor(255, 255, 255);
    myGLCD.fillRect(P1_XBL, yT - 5, P1_XBL + 70, yT - 1);

    p1_consoAvant = consoW;
    p1_prodAvant = prodW;
  }

  // ---- valeurs de la colonne de droite ----
  myGLCD.setFont(BigFont);
  if (prodW != p1_affP) {
    p1_affP = prodW;
    p1_affValeur(220, 52, prodW, 0, 130, 255);        // bleu
  }
  if (consoW != p1_affC) {
    p1_affC = consoW;
    p1_affValeur(220, 117, consoW, 255, 165, 0);     // ambre
  }
  if (total != p1_affT) {
    p1_affT = total;
    p1_affValeur(220, 182, total, 255, 255, 255);    // blanc
  }

  // ---- symbole 8 : EXCEDENT si production >= consommation ----
  static bool p1_excedentAffiche = false;
  bool excedent = (total <= 0);                 // prod >= conso
  if (excedent != p1_excedentAffiche) {
    p1_excedentAffiche = excedent;
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillRect(180, 212, 315, 227);         // efface la zone (fixe)
    if (excedent) {
      myGLCD.setColor(0, 130, 255);
      myGLCD.print("EXCEDENT", 180, 212);        // bleu, BigFont
    }
  }
}
