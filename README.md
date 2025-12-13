# LPS DUO PRO

## Alimentation Linéaire Double Rail Premium pour Audiophiles

![Version](https://img.shields.io/badge/version-3.1.0-blue)
![License](https://img.shields.io/badge/license-Proprietary-red)
![Made in](https://img.shields.io/badge/Made%20in-France-blue)

---

## 🎯 Présentation

Le **LPS DUO PRO** est une alimentation linéaire ultra-low noise conçue pour les audiophiles exigeants. Elle permet d'alimenter DACs, streamers, amplificateurs casque et autres équipements audio avec une qualité d'alimentation inégalée dans sa catégorie de prix.

**Prix cible : 599€ TTC**

### Positionnement Marché

| Segment | Exemples | Prix |
|---------|----------|------|
| Entrée de gamme | Alimentations chinoises | 50-150€ |
| **→ LPS DUO PRO** | **Notre produit** | **599€** |
| Premium | Farad Super3, Keces P8 | 700-1200€ |
| Ultra Premium | Uptone JS-2, Ferrum HYPSOS | 1000-2000€ |

---

## ⚡ Spécifications Techniques

### Performance Audio

| Paramètre | Valeur | Comparaison |
|-----------|--------|-------------|
| **Bruit de sortie** | **0.46 µV RMS** | Farad: 1µV, Sbooster: 3µV |
| Ondulation résiduelle | < 1 mV | — |
| Régulation ligne | 0.01% | — |
| Régulation charge | 0.02% | — |

### Alimentation

| Paramètre | Spécification |
|-----------|---------------|
| Entrée | 100-240V AC 50/60Hz |
| Sorties | 2 rails indépendants |
| Tension de sortie | **5-15V ajustable** (par pas de 0.01V) |
| Courant max par rail | **1500 mA** |
| USB 5V fixe | 500 mA |

### Architecture Unique

```
AC IN → Transformateur torique → Redresseur Schottky
     → Pré-régulateur LM338T (absorbe les variations)
     → 6× LT3045 en parallèle (ultra-low noise)
     → Sortie régulée
```

**Pourquoi 6× LT3045 en parallèle ?**
- Réduction du bruit : √6 = 2.45× moins de bruit
- Bruit théorique : 0.8µV ÷ 2.45 = **0.46 µV RMS**
- Capacité courant : 6 × 500mA = 3A (headroom)
- Meilleure dissipation thermique

### Protections

| Protection | Seuil | Description |
|------------|-------|-------------|
| **OVP** | >16V | Surtension |
| **OCP** | >1.6A | Surintensité |
| **OTP** | >70°C | Surchauffe |
| Court-circuit | Auto-recovery | Protection fusible |

---

## 🖥️ Interface Utilisateur

### Écran Tactile

| Paramètre | Spécification |
|-----------|---------------|
| Board | **STM32H750B-DK** |
| Écran | 4.3" TFT 480×272 |
| Processeur | Cortex-M7 @ 480MHz |
| Framework | **TouchGFX** |
| Prix | ~76€ |

### Design UI : Style FIBARO

L'interface adopte une esthétique **FIBARO** - minimaliste, douce, premium :

- Palette : Bleu doux (#7eb8da), blanc cassé, pas de vert
- Jauges : Style **VU-mètre vintage** audiophile
- Effets : Verre givré, ombres subtiles, glow bleu
- Typographie : SF Pro Display / SF Mono

### Pages de l'Interface

| Page | Fonction |
|------|----------|
| **ECOUTE** | 3 jauges voltage (USB, SORTIE 1, SORTIE 2), métriques, status |
| **DETAILS** | Détails par rail : voltage, courant, ondulation, puissance |
| **SANTE** | Gardien IA, 7 métriques santé, voyants protection |
| **SESSION** | Timer, énergie consommée, animation synthwave |
| **CONFIG** | Son, langue, simulation défauts, infos produit |

### Gardien IA

Indicateur visuel intelligent avec visage émotionnel :

| État | Couleur | Expression | Message |
|------|---------|------------|---------|
| OPTIMAL | Bleu | ◡ (sourire) | "TOUT VA BIEN!" |
| ATTENTION | Jaune | — (neutre) | "ATTENTION REQUISE" |
| CRITIQUE | Rouge | ︵ (triste) | "INTERVENTION URGENTE" |

---

## 📁 Structure du Projet

```
LPS-Audiophile-DuoPro/
│
├── Circuit_LPS_Audiophile_V*.md    # Documentation circuits par version
│
├── LPS_Audiophile_V*.ino           # Firmware Arduino (ATmega328P)
│
├── lps_duo_pro_v*.py               # Prototypes UI PyGame
│
├── PREMORTEM_V*.md                 # Protocoles validation
│
├── TouchGFX/                       # Projet TouchGFX (à venir)
│   ├── assets/
│   │   ├── images/
│   │   │   ├── bg_main.png         # Fond principal 480×272
│   │   │   ├── gauge_frame.png     # Cadran jauge 145×110
│   │   │   ├── gauge_needle.png    # Aiguille 145×145
│   │   │   └── gauge_pivot.png     # Pivot central 24×24
│   │   └── fonts/
│   └── gui/
│
└── README.md
```

---

## 🔧 Hardware

### Bill of Materials (Composants Clés)

| Composant | Référence | Fonction | Qté |
|-----------|-----------|----------|-----|
| Régulateur LDO | **LT3045** | Ultra-low noise 500mA | 12 (6×2 rails) |
| Pré-régulateur | **LM338T** | Absorbe dropout | 2 |
| Isolation | **ADuM1201** | Isolation galvanique | 2 |
| Monitoring | **INA219** | Mesure courant/tension | 3 |
| Température | **NTC 10K** | Mesure thermique | 2 |
| Digipot | **MCP4661** | Réglage voltage I2C | 2 |
| MCU Firmware | **ATmega328P** | Contrôle principal | 1 |
| MCU Display | **STM32H750** | Interface TouchGFX | 1 |

### Budget Estimé

| Catégorie | Coût |
|-----------|------|
| Composants électroniques | ~200€ |
| PCB fabrication | ~30€ |
| Boîtier aluminium | ~50€ |
| Écran STM32H750B-DK | ~76€ |
| Connectique, câbles | ~20€ |
| **Total** | **~315€** |

---

## 💻 Firmware

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ATmega328P (Contrôle)                    │
├─────────────────────────────────────────────────────────────┤
│  - Lecture INA219 (I2C)                                     │
│  - Lecture NTC (ADC)                                        │
│  - Contrôle Digipots MCP4661 (I2C)                         │
│  - Protections OVP/OCP/OTP                                  │
│  - Communication UART → STM32                               │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ UART
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    STM32H750 (Interface)                    │
├─────────────────────────────────────────────────────────────┤
│  - TouchGFX UI                                              │
│  - Écran tactile 480×272                                    │
│  - Réception données capteurs                               │
│  - Affichage jauges/métriques                               │
│  - Gestion interactions utilisateur                         │
└─────────────────────────────────────────────────────────────┘
```

### Versions Firmware

| Version | Date | Changements |
|---------|------|-------------|
| V2.4.6 | Nov 2024 | Production stable, toutes protections |
| V2.4.4 | Nov 2024 | Ajout digipot, ajustement fin voltage |
| V2.2.1 | Oct 2024 | OVP/OCP/OTP complets |
| V2.1.1 | Oct 2024 | Double OLED, INA219 |

---

## 🎨 Création des Assets UI

### Workflow

```
Midjourney V7 → Nano Banana (édition) → Magnific (upscale) → TouchGFX
```

### Outils Utilisés

| Outil | Usage |
|-------|-------|
| **Midjourney V7** | Génération initiale des designs |
| **Nano Banana** (Gemini) | Édition IA (suppression éléments, fond transparent) |
| **Magnific AI** | Upscaling haute qualité |
| **Photopea** | Recadrage, ajustements finaux |

### Assets Requis

| Fichier | Dimensions | Description |
|---------|------------|-------------|
| `bg_main.png` | 480×272 | Fond gradient bleu doux |
| `gauge_frame.png` | 145×110 | Cadran VU-mètre sans aiguille |
| `gauge_needle.png` | 145×145 | Aiguille seule (pivot centré) |
| `gauge_pivot.png` | 24×24 | Bouton central doré |

---

## 📋 Protocole PREMORTEM

Ce projet utilise le **Protocole PREMORTEM V3.5** - une méthodologie de validation systématique pour la conception électronique :

### Modules Clés

| Module | Fonction |
|--------|----------|
| **Module 0** | Triggers automatiques (composants critiques) |
| **Module 1** | Hiérarchie des règles (NIVEAU 0/1/2) |
| **Module 3** | Méthode datasheet |
| **Module 4** | Protections par type de circuit |
| **Module 5** | Calculs obligatoires |
| **Module 9** | Validation code firmware |

### Niveaux de Blocage

| Niveau | Action | Exemple |
|--------|--------|---------|
| **NIVEAU 0** | Blocage total | Pas de datasheet semiconducteur |
| **NIVEAU 1** | Confirmation requise | Configuration inhabituelle |
| **NIVEAU 2** | Avertissement | Surdimensionnement |

---

## 🚀 Roadmap

### ✅ Terminé

- [x] Architecture électronique V3.1.0
- [x] Firmware ATmega328P V2.4.6
- [x] Prototype UI PyGame V92
- [x] Design UI React V10
- [x] Assets Midjourney (fond, jauge)

### 🔄 En Cours

- [ ] Implémentation TouchGFX
- [ ] Extraction assets (aiguille, pivot)
- [ ] Intégration écran STM32H750B-DK

### 📋 À Faire

- [ ] PCB final V3.1.0
- [ ] Boîtier aluminium
- [ ] Tests EMC
- [ ] Certification CE
- [ ] Production série

---

## 🇫🇷 Fabriqué en France

Ce produit est conçu et assemblé en France, avec une attention particulière portée à la qualité et au support client.

---

## 📄 Licence

Projet propriétaire - Tous droits réservés © 2024-2025

---

## 📞 Contact

**Auteur** : Mehdi

**Repository** : [github.com/mmmprod/LPS-Audiophile-DuoPro](https://github.com/mmmprod/LPS-Audiophile-DuoPro)
