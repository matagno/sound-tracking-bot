import serial
import math
import matplotlib.pyplot as plt
from collections import deque

# Config serial port
ser = serial.Serial("COM3", 115200)

# Buffers 
buffer_raw_L = deque(maxlen=500)
buffer_dB_L  = deque(maxlen=500)
buffer_raw_R = deque(maxlen=500)
buffer_dB_R  = deque(maxlen=500)

plt.ion()
fig, axes = plt.subplots(2, 2, figsize=(10, 6)) 
axL_raw, axL_dB, axR_raw, axR_dB = axes[0,0], axes[1,0], axes[0,1], axes[1,1]

axL_raw.set_title("Signal brut - Gauche")
axL_raw.set_ylim(-1.2, 1.2)
axL_dB.set_title("Signal en dB SPL - Gauche")
axL_dB.set_ylim(0, 200)

axR_raw.set_title("Signal brut - Droite")
axR_raw.set_ylim(-1.2, 1.2)
axR_dB.set_title("Signal en dB SPL - Droite")
axR_dB.set_ylim(0, 200)

while True:
    line = ser.readline().decode(errors="ignore").strip()

    if line.startswith("New Data"):
        continue

    elif "Sample Left" in line :
        try:
            val = float(line.split("Sample Left:")[1].strip())
        except:
            continue
        buffer_raw_L.append(val)
        dBFS = 20 * math.log10(abs(val) + 1e-10)
        buffer_dB_L.append(dBFS + 120)

    elif "Sample Right" in line:
        try:
            val = float(line.split("Sample Right:")[1].strip())
        except:
            continue
        buffer_raw_R.append(val)
        dBFS = 20 * math.log10(abs(val) + 1e-10)
        buffer_dB_R.append(dBFS + 120)


    # Update graphs
    axL_raw.clear()
    axL_dB.clear()
    axR_raw.clear()
    axR_dB.clear()

    axL_raw.plot(list(buffer_raw_L))
    axL_raw.set_title("Signal brut - Gauche")
    axL_raw.set_ylim(-0.01, 0.01)

    axL_dB.plot(list(buffer_dB_L))
    axL_dB.set_title("Signal en dB SPL - Gauche")
    axL_dB.set_ylim(0, 200)

    axR_raw.plot(list(buffer_raw_R))
    axR_raw.set_title("Signal brut - Droite")
    axR_raw.set_ylim(-0.01, 0.01)

    axR_dB.plot(list(buffer_dB_R))
    axR_dB.set_title("Signal en dB SPL - Droite")
    axR_dB.set_ylim(0, 200)

    plt.pause(0.01)
