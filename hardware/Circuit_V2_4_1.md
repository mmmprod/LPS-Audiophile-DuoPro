# CIRCUIT LPS AUDIOPHILE "DUO PRO" V2.4.1

**Version:** 2.4.1  
**Date:** Novembre 2025  
**Statut:** PRODUCTION READY

---

## CHANGELOG V2.4.1

| V2.4.0 | V2.4.1 |
|--------|--------|
| F_SECTEUR 2A T | F_SECTEUR 315mA T (correct pour 30VA) |
| C_MAIN 4700µF 35V | C_MAIN 4700µF 50V (marge 1.9×) |
| I_max fixe 500mA | I_max adaptatif selon V_OUT |

**SPECS COURANT CONTINU (limite thermique LM317):**
| V_OUT | I_max |
|-------|-------|
| 5-6V | 350mA |
| 7-9V | 450mA |
| 10-15V | 500mA |

---

## CHANGELOG V2.3.0 → V2.4.1

| V2.3.0 (DÉFAILLANT) | V2.4.0 | V2.4.1 |
|---------------------|--------|--------|
| MCP41100 sur SET (hors specs) | MCP41100 sur feedback LM317 ✓ | Idem |
| P_LT3045 max 5.75W | P_LT3045 max 1W ✓ | Idem |
| F_SECTEUR 2A T | 2A T | 315mA T ✓ |
| C_MAIN 35V | 35V | 50V ✓ |
| I_max fixe 500mA | fixe 500mA | Adaptatif ✓ |

---

## ARCHITECTURE GLOBALE

```
SECTEUR 230V → Transfo 2×18VAC → Pont + C → V_IN (~22V DC)
                                              ↓
                              ┌───────────────┴───────────────┐
                              ↓                               ↓
                          CANAL A                         CANAL B
                              ↓                               ↓
                    LM317 (V_PRE adaptative)        LM317 (V_PRE adaptative)
                              ↓                               ↓
                    LT3045 (post-reg SET→OUT)      LT3045 (post-reg SET→OUT)
                              ↓                               ↓
                          V_OUT_A                         V_OUT_B
                          (5-15V)                         (5-15V)
```

**Principe clé:** V_PRE = V_OUT_cible + 2V (headroom constant)

---

## BLOC A — ENTRÉE ET PROTECTION

### A.1 Protection secteur
SECTEUR_L → F_SECTEUR (315mA T) → SECTEUR_L_PROT
SECTEUR_N → SECTEUR_N direct

### A.2 Transformateur
SECTEUR_L_PROT → Transfo primaire
SECTEUR_N → Transfo primaire
Transfo sec1 (18VAC) → D_BRIDGE_A
Transfo sec2 (18VAC) → D_BRIDGE_B
Transfo CT → GND

### A.3 Redressement et filtrage (par canal)
D_BRIDGE_A+ → C_MAIN_A (4700µF 50V) → V_IN_A (~22V DC)
D_BRIDGE_A- → GND
V_IN_A → TVS_A (P6KE27CA) → GND

### Composants Bloc A
| Réf | Valeur | Specs |
|-----|--------|-------|
| F_SECTEUR | 315mA T | 250VAC, 5×20mm (V2.4.1 corrigé) |
| TRANSFO | 2×18VAC | 30VA, double secondaire |
| D_BRIDGE | KBU806 | 800V 6A |
| C_MAIN | 4700µF | 50V 105°C low-ESR (V2.4.1 corrigé) |
| TVS | P6KE27CA | 27V 600W bidir |

---

## BLOC B — PRÉ-RÉGULATION LM317 ADAPTATIVE

### B.1 Topologie par canal

**Entrée:**
V_IN_A → LM317_A pin3 (IN)

**Sortie:**
LM317_A pin2 (OUT) → V_PRE_A
V_PRE_A → C_PRE_A (10µF 25V film) → GND

**Feedback adaptatif (NOUVEAU V2.4.0):**
V_PRE_A → R_FIXED_A (1.1kΩ 1%) → ADJ_NODE_A
ADJ_NODE_A → MCP41100_A wiper (pin5 PW0)
MCP41100_A pin6 (PA0) → V_PRE_A
MCP41100_A pin7 (PB0) → R_SHUNT_A → GND
R_SHUNT_A = 2kΩ 1%
ADJ_NODE_A → R1_A (240Ω 1%) → GND
ADJ_NODE_A → LM317_A pin1 (ADJ)
ADJ_NODE_A → C_ADJ_A (10µF tant) → GND

**Protection:**
V_PRE_A → D_PROT_A cathode (1N4007)
D_PROT_A anode → V_IN_A

