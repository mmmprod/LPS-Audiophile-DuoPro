# LPS DUO PRO - UI Simulator (PyGame V92)

PC simulator for rapid interface development and testing.

> ⚠️ **Note**: This simulator targets desktop development only.
> Production UI runs on **ESP32-8048S050C** with LVGL (C++ code in `/esp32/`).

## System Architecture

- **ESP32-8048S050C**: Integrated 5" 800×480 LCD, capacitive touch, ESP32-S3
- **ATmega328P**: LDO regulation, OVP/OCP/OTP protections, ADC measurements
- **Communication**: UART between ESP32 and ATmega

## Simulator Requirements

- Python 3.7+
- 800×480 display minimum

## Installation

```bash
cd ui-simulator
pip install -r requirements.txt
```

## Run

```bash
python lps_duo_pro.py
```

## Keyboard Shortcuts

| Key | Action |
|--------|--------|
| `1-5` | Page navigation |
| `ESC` | Close popup / Quit |
| `ENTER` | Start (boot screen) |

## Pages

1. **LISTEN** - Core voltage VU meters
2. **DETAILS** - Per-rail metrics with Nixie bars
3. **HEALTH** - System status and OVP/OCP/OTP protections
4. **SESSION** - Timer and consumed energy
5. **CONFIG** - Settings, language, failure simulations

## Simulation Mode

The simulator generates synthetic data to test the UI without hardware.
Fault simulations are available on the CONFIG page:
- NORM: Normal operation
- HOT: Over-temp (OTP)
- RIP: Excess ripple
- LOAD: Over-current (OCP)
- LO-V: Under-voltage
- HI-V: Over-voltage (OVP)

## V92 Optimizations

- `draw.lines()` for oscilloscopes (690→3 calls/frame)
- Cached `get_all_problems()` per frame
- Centralized layout (ECOUTE_LAYOUT, DETAILS_LAYOUT, etc.)
- Enlarged LCD by 95px
- Headroom 2.0V aligned with circuit V2.4.5
