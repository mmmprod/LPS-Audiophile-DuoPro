# LPS DUO PRO - Simulateur UI (PyGame V92)

Simulateur PC de l'interface pour développement et tests.

> ⚠️ **Note** : Ce simulateur est pour le développement sur PC uniquement.
> L'interface production tourne sur **ESP32-8048S050C** avec LVGL (code C++ dans `/esp32/`).

## Architecture système

- **ESP32-8048S050C** : Écran LCD 5" intégré 800×480, tactile capacitif, ESP32-S3
- **ATmega328P** : Régulation LDO, protections OVP/OCP/OTP, mesures ADC
- **Communication** : UART entre ESP32 et ATmega

## Prérequis simulateur

- Python 3.7+
- Écran 800×480 minimum

## Installation

```bash
cd ui-simulator
pip install -r requirements.txt
```

## Lancement

```bash
python lps_duo_pro.py
```

## Raccourcis clavier

| Touche | Action |
|--------|--------|
| `1-5` | Navigation pages |
| `ESC` | Fermer popup / Quitter |
| `ENTER` | Démarrer (écran boot) |

## Pages

1. **ÉCOUTE** - Jauges voltage principales style VU-mètre
2. **DÉTAILS** - Métriques détaillées par rail avec Nixie bars
3. **SANTÉ** - Statut système et protections OVP/OCP/OTP
4. **SESSION** - Timer et énergie consommée
5. **CONFIG** - Paramètres, langue, simulations de pannes

## Mode Simulation

Le simulateur génère des données fictives pour tester l'affichage sans matériel.
Les simulations de pannes sont disponibles dans la page CONFIG :
- NORM : Fonctionnement normal
- HOT : Surchauffe (OTP)
- RIP : Bruit excessif (ripple)
- LOAD : Surcharge (OCP)
- LO-V : Sous-tension
- HI-V : Surtension (OVP)

## Optimisations V92

- `draw.lines()` pour oscilloscopes (690→3 appels/frame)
- Cache `get_all_problems()` par frame
- Layout centralisé (ECOUTE_LAYOUT, DETAILS_LAYOUT, etc.)
- LCD agrandi 95px
- Headroom 2.0V conforme circuit V2.4.5