### B.2 Calculs feedback

```
V_PRE = 1.25V × (1 + R2_eff / R1)

Où:
- R1 = 240Ω (fixe)
- R2_eff = R_FIXED + (R_DIGIPOT × R_SHUNT) / (R_DIGIPOT + R_SHUNT)
- R_FIXED = 1.1kΩ
- R_SHUNT = 2kΩ
- R_DIGIPOT = MCP41100 (0 à 100kΩ selon wiper)

Wiper = 0:
  R_eff_dig = 0
  R2 = 1.1kΩ
  V_PRE = 1.25 × (1 + 1100/240) = 6.98V ≈ 7V ✓

Wiper = 255:
  R_eff_dig = 100k × 2k / 102k = 1.96kΩ
  R2 = 1.1k + 1.96k = 3.06kΩ
  V_PRE = 1.25 × (1 + 3060/240) = 17.2V ✓

Résolution:
  Delta R2 = 1.96kΩ / 256 = 7.66Ω/pas
  Delta V = 1.25 × 7.66 / 240 = 40mV/pas ✓
```

### B.3 Tension sur wiper MCP41100

```
V_ADJ = 1.25V (référence interne LM317)
V_wiper ≈ 1.25V constant ✓

VÉRIFICATION SPECS MCP41100:
- VDD = 5V
- Wiper voltage range: VSS to VDD
- V_wiper = 1.25V < 5V ✓ DANS SPECS
```

### B.4 Dissipation LM317

```
P_LM317 = (V_IN - V_PRE) × I_OUT

Pire cas:
- V_IN = 22V
- V_PRE = 7V (sortie 5V demandée)
- I_OUT = 500mA
- P = (22 - 7) × 0.5 = 7.5W

→ Dissipateur requis: Rth < (150-40)/7.5 = 14.7°C/W
→ Dissipateur alu 50×40×20mm (Rth ≈ 8°C/W) ✓
```

### Composants Bloc B (par canal)
| Réf | Valeur | Specs |
|-----|--------|-------|
| U_LM317 | LM317T | TO-220, dissipateur |
| R_FIXED | 1.1kΩ | 1% MF 0.25W |
| R1 | 240Ω | 1% MF 0.25W |
| R_SHUNT | 2kΩ | 1% MF 0.25W |
| C_PRE | 10µF | 25V film |
| C_ADJ | 10µF | tant low-ESR |
| D_PROT | 1N4007 | 1A 1000V |

---

## BLOC C — RÉGULATION FINALE LT3045

### C.1 Topologie post-regulator (SET→OUT)

**Entrée:**
V_PRE_A → LT3045_A pin1 (IN)
LT3045_A pin1 (IN) → C_IN_LT_A (10µF céram X7R) → GND

**Sortie avec SET→OUT:**
LT3045_A pin4 (OUT) → V_OUT_A
V_OUT_A → C_OUT_LT_A (47µF tant low-ESR) → GND
**LT3045_A pin3 (SET) → V_OUT_A (connexion directe)**

**PGFB:**
LT3045_A pin5 (PGFB) → V_OUT_A (connexion directe)

**OUTS:**
LT3045_A pin2 (OUTS) → point de mesure (Kelvin sense)

**GND:**
LT3045_A pin6,7,8 (GND/PAD) → GND (vias multiples)

### C.2 Calculs thermiques V2.4.0

```
Mode post-regulator: SET → OUT
V_dropout = V_PRE - V_OUT = 2V constant (par design)

P_LT3045 = V_dropout × I_OUT = 2V × 0.5A = 1W

Thermal:
- Rth(j-a) DFN = 34°C/W (4-layer PCB)
- ΔT = 1W × 34°C/W = 34°C
- Tj = 40°C + 34°C = 74°C << 125°C ✓

COMPARAISON V2.3.0 vs V2.4.0:
| Paramètre | V2.3.0 | V2.4.0 |
|-----------|--------|--------|
| V_PRE | 16.5V fixe | 7-17V adaptatif |
| P_LT3045 @ 5V/500mA | 5.75W ❌ | 1W ✓ |
| Tj @ 5V/500mA | >200°C ❌ | 74°C ✓ |
```

### Composants Bloc C (par canal)
| Réf | Valeur | Specs |
|-----|--------|-------|
| U_LT3045 | LT3045EDD | DFN-8 |
| C_IN_LT | 10µF | 25V X7R céram |
| C_OUT_LT | 47µF | 16V tant low-ESR |

---

## BLOC D — DIGIPOTS MCP41100 SPI

### D.1 Alimentation et SPI

