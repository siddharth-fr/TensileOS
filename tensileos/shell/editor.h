#ifndef TENSILE_SHELL_EDITOR_H
#define TENSILE_SHELL_EDITOR_H

#pragma once
// Notepad / full-screen text editor. Ctrl+S save, Ctrl+Q quit, Ctrl+K delete line.
#include "core/ui.h"

class Editor : public App {
 public:
  enum { MAXL = 500, MAXC = 200, MAXTOTAL = 48000 };
  char path[PATH_LEN];
  std::vector<std::string> L;
  int cy = 0, cx = 0, top = 0, left = 0, lastTh = 10;
  bool mod = false, armed = false, naming = false, locked = false;
  char msg[48] = "";
  char nm[PATH_LEN]; int nlen = 0;
  char tbuf[64];

  Editor(const char* p, bool full) {
    prefW = full ? 0 : 56; prefH = full ? 0 : 20;
    snprintf(path, sizeof path, "%s", p ? p : "");
    nm[0] = 0;
    load();
    if (L.empty()) L.push_back("");
  }
  const char* title() override {
    snprintf(tbuf, sizeof tbuf, "Notepad - %s%s", path[0] ? baseName(path) : "Untitled", mod ? "*" : "");
    return tbuf;
  }

  const char* openPath() override { return path[0] ? path : NULL; }

  void load() {
    L.clear();
    std::string cur; size_t total = 0; bool trunc = false;
    if (path[0] && fsExists(path) && !fsIsDir(path)) {
      File f = LittleFS.open(path, "r");
      if (f) {
        uint8_t buf[128]; int n;
        while (!trunc && (n = f.read(buf, sizeof buf)) > 0) {
          for (int i = 0; i < n && !trunc; i++) {
            uint8_t c = buf[i];
            if (c == '\r') continue;
            if (c == '\n') { L.push_back(cur); cur.clear(); if (L.size() >= MAXL) trunc = true; continue; }
            if (c == '\t') cur.append("    ");
            else cur.push_back(c < 32 ? '?' : (char)c);
            if (cur.size() >= MAXC) { L.push_back(cur); cur.clear(); if (L.size() >= MAXL) trunc = true; }
            if (++total > MAXTOTAL) trunc = true;
          }
        }
        f.close();
      }
    } else if (path[0]) {
      snprintf(msg, sizeof msg, "New file");
    }
    if (trunc) { locked = true; snprintf(msg, sizeof msg, "File too big: view only"); }
    else L.push_back(cur);
    if (L.empty()) L.push_back("");
  }

  void save() {
    if (locked) { snprintf(msg, sizeof msg, "View only (file too big)"); return; }
    if (!path[0]) { naming = true; nlen = 0; nm[0] = 0; return; }
    File f = LittleFS.open(path, "w");
    if (!f) { snprintf(msg, sizeof msg, "Save failed!"); return; }
    size_t tot = 0;
    for (size_t i = 0; i < L.size(); i++) {
      f.write((const uint8_t*)L[i].data(), L[i].size()); tot += L[i].size();
      if (i + 1 < L.size()) { f.write((const uint8_t*)"\n", 1); tot++; }
    }
    f.close();
    mod = false;
    snprintf(msg, sizeof msg, "Saved %u bytes", (unsigned)tot);
  }

  void clamp() {
    if (cy < 0) cy = 0;
    if (cy >= (int)L.size()) cy = L.size() - 1;
    if (cx < 0) cx = 0;
    if (cx > (int)L[cy].size()) cx = L[cy].size();
  }

