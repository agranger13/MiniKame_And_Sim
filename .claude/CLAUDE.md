# Projet miniKame — Sim2Real (Webots + PlatformIO)

## Contexte et objectif

Construire un quadrupède **miniKame** (robot open-source de JavierIH : 8 servos SG90, 2 par patte, ESP8266) avec le workflow suivant :

1. Développer et tester les gaits dans un **simulateur (Webots)**.
2. Réutiliser **le même code C++** sur le robot physique via **PlatformIO** (sans copier-coller : lib partagée).
3. À terme : entraîner des politiques de **RL** (PyBullet/Gym, politique exécutée sur PC, angles envoyés en UDP par WiFi à l'ESP8266).

## Décisions déjà actées

- **Simulateur : Webots** (pas Gazebo — pas de ROS, natif Windows, contrôleurs C++).
- **MCU : NodeMCU V3 (ESP8266)** → board PlatformIO **`nodemcuv2`** (la "V3" n'est pas une board officielle ; même ESP12E/12F, brochage identique, puce USB CH340).
- **OS de dev : Windows** (100 % natif, pas de WSL nécessaire). Pas de symlinks (capricieux sous Windows) : utiliser `lib_extra_dirs` côté PlatformIO et `INCLUDE`/`CXX_SOURCES` dans le Makefile du contrôleur Webots.
- **Architecture : HAL (Hardware Abstraction Layer)** — le cœur (oscillateurs, gaits, cinématique) est en C++ pur sans dépendance, avec deux backends : Webots et ESP8266.
- **Priorité immédiate : coder les gaits à la main** (le RL viendra après).
- Profil utilisateur : à l'aise en C++/Arduino, novice en simulation.

## Structure de projet cible

```
minikame-project/
├── kame-core/                  # lib partagée, C++ pur, ZÉRO dépendance (ni Arduino.h ni webots)
│   ├── hal.h                   # interface abstraite
│   ├── oscillator.h/.cpp       # sinusoïdes : amplitude, offset, période, déphasage
│   ├── gaits.h/.cpp            # walk, turn, jump... = jeux de paramètres des 8 oscillateurs
│   └── kame.h/.cpp             # logique robot, n'appelle QUE le HAL
├── firmware/                   # projet PlatformIO
│   ├── platformio.ini
│   └── src/main.cpp            # backend HAL → Servo.h ESP8266 + trims + serveur UDP (plus tard)
└── simulation/                 # projet Webots
    ├── worlds/kame.wbt
    ├── protos/MiniKame.proto
    └── controllers/kame_ctrl/
        ├── Makefile            # inclut ../../../kame-core
        └── kame_ctrl.cpp       # backend HAL → API Webots (Motor)
```

### Le HAL

```cpp
// hal.h — kame-core n'inclut que ça
struct KameHAL {
    virtual void setServo(int id, float angleDeg) = 0;   // convention : 0–180°, 90 = neutre
    virtual unsigned long millis() = 0;
    virtual void delayMs(unsigned long ms) = 0;
};
```

- Backend ESP8266 : `servos[id].write(angle + trim[id])`.
- Backend Webots : `motors[id]->setPosition(deg2rad(angle - 90))` ; `millis()` = `robot->getTime() * 1000` ; `delayMs` = boucle de `robot->step(timeStep)`.
- Les gaits sont écrits en `position = f(t)` (comme le miniKame original) → transposition sim/réel sans douleur.

### platformio.ini

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 115200
upload_speed = 921600        ; si flash instable avec le CH340 : 460800 ou 115200
lib_extra_dirs = ../         ; pour trouver kame-core
```

## Contraintes matérielles à respecter (sim ET réel)

- **Servos SG90** : plage ±90°, vitesse ~0,1 s/60° (**maxVelocity ≈ 8–10 rad/s** dans Webots), couple ~1,8 kg·cm (**maxTorque ≈ 0,15–0,18 N·m**). Sans ces limites en simu, on obtient des gaits infaisables sur le vrai robot.
- **Masse totale** du miniKame : ~300–400 g. Mettre des masses/inerties réalistes.
- **Pins ESP8266** : la sérigraphie D5 = constante Arduino `D5` (GPIO14). Servos branchés en direct sur les GPIO (comme l'original). **Éviter D3, D4, D8** (pins de boot → à-coups servo au démarrage).
- **Driver USB Windows** : CH340 (à installer si pas de port COM).
- Prévoir une **calibration des trims** par servo (offsets mécaniques du montage), réglable via port série ou page web.

## Roadmap

1. **Prise en main Webots** : tutoriels officiels 1–4 (monde, robot, contrôleur C++). Notions : `Robot`, `Motor`, `timeStep`, `robot->step()`.
2. **Premier jalon minimal** : un cube + UNE patte à 2 servos (HingeJoint + RotationalMotor) qui oscille. Ensuite seulement dupliquer ×4.
3. **Modèle complet** : d'abord en primitives (boîtes/cylindres, physiquement correct) ; plus tard, meshes STL exportés des sources FreeCAD du repo miniKame comme géométrie visuelle uniquement (collisions = primitives).
4. **Porter le code miniKame** dans kame-core : classes Oscillator + gaits, réécrites au-dessus du HAL. Logique originale : chaque servo suit une sinusoïde ; pattes diagonales déphasées (90°/270°) ; les servos de pied sont désactivés une demi-période pour lever le pied au retour.
5. **Firmware + validation croisée** : flasher, calibrer les trims, filmer sim et réel côte à côte, ajuster friction Webots (`contactProperties`) et paramètres.
6. **(Plus tard) RL** : URDF → PyBullet + Gymnasium + Stable-Baselines3 (PPO). Politique sur PC, commande UDP ~50 Hz vers l'ESP8266. Étudier le sim2real gap : domain randomization, modélisation servo/latence.

## Références

- Repo original : https://github.com/JavierIH/miniKame (sources FreeCAD, `code/arduino/src/minikame.cpp` pour les oscillateurs/gaits, wiki de montage)
- Projet Hackaday : https://hackaday.io/project/9334-kame-esp8266-based-quadruped
- Explication interactive des oscillateurs : https://oberschweiber.com/2020/03/06/minikame-motion.html (+ son approche UDP : ESP8266 en simple récepteur d'angles, contrôle en Python sur PC)
- Tutoriels Webots : https://cyberbotics.com/doc/guide/tutorials
- Wrapper Gym pour Webots : https://github.com/aidudezzz/deepbots
- RL quadrupède de référence : exemple **Minitaur** de PyBullet + papier Google "Sim-to-Real: Learning Agile Locomotion For Quadruped Robots" ; projet **rex-gym**
- Framework quadrupède ROS (si besoin un jour) : CHAMP — https://github.com/chvmp/champ

## Prochaine action suggérée

Créer le squelette du repo : `kame-core` (hal.h + Oscillator), le monde Webots minimal avec une patte 2 servos, et le projet PlatformIO compilable à vide. Puis jalon 2 de la roadmap.