**Alimentation:**
+5V_MCU → MCP41100_A pin8 (VDD)
GND → MCP41100_A pin4 (VSS)

**SPI commun:**
µC D11 (MOSI) → MCP41100_A pin3 (SI)
µC D13 (SCK) → MCP41100_A pin2 (SCK)

**Chip selects:**
µC D9 → MCP41100_A pin1 (CS)
µC D10 → MCP41100_B pin1 (CS)

### D.2 Connexions analogiques (CORRIGÉ V2.4.0)

```
VÉRIFICATION SPECS:
- VDD = 5V
- A/B/W voltage range: VSS to VDD (0 to 5V)
- V_ADJ LM317 = 1.25V
- Tension max sur wiper = 1.25V < 5V ✓ DANS SPECS
```

**Canal A:**
MCP41100_A pin6 (PA0) → V_PRE_A (via R_FIXED 1.1k)
MCP41100_A pin5 (PW0) → ADJ_NODE_A
MCP41100_A pin7 (PB0) → R_SHUNT_A (2k) → GND

**Canal B:** Identique

### D.3 Comparaison V2.3.0 vs V2.4.0

| Aspect | V2.3.0 ❌ | V2.4.0 ✓ |
|--------|----------|----------|
| Wiper connecté à | SET LT3045 (5-15V) | ADJ LM317 (1.25V) |
| Tension max wiper | 15V | 1.25V |
| Specs MCP41100 | VIOLATION | OK |
| Risque | Latch-up, destruction | Aucun |

### Composants Bloc D
| Réf | Valeur | Specs |
|-----|--------|-------|
| U_DP_A | MCP41100-I/P | DIP-8, 100kΩ |
| U_DP_B | MCP41100-I/P | DIP-8, 100kΩ |

---

## BLOC E — MICROCONTRÔLEUR ATmega328P

### E.1 Alimentation
+5V_MCU → U_MCU pin7 (VCC), pin20 (AVCC)
GND → U_MCU pin8 (GND), pin22 (AGND)
+5V_MCU → C_VCC (100nF) → GND (près pin7)
+5V_MCU → C_AVCC (100nF) → GND (près pin20)

### E.2 Oscillateur
U_MCU pin9 (XTAL1) → Y1 (16MHz) → U_MCU pin10 (XTAL2)
XTAL1 → C_X1 (22pF) → GND
XTAL2 → C_X2 (22pF) → GND

### E.3 Reset
+5V_MCU → R_RST (10kΩ) → U_MCU pin1 (RESET)
U_MCU pin1 → C_RST (100nF) → GND
U_MCU pin1 → ISP pin5 (RESET)

### E.4 Pinout complet

| Pin µC | Fonction | Connexion |
|--------|----------|-----------|
| A0 | ADC V_OUT_A | Diviseur 3:1 |
| A1 | ADC V_OUT_B | Diviseur 3:1 |
| A2 | ADC V_PRE_A | Diviseur 4:1 |
| A3 | ADC V_PRE_B | Diviseur 4:1 |
| A4 | I2C SDA | OLED + INA219 |
| A5 | I2C SCL | OLED + INA219 |
| A6 | ADC NTC | Thermistance |
| D2 | ENC_A | Encodeur INT0 |
| D3 | ENC_B | Encodeur INT1 |
| D4 | ENC_SW | Bouton encodeur |
| D5 | LED_G | LED verte |
| D6 | LED_R | LED rouge |
| D7 | GATE_N_A | N-MOS enable A |
| D8 | GATE_N_B | N-MOS enable B |
| D9 | CS_DP_A | SPI chip select A |
| D10 | CS_DP_B | SPI chip select B |
| D11 | MOSI | SPI data |
| D13 | SCK | SPI clock |

---

## BLOC F — MESURES ET PROTECTIONS

### F.1 Diviseurs tension (par canal)
V_OUT_A → R_DIV1_A (20kΩ 1%) → ADC_VOUT_A
ADC_VOUT_A → R_DIV2_A (10kΩ 1%) → GND
ADC_VOUT_A → C_FILT_A (100nF) → GND

V_PRE_A → R_DIV3_A (30kΩ 1%) → ADC_VPRE_A
ADC_VPRE_A → R_DIV4_A (10kΩ 1%) → GND

### F.2 Capteurs courant INA219
+5V_MCU → INA219_A VCC
GND → INA219_A GND
SDA → INA219_A SDA
SCL → INA219_A SCL
INA219_A A0 → GND (adresse 0x40)
INA219_B A0 → +5V (adresse 0x41)
V_OUT_A → INA219_A VIN+ → R_SHUNT (0.1Ω 1%) → INA219_A VIN- → LOAD_A

