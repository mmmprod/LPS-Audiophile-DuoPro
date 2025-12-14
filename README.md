# LPS DUO PRO

## Premium Dual-Rail Linear Power Supply for Audiophiles

![Version](https://img.shields.io/badge/version-3.1.0-blue)
![License](https://img.shields.io/badge/license-Proprietary-red)
![Made in](https://img.shields.io/badge/Made%20in-France-blue)

---

## 🎯 Overview

The **LPS DUO PRO** is an ultra–low-noise linear supply crafted for obsessive music lovers. It feeds DACs, streamers, headphone
amps, and other gear with a level of rail purity rarely seen in this price class.

**Target price: €599 (incl. VAT)**

### Market Position

| Segment | Examples | Price |
|---------|----------|------|
| Entry-level | Various imports | €50–150 |
| **→ LPS DUO PRO** | **Our product** | **€599** |
| Premium | Farad Super3, Keces P8 | 700-1200€ |
| Ultra Premium | Uptone JS-2, Ferrum HYPSOS | 1000-2000€ |

---

## ⚡ Technical Specifications

### Audio Performance

| Parameter | Value | Comparison |
|-----------|--------|-------------|
| **Output noise** | **0.46 µV RMS** | Farad: 1µV, Sbooster: 3µV |
| Residual ripple | < 1 mV | — |
| Line regulation | 0.01% | — |
| Load regulation | 0.02% | — |

### Power

| Parameter | Specification |
|-----------|---------------|
| Input | 100–240V AC 50/60Hz |
| Outputs | 2 independent rails |
| Output voltage | **5–15V adjustable** (0.01V steps) |
| Max current per rail | **1500 mA** |
| Fixed 5V USB | 500 mA |

### Signature Architecture

```
AC IN → Toroidal transformer → Schottky bridge
     → LM338T pre-regulator (absorbs swings)
     → 6× LT3045 in parallel (ultra-low noise)
     → Regulated output
```

**Why six LT3045s in parallel?**
- Noise reduction: √6 = 2.45× lower noise
- Theoretical noise: 0.8µV ÷ 2.45 = **0.46 µV RMS**
- Current capacity: 6 × 500mA = 3A of headroom
- Improved thermal spreading

### Protections

| Protection | Threshold | Description |
|------------|-------|-------------|
| **OVP** | >16V | Over-voltage |
| **OCP** | >1.6A | Over-current |
| **OTP** | >70°C | Over-temp |
| Short circuit | Auto-recovery | Fuse protection |

---

## 🖥️ User Interface

### Touch Display

| Parameter | Specification |
|-----------|---------------|
| Board | **STM32H750B-DK** |
| Display | 4.3" TFT 480×272 |
| Processor | Cortex-M7 @ 480MHz |
| Framework | **TouchGFX** |
| Cost | ~€76 |

### UI Design: FIBARO-inspired Style

The UI leans into a **FIBARO**-like aesthetic—minimal, soft, and premium:

- Palette: Gentle blue (#7eb8da), off-white, no green
- Gauges: **Vintage VU-meter** vibe for audiophiles
- Effects: Frosted glass, subtle shadows, blue glow
- Typography: SF Pro Display / SF Mono

### Interface Pages

| Page | Function |
|------|----------|
| **LISTEN** | 3 voltage gauges (USB, OUTPUT 1, OUTPUT 2), metrics, status |
| **DETAILS** | Per-rail breakdown: voltage, current, ripple, power |
| **HEALTH** | AI guardian, 7 wellness metrics, protection indicators |
| **SESSION** | Timer, consumed energy, synthwave animation |
| **CONFIG** | Sound, language, fault simulations, product info |

### AI Guardian

An expressive visual indicator with emotive feedback:

| State | Color | Expression | Message |
|------|---------|------------|---------|
| OPTIMAL | Blue | ◡ (smile) | "ALL GOOD!" |
| WARNING | Yellow | — (neutral) | "NEEDS ATTENTION" |
| CRITICAL | Red | ︵ (sad) | "URGENT INTERVENTION" |

---

## 📁 Project Structure

```
LPS-Audiophile-DuoPro/
│
├── Circuit_LPS_Audiophile_V*.md    # Circuit documentation by version
├── LPS_Audiophile_V*.ino           # Arduino firmware (ATmega328P)
├── lps_duo_pro_v*.py               # PyGame UI prototypes
├── PREMORTEM_V*.md                 # Validation protocols
├── TouchGFX/                       # TouchGFX project (TBD)
│   ├── assets/
│   │   ├── images/
│   │   │   ├── bg_main.png         # Main background 480×272
│   │   │   ├── gauge_frame.png     # Gauge frame 145×110
│   │   │   ├── gauge_needle.png    # Needle 145×145
│   │   │   └── gauge_pivot.png     # Center pivot 24×24
│   │   └── fonts/
│   └── gui/
│
└── README.md
```

---

## 🔧 Hardware

### Bill of Materials (Key Components)

| Component | Reference | Function | Qty |
|-----------|-----------|----------|-----|
| LDO regulator | **LT3045** | Ultra-low noise 500mA | 12 (6×2 rails) |
| Pre-regulator | **LM338T** | Absorbs dropout | 2 |
| Isolation | **ADuM1201** | Galvanic isolation | 2 |
| Monitoring | **INA219** | Voltage/current sensing | 3 |
| Temperature | **NTC 10K** | Thermal sensing | 2 |
| Digipot | **MCP4661** | I2C voltage adjustment | 2 |
| MCU Firmware | **ATmega328P** | Main control | 1 |
| MCU Display | **STM32H750** | TouchGFX interface | 1 |

### Estimated Budget

| Category | Cost |
|-----------|------|
| Electronic components | ~€200 |
| PCB fabrication | ~€30 |
| Aluminum enclosure | ~€50 |
| STM32H750B-DK display | ~€76 |
| Connectors & cabling | ~€20 |
| **Total** | **~€315** |

---

## 💻 Firmware

### Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ATmega328P (Control)                     │
├─────────────────────────────────────────────────────────────┤
│  - INA219 sensing (I2C)                                     │
│  - NTC sensing (ADC)                                        │
│  - MCP4661 digipot control (I2C)                            │
│  - OVP/OCP/OTP protections                                  │
│  - UART communication → STM32                               │
└─────────────────────────────────────────────────────────────┘
                              │
                              │ UART
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    STM32H750 (Interface)                    │
├─────────────────────────────────────────────────────────────┤
│  - TouchGFX UI                                              │
│  - 480×272 touch display                                    │
│  - Sensor data ingestion                                    │
│  - Gauge/metric display                                     │
│  - User interaction handling                                │
└─────────────────────────────────────────────────────────────┘
```
