# Guide de setup — miniKame Sim2Real (Webots + PlatformIO)

Checklist à suivre soi-même. Basé sur `.claude/CLAUDE.md` et l'état du repo au moment de la rédaction.

## État actuel du repo

| Élément | État |
|---|---|
| `kame-core/hal.h` | ✅ présent (interface HAL définie) |
| `kame-core/oscillator.h/.cpp` | ❌ à créer |
| `kame-core/gaits.h/.cpp` | ❌ à créer |
| `kame-core/kame.h/.cpp` | ❌ à créer |
| `firmware/` (PlatformIO) | ✅ scaffoldé (`platformio.ini`, `src/main.cpp`) mais **template par défaut**, pas encore branché sur `kame-core` ni le HAL |
| `firmware/platformio.ini` | ⚠️ ne contient pas encore `lib_extra_dirs = ../` |
| `simulation/worlds/MiniKame.wbt` | ✅ un monde Webots existe déjà |
| `simulation/controllers/` | ❌ vide — pas de contrôleur C++ ni de Makefile |
| `simulation/protos/` | ❌ vide — pas de PROTO `MiniKame.proto` |

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
│   ├── hal.h                   # ✅ déjà fait
│   ├── oscillator.h/.cpp       # à créer — sinusoïdes (amplitude, offset, période, phase)
│   ├── gaits.h/.cpp            # à créer — walk/turn/jump = params des 8 oscillateurs
│   └── kame.h/.cpp             # à créer — logique robot, n'appelle QUE le HAL
├── firmware/                   # ✅ scaffoldé par PlatformIO
│   ├── platformio.ini          # ⚠️ à compléter (lib_extra_dirs)
│   └── src/main.cpp            # ⚠️ encore le template par défaut
└── simulation/                 # ✅ dossier existe, monde présent
    ├── worlds/kame.wbt         # ✅ MiniKame.wbt déjà là (nom à harmoniser si besoin)
    ├── protos/MiniKame.proto   # ❌ à créer (plus tard, jalon 3)
    └── controllers/kame_ctrl/
        ├── Makefile             # ❌ à créer — doit inclure ../../../kame-core
        └── kame_ctrl.cpp         # ❌ à créer
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

1. **Prise en main Webots** — tutos 1 à 4, en particulier `HingeJoint`/`RotationalMotor` (tuto 4).
2. **Jalon minimal** — via Webots, `Wizards > New Project Directory`, créer un monde avec un cube + UNE patte à 2 servos qui oscille (moteur codé en dur dans le contrôleur généré, sans passer par `kame-core` pour l'instant — juste pour valider la mécanique HingeJoint/moteur).
3. **Limites matérielles à poser dans Webots dès ce jalon** (sinon gaits infaisables sur le vrai robot) :
   - `maxVelocity` ≈ 8–10 rad/s (SG90 : ~0,1s/60°)
   - `maxTorque` ≈ 0,15–0,18 N·m (SG90 : ~1,8 kg·cm)
   - masse totale du robot ~300–400 g répartie sur les pièces
4. **Modèle complet** — dupliquer la patte ×4, en primitives (boîtes/cylindres) d'abord.
5. **Porter le code dans `kame-core`** — créer `oscillator.h/.cpp`, `gaits.h/.cpp`, `kame.h/.cpp` au-dessus du HAL, en se basant sur `code/arduino/src/minikame.cpp` du repo original JavierIH.
6. **Brancher les deux backends** — `simulation/controllers/kame_ctrl/kame_ctrl.cpp` (Webots Motor) et `firmware/src/main.cpp` (Servo.h ESP8266 + trims), tous deux au-dessus de `KameHAL`.
7. **Validation croisée** — flasher le NodeMCU, calibrer les trims, filmer sim et réel côte à côte, ajuster `contactProperties` (friction) dans Webots.
8. **(Plus tard) RL** — URDF → PyBullet + Gymnasium + Stable-Baselines3 (PPO), communication UDP ~50 Hz PC → ESP8266.

---

## 5. Pièges Windows à connaître (déjà actés dans `.claude/CLAUDE.md`)

- **Pas de symlinks** entre `kame-core` et les deux projets consommateurs — capricieux sous Windows. Utiliser `lib_extra_dirs` (PlatformIO) et chemins relatifs `INCLUDE`/`CXX_SOURCES` (Makefile Webots) comme montré ci-dessus.
- **Pins ESP8266 à éviter** : D3, D4, D8 (pins de boot → à-coups servo au démarrage). D5 = GPIO14, ok.
- Board PlatformIO correcte pour un NodeMCU V3 : `nodemcuv2` (pas une board "V3" officielle distincte — même ESP12E/12F).

---

## Résumé — prochaines actions dans l'ordre

1. [ ] Installer Webots + faire les tutos 1-4
2. [ ] Vérifier PlatformIO (extension + `pio run` sur `firmware/` tel quel)
3. [ ] Vérifier/installer le driver CH340, noter le port COM
4. [ ] Dans Webots, créer le jalon 2 (cube + 1 patte 2 servos qui oscille) via le Wizard
5. [ ] Une fois la patte validée en simu → attaquer `kame-core` (oscillateurs/gaits) et le branchement des deux backends
