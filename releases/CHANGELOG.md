
# Changelog

Toutes les modifications notables de ce projet sont documentées ici.

## [2.4.2] - 2025-11-26

### Corrigé
- Calcul énergie updateEnergy() avec delta temps réel
  - Bug: mode Purist gonflait l'énergie ×5 (appels à 200ms au lieu de 1s)
  - Fix: calcul delta_ms avec millis() et accumulateur uptime

### Ajouté
- Tests unitaires conversions digipot (20 tests, 100% PASS)
  - test_digipot_conversion.cpp

## [2.4.1] - 2025-11-26

### Ajouté
- OCP adaptatif selon V_OUT (protection thermique LM317)
  - 5-6V: 350mA max
  - 7-9V: 450mA max
  - 10-15V: 500mA max
- Validation plage digipot au boot (calcul brut sans constrain)

### Corrigé
- Documentation hardware: F_SECTEUR 315mA T, C_MAIN 50V

## [2.4.0] - 2025-11-26

### Ajouté
- Architecture post-regulator adaptative (MCP41100 sur feedback LM317)
- Fonction getActualHeadroom() pour monitoring
- Validation plage digipot au boot
- Affichage headroom max sur page Santé

### Corrigé
- MCP41100 maintenant dans specs (wiper à 1. 25V)
- Dissipation LT3045 réduite à ~1W (était 5. 75W)

## [2.3.1] - 2025-11-25

### Corrigé
- Protection courant négatif INA219 restaurée
- Pattern sleep atomique (cli/sei) restauré
- Ordre conditions displayOutputScreen corrigé

## [2.3.0] - 2025-11-25

### Ajouté
- Multi-langue (EN/FR/DE)
- Sweep tuning avec MCP41100
- Solutions guidées fautes (4 pages)

## [2.2. 1] - 2025-11-24

### Ajouté
- Interface cosmétique améliorée
- Score stabilité temps réel
