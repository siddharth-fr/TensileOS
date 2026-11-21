#ifndef TENSILE_NET_NETMGR_H
#define TENSILE_NET_NETMGR_H

#pragma once
// Wi-Fi Control Panel & Network Manager: live scanner, signal meters, connect dialog, persistent credential storage.
#include "core/ui.h"
#include "net/net.h"
#include "fs/tensilefs.h"

class NetManager : public App {
 public:
  enum { MAX_NETS = 14 };
  struct NetInfo {
    char ssid[33];
    int32_t rssi;
    bool open;
    uint8_t channel;
  };

  NetInfo nets[MAX_NETS];
  int netCount = 0;
  int sel = 0;
  int top = 0;
  bool scanning = false;
  uint32_t scanStartTime = 0;

  // Password entry
  bool typingPass = false;
  char pass[40] = "";
  int passLen = 0;
  char statusMsg[56] = "Press S to scan networks";

  NetManager() {
    prefW = 56; prefH = 18;
    fixed = false;
    tickMs = 150;
    // Load last saved password if available
    loadSaved();
    startScan();
  }
  const char* title() override { return "Network Manager"; }

  void loadSaved() {
    if (!LittleFS.exists("/CONFIG/WIFI.DAT")) return;
    File f = LittleFS.open("/CONFIG/WIFI.DAT", "r");
    if (!f) return;
    String s = f.readStringUntil('\n'); s.trim();
    String p = f.readStringUntil('\n'); p.trim();
    f.close();
    if (p.length() > 0) {
      strncpy(pass, p.c_str(), sizeof(pass) - 1);
      passLen = strlen(pass);
    }
  }

  void saveCredentials(const char* s, const char* p) {
    if (!LittleFS.exists("/CONFIG")) LittleFS.mkdir("/CONFIG");
    File f = LittleFS.open("/CONFIG/WIFI.DAT", "w");
    if (f) {
      f.println(s);
      f.println(p);
      f.close();
    }
  }

  void startScan() {
    if (scanning) return;
    scanning = true;
    scanStartTime = millis();
    snprintf(statusMsg, sizeof(statusMsg), "Scanning Wi-Fi channels...");
    WiFi.scanNetworks(true); // Asynchronous scan
  }

  void checkScan() {
    if (!scanning) return;
    int16_t n = WiFi.scanComplete();
    if (n >= 0) {
      scanning = false;
      netCount = n > MAX_NETS ? MAX_NETS : n;
      for (int i = 0; i < netCount; i++) {
        String s = WiFi.SSID(i);
        strncpy(nets[i].ssid, s.c_str(), 32);
        nets[i].ssid[32] = 0;
        nets[i].rssi = WiFi.RSSI(i);
        nets[i].open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
        nets[i].channel = WiFi.channel(i);
      }
      WiFi.scanDelete();
      // Sort networks by signal strength descending
      for (int i = 0; i < netCount - 1; i++) {
        for (int j = i + 1; j < netCount; j++) {
          if (nets[j].rssi > nets[i].rssi) {
            NetInfo tmp = nets[i]; nets[i] = nets[j]; nets[j] = tmp;
          }
        }
      }
      snprintf(statusMsg, sizeof(statusMsg), "Found %d networks. Select & enter pass", netCount);
      if (sel >= netCount) sel = netCount ? netCount - 1 : 0;
    } else if (millis() - scanStartTime > 10000) {
      scanning = false;
      WiFi.scanDelete();
      snprintf(statusMsg, sizeof(statusMsg), "Scan timed out. Press S to retry");
    }
  }

  void connectSelected() {
    if (netCount == 0 || sel >= netCount) return;
    typingPass = false;
    const char* targetSSID = nets[sel].ssid;
    saveCredentials(targetSSID, pass);
    snprintf(statusMsg, sizeof(statusMsg), "Connecting to '%s'...", targetSSID);
    WiFi.begin(targetSSID, pass);
  }

  void tick(uint32_t) override {
    checkScan();
    // Update live status if connected
    if (WiFi.status() == WL_CONNECTED && !scanning && !typingPass) {
      if (strncmp(statusMsg, "Connected", 9) != 0) {
        snprintf(statusMsg, sizeof(statusMsg), "Connected! IP: %s (%d dBm)",
                 WiFi.localIP().toString().c_str(), (int)WiFi.RSSI());
      }
    }
  }

