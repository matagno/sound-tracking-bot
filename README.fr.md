# Sound-Tracking Bot

[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/matagno/sound-tracking-bot/blob/master/README.md)
[![fr](https://img.shields.io/badge/lang-fr-blue.svg)](https://github.com/matagno/sound-tracking-bot/blob/master/README.fr.md)

## 📋 Description

**Sound-Tracking Bot** est un robot quadripode autonome contrôlé par **détection acoustique** et capable de :
- 🎵 Localiser une source sonore via corrélation croisée stéréo
- 🤖 Se mouvoir de manière autonome ou téléopérée
- 🌐 Communiquer via WebSocket pour le contrôle à distance
- 🎚️ Filtrer et traiter les signaux audio en temps réel

Le système fonctionne sur un **ESP32** avec acquisition audio I2S stéréo et contrôle de 12 servomoteurs via PCA9685.

---

## 🏗️ Architecture

### Structure du projet
```
src/
├── main.cpp                    # Point d'entrée, tâches FreeRTOS
├── bot/                        # Contrôle du robot
│   ├── bot_ctrl.cpp/hpp        # Cinématique inverse, locomotion
│   └── utils/
│       ├── ik_calcul.cpp/hpp   # Calcul IK pour les pattes
│       └── pca9685.cpp/hpp     # Driver PWM pour servos
├── sound/                      # Traitement audio
│   ├── i2s_sound_acquisition.cpp/hpp  # Acquisition I2S stéréo
│   ├── st_sample_data.hpp      # Structure de données audio
│   └── utils/
│       └── biquad_filter.cpp/hpp      # Filtre passe-bande
└── ws_com/                     # Communication WebSocket
    ├── web_socket_server.cpp/hpp      # Serveur WebSocket
    └── st_cmd_data.hpp         # Commandes reçues
```

---

## 🎯 Fonctionnalités principales

### 1️⃣ Détection acoustique
```cpp
// Localisation par corrélation croisée des signaux L/R
// Calcule l'angle de la source sonore en degrés
int calculate_angle(const std::vector<float>& sigL, const std::vector<float>& sigR)
```
- **Fréquence d'échantillonnage** : 44.1 kHz
- **Bande de fréquence** : 1-1.2 kHz (filtrage passe-bande)
- **Distance inter-micros** : 10 cm
- **Résolution** : Fenêtres de 441 échantillons

### 2️⃣ Cinématique inverse
```cpp
std::array<double,3> ik_leg(const std::array<double,3>& target, ...)
// Retourne [hip_angle, knee_angle, foot_angle]
```
- Calcul IK pour **4 pattes** (2 avant, 2 arrière)
- Longueurs : coxa=60mm, tibia=76.84mm, tarse=128.05mm

### 3️⃣ Locomotion
- **Mode autonome** : Suivi du son avec gait tripod
- **Mode téléopéré** : Contrôle via WebSocket
- **Gait parameters** : Longueur de pas=130mm, hauteur=70mm, période=2s

### 4️⃣ Contrôle WebSocket
```
ws://192.168.4.1/ws

Commandes :
- "ping" → "pong"
- "get_angle" → angle en degrés
- "set_auto-true/false"
- "set_manual-true/false"
- "set_teleop-true/false"
- "set_qTarget-<float>-<index>"
- "set_qActive-<true/false>-<index>"
```

---

## 🔧 Configuration matérielle

### ESP32 D1 Mini
| Composant | GPIO | Notes |
|-----------|------|-------|
| **I2S Audio** | | Acquisition stéréo |
| BCK (Bit Clock) | 26 | |
| WS (Word Select) | 25 | |
| DATA_IN | 17 | Données L/R 32-bit |
| **PCA9685** | I2C | PWM servo driver @ 50 Hz |
| SDA | 21 | |
| SCL | 22 | |
| **WiFi** | SoftAP | SSID: `ESP_Spider` |

### Servomoteurs
- **Nombre** : 12 (3 par patte × 4 pattes)
- **Plage** : 0°-180°
- **Fréquence** : 50 Hz

### Schéma électrique
![Schéma électrique](schematic_spider.png)

---

## 🚀 Démarrage rapide

### 1. Configuration
```bash
# Copier la configuration
cp sdkconfig.esp32_d1_mini sdkconfig
```

### 2. Build et upload
```bash
idf.py build
idf.py flash monitor
```

### 3. Connexion WebSocket
```bash
# Via wscat
wscat -c ws://192.168.4.1/ws
```

---

## 📊 Traitement du signal audio

### Pipeline audio
```
I2S Input (44.1 kHz) 
    ↓
Biquad Filter (1-1.2 kHz)
    ↓
Fenêtre glissante (441 échantillons)
    ↓
Corrélation croisée L-R
    ↓
Calcul angle (arcsin + conversion degrés)
```

### Filtre passe-bande
```cpp
void setup_bandpass(float f1, float f2, float fs)
// Fréquence centrale : sqrt(f1*f2)
// Facteur Q : sqrt(f2/f1)
```

---

## 🤝 Modes de fonctionnement

### Mode Autonome
- Écoute active du bruit
- Si angle valide : tourne vers la source
- Si angle < 20° : avance
- Sinon : continue à tourner

### Mode Manuel
- Contrôle directe des angles cibles
- Activation/désactivation des servos individuels

### Mode Téléopéré
- Commandes de **run** (avancer) et **turn** (tourner)
- Angle de virage paramétrable

---

## 📈 Tâches FreeRTOS

| Tâche | Priorité | Période | Fonction |
|-------|----------|---------|----------|
| `sound_task` | 5 | Continu | Acquisition audio I2S |
| `cycle_task` | 4 | 100 ms | Contrôle moteurs, traitement |

---

## 📚 Références

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [FreeRTOS](https://www.freertos.org/)

---
