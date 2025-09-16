import serial
import math
import matplotlib.pyplot as plt
from collections import deque

# Configuration port série
ser = serial.Serial("COM3", 115200) 
buffer_raw = deque(maxlen=500)
buffer_dB = deque(maxlen=500)

plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))  # 2 sous-graphiques

ax1.set_title("Signal brut")
ax1.set_ylim(-1.2, 1.2)  # Normalisé entre -1 et 1
ax2.set_title("Signal en dB SPL")
ax2.set_ylim(0, 200)     # Ajuste selon ton micro

while True:
    line = ser.readline().decode().strip()
    if line:
        try:
            val = float(line.split(":")[1].strip())
        except:
            continue

        # Buffers
        buffer_raw.append(val)
        dBFS = 20 * math.log10(abs(val) + 1e-10)
        dBSPL = dBFS + 120
        buffer_dB.append(dBSPL)

        # Affichage
        ax1.clear()
        ax2.clear()

        ax1.plot(list(buffer_raw))
        ax1.set_title("Signal brut")
        ax1.set_ylim(-0.01, 0.01)

        ax2.plot(list(buffer_dB))
        ax2.set_title("Signal en dB SPL")
        ax2.set_ylim(0, 200)

        plt.pause(0.01)
