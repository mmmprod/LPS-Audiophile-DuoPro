# Changelog

Toutes les modifications notables de ce projet sont documentées ici.

## [2.4.5] - 2025-11-26

### 🔴 Fix Critique
- **Backfeed detection** : utilisait `i_out` (tronqué à 0) au lieu de `raw_i` (signé)
  - Bug : `i_out` toujours ≥0, donc condition `< -20mA` jamais vraie
  - Fix : variables `raw_i_A` et `raw_i_B` ajoutées pour conserver valeur signée

### Corrigé
- `displaySettingScreen` : ajout instruction "Click=save" manquante

### Nettoyé
- `I_OUT_HYSTERESIS` : constante inutilisée supprimée

### Documentation
- PREMORTEM V3.11 avec cohérence variable/usage

## [2.4.4] - 2025-11-26

### Amélioré
- **Diagnostic calibration** : affichage V_min/V_max calculés sur écran OLED
- Message "Attendu: 4.5-5.5 / 14. 5-16" pour faciliter debug résistances

## [2.4. 3] - 2025-11-26

### Sécurité
- **Validation boot bloquante** : échec `validateDigipotRange()` → blocage total
- Sorties désactivées immédiatement
- Message OLED + LED rouge clignotante

## [2.4.2] - 2025-11-26

### Corrigé
- `updateEnergy()` : delta temps réel (bug Purist ×5 corrigé)

### Ajouté
- Tests unitaires `test_digipot_conversion. cpp` (20 PASS)

## [2. 4.1] - 2025-11-26

### Ajouté
- OCP adaptatif selon V_OUT (protection thermique LM317)
  - 5-6V: 350mA max
  - 7-9V: 450mA max
  - 10-15V: 500mA max

### Corrigé
- `validateDigipotRange()` : calcul brut sans constrain

## [2.4.0] - 2025-11-26

### Ajouté
- Architecture post-regulator adaptative (MCP41100 sur feedback LM317)
- Fonction `getActualHeadroom()` pour monitoring
- Validation plage digipot au boot

### Corrigé
- MCP41100 maintenant dans specs (wiper à 1. 25V)
- Dissipation LT3045 réduite à ~1W

## [2. 3.1] - 2025-11-25

### Corrigé
- Protection courant négatif INA219 restaurée
- Pattern sleep atomique (cli/sei) restauré
- Ordre conditions displayOutputScreen corrigé

## [2.3.0] - 2025-11-25

### Ajouté
- Multi-langue (EN/FR/DE)
- Sweep tuning avec MCP41100
- Solutions guidées fautes (4 pages)
