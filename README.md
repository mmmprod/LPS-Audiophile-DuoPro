# LPS Audiophile "DUO PRO" 🎵

**Alimentation linéaire audiophile double rail avec contrôle numérique**

![License](https://img.shields.io/badge/License-MIT-yellow)

## 📋 Description

Le LPS DUO PRO est une alimentation linéaire haute qualité conçue pour les équipements audio exigeants. Elle offre deux rails de sortie indépendants (5-15V) avec une régulation ultra-basse bruit grâce à l'architecture LM317 + LT3045.

## ✨ Caractéristiques

- **Double rail indépendant** : 5-15V ajustable par encodeur
- **Ultra-basse bruit** : LT3045 post-regulator (<1µVrms)
- **Protections complètes** : OVP, OCP adaptatif, OTP, Backfeed
- **Interface OLED** : Double écran avec 5 pages d'information
- **Multi-langue** : EN/FR/DE
- **Mode Purist** : Écrans OFF pour réduire les perturbations EMI

## 🔧 Spécifications

| Paramètre | Valeur |
|-----------|--------|
| Tension entrée | 230VAC 50Hz |
| Tension sortie | 5-15V DC (par rail) |
| Courant max | 350-500mA (selon tension) |
| Bruit sortie | <10µVrms |
| Protections | OVP/OCP/OTP/Backfeed |

## 📁 Structure du projet

```
├── firmware/          # Code Arduino (ATmega328P)
├── hardware/          # Documentation circuit et schémas
├── docs/              # Manuel utilisateur
└── releases/          # Changelog
```

## 🚀 Installation

1. Ouvrir `firmware/LPS_Audiophile_V2_4_1/LPS_Audiophile_V2_4_1.ino` dans Arduino IDE
2. Installer les bibliothèques requises (voir ci-dessous)
3. Sélectionner "Arduino Nano" ou "ATmega328P"
4. Téléverser

### Bibliothèques requises

- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit INA219

## 📄 Licence

MIT License - Voir [LICENSE](LICENSE)

## 👨‍💻 Auteur

Mehdi - Made in France 🇫🇷
