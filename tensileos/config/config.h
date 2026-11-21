#ifndef TENSILE_CONFIG_CONFIG_H
#define TENSILE_CONFIG_CONFIG_H

#pragma once
// ============ TensileOS Configuration ============

// Option A: join your home Wi-Fi (the OS prints its IP on the screen).
// Leave SSID empty ("") to skip this and use Option B.
#define WIFI_STA_SSID ""
#define WIFI_STA_PASS ""

// Option B: the ESP32 makes its own hotspot (used if A is empty or fails).
// Connect your laptop or phone to it, then open http://192.168.4.1/
#define AP_SSID   "TensileOS"
#define AP_PASS   "tensile123"   // min 8 chars, or "" for an open network

#define HOSTNAME   "tensileos"   // http://tensileos.local/ on home Wi-Fi
#define OS_NAME    "TensileOS"
#define OS_VERSION "0.2.0"



#endif // TENSILE_CONFIG_CONFIG_H