  void draw(View& v) override {
    if (v.pixelMode) {
      // Authentic TensileOS Notepad: Menu bar, white canvas, crisp black text
      v.fillRect(0, 0, v.w, 1, ' ', 0x70);
      v.text(1, 0, "File  Edit  Search  Help", 0x70);

      int rOffset = 1;
      int tw = v.w - 2, th = v.h - 2;
      lastTh = th;
      if (tw < 4 || th < 1) return;
      if (cy < top) top = cy;
      if (cy >= top + th) top = cy - th + 1;
      if (cx < left) left = cx;
      if (cx >= left + tw) left = cx - tw + 1;

      // Fill paper with white
      v.fillRect(0, rOffset, v.w, th, ' ', 0xF0);

      for (int r = 0; r < th; r++) {
        int ln = top + r;
        if (ln < (int)L.size()) {
          const std::string& s = L[ln];
          for (int j = 0; j < tw; j++) {
            int idx = left + j;
            if (idx < (int)s.size()) v.put(1 + j, rOffset + r, (uint8_t)s[idx], 0xF0);
          }
        }
      }

      // Bottom status bar
      v.fillRect(0, v.h - 1, v.w, 1, ' ', 0x70);
      char st[96];
      if (naming) snprintf(st, sizeof st, " Save as: %s_", nm);
      else if (msg[0]) snprintf(st, sizeof st, " %s", msg);
      else snprintf(st, sizeof st, " Ln %d, Col %d%s", cy + 1, cx + 1, mod ? " [modified]" : "");
      v.text(1, v.h - 1, st, 0x70);
      if (!naming) v.textr(v.w - 1, v.h - 1, "^S Save  ^Q Close", 0x78);

      if (naming) v.cursor(10 + nlen, v.h - 1);
      else v.cursor(1 + cx - left, rOffset + cy - top);
      return;
    }

    const int gw = 4;
    int tw = v.w - gw, th = v.h - 1;
    lastTh = th;
    if (tw < 4 || th < 2) return;
    if (cy < top) top = cy;
    if (cy >= top + th) top = cy - th + 1;
    if (cx < left) left = cx;
    if (cx >= left + tw) left = cx - tw + 1;
    for (int r = 0; r < th; r++) {
      int ln = top + r;
      v.fillRect(0, r, gw, 1, ' ', 0x13);
      v.fillRect(gw, r, tw, 1, ' ', 0x1F);
      if (ln < (int)L.size()) {
        char g[8]; snprintf(g, sizeof g, "%3d", ln + 1);
        v.text(0, r, g, 0x13);
        const std::string& s = L[ln];
        for (int j = 0; j < tw; j++) {
          int idx = left + j;
          if (idx < (int)s.size()) v.put(gw + j, r, (uint8_t)s[idx], 0x1F);
        }
      }
    }
    v.fillRect(0, th, v.w, 1, ' ', 0x70);
    char st[96];
    if (naming) snprintf(st, sizeof st, " Save as: %s_", nm);
    else if (msg[0]) snprintf(st, sizeof st, " %s", msg);
    else snprintf(st, sizeof st, " Ln %d/%d  Col %d%s", cy + 1, (int)L.size(), cx + 1, mod ? "  [modified]" : "");
    v.text(0, th, st, 0x70);
    if (!naming) v.textr(v.w - 1, th, v.w >= 56 ? "^S Save  ^Q Quit  ^K DelLine" : "^S Save ^Q Quit", 0x78);
    if (naming) v.cursor(10 + nlen, th); else v.cursor(gw + cx - left, cy - top);
  }

  void insertChar(char c) {
    if (L[cy].size() >= MAXC) return;
    L[cy].insert(cx, 1, c); cx++; mod = true;
  }

  void key(uint8_t k) override {
    if (naming) {
      if (k == KEY_ENTER) {
        char full[PATH_LEN];
        if (nlen && fsResolve(nm, full, sizeof full) && !fsIsDir(full)) { snprintf(path, sizeof path, "%s", full); naming = false; save(); }
        else snprintf(msg, sizeof msg, "Bad file name");
      } else if (k == KEY_ESC) naming = false;
      else if (k == KEY_BKSP && nlen) nm[--nlen] = 0;
      else if (k >= 32 && k < 127 && nlen < 40) { nm[nlen++] = (char)k; nm[nlen] = 0; }
      return;
    }
    if (k != KEY_CTRL_Q) armed = false;
    msg[0] = 0;
    switch (k) {
      case KEY_CTRL_S: save(); break;
      case KEY_CTRL_Q:
        if (mod && !armed) { armed = true; snprintf(msg, sizeof msg, "Unsaved! Press ^Q again to discard"); }
        else closeReq = true;
        break;
      case KEY_CTRL_K:
        if (L.size() > 1) { L.erase(L.begin() + cy); mod = true; } else { L[0].clear(); mod = true; }
        clamp(); break;
      case KEY_UP:    cy--; clamp(); break;
      case KEY_DOWN:  cy++; clamp(); break;
      case KEY_LEFT:  if (cx > 0) cx--; else if (cy > 0) { cy--; cx = L[cy].size(); } break;
      case KEY_RIGHT: if (cx < (int)L[cy].size()) cx++; else if (cy + 1 < (int)L.size()) { cy++; cx = 0; } break;
      case KEY_HOME:  cx = 0; break;
      case KEY_END:   cx = L[cy].size(); break;
      case KEY_PGUP:  cy -= 10; clamp(); break;
      case KEY_PGDN:  cy += 10; clamp(); break;
      case KEY_ENTER:
        if (L.size() < MAXL) {
          std::string rest = L[cy].substr(cx);
          L[cy].erase(cx);
          L.insert(L.begin() + cy + 1, rest);
          cy++; cx = 0; mod = true;
        }
        break;
      case KEY_BKSP:
        if (cx > 0) { L[cy].erase(cx - 1, 1); cx--; mod = true; }
        else if (cy > 0 && L[cy - 1].size() + L[cy].size() <= MAXC) {
          cx = L[cy - 1].size(); L[cy - 1] += L[cy]; L.erase(L.begin() + cy); cy--; mod = true;
        }
        break;
      case KEY_DEL:
        if (cx < (int)L[cy].size()) { L[cy].erase(cx, 1); mod = true; }
        else if (cy + 1 < (int)L.size() && L[cy].size() + L[cy + 1].size() <= MAXC) { L[cy] += L[cy + 1]; L.erase(L.begin() + cy + 1); mod = true; }
        break;
      case KEY_TAB: insertChar(' '); insertChar(' '); break;
      default: if (k >= 32 && k < 127) insertChar((char)k);
    }
  }

  void mouse(int mx, int my, uint8_t b, uint8_t prev) override {
    if (!(b & ~prev & 1) || naming) return;
    if (my < 0 || my >= lastTh) return;
    cy = top + my; cx = left + mx - 4;
    clamp();
  }
};


#endif // TENSILE_SHELL_EDITOR_H
