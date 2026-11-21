# TensileOS Troubleshooting & Common Errors

This guide addresses common compile, flash, Wi-Fi, and rendering issues with practical solutions.

---

## 1. Compilation Errors

### Error: `Sketch too big / Flash overflow`
* **Symptom:** `text section exceeds available space` or `sketch uses 1275971 bytes ... Maximum is 1310720 bytes`.
* **Cause:** The default Arduino IDE partition scheme allocates only 1.2 MB to the program partition.
* **Fix:** In Arduino IDE, go to **Tools → Partition Scheme** and choose **Huge APP (3MB No OTA/1MB SPIFFS)**. If using `arduino-cli`, always append `--fqbn esp32:esp32:esp32:PartitionScheme=huge_app`.

### Error: `HTTPD_WS_SUPPORT not defined`
* **Symptom:** `error: This core was built without WebSocket support.`
* **Cause:** Outdated ESP32 Arduino core (version 2.0.5 or older).
* **Fix:** Open **Boards Manager** in Arduino IDE and update `esp32` by Espressif Systems to version 2.0.6 or newer (3.x is supported).

---

## 2. Upload & Flashing Errors

### Error: `Could not open port COMx: FileNotFoundError / Resource busy`
* **Cause:** Serial port is held open by another program (Serial Monitor, PuTTY, Cura) or the USB cable is charge-only.
* **Fix:**
  1. Close any open Serial Monitor or terminal window.
  2. Unplug and re-plug the ESP32 USB cable.
  3. Ensure the cable is a data cable, not charge-only.

### Error: `A fatal error occurred: Failed to connect to ESP32: Timed out waiting for packet header`
* **Cause:** ESP32 failed to enter the ROM bootloader automatically.
* **Fix:** Hold down the **BOOT** (or **IO0**) button on your ESP32 board while clicking Upload. Release it when the upload output prints `Connecting........_____.....`.

---

## 3. Wi-Fi & Connection Issues

### Problem: Cannot find `TensileOS` Wi-Fi network
* **Cause:** ESP32 is still booting, or `config.h` was set to Station mode.
* **Fix:** Open Serial Monitor at **115200 baud** and press the **EN/RST** button. Check the boot message to verify whether it started in SoftAP mode (`TensileOS`) or connected to your home router.

### Problem: Browser says `Disconnected - retrying`
* **Cause:** Network congestion or IP mismatch.
* **Fix:**
  1. Make sure your device is connected to the `TensileOS` Wi-Fi network.
  2. Navigate to `http://192.168.4.1/` (not `https://`).
  3. Disable any active VPN or cellular data on your phone while testing local hotspot mode.

---

## 4. Performance & Audio Tips

### Sound is not playing in the browser
* **Cause:** Modern browsers (Chrome, Edge, Safari) block WebAudio auto-play until the user interacts with the page.
* **Fix:** Click anywhere on the TensileOS monitor canvas to grant audio permission; audio tones and chimes will begin playing immediately.

### Frame rate seems sluggish on older mobile devices
* **Fix:** The desktop resolution is 320×240. Ensure hardware 
acceleration is enabled in your browser settings. TensileOS automatically scales using CSS `image-rendering: pixelated` for maximum GPU blitting efficiency.
