#ifndef TENSILE_NET_NET_H
#define TENSILE_NET_NET_H

#pragma once
#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <esp_wifi.h>
#include "config/config.h"

static bool g_apMode = false;

static String netIP() {
  return (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

static void startNetwork() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, strlen(AP_PASS) ? AP_PASS : NULL);
  g_apMode = true;

  char staSSID[33] = "";
  char staPass[64] = "";

  if (strlen(WIFI_STA_SSID) > 0) {
    strncpy(staSSID, WIFI_STA_SSID, sizeof(staSSID) - 1);
    strncpy(staPass, WIFI_STA_PASS, sizeof(staPass) - 1);
  } else {
    // Check persistent storage in LittleFS
    if (LittleFS.begin(true) && LittleFS.exists("/CONFIG/WIFI.DAT")) {
      File f = LittleFS.open("/CONFIG/WIFI.DAT", "r");
      if (f) {
        String s = f.readStringUntil('\n'); s.trim();
        String p = f.readStringUntil('\n'); p.trim();
        strncpy(staSSID, s.c_str(), sizeof(staSSID) - 1);
        strncpy(staPass, p.c_str(), sizeof(staPass) - 1);
        f.close();
      }
    }
  }

  if (staSSID[0]) {
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(staSSID, staPass);
    Serial.printf("Connecting to Wi-Fi '%s'...", staSSID);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
      delay(250);
      Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      g_apMode = false;
      Serial.printf("Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
      Serial.println("Wi-Fi connection failed; running in AP mode.");
    }
  }

  WiFi.setSleep(false);              // lower latency
  esp_wifi_set_ps(WIFI_PS_NONE);      // disable Wi-Fi modem sleep on PHY layer for rock-solid 1-2ms latency
  if (MDNS.begin(HOSTNAME)) MDNS.addService("http", "tcp", 80);
  Serial.printf(OS_NAME ": open http://%s/ (AP: 192.168.4.1)\n", netIP().c_str());
}


#endif // TENSILE_NET_NET_H
