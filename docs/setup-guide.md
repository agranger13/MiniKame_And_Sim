# Guide de setup — miniKame Sim2Real (Webots + PlatformIO)

Checklist à suivre soi-même. Basé sur `.claude/CLAUDE.md` et l'état du repo au moment de la rédaction.

## État actuel du repo

| Élément | État |
|---|---|
| `kame-core/KameHAL.h` | ✅ présent (interface HAL définie, include guard) |
| `kame-core/Oscillator/Oscillator.h/.cpp` | ✅ porté depuis le miniKame original, au-dessus du HAL (zéro dépendance Arduino) |
| `kame-core/gaits.h/.cpp` | ❌ à créer (jalon 5, suite) |
| `kame-core/kame.h/.cpp` | ❌ à créer (jalon 5, suite) |
| `simulation/controllers/MinikameAvance/WebotsHAL.h/.cpp` | ✅ backend Webots du HAL (branché sur `Motor::setPosition`, `robot->getTime()`) |
| `firmware/` (PlatformIO) | ✅ scaffoldé (`platformio.ini`, `src/main.cpp`) mais **template par défaut**, pas encore branché sur `kame-core` ni le HAL |
| `firmware/platformio.ini` | ⚠️ ne contient pas encore `lib_extra_dirs = ../` |
| `simulation/worlds/MiniKame.wbt` | ✅ monde Webots avec **4 pattes** (hip+knee ×4, limites `maxVelocity`/`maxTorque` posées, `DEF`/`USE` pour les Shapes) |
| `simulation/controllers/MinikameAvance/` | ✅ contrôleur branché sur `kame-core/Oscillator` via `WebotsHAL` (trot diagonal), robot **debout et stable** en simu après calibration (genou centré à 45°, miroir droite/gauche corrigé) — avancée réelle du trot pas encore validée |
| `simulation/protos/` | ❌ vide — pas de PROTO `MiniKame.proto` (prévu jalon 3 avancé / plus tard) |

---

## 1. Outils à installer

### Webots
1. Télécharger l'installeur Windows sur https://cyberbotics.com/#download
2. Installer (chemin par défaut `C:\Program Files\Webots` ok).
3. Lancer Webots une fois pour vérifier qu'il démarre, ouvrir un monde d'exemple (`File > Open Sample World`).
4. Suivre les **tutoriels officiels 1 à 4** (obligatoire avant de coder le contrôleur) : https://cyberbotics.com/doc/guide/tutorials
   - Tuto 1 : notions de base (monde, `Robot`, `step`)
   - Tuto 2 : contrôleur simple
   - Tuto 3 : capteurs
   - Tuto 4 : `HingeJoint` / moteurs rotatifs — **le plus important pour miniKame** (patte = 2 `RotationalMotor`)

### PlatformIO
1. Vérifier si l'extension VS Code est installée : dans VS Code, `Ctrl+Shift+X` → chercher "PlatformIO IDE".
2. Si absent, installer l'extension (marketplace).
3. Vérifier la CLI (utile pour build headless) :
   ```
   pio --version
   ```
   Si `pio` n'est pas trouvé, PlatformIO Core s'installe automatiquement avec l'extension VS Code (ou `pip install platformio`).
4. Tester que le projet `firmware/` compile déjà tel quel (template vide) :
   ```
   cd firmware
   pio run
   ```
   Ça valide que la board `nodemcuv2` et le toolchain ESP8266 sont bien installés, avant même de toucher au code.

### Driver CH340 (port série NodeMCU)
1. Brancher le NodeMCU en USB.
2. Vérifier dans le Gestionnaire de périphériques Windows si un port COM apparaît (`Ports (COM & LPT)`).
3. Si le NodeMCU apparaît en périphérique inconnu / pas de port COM : installer le driver CH340
   → https://www.wch.cn/downloads/CH341SER_EXE.html (ou chercher "CH340 driver Windows" — utilitaire connu de l'écosystème Arduino/NodeMCU).
4. Redémarrer, rebrancher, vérifier que le port COM apparaît.
5. Noter le numéro de port (`COMx`) — servira pour l'upload PlatformIO plus tard (`upload_port` dans `platformio.ini` si l'auto-détection échoue).

---

## 2. Structure de dossiers cible (rappel du plan)

```
minikame-project/
├── kame-core/                  # C++ pur, ZÉRO dépendance (ni Arduino.h ni webots)
│   ├── KameHAL.h                # ✅ déjà fait
│   ├── Oscillator/Oscillator.h/.cpp  # ✅ déjà fait — sinusoïdes (amplitude, offset, période, phase)
│   ├── gaits.h/.cpp             # à créer — walk/turn/jump = params des 8 oscillateurs
│   └── kame.h/.cpp              # à créer — logique robot, n'appelle QUE le HAL
├── firmware/                   # ✅ scaffoldé par PlatformIO
│   ├── platformio.ini          # ⚠️ à compléter (lib_extra_dirs)
│   └── src/main.cpp            # ⚠️ encore le template par défaut, pas branché sur KameHAL
└── simulation/                 # ✅ dossier existe, monde présent
    ├── worlds/MiniKame.wbt     # ✅ 4 pattes, limites SG90 posées
    ├── protos/MiniKame.proto   # ❌ à créer (plus tard, jalon 3 avancé)
    └── controllers/MinikameAvance/
        ├── Makefile              # ✅ inclut ../../../kame-core
        ├── MinikameAvance.cpp    # ✅ branché sur Oscillator/WebotsHAL, mode calibration + marche
        └── WebotsHAL.h/.cpp      # ✅ backend Webots du HAL
```

