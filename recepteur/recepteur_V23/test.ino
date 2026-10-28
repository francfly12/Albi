/* TEST : simule conso/prod/tension pour animer les graphiques
   sans circuits reels. testValeurs() dans loop() et
   remplirTest() dans setup(), seulement si MODE_TEST = 1. */

float delta_conso = 40.0;
float delta_prod   = 25.0;

void testValeurs() {
  // conso : randonne entre 0 et 3000 W avec bruit
  consoW += delta_conso + random(100) - 50;
  if (consoW > 3000)  { consoW = 3000; delta_conso = -delta_conso; }
  if (consoW < 0)     { consoW = 0;     delta_conso = -delta_conso; }

  // prod : cycle lent 0..800 W (simule le passage du soleil)
  static float temps = 0;
  temps += 0.02;
  prodW = 400.0 + 400.0 * sin(temps);      // entre ~0 et 800 W
  if (prodW < 0) prodW = 0;

  // tension : 232 V +/- 3 V de bruit
  tensionV = 232.0 + random(7) - 3;
}   // <-- fermeture de testValeurs, AVANT le #if

#if MODE_TEST

void remplirTest() {
  for (int i = 0; i < NB_JOUR; i++) {
    // production : cloche de midi (0 a 800 Wc), nulle la nuit
    float angle = (i - 70) / 100.0;               // sommet vers midi
    uint16_t p = (uint16_t)(800.0 * exp(-angle * angle));
    if (i < 40 || i > 220) p = 0;                 // nuit
    // conso : pic matin + soiree, fond permanent
    uint16_t c = 300 + ((i % 12) < 3 ? 2200 : 0)  // mini-cycles
               + (i > 200 && i < 240 ? 3500 : 0)  // soir
               + (i > 80 && i < 100 ? 1500 : 0);  // matin
    tabJour[i]  = min(p, (uint16_t)65534);
    tabJourC[i] = min(c, (uint16_t)65534);
  }
}
void remplirTestSemaine() {
  for (int i = 0; i < NB_SEMAINE; i++) {
    int heure = (i % 48) * 30 / 60;              // 0..23 h dans le jour
    int jour  = i / 48;                          // 0..6
    // production : cloche de midi, amplitude variable selon le jour (meteo)
    float centre = 12.0;
    float p = 800.0 * exp(-((heure - centre) * (heure - centre)) / 12.0)
              * (0.5 + 0.5 * (jour % 3) / 2.0); // jour 1 fort, 2 moyen, 3 faible...
    if (heure < 6 || heure > 19) p = 0;
    // conso : fond + pics matin/soir, plus fort le week-end (jours 5,6)
    uint16_t c = 300
               + (heure >= 7  && heure <= 9  ? 2000 : 0)
               + (heure >= 18 && heure <= 22 ? (jour >= 5 ? 4500 : 3500) : 0);
    tabSemaine[i]  = min((uint16_t)p, (uint16_t)65534);
    tabSemaineC[i] = min(c, (uint16_t)65534);
    // Serial.print (tabSemaine[i]); Serial.print ("  ");
    // Serial.println  (tabSemaineC[i]);
  }
}

#endif
