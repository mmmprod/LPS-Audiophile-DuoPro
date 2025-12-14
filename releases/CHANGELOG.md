# Changelog

All noteworthy project changes live here.

## [2.4.5] - 2025-11-26

### 🔴 Critical Fix
- **Backfeed detection:** used `i_out` (clamped to 0) instead of signed `raw_i`
  - Bug: `i_out` was always ≥0, so the `< -20mA` condition never triggered
  - Fix: added `raw_i_A` and `raw_i_B` to keep signed values

### Fixed
- `displaySettingScreen`: added the missing "Click=save" instruction

### Cleaned
- `I_OUT_HYSTERESIS`: removed unused constant

### Documentation
- PREMORTEM V3.11 aligns variable names with their usage

## [2.4.4] - 2025-11-26

### Improved
- **Calibration diagnostics:** show computed V_min/V_max on the OLED
- Added "Expected: 4.5-5.5 / 14.5-16" message to simplify resistor debugging

## [2.4.3] - 2025-11-26

### Safety
- **Blocking boot validation:** failed `validateDigipotRange()` now halts startup
- Outputs are immediately disabled
- OLED message + flashing red LED

## [2.4.2] - 2025-11-26

### Fixed
- `updateEnergy()`: real-time delta (Purist ×5 bug corrected)

### Added
- Unit tests `test_digipot_conversion.cpp` (20 PASS)

## [2.4.1] - 2025-11-26

### Added
- Adaptive OCP based on V_OUT (LM317 thermal protection)
  - 5-6V: 350mA max
  - 7-9V: 450mA max
  - 10-15V: 500mA max

### Fixed
- `validateDigipotRange()`: raw calculation without constrain

## [2.4.0] - 2025-11-26

### Added
- Adaptive post-regulator architecture (MCP41100 on LM317 feedback)
- `getActualHeadroom()` for monitoring
- Digipot range validation at boot

### Fixed
- MCP41100 now within specs (wiper at 1.25V)
- LT3045 dissipation reduced to ~1W

## [2.3.1] - 2025-11-25

### Fixed
- Restored INA219 negative-current protection
- Restored atomic sleep pattern (cli/sei)
- Corrected condition order in `displayOutputScreen`

## [2.3.0] - 2025-11-25

### Added
- Multi-language (EN/FR/DE)
- Sweep tuning with MCP41100
- Guided fault solutions (4 pages)