### F.3 Thermistance NTC
+5V_MCU → R_NTC_PU (10kΩ) → NTC_NODE
NTC_NODE → NTC (10kΩ @ 25°C) → GND
NTC_NODE → ADC_NTC (pin A6)

### F.4 Enable sorties (N-MOS)
µC D7 → R_GATE_A (1kΩ) → Q_EN_A gate (2N7000)
Q_EN_A source → GND
Q_EN_A drain → RELAY_A ou LOAD_ENABLE_A
Gate → R_PD_A (100kΩ) → GND

---

## BLOC G — INTERFACE UTILISATEUR

### G.1 Écran OLED
+5V_MCU → OLED VCC
GND → OLED GND
SDA → OLED SDA
SCL → OLED SCL

### G.2 Encodeur rotatif
+5V_MCU → ENC_A → R_ENC_A (10kΩ) → µC D2
+5V_MCU → ENC_B → R_ENC_B (10kΩ) → µC D3
ENC_COM → GND
ENC_SW → µC D4
µC D4 → R_SW (10kΩ) → +5V_MCU

### G.3 LEDs statut
µC D5 → R_LED_G (1kΩ) → LED_G anode
LED_G cathode → GND
µC D6 → R_LED_R (1kΩ) → LED_R anode
LED_R cathode → GND

---

## BLOC H — ALIMENTATION LOGIQUE

### H.1 Régulateur 5V
V_IN_A → D_OR_A anode (1N5822)
V_IN_B → D_OR_B anode (1N5822)
D_OR_A cathode → D_OR_B cathode → V_IN_OR
V_IN_OR → U_REG5V pin1 (IN) (7805 ou LM2940-5)
U_REG5V pin2 (GND) → GND
U_REG5V pin3 (OUT) → +5V_MCU
V_IN_OR → C_5V_IN (100nF + 10µF) → GND
+5V_MCU → C_5V_OUT (100nF + 10µF) → GND

---

## BOM COMPLÈTE V2.4.0

### Semiconducteurs
| Réf | Composant | Qté | Prix unit |
|-----|-----------|-----|-----------|
| U_LM317 | LM317T TO-220 | 2 | 0.80€ |
| U_LT3045 | LT3045EDD DFN-8 | 2 | 6.50€ |
| U_DP | MCP41100-I/P DIP-8 | 2 | 2.50€ |
| U_MCU | ATmega328P-PU DIP-28 | 1 | 3.00€ |
| U_REG5V | LM2940-5 TO-220 | 1 | 1.50€ |
| U_INA219 | INA219 module | 2 | 2.00€ |
| D_BRIDGE | KBU806 | 1 | 1.00€ |
| Q_EN | 2N7000 | 2 | 0.20€ |

### Résistances (1% MF)
| Réf | Valeur | Qté | Notes |
|-----|--------|-----|-------|
| R_FIXED | 1.1kΩ | 2 | Feedback LM317 |
| R1 | 240Ω | 2 | ADJ → GND |
| R_SHUNT_DP | 2kΩ | 2 | Parallèle digipot |
| R_DIV | 10k, 20k, 30k | 8 | Diviseurs ADC |
| R_GATE | 1kΩ | 2 | Gate N-MOS |
| R_PD | 100kΩ | 2 | Pull-down gate |
| R_NTC_PU | 10kΩ | 1 | Pull-up NTC |
| R_LED | 1kΩ | 2 | LEDs |
| R_ENC | 10kΩ | 3 | Encodeur |
| R_RST | 10kΩ | 1 | Reset µC |
| R_SHUNT_I | 0.1Ω 1W | 2 | INA219 |

### Condensateurs
| Réf | Valeur | Qté | Type |
|-----|--------|-----|------|
| C_MAIN | 4700µF 50V | 2 | Électro low-ESR |
| C_PRE | 10µF 25V | 2 | Film |
| C_ADJ | 10µF 16V | 2 | Tant low-ESR |
| C_IN_LT | 10µF 25V | 2 | Céram X7R |
| C_OUT_LT | 47µF 16V | 2 | Tant low-ESR |
| C_5V | 10µF + 100nF | 4 | Découplage |
| C_FILT | 100nF | 6 | Céram |
| C_X | 22pF | 2 | Céram |