  void draw(View& v) override {
    v.fill(0x70);

    // Header bar
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    bool conn = (WiFi.status() == WL_CONNECTED);
    char hbuf[64];
    snprintf(hbuf, sizeof(hbuf), "Network Manager  Status: %s", conn ? "CONNECTED" : "HOTSPOT (AP)");
    v.text(1, 0, hbuf, 0x1F);

    // Live IP and Gateway info
    char ipBuf[64];
    snprintf(ipBuf, sizeof(ipBuf), "IP: %-15s   AP Hotspot: 192.168.4.1", netIP().c_str());
    v.text(1, 1, ipBuf, 0x71);

    // Network list column headers
    v.fillRect(1, 3, v.w - 2, 1, ' ', 0x78);
    v.text(2, 3, "SSID", 0x70);
    v.text(32, 3, "SIGNAL", 0x70);
    v.text(42, 3, "AUTH", 0x70);

    // List visible networks (up to 6 rows)
    const int rows = 6;
    for (int r = 0; r < rows; r++) {
      int idx = top + r;
      if (idx >= netCount) {
        v.fillRect(1, 4 + r, v.w - 2, 1, ' ', 0x70);
        continue;
      }
      bool selected = (idx == sel);
      uint8_t a = selected ? 0x1F : 0x70;
      v.fillRect(1, 4 + r, v.w - 2, 1, ' ', a);

      // SSID
      char sbuf[30];
      snprintf(sbuf, sizeof(sbuf), "%-26s", nets[idx].ssid);
      v.text(2, 4 + r, sbuf, a);

      // Signal Bar: [####], [### ], etc.
      int bars = 1;
      int32_t rssi = nets[idx].rssi;
      if (rssi > -60) bars = 4;
      else if (rssi > -70) bars = 3;
      else if (rssi > -80) bars = 2;

      char sig[8];
      snprintf(sig, sizeof(sig), "[%c%c%c%c]",
               bars >= 1 ? G_FULL : ' ',
               bars >= 2 ? G_FULL : ' ',
               bars >= 3 ? G_FULL : ' ',
               bars >= 4 ? G_FULL : ' ');
      v.text(32, 4 + r, sig, selected ? 0x1A : 0x72);

      // Security
      v.text(42, 4 + r, nets[idx].open ? "Open" : "WPA2", a);
    }

    if (netCount == 0 && scanning) {
      v.text(2, 5, "Scanning for Wi-Fi networks...", 0x78);
    } else if (netCount == 0) {
      v.text(2, 5, "No networks found. Press S to scan.", 0x78);
    }

    // Password input box
    v.fillRect(1, 11, v.w - 2, 1, G_H1, 0x78);
    v.text(1, 12, "Selected SSID:", 0x70);
    v.text(16, 12, netCount ? nets[sel].ssid : "(none)", 0x1F);

    v.text(1, 13, "Password     :", 0x70);
    uint8_t pAttr = typingPass ? 0x1E : 0x7F;
    v.fillRect(16, 13, 26, 1, ' ', pAttr);
    // Show masked password
    char mask[40];
    for (int i = 0; i < passLen; i++) mask[i] = '*';
    mask[passLen] = 0;
    v.text(17, 13, mask, pAttr);
    if (typingPass) v.cursor(17 + passLen, 13);

    // Connect and Scan buttons
    v.fillRect(44, 13, 10, 1, ' ', 0x2F);
    v.text(45, 13, "Connect", 0x2F);

    // Status message bar
    v.fillRect(0, v.h - 2, v.w, 1, ' ', 0x78);
    v.text(1, v.h - 2, statusMsg, 0x70);

    // Help hotkeys
    v.fillRect(0, v.h - 1, v.w, 1, ' ', 0x30);
    v.text(1, v.h - 1, "S: Scan  P: Password  Enter: Connect  D: Disconnect", 0x30);
  }

  void key(uint8_t k) override {
    if (typingPass) {
      if (k == KEY_ENTER) {
        typingPass = false;
        connectSelected();
      } else if (k == KEY_ESC) {
        typingPass = false;
      } else if ((k == KEY_BKSP || k == 127) && passLen > 0) {
        pass[--passLen] = 0;
      } else if (k >= 32 && k < 127 && passLen < 32) {
        pass[passLen++] = (char)k;
        pass[passLen] = 0;
      }
      return;
    }

    if (k == KEY_UP && sel > 0) {
      sel--;
      if (sel < top) top = sel;
    } else if (k == KEY_DOWN && sel + 1 < netCount) {
      sel++;
      if (sel >= top + 6) top = sel - 5;
    } else if (k == 's' || k == 'S') {
      startScan();
    } else if (k == 'p' || k == 'P') {
      typingPass = true;
    } else if (k == KEY_ENTER) {
      connectSelected();
    } else if (k == 'd' || k == 'D') {
      WiFi.disconnect();
      snprintf(statusMsg, sizeof(statusMsg), "Disconnected from Wi-Fi");
    } else if (k == KEY_ESC) {
      closeReq = true;
    }
  }

  void mouse(int mx, int my, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;

    // Click on network list rows (y = 4..9)
    if (my >= 4 && my <= 9) {
      int clickedIdx = top + (my - 4);
      if (clickedIdx < netCount) {
        sel = clickedIdx;
        typingPass = true;
      }
      return;
    }

    // Click on password input field (y = 13, x = 16..41)
    if (my == 13 && mx >= 16 && mx <= 41) {
      typingPass = true;
      return;
    }

    // Click on [Connect] button (y = 13, x = 44..54)
    if (my == 13 && mx >= 44 && mx <= 54) {
      connectSelected();
      return;
    }
  }
};


#endif // TENSILE_NET_NETMGR_H
