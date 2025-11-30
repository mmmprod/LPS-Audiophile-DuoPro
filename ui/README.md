# LPS DUO PRO - Interface PyGame V92

Interface tactile pour écran Raspberry Pi 7" (800×480).

## Fonctionnalités

- **5 pages** : Écoute, Détails, Santé, Session, Config
- **Style Soviet Nixie** : Jauges VU-mètre vintage, tubes Nixie, phosphore ambre
- **Mode simulation** intégré pour tests sans matériel
- **4 langues** : FR, EN, ES, DE
- **Protections** : OVP, OCP, OTP avec indicateurs visuels
- **Mise à jour USB** : Support MAJ firmware et UI via clé USB

## Prérequis

- Python 3.7+
- Raspberry Pi avec écran tactile 7" (800×480)

## Installation

```bash
cd ui
pip install -r requirements.txt
```

## Lancement

```bash
python lps_duo_pro.py
```

## Raccourcis clavier

| Touche | Action |
|--------|--------|
| `1-5` | Navigation pages |
| `ESC` | Fermer popup / Quitter |
| `ENTER` | Démarrer (écran boot) |

## Optimisations V92

- `draw.lines()` pour oscilloscopes (690→3 appels/frame)
- Cache `get_all_problems()` par frame
- LCD agrandi 95px
- Headroom 2.0V (conforme circuit V2.4.5)
