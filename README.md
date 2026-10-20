# ALBI — mesure et affichage énergie maison

Mesure et affichage de la **consommation** et de la **production photovoltaïque** de la maison, par radio.

## Matériel

- **Émetteur** : Arduino + JSY-MK-163T (mesure de puissance) + HC12 (radio 433 MHz)
- **Récepteur** : Arduino Mega 2560 + shield TFT ILI9341 240x320 (UTFTGLUE) + PZEM-004T v3.0 (production PV) + HC12
- Trame radio `P<puissance>I<import>E<export>\n` à 9600 bauds

## Câblage récepteur (Mega 2560)

- HC12 sur **Serial3** : TX3 = D14, RX3 = D15
- PZEM-004T v3.0 sur **Serial1** : TX1 = D18, RX1 = D19
- Boutons : blanc D26/D30, bleu D44/D48 (une broche de chaque paire sert de masse commune)

## Sketchs

- `recepteur/recepteur_v1.4.ino` — récepteur : affichage consommation + production, interrogation PZEM 1/s, rafraîchissement conditionnel
- `testeur/test_pzem_brut.ino` — diagnostic PZEM : requête Modbus brute + affichage hexadécimal

## Historique des corrections

- Radio : HC12 en réalité sur Serial3, PZEM sur Serial1 (v1.3)
- PZEM « ? » : défaut de câblage sur D19 (corrigé oct. 2026)

## Licence

MIT — voir [LICENSE](LICENSE)
