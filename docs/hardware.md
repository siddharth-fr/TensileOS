# TensileOS Hardware Guide

TensileOS runs on standard Espressif ESP32 dual-core microcontrollers. This guide details supported boards, pin assignments, analog ADC considerations, and logic output wiring.

---

## 1. Supported Microcontroller Boards

TensileOS requires an ESP32 chip with **4 MB Flash** and standard internal SRAM.

| Board Model | Chip Variant | Clock Speed | Flash Size | Status |
| :--- | :--- | :---: | :---: | :---: |
| **ESP32 DevKitC** | ESP32-WROOM-32D / 32U | 240 MHz | 4 MB | ✅ Fully Tested |
| **NodeMCU-32S** | ESP32-D0WD-V3 | 240 MHz | 4 MB | ✅ Fully Tested |
| **ESP32-CAM (without camera use)** | ESP32-S | 240 MHz | 4 MB | ✅ Supported |
| **TTGO T-Display / T-Beam** | ESP32-D0WDQ6 | 240 MHz | 4 MB / 8 MB | ✅ Supported |
| **ESP32-S2 / ESP32-C3 / ESP32-C6** | Single-core / RISC-V | Various | 4 MB | ⚠️ Experimental (Single-core FreeRTOS changes needed) |

---

## 2. Pinout Map

TensileOS uses GPIO pins for its **Hardware Lab** oscilloscope and digital logic outputs:

```
                          ┌───────────────────────────┐
                          │         ESP32 SoC         │
                          │                           │
  Analog In (0-3.3V) ────►│ GPIO 34 (ADC1_CH6)        │ Oscilloscope Channel 1
  Analog In (0-3.3V) ────►│ GPIO 35 (ADC1_CH7)        │ Oscilloscope Channel 2
                          │                           │
  Logic Out (0/3.3V) ◄────│ GPIO 02 (Digital Out)     │ Logic Line 1 (Onboard LED)
  Logic Out (0/3.3V) ◄────│ GPIO 04 (Digital Out)     │ Logic Line 2
  Logic Out (0/3.3V) ◄────│ GPIO 18 (Digital Out)     │ Logic Line 3
  Logic Out (0/3.3V) ◄────│ GPIO 19 (Digital Out)     │ Logic Line 4
  Logic Out (0/3.3V) ◄────│ GPIO 21 (Digital Out)     │ Logic Line 5
  Logic Out (0/3.3V) ◄────│ GPIO 22 (Digital Out)     │ Logic Line 6
                          └───────────────────────────┘
```

---

## 3. Important ADC Considerations (Wi-Fi Safety)

> [!WARNING]
> **Never connect analog inputs to ADC2 pins (GPIO 0, 2, 4, 12, 13, 14, 15, 25, 26, 27).**
> The ESP32's internal ADC2 peripheral is shared with the Wi-Fi SAR radio. Attempting to sample ADC2 while Wi-Fi is active will cause reading conflicts or system crashes.

* **Safe Analog Pins:** TensileOS strictly samples from **ADC1** pins:
  * **GPIO 34 (ADC1 Channel 6)**
  * **GPIO 35 (ADC1 Channel 7)**
* **Input Voltage Range:** 0.0V to 3.3V. **Do not exceed 3.3V**, as doing so will permanently damage the ESP32 silicon. For signals above 3.3V, use a resistive voltage divider.

---

## 4. Hardware Lab Logic Wiring Example

To test the Hardware Lab's digital outputs:
1. Connect an LED anode (longer leg) to **GPIO 18**.
2. Connect a 220Ω or 330Ω current-limiting resistor to the cathode (shorter leg).
3. Connect the other end of the resistor to **GND**.
4. Open **Hardware Lab** on the TensileOS desktop and click the **GPIO 18** toggle switch. The physical LED and the virtual on-screen green LED indicator will toggle in sync!
