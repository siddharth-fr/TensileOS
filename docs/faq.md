# TensileOS Frequently Asked Questions (FAQ)

### What is TensileOS?
TensileOS is a bare-metal, open-source 32-bit retro operating system for the ESP32 microcontroller. It renders an authentic Classic Desktop / Command Prompt style GUI, multitasking desktop, and application suite entirely on-chip and streams it to any web browser over Wi-Fi.

---

### Is TensileOS an emulator (like DOSBox or Bochs)?
**No.** TensileOS is not an x86 emulator. It runs natively compiled C++ code directly on the ESP32's dual-core Xtensa LX6 CPU at 240 MHz. Every window, widget, and game executes bare-metal instructions on the microcontroller.

---

### Do I need to buy a screen, display shield, or breadboard?
**No.** TensileOS was specifically designed to eliminate hardware wiring. The ESP32 hosts its own Wi-Fi Access Point and WebSocket streaming server. You connect to it from any laptop, tablet, or phone browser.

---

### Can I run actual Classic Desktop `.EXE` or Command Prompt `.COM` binaries?
No. TensileOS is an original, clean-room operating system that recreates the interface ergonomics, visual aesthetic, and behavioral feel of Classic Desktop and Command Prompt. Programs are written in standard C++ using the TensileOS `App` API.

---

### How much RAM does TensileOS consume?
TensileOS uses a custom 4-bit packed framebuffer (2 pixels per byte), so the entire 320×240 screen consumes only **38,400 bytes** of RAM. After Wi-Fi, background wallpaper cache, and audio queues, more than **240 KB of dynamic heap memory** remains completely free for user applications.

---

### Can I save files permanently?
**Yes.** TensileOS includes a LittleFS flash virtual disk mounted at `C:\`. Files saved in **Paint** (`.DAT`), **Notepad** (`.TXT`), or downloaded in **RetroWeb** persist across power cycles in the ESP32's onboard SPI flash memory.

---

### Can I connect TensileOS to my home Wi-Fi instead of a hotspot?
**Yes.** Open `tensileos/config/config.h` and enter your home network credentials:
```cpp
#define WIFI_STA_SSID "MyHomeNetwork"
#define WIFI_STA_PASS "MyPassword"
```
When flashed, TensileOS will join your local network, and you can access it from any device on your LAN at `http://tensileos.local/`.

---

### How fast is the display stream?
TensileOS renders at **60 Hz** on the ESP32. With client-side local cursor rendering at 144 Hz and coalesced 120-line WebSocket slicing, window dragging and interactions feel crisp, smooth, and immediate.