**Rien à créer tout de suite** — voir jalons ci-dessous, l'ordre compte (pas la peine de créer `gaits.h` avant d'avoir une patte qui bouge en simu).

---

## 3. Config à ajouter manuellement (quand tu seras prêt à coder)

Extraits de référence issus de `.claude/CLAUDE.md`, à copier le moment venu.

**`firmware/platformio.ini`** — ajouter une ligne pour que le firmware retrouve `kame-core` :
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
upload_speed = 921600        ; si instable : 460800 ou 115200
lib_extra_dirs = ../         ; pour trouver kame-core
```

**`simulation/controllers/kame_ctrl/Makefile`** — doit inclure les sources de `kame-core` (pas de symlink sous Windows, on référence le chemin directement) :
```makefile
INCLUDE = -I../../../kame-core
CXX_SOURCES = kame_ctrl.cpp ../../../kame-core/oscillator.cpp ../../../kame-core/gaits.cpp ../../../kame-core/kame.cpp
```
(le Makefile exact dépendra du template généré par Webots — New Project Wizard C++ — à ajuster une fois généré)

---

## 4. Ordre des jalons (roadmap, ne rien sauter)

1. ✅ **Prise en main Webots** — tutos 1 à 4, en particulier `HingeJoint`/`RotationalMotor` (tuto 4).
2. ✅ **Jalon minimal** — cube + UNE patte à 2 servos qui oscille (moteur codé en dur dans le contrôleur, sans passer par `kame-core`) — mécanique HingeJoint/moteur validée.
3. ✅ **Limites matérielles posées dans Webots** (`maxVelocity`, `maxTorque`, masse) + **passage à 2 pattes** dans `MiniKame.wbt` / `MinikameAvance.cpp`.
4. ✅ **Modèle complet** — 4 pattes dupliquées via `DEF`/`USE` dans `MiniKame.wbt`, contrôleur `MinikameAvance.cpp` animant les 8 moteurs (trot diagonal en dur, sans `kame-core`).
5. ✅ **Porter le code dans `kame-core` + brancher le backend Webots** — `Oscillator.h/.cpp` porté au-dessus du HAL (zéro dépendance Arduino), `WebotsHAL` créé et branché dans `MinikameAvance.cpp`. Bug de montage en miroir (pattes droites 1/3 vs gauches 2/4) diagnostiqué via un mode calibration dédié (paliers d'angle + logs console) et corrigé (reflet autour de 90°, le centre mécanique réel du genou). **Robot debout et stable en simu** (genou centré à 45° ±15°, hanche ±29° autour de 90°). `gaits.h/.cpp` et `kame.h/.cpp` restent à créer.
6. 🔄 **En cours — Valider la marche** — vérifier que le trot fait réellement avancer le robot (pas juste du surplace/instable en mouvement), ajuster `hip_amp_deg`/`period`/`knee_amp_deg` si besoin. Puis brancher `firmware/src/main.cpp` (Servo.h ESP8266 + trims) sur le même `KameHAL`.
7. **Validation croisée** — flasher le NodeMCU, calibrer les trims, filmer sim et réel côte à côte, ajuster `contactProperties` (friction) dans Webots.
8. **(Plus tard) RL** — URDF → PyBullet + Gymnasium + Stable-Baselines3 (PPO), communication UDP ~50 Hz PC → ESP8266.

---

## 5. Pièges Windows à connaître (déjà actés dans `.claude/CLAUDE.md`)

- **Pas de symlinks** entre `kame-core` et les deux projets consommateurs — capricieux sous Windows. Utiliser `lib_extra_dirs` (PlatformIO) et chemins relatifs `INCLUDE`/`CXX_SOURCES` (Makefile Webots) comme montré ci-dessus.
- **Pins ESP8266 à éviter** : D3, D4, D8 (pins de boot → à-coups servo au démarrage). D5 = GPIO14, ok.
- Board PlatformIO correcte pour un NodeMCU V3 : `nodemcuv2` (pas une board "V3" officielle distincte — même ESP12E/12F).

---

## Résumé — prochaines actions dans l'ordre

1. [x] Installer Webots + faire les tutos 1-4
2. [x] Vérifier PlatformIO (extension + `pio run` sur `firmware/` tel quel)
3. [ ] Vérifier/installer le driver CH340, noter le port COM (à faire avant le premier flash réel)
4. [x] Jalon 2 (cube + 1 patte 2 servos qui oscille) validé
5. [x] Jalon 3 (limites matérielles + passage à 2 pattes) validé
6. [x] Jalon 4 : 4 pattes dupliquées dans `MiniKame.wbt`, contrôleur animant les 8 moteurs (trot diagonal)
7. [x] Jalon 5 : `Oscillator` porté dans `kame-core` au-dessus du HAL, `WebotsHAL` branché, bug de miroir droite/gauche corrigé, robot debout et stable en simu
8. [ ] **En cours** — Jalon 6 : valider que le trot fait avancer le robot, puis brancher `firmware/src/main.cpp` (backend ESP8266) sur le même `KameHAL`
