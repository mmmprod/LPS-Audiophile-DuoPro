# LPS Audiophile "DUO PRO" 🎵

**Alimentation linéaire audiophile double rail avec contrôle numérique**

![Made in France](https://img.shields.io/badge/Made%20in-France%20🇫🇷-blue)
![Version](https://img. shields.io/badge/Version-2. 4.2-green)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Tests](https://img.shields.io/badge/Tests-20%20PASS-brightgreen)

---

## 📋 Description

Le **LPS DUO PRO** est une alimentation linéaire haute qualité conçue pour les équipements audio exigeants (DAC, préampli, streamer.. .). Elle offre deux rails de sortie indépendants avec une régulation ultra-basse bruit grâce à l'architecture **LM317 + LT3045**.

### ✨ Caractéristiques principales

- 🎚️ **Double rail indépendant** : 5-15V ajustable par encodeur
- 🔇 **Ultra-basse bruit** : LT3045 post-regulator (<1µVrms)
- 🛡️ **Protections complètes** : OVP, OCP adaptatif, OTP, Backfeed
- 📺 **Interface OLED** : Double écran 128×64 avec 5 pages d'information
- 🌍 **Multi-langue** : Anglais / Français / Allemand
- 🎧 **Mode Purist** : Écrans OFF pour réduire les perturbations EMI
- ⚡ **Tension ajustable** : Réglage en temps réel via digipot MCP41100

---

## 🔧 Spécifications techniques

| Paramètre | Valeur |
|-----------|--------|
| Tension entrée | 230VAC 50Hz |
| Tension sortie | 5-15V DC (par rail) |
| Courant max | 350-500mA (adaptatif selon tension) |
| Bruit sortie | <10µVrms typique |
| Régulation | LM317 + LT3045 cascade |
| Affichage | 2× OLED SSD1306 128×64 |
| Contrôleur | ATmega328P @ 16MHz |

### Limites de courant adaptatives (protection thermique LM317)

| Tension sortie | Courant max |
|----------------|-------------|
| 5-6V | 350mA |
| 7-9V | 450mA |
| 10-15V | 500mA |

---

## 📁 Structure du projet

```
├── firmware/                    # Code Arduino (ATmega328P)
│   └── LPS_Audiophile_V2_4_2/
│       └── LPS_Audiophile_V2_4_2.ino
│
├── hardware/                    # Documentation circuit
│   └── Circuit_V2_4_1. md
│
├── tests/                       # Tests unitaires (PC)
│   └── test_digipot_conversion.cpp
│
└── docs/                        # Documentation utilisateur
```

---

## 🚀 Installation du firmware

### Prérequis

- Arduino IDE 1.8+ ou 2.x
- Carte : Arduino Nano / ATmega328P
- Câble USB

### Bibliothèques requises

Installer via le Gestionnaire de bibliothèques Arduino :

| Bibliothèque | Auteur |
|--------------|--------|
| Adafruit GFX Library | Adafruit |
| Adafruit SSD1306 | Adafruit |
| Adafruit INA219 | Adafruit |

### Téléversement

1. Ouvrir `firmware/LPS_Audiophile_V2_4_2/LPS_Audiophile_V2_4_2.ino`
2. Sélectionner **Outils → Type de carte → Arduino Nano**
3. Sélectionner **Outils → Processeur → ATmega328P**
4.  Sélectionner le port COM
5. Cliquer sur **Téléverser**

---

## 🧪 Tests unitaires

Les conversions digipot ↔ tension peuvent être testées sur PC :

### Compilation (Windows avec MinGW)

```cmd
cd tests
g++ -o test_digipot. exe test_digipot_conversion.cpp
test_digipot. exe
```

### Compilation (Linux/Mac)

```bash
cd tests
g++ -o test_digipot test_digipot_conversion.cpp -lm
./test_digipot
```

### Résultat attendu

```
============================================================
  RÉSULTATS: 20 PASS, 0 FAIL
============================================================
```

---

## 📖 Pages de l'interface

| Page | Contenu |
|------|---------|
| 1 | Sortie Rail A (tension, courant, état) |
| 2 | Sortie Rail B (tension, courant, état) |
| 3 | Score qualité alimentation |
| 4 | Session (uptime, énergie) |
| 5 | Santé (température, headroom) |

### Navigation

- **Rotation encodeur** : Changer de page
- **Clic court** (page 1 ou 2) : Mode réglage tension
- **Appui 3s** : Mode Purist (écrans OFF)
- **Appui 5s** (page 5) : Changer la langue

---

## 🛡️ Protections

| Protection | Seuil | Action |
|------------|-------|--------|
| OVP (surtension) | >16V | Coupure sortie |
| OVP Pre | >17. 5V | Coupure sortie |
| OCP (surintensité) | Adaptatif | Coupure + message |
| OTP (surchauffe) | >85°C | Coupure (auto-reset <60°C) |
| Backfeed | <-20mA | Coupure sortie |

---

## 📝 Changelog

### V2.4.2 (Novembre 2025)
- ✅ Fix `updateEnergy()` : delta temps réel (bug Purist ×5)
- ✅ Ajout tests unitaires `test_digipot_conversion.cpp`

### V2.4.1
- ✅ OCP adaptatif selon V_OUT (protection thermique LM317)
- ✅ `validateDigipotRange()` calcul brut sans constrain
- ✅ Documentation hardware corrigée (F_SECTEUR 315mA, C_MAIN 50V)

### V2.4.0
- ✅ Architecture post-regulator adaptative
- ✅ MCP41100 sur feedback LM317 (wiper à 1. 25V, dans specs)
- ✅ Dissipation LT3045 réduite à ~1W

### V2.3.x
- Multi-langue (EN/FR/DE)
- Sweep tuning avec digipot
- Solutions guidées fautes

---

## 🔌 Hardware requis

| Composant | Référence | Quantité |
|-----------|-----------|----------|
| Régulateur | LM317T TO-220 | 2 |
| Post-régulateur | LT3045EDD DFN-8 | 2 |
| Digipot | MCP41100-I/P | 2 |
| Microcontrôleur | ATmega328P-PU | 1 |
| Écran OLED | SSD1306 128×64 I2C | 2 |
| Capteur courant | INA219 module | 2 |
| Transformateur | 2×18VAC 30VA | 1 |

Voir `hardware/Circuit_V2_4_1.md` pour le schéma complet. 

---

## 📄 Licence

MIT License - Voir [LICENSE](LICENSE)

---

## 👨‍💻 Auteur

**Mehdi** - Made in France 🇫🇷

---

## 🙏 Remerciements

- Analog Devices pour le LT3045
- Microchip pour le MCP41100 et ATmega328P
- Adafruit pour les bibliothèques Arduino
