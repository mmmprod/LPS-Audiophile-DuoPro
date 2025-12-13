# 🎵 Amplificateur Audiophile Portable

Amplificateur stéréo Class-D 2×20W avec préampli phono, Bluetooth LDAC, égaliseur 3 bandes et contrôle numérique.

![Hardware](https://img.shields.io/badge/Hardware-v1.5-blue)
![Firmware](https://img.shields.io/badge/Firmware-v1.5-green)
![Status](https://img.shields.io/badge/status-active-success)

## ✅ Versions recommandées

| Hardware | Firmware | Statut | Notes |
|----------|----------|--------|-------|
| **V1.5** | **V1.5** | ✅ Recommandé | Corrections audit sécurité (protection PVDD, TVS, nappe blindée) |
| V1.4 | V1.4 | 📦 Stable | TDA7439 (EQ 3 bandes), corrections fiabilité |
| V1.3 | V1.3 | 📦 Archive | PT2314 + MCP4261 (legacy) |

> ⚠️ **V1.5 fortement recommandée** : corrige un risque de destruction de l'ampli MA12070 en cas de batterie pleine + back EMF.

## ✨ Caractéristiques

- **Puissance** : 2 × 20W RMS @ 8Ω (MA12070 Class-D)
- **Sources** : Bluetooth LDAC/aptX HD (BTM525 QCC5125), AUX 3.5mm, Phono MM (préampli RIAA OPA2134)
- **Égaliseur 3 bandes** : Bass/Mid/Treble ±14dB (pas 2dB), loudness automatique, effet spatial, 8 presets
- **Volume & gain** : contrôle intégré TDA7439 (0 à -47dB + mute), gain d'entrée ajustable 0-30dB
- **Contrôle** : encodeur rotatif + OLED 128×64 + télécommande IR
- **Alimentation** : Batterie LiPo 6S (18-25V) avec BMS, autonomie 4-6h
- **Sécurité** : 5 niveaux de protection batterie (BMS, TCO, relais, fusible, TVS)

## 🛡️ Corrections V1.5 (Audit Sécurité)

| Problème identifié | Solution V1.5 |
|--------------------|---------------|
| MA12070 PVDD 26V max vs batterie 25.2V (marge 0.8V insuffisante) | Diode Schottky D3 (SS54) série → PVDD 24.7V max (marge 1.3V) |
| TVS SMBJ26CA clamp trop tard (Vbr=28.9V > 26V) | TVS SMBJ24CA (Vbr=26.7V) → clamp AVANT destruction |
| Crosstalk I2C → Audio sur nappe 14 pins | Nappe 16 pins avec GND blindage entre signaux |
| Risque blocage I2C (vibrations connecteur) | Timeout I2C 10ms anti-blocage |

## 🚀 Démarrage rapide

### 1) Choisir la documentation hardware

| Version | Guide |
|---------|-------|
| **V1.5 (recommandée)** | [docs/Ampli_Audiophile_Portable_V1_5.md](docs/Ampli_Audiophile_Portable_V1_5.md) |
| V1.4 | [docs/Hardware_V1_4.md](docs/Hardware_V1_4.md) |
| V1.3 (archive) | [docs/Ampli_Audiophile_Portable_V1_3.md](docs/Ampli_Audiophile_Portable_V1_3.md) |
| Outil de test | [docs/Breakout_Box_V1.md](docs/Breakout_Box_V1.md) |

### 2) Sélectionner le firmware

| Votre hardware | Firmware à flasher |
|----------------|-------------------|
| **V1.5** (TDA7439 + protections) | `firmware/Firmware_Ampli_V1_5.ino` |
| V1.4 (TDA7439) | `firmware/Firmware_Ampli_V1_4.ino` |
| V1.3 (PT2314 + MCP4261) | `firmware/Ampli_V1_3.ino` |

1. Installer l'IDE Arduino + ESP32 Core 2.0+
2. Ajouter les bibliothèques : `Adafruit_GFX`, `Adafruit_SSD1306`, `IRremoteESP8266`
3. Ouvrir le fichier `.ino` correspondant, sélectionner **ESP32S3 Dev Module**, puis uploader

### 3) Assemblage

- Architecture bi-carte :
  - **Carte 1** (80×100mm) : alimentation/BMS + MA12070 + protection PVDD
  - **Carte 2** (80×120mm) : ESP32, Bluetooth, DAC, égaliseur, préampli phono
- Liaison par **nappe JST XH 16 pins** (V1.5) avec blindage GND
- Détails dans [docs/Ampli_Audiophile_Portable_V1_5.md](docs/Ampli_Audiophile_Portable_V1_5.md)

## 📊 Architecture Simplifiée

```
┌─────────────────────────────────────────────────────────────────┐
│                        CARTE 1 (Puissance)                      │
│  BATTERIE 6S → BMS → TCO → RELAIS → FUSIBLE → D1 (SS54)        │
│                                                ↓                │
│                                            +22V_RAW             │
│                                                ↓                │
│                              D3 (SS54) ← Protection PVDD V1.5   │
│                                                ↓                │
│                                          +PVDD_SAFE             │
│                                                ↓                │
│                                            MA12070              │
│                                           2×20W → HP            │
└─────────────────────────────────────────────────────────────────┘
                              ↕ Nappe 16 pins (blindée)
┌─────────────────────────────────────────────────────────────────┐
│                        CARTE 2 (Signal)                         │
│  PHONO → OPA2134 → ┐                                            │
│  AUX   ───────────→├→ CD4053 MUX → TDA7439 EQ → Buffer → Nappe │
│  BT    → PCM5102A →┘      ↑                                     │
│                        ESP32-S3                                 │
│                    (OLED + Encodeur + IR)                       │
└─────────────────────────────────────────────────────────────────┘
```

## 🧪 Tests et diagnostics

Firmware V1.5 inclut des commandes série :

- `i2ctest` : détection des périphériques et comptage des erreurs
- `adctest` : filtre médian sur 5 échantillons
- `stats` : statistiques complètes et watchdog
- `eqtest` : test égaliseur TDA7439

## 📦 BOM Estimatif

| Catégorie | Coût approx. |
|-----------|--------------|
| Semiconducteurs (ESP32, MA12070, TDA7439, etc.) | ~45 € |
| Passifs (résistances, condensateurs) | ~15 € |
| Connecteurs et câbles | ~12 € |
| Module Bluetooth BTM525 | ~15 € |
| Divers (PCB, boîtier, etc.) | ~15 € |
| **Total (hors batterie et HP)** | **~102 €** |

## 🤝 Contribution

Les contributions sont les bienvenues : ouverture d'issues, propositions d'amélioration et pull requests. Consultez la licence pour les conditions d'usage.

## 📜 Licence

Projet sous licence propriétaire à usage non commercial. Usage commercial sur demande. Voir le fichier [LICENSE](LICENSE).

---

## 📋 Changelog

### V1.5 (Décembre 2025)
- 🛡️ **Protection PVDD** : Diode Schottky SS54 série (24.7V max vs 26V limit)
- 🛡️ **TVS adaptée** : SMBJ24CA (Vbr=26.7V) remplace SMBJ26CA
- 🔇 **Anti-crosstalk** : Nappe 16 pins avec GND blindage
- ⏱️ **I2C robuste** : Timeout 10ms anti-blocage
- 📖 Documentation protection PVDD dans firmware

### V1.4 (Décembre 2025)
- 🎛️ TDA7439 remplace PT2314+MCP4261 (EQ 3 bandes intégré)
- 🔊 Loudness automatique selon volume
- 🎚️ Effet spatial/surround
- 🎵 8 presets sonores
- 🛡️ Filtre médian ADC, section critique encodeur, I2C retry

### V1.3 (Novembre 2025)
- Version initiale avec PT2314 + MCP4261

---

**🎵 Enjoy high-fidelity audio!**
