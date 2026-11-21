# Getting Started with TensileOS

This guide walks you through setting up your development environment, compiling TensileOS, flashing it to your ESP32, and connecting to your personal retro desktop over Wi-Fi.

---

## 1. What You Need

* **Hardware:** Any standard ESP32 development board (e.g. ESP32-WROOM-32, NodeMCU-32S, ESP32 Dev Module) with 4 MB Flash.
* **USB Cable:** Micro-USB or USB-C (depending on your board) with data sync capability.
* **Computer:** Windows, macOS, or Linux with a modern web browser (Google Chrome, Edge, Safari, Firefox).
* **Software:** [Arduino IDE 2.x](https://www.arduino.cc/en/software) OR [arduino-cli](https://arduino.github.io/arduino-cli/).

---

## 2. Environment Setup

### Method A: Arduino IDE 2.x (GUI)
1. Open Arduino IDE.
2. Go to **File → Preferences** (or **Arduino IDE → Settings** on macOS).
3. In **Additional boards manager URLs**, add:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Open **Tools → Board → Boards Manager**, search for `esp32`, and install `esp32` by Espressif Systems (version 2.0.6 or newer; version 3.x is fully supported).

### Method B: Arduino CLI (Command Line)
```bash
arduino-cli config init
arduino-cli config add board_manager.additional_urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index
arduino-cli core install esp32:esp32
```

---

## 3. Configuring TensileOS

Before compiling, you can optionally configure your network settings in [`tensileos/config/config.h`](../tensileos/config/config.h):

```cpp
// Option A: Join your home Wi-Fi network
#define WIFI_STA_SSID "YourHomeSSID"
#define WIFI_STA_PASS "YourHomePassword"

// Option B: Standalone Hotspot (Default)
#define AP_SSID       "TensileOS"
#define AP_PASS       "tensile123"

#define HOSTNAME      "tensileos"
```

* **Standalone Hotspot (Default):** If `WIFI_STA_SSID` is empty, the ESP32 broadcasts its own Wi-Fi hotspot named `TensileOS`.
* **Home Wi-Fi (Station Mode):** If configured, the ESP32 joins your router and becomes accessible at `http://tensileos.local/` or its assigned local IP.

---

## 4. Compiling & Flashing

### Crucial Step: Partition Scheme
> [!IMPORTANT]
> TensileOS includes a rich suite of 16 applications, graphics sprites, and fonts totaling ~1.27 MB. You **MUST** select the `Huge APP` partition scheme; the default 1.2 MB app partition is too small.

### Via Arduino IDE:
1. Open `tensileos/tensileos.ino`.
2. Connect your ESP32 to your PC via USB.
3. Select your board: **Tools → Board → ESP32 Arduino → ESP32 Dev Module**.
4. Configure board settings under **Tools**:
   * **CPU Frequency:** 240MHz (WiFi/BT)
   * **Partition Scheme:** `Huge APP (3MB No OTA/1MB SPIFFS)`
   * **Upload Speed:** 921600 (or 115200 if upload fails)
   * **Port:** Select your ESP32's COM port (e.g. `COM8` on Windows, `/dev/ttyUSB0` on Linux).
5. Click the **Upload** button (`Ctrl+U` or `Cmd+U`).

### Via Arduino CLI:
```powershell
# Compile
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app tensileos

# Upload (replace COM8 with your serial port)
arduino-cli upload -p COM8 --fqbn esp32:esp32:esp32:PartitionScheme=huge_app tensileos
```

---

## 5. Connecting to the Desktop

1. Once flashing completes, the ESP32 reboots automatically.
2. On your computer, tablet, or smartphone:
   * Open Wi-Fi settings and connect to `TensileOS` (Password: `tensile123`).
3. Open your browser and go to:
   ```
   http://192.168.4.1/
   ```
4. You will see the Classic Desktop boot screen, hear the signature chime, and the desktop will load in crisp 320×240 pixel resolution.

---

## 6. Next Steps
* Learn the architecture: read [Architecture Guide](architecture.md).
* Explore the Command Prompt: read [CLI Reference](cli-reference.md).
* Add your own custom app or game: read [Development Guide](development-guide.md).