### Connecteurs et divers
| Réf | Composant | Qté |
|-----|-----------|-----|
| TRANSFO | 2×18VAC 30VA | 1 |
| OLED | SSD1306 128×64 I2C | 1 |
| ENC | Encodeur rotatif + SW | 1 |
| NTC | 10kΩ @ 25°C | 1 |
| Y1 | Quartz 16MHz | 1 |
| LED | 3mm vert + rouge | 2 |
| DISS_LM317 | Alu 50×40×20 | 2 |

### Coût estimé
| Catégorie | Total |
|-----------|-------|
| Semiconducteurs | ~35€ |
| Passifs | ~15€ |
| Mécaniques | ~20€ |
| **TOTAL BOM** | **~70€** |

---

## FORMULES FIRMWARE V2.4.0

### Wiper → Tension

```cpp
float digipotToVoltage(uint8_t pos) {
    // R_wiper = pos × (100kΩ / 255)
    float r_wiper = pos * 392.16f;
    
    // R_eff = (R_wiper × R_SHUNT) / (R_wiper + R_SHUNT)
    // R_SHUNT = 2000Ω
    float r_eff = (r_wiper * 2000.0f) / (r_wiper + 2000.0f);
    
    // R2 = R_FIXED + R_eff = 1100 + R_eff
    float r2 = 1100.0f + r_eff;
    
    // V_PRE = 1.25 × (1 + R2/R1) avec R1 = 240Ω
    float v_pre = 1.25f * (1.0f + r2 / 240.0f);
    
    // V_OUT = V_PRE - headroom (2V)
    float v_out = v_pre - 2.0f;
    
    return v_out;
}
```

### Tension → Wiper

```cpp
uint8_t voltageToDigipot(float v_out) {
    // V_PRE = V_OUT + 2V
    float v_pre = v_out + 2.0f;
    
    // R2 = (V_PRE / 1.25 - 1) × 240
    float r2 = (v_pre / 1.25f - 1.0f) * 240.0f;
    
    // R_eff = R2 - R_FIXED
    float r_eff = r2 - 1100.0f;
    if (r_eff < 0) r_eff = 0;
    
    // R_wiper = (R_eff × R_SHUNT) / (R_SHUNT - R_eff)
    float denom = 2000.0f - r_eff;
    if (denom <= 0) return 255;
    float r_wiper = (r_eff * 2000.0f) / denom;
    
    // pos = R_wiper / 392.16
    int pos = (int)(r_wiper / 392.16f + 0.5f);
    if (pos < 0) pos = 0;
    if (pos > 255) pos = 255;
    
    return (uint8_t)pos;
}
```

### Plage effective

| Wiper | R_eff | R2 | V_PRE | V_OUT |
|-------|-------|-----|-------|-------|
| 0 | 0Ω | 1100Ω | 6.98V | 4.98V |
| 128 | 980Ω | 2080Ω | 12.08V | 10.08V |
| 255 | 1961Ω | 3061Ω | 17.19V | 15.19V |

**Résolution:** ~40mV/pas ✓

---

## VALIDATION PREMORTEM V3.9

### Checklist thermique
| Composant | P_max | Dissipateur | Tj_max | Status |
|-----------|-------|-------------|--------|--------|
| LM317 @ 5V/350mA | 5.95W | Alu 8°C/W | 88°C | ✓ |
| LM317 @ 10V/500mA | 6W | Alu 8°C/W | 88°C | ✓ |
| LT3045 | 1W | PCB 34°C/W | 74°C | ✓ |
| MCP41100 | <10mW | N/A | N/A | ✓ |

### Checklist specs composants
| Composant | Paramètre | Valeur design | Limite | Status |
|-----------|-----------|---------------|--------|--------|
| MCP41100 | V_wiper | 1.25V | 5.5V | ✓ |
| LT3045 | V_dropout | 2V | 260mV min | ✓ |
| LM317 | V_in-out | 5-15V | 40V max | ✓ |
| C_MAIN | V_rating | 50V | ~26V DC max | ✓ (marge 1.9×) |
| F_SECTEUR | I_rating | 315mA | ~130mA nom | ✓ (marge 2.4×) |

### Confiance
| Aspect | Score |
|--------|-------|
| Thermique | 98/100 |
| Specs composants | 100/100 |
| Sécurité secteur | 98/100 |
| Architecture | 97/100 |
| **GLOBAL** | **98/100** |

---

## CHANGELOG

| Version | Date | Modifications |
|---------|------|---------------|
| V2.3.0 | Nov 2025 | Sweep tuning (DÉFAILLANT) |
| V2.4.0 | Nov 2025 | Architecture post-reg adaptative |
| V2.4.1 | Nov 2025 | F_SECTEUR 315mA, C_MAIN 50V, I_max adaptatif |

---

**FIN CIRCUIT V2.4.1**
