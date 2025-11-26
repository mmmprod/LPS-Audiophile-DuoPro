# Changelog

Toutes les modifications notables de ce projet sont documentées ici.

## [2.4.4] - 2025-11-26

### Amélioré
- **Diagnostic calibration** : affichage V_min/V_max calculés sur écran OLED
- Message "Attendu: 4.5-5.5 / 14.5-16" pour faciliter debug résistances
- Variables `calib_v_out_min` et `calib_v_out_max` pour diagnostic

### Documentation
- Test unitaire mis à jour V2.4.4

## [2.4. 3] - 2025-11-26

### Sécurité
- **Validation boot bloquante** : échec `validateDigipotRange()` → blocage total
- Sorties désactivées immédiatement (`setOutputEnable OFF`)
- Message OLED : "ERREUR CALIBRATION" + "Verif R_FIXED/R1/R_SHUNT"
- Boucle infinie avec LED rouge clignotante + `wdt_reset()`

### Documentation
- PREMORTEM V3.10 avec modules 9. 8, 9.9, 12

## [2.4.2] - 2025-11-26

### Corrigé
- `updateEnergy()` : delta temps réel (bug Purist ×5 corrigé)

### Ajouté
- Tests unitaires `test_digipot_conversion.cpp` (20 PASS)

## [2.4.1] - 2025-11-26

### Ajouté
- OCP adaptatif selon V_OUT (protection thermique LM317)
  - 5-6V: 350mA max
  - 7-9V: 450mA max
  - 10-15V: 500mA max

### Corrigé
- `validateDigipotRange()` : calcul brut sans constrain
- Documentation hardware: F_SECTEUR 315mA T, C_MAIN 50V

## [2. 4.0] - 2025-11-26

### Ajouté
- Architecture post-regulator adaptative (MCP41100 sur feedback LM317)
- Fonction `getActualHeadroom()` pour monitoring
- Validation plage digipot au boot
- Affichage headroom max sur page Santé

### Corrigé
- MCP41100 maintenant dans specs (wiper à 1.25V)
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

## [2.2.1] - 2025-11-24

### Ajouté
- Interface cosmétique améliorée
- Score stabilité temps réel
