# ALBI — mesure et affichage énergie maison

Mesure et affichage de la **consommation** et de la **production photovoltaïque** de la maison, par radio.
<img width="1440" height="1080" alt="20261023_150039 (Grand)" src="https://github.com/user-attachments/assets/2f037278-31c3-4eea-9fb8-72a12122beb9" />

## Matériel

- **Émetteur** : Arduino Mega 2560 + JSY-MK-163T v1.2 (mesure au compteur) + HC12 (radio, 9600 bauds)
- **Récepteur** : Arduino Mega 2560 + shield TFT ILI9341 240x320 (UTFTGLUE) + PZEM-004T v3.0 (production PV) + HC12
- **Testeur radio** : Arduino UNO R4 Minima + HC12 (sniffeur de trames)
- Trame radio `P<puissance>;I<import>;E<export>\n` à 9600 bauds

## Câblage émetteur (Mega 2560)

- HC12 sur **Serial3** : TX3 = D14 (jaune) → RXD HC12 ; RX3 = D15 (vert) → TXD HC12
- JSY-MK-163T sur **Serial2** : TX2 = D16 (orange) → RX JSY ; RX2 = D17 (rouge) → TX JSY, 4800 bauds
- Bouton poussoir D50, LED bleue D52 (résistance 390 Ω), masse commune D48 (maintenu à 0 V)

## Câblage récepteur (Mega 2560)

- HC12 sur **Serial3** : TX3 = D14, RX3 = D15
- PZEM-004T v3.0 sur **Serial1** : TX1 = D18, RX1 = D19
- Boutons : blanc D26/D30, bleu D44/D48 (une broche de chaque paire sert de masse commune)
- Écran : `InitLCD(LANDSCAPE)` (la constante `LANDSCAPE_REV` n'est pas acceptée par cette version d'UTFTGLUE)

## Câblage testeur radio (UNO R4 Minima)

- HC12 sur **Serial1** : HC12 RXD ← D1 (TX1), HC12 TXD → D0 (RX1), 9600 bauds
- Broche SET en l'air (mode communication). D0/D1 partagés avec l'USB.

## Sketchs

| Fichier | Rôle |
| --- | --- |
| `emetteur/emetteur_v5.ino` | Mesure JSY-MK-163T + émission radio (trame avec `\n` final), sens inversable (bouton, EEPROM) |
| `recepteur/recepteur_v1.6.ino` | Réception radio + PZEM, affichage ILI9341 (version de production) |
| `testeur/test_radio_sniffeur.ino` | Sniffeur HC12 : trames horodatées, débit moyen |
| `testeur/test_pzem_brut.ino` | Diagnostic PZEM : requête Modbus brute + affichage hexa |
| `testeur/test_radio_sniffeur.ino` | Sniffeur HC12 |

## Bibliothèques

- `ApJSY163T` (antonioprevitali) : installation manuelle en ZIP — https://github.com/antonioprevitali/ApJSY163T
- `PZEM004Tv30` (Jakub Mandula) : gestionnaire de bibliothèques de l'IDE
- `UTFTGLUE` : afficheur ILI9341 en parallèle 8 bits

## Historique des corrections

- **Radio émetteur** : HC12 cramé, remplacé (découvert via le sniffeur)
- **Radio récepteur** : HC12 en réalité sur Serial3, PZEM sur Serial1 (v1.3)
- **PZEM « ? »** : défaut de câblage sur D19 (corrigé oct. 2026)
- **« Pas de radio » intermittents (~30 %)** : la trame émetteur ne comportait pas de `\n` final ; corrigé côté émetteur (v5, `Serial3.print('\n')`) et côté récepteur (v1.5, fin de trame par `\n` OU silence de 100 ms). Reste ~10 % de trames perdues, accepté.

## Licence

MIT — voir [LICENSE](LICENSE)
