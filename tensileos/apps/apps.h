#ifndef TENSILE_APPS_APPS_H
#define TENSILE_APPS_APPS_H

#pragma once
// Desktop utilities: File Manager, Calculator, System Monitor.
#include "shell/editor.h"

void desktopLaunch(const char* name, const char* arg);   // defined in desktop_wm.h
extern int g_rtcHour, g_rtcMin, g_rtcSec;
void rtcSet(int h, int m, int s = 0);

// ================= File Manager =================
class FileManager : public App {
 public:
  enum { MAXE = 64 };
  char dir[PATH_LEN];
  FsEntry* E; int n = 0, sel = 0, top = 0, rows = 8;
  int mode = 0;               // 0 browse, 1 typing a name, 2 confirm delete
  int act = 0;                // 1 new file, 2 new folder, 3 rename
  char in[32]; int inlen = 0;
  char msg[48] = "";
  uint32_t lastClick = 0; int lastRow = -1;

  FileManager(const char* start) {
    prefW = 52; prefH = 18;
    E = (FsEntry*)malloc(sizeof(FsEntry) * MAXE);
    snprintf(dir, sizeof dir, "%s", start);
    in[0] = 0;
    if (E) refresh();
    else snprintf(msg, sizeof msg, "Out of memory");
  }
  ~FileManager() { if (E) free(E); }
  const char* title() override { return "File Manager"; }

  void refresh() {
    if (!E) { n = 0; return; }
    int off = strcmp(dir, "/") ? 1 : 0;
    if (off) { strcpy(E[0].name, ".."); E[0].dir = true; E[0].size = 0; }
    int c = fsList(dir, E + off, MAXE - off);
    if (c < 0) { strcpy(dir, "/"); off = 0; c = fsList(dir, E, MAXE); if (c < 0) c = 0; }
    n = off + c;
    if (sel >= n) sel = n - 1;
    if (sel < 0) sel = 0;
  }
  void up() {
    if (!E || !strcmp(dir, "/")) return;
    char child[32]; snprintf(child, sizeof child, "%s", baseName(dir));
    char* s = strrchr(dir, '/');
    if (s == dir) dir[1] = 0; else *s = 0;
    refresh(); top = 0; sel = 0;
    for (int i = 0; i < n; i++) if (!strcmp(E[i].name, child)) sel = i;
  }
  void activate() {
    if (!E || !n) return;
    FsEntry& e = E[sel];
    if (!strcmp(e.name, "..")) { up(); return; }
    char p[PATH_LEN]; fsJoin(dir, e.name, p, sizeof p);
    if (e.dir) { snprintf(dir, sizeof dir, "%s", p); sel = 0; top = 0; refresh(); }
    else {
      const char* ext = strrchr(e.name, '.');
      if (ext && !strcasecmp(ext, ".ans")) desktopLaunch("paint", p);
      else if (ext && (!strcasecmp(ext, ".htm") || !strcasecmp(ext, ".html"))) desktopLaunch("browser", p);
      else desktopLaunch("notepad", p);
    }
  }

  void draw(View& v) override {
    if (!E) {
      v.fill(0x70);
      v.text(1, 1, "Error: out of memory", 0x74);
      return;
    }
    rows = v.h - 3; if (rows < 1) rows = 1;
    if (sel < top) top = sel;
    if (sel >= top + rows) top = sel - rows + 1;
    v.fill(0x70);
    char d[PATH_LEN + 4]; fsDisp(dir, d, sizeof d);
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F); v.text(1, 0, d, 0x1F);
    for (int r = 0; r < rows; r++) {
      int i = top + r; if (i >= n) break;
      bool s = (i == sel);
      uint8_t a = s ? 0x1F : (E[i].dir ? 0x71 : 0x70);
      v.fillRect(0, 1 + r, v.w, 1, ' ', a);
      char nm[40]; snprintf(nm, sizeof nm, E[i].dir ? "[%s]" : " %s", E[i].name);
      v.text(0, 1 + r, nm, a);
      char sz[16];
      if (E[i].dir) snprintf(sz, sizeof sz, "<DIR>"); else snprintf(sz, sizeof sz, "%u", (unsigned)E[i].size);
      v.textr(v.w - 1, 1 + r, sz, a);
    }
    if (n == 0) v.text(1, 1, "(empty folder)", 0x78);
    v.fillRect(0, v.h - 2, v.w, 1, ' ', 0x30);
    v.text(0, v.h - 2, v.w >= 50 ? " Enter open  N file  F folder  R rename  D delete  Bksp up" : " Enter  N  F  R  D  Bksp", 0x30);
    char st[64];
    if (mode == 1) snprintf(st, sizeof st, " %s: %s_", act == 1 ? "New file" : act == 2 ? "New folder" : "Rename to", in);
    else if (mode == 2) snprintf(st, sizeof st, " Delete %s ? (y/n)", n ? E[sel].name : "");
    else snprintf(st, sizeof st, " %s", msg[0] ? msg : "");
    v.text(0, v.h - 1, st, mode ? 0x4F : 0x70);
    if (mode == 1) v.cursor(1 + (int)strlen(st) - 2, v.h - 1);
  }

  void key(uint8_t k) override {
    if (!E) { if (k == KEY_ESC) closeReq = true; return; }
    if (mode == 1) {
      if (k == KEY_ESC) mode = 0;
      else if (k == KEY_ENTER) { doInput(); mode = 0; }
      else if (k == KEY_BKSP && inlen) in[--inlen] = 0;
      else if (k >= 32 && k < 127 && k != '/' && k != '\\' && inlen < 28) { in[inlen++] = (char)k; in[inlen] = 0; }
      return;
    }
    if (mode == 2) {
      if (k == 'y' || k == 'Y') {
        char p[PATH_LEN]; fsJoin(dir, E[sel].name, p, sizeof p);
        snprintf(msg, sizeof msg, fsRemoveAll(p) ? "Deleted" : "Delete failed");
        refresh();
      }
      mode = 0; return;
    }
    msg[0] = 0;
    switch (k) {
      case KEY_UP: if (sel > 0) sel--; break;
      case KEY_DOWN: if (sel + 1 < n) sel++; break;
      case KEY_PGUP: sel = imax(0, sel - rows); break;
      case KEY_PGDN: sel = imin(n - 1, sel + rows); break;
      case KEY_HOME: sel = 0; break;
      case KEY_END: sel = imax(0, n - 1); break;
      case KEY_ENTER: case KEY_RIGHT: activate(); break;
      case KEY_BKSP: case KEY_LEFT: up(); break;
      case 'n': case 'N': mode = 1; act = 1; in[0] = 0; inlen = 0; break;
      case 'f': case 'F': mode = 1; act = 2; in[0] = 0; inlen = 0; break;
      case 'r': case 'R':
        if (n && strcmp(E[sel].name, "..")) { mode = 1; act = 3; snprintf(in, sizeof in, "%s", E[sel].name); inlen = strlen(in); }
        break;
      case 'd': case 'D': case KEY_DEL:
        if (n && strcmp(E[sel].name, "..")) mode = 2;
        break;
    }
  }
  void doInput() {
    if (!inlen) return;
    char p[PATH_LEN]; fsJoin(dir, in, p, sizeof p);
    if (act == 3) {
      if (fsExists(p)) { snprintf(msg, sizeof msg, "Name already exists"); return; }
      char old[PATH_LEN]; fsJoin(dir, E[sel].name, old, sizeof old);
      snprintf(msg, sizeof msg, LittleFS.rename(old, p) ? "Renamed" : "Rename failed");
    } else if (fsExists(p)) {
      snprintf(msg, sizeof msg, "Already exists");
    } else if (act == 2) {
      snprintf(msg, sizeof msg, LittleFS.mkdir(p) ? "Folder created" : "Could not create folder");
    } else if (act == 1) {
      if (fsWriteText(p, "")) { refresh(); desktopLaunch("notepad", p); }
      else snprintf(msg, sizeof msg, "Could not create file");
    }
    refresh();
    for (int i = 0; i < n; i++) if (!strcmp(E[i].name, in)) sel = i;
  }
  void mouse(int mx, int my, uint8_t b, uint8_t prev) override {
    if (!E || !(b & ~prev & 1) || mode) return;
    if (my >= 1 && my <= rows) {
      int i = top + my - 1;
      if (i < n) {
        uint32_t now = millis();
        if (i == sel && lastRow == i && now - lastClick < 600) { activate(); lastRow = -1; return; }
        sel = i; lastRow = i; lastClick = now;
      }
    }
  }
};

// ================= Calculator =================
struct CalcParser {
  const char* s; bool err; double ans;
  void ws() { while (*s == ' ') s++; }
  double expr() {
    double v = term();
    for (;;) { ws(); if (*s == '+') { s++; v += term(); } else if (*s == '-') { s++; v -= term(); } else break; }
    return v;
  }
  double term() {
    double v = unary();
    for (;;) {
      ws();
      if (*s == '*') { s++; v *= unary(); }
      else if (*s == '/' || *s == '%') {
        char op = *s++; double d = unary();
        if (d == 0) { err = true; return 0; }
        v = (op == '/') ? v / d : fmod(v, d);
      } else break;
    }
    return v;
  }
  double unary() {
    ws();
    if (*s == '-') { s++; return -unary(); }
    if (*s == '+') { s++; return unary(); }
    return power();
  }
  double power() {
    double b = primary(); ws();
    if (*s == '^') { s++; return pow(b, unary()); }
    return b;
  }
  double primary() {
    ws();
    if (*s == '(') { s++; double v = expr(); ws(); if (*s == ')') s++; else err = true; return v; }
    if ((*s >= '0' && *s <= '9') || *s == '.') { char* e; double v = strtod(s, &e); s = e; return v; }
    if (isalpha((unsigned char)*s)) {
      char id[8]; int n = 0;
      while (isalpha((unsigned char)*s) && n < 7) id[n++] = *s++;
      id[n] = 0;
      if (!strcmp(id, "pi")) return 3.14159265358979323846;
      if (!strcmp(id, "e")) return 2.71828182845904523536;
      if (!strcmp(id, "ans")) return ans;
      ws();
      if (*s != '(') { err = true; return 0; }
      s++; double a = expr(); ws();
      if (*s == ')') s++; else err = true;
      if (!strcmp(id, "sqrt")) { if (a < 0) { err = true; return 0; } return sqrt(a); }
      if (!strcmp(id, "sin")) return sin(a);
      if (!strcmp(id, "cos")) return cos(a);
      if (!strcmp(id, "tan")) return tan(a);
      if (!strcmp(id, "ln")) { if (a <= 0) { err = true; return 0; } return log(a); }
      if (!strcmp(id, "log")) { if (a <= 0) { err = true; return 0; } return log10(a); }
      if (!strcmp(id, "abs")) return fabs(a);
    }
    err = true; return 0;
  }
};

class Calc : public App {
 public:
  char buf[48]; int len = 0; double ans = 0;
  char hist[6][56]; int nh = 0;
  Calc() { prefW = 44; prefH = 14; buf[0] = 0; }
  const char* title() override { return "Calculator"; }
  void draw(View& v) override {
    v.fill(0x70);
    for (int i = 0; i < nh; i++) { int r = v.h - 4 - (nh - 1 - i); if (r >= 0) v.textr(v.w - 1, r, hist[i], i == nh - 1 ? 0x70 : 0x78); }
    v.fillRect(0, v.h - 3, v.w, 1, ' ', 0x1F);
    char l[64]; snprintf(l, sizeof l, " > %s", buf);
    v.text(0, v.h - 3, l, 0x1F);
    v.text(1, v.h - 2, "+ - * / % ^ ( )  sqrt sin cos ln pi ans", 0x78);
    v.text(1, v.h - 1, "Enter = calculate   Esc = clear", 0x78);
    v.cursor(3 + len, v.h - 3);
  }
  void key(uint8_t k) override {
    if (k == KEY_ENTER && len) {
      CalcParser p = {buf, false, ans};
      double r = p.expr(); p.ws();
      if (*p.s) p.err = true;
      if (nh == 6) { memmove(hist[0], hist[1], sizeof(hist[0]) * 5); nh--; }
      if (p.err || r != r) snprintf(hist[nh++], sizeof hist[0], "%s = ERROR", buf);
      else { ans = r; snprintf(hist[nh++], sizeof hist[0], "%s = %.10g", buf, r); }
      len = 0; buf[0] = 0;
    } else if (k == KEY_ESC) { len = 0; buf[0] = 0; }
    else if (k == KEY_BKSP && len) buf[--len] = 0;
    else if (k >= 32 && k < 127 && len < 40) { buf[len++] = (char)k; buf[len] = 0; }
  }
};

// ================= System Monitor =================
class Sysmon : public App {
 public:
  Sysmon() { prefW = 46; prefH = 16; tickMs = 500; }
  const char* title() override { return "System Monitor"; }
  void bar(View& v, int x, int y, int w, uint32_t used, uint32_t total) {
    int fill = total ? (int)((uint64_t)used * w / total) : 0;
    for (int i = 0; i < w; i++) v.put(x + i, y, i < fill ? G_FULL : G_LIGHT, 0x72);
  }
  void draw(View& v) override {
    v.fill(0x70);
    char b[64];
    uint32_t tot = ESP.getHeapSize(), fr = ESP.getFreeHeap();
    v.text(1, 1, "Memory (heap)", 0x71);
    snprintf(b, sizeof b, "%uK free of %uK  (low: %uK)", (unsigned)(fr / 1024), (unsigned)(tot / 1024), (unsigned)(ESP.getMinFreeHeap() / 1024));
    v.text(1, 2, b, 0x70);
    bar(v, 1, 3, v.w - 2, tot - fr, tot);
    v.text(1, 5, "Flash storage", 0x71);
    uint32_t ft = LittleFS.totalBytes(), fu = LittleFS.usedBytes();
    snprintf(b, sizeof b, "%uK used of %uK", (unsigned)(fu / 1024), (unsigned)(ft / 1024));
    v.text(1, 6, b, 0x70);
    bar(v, 1, 7, v.w - 2, fu, ft);
    uint32_t s = millis() / 1000;
    snprintf(b, sizeof b, "Uptime  %02u:%02u:%02u   CPU %u MHz x%u", (unsigned)(s / 3600), (unsigned)((s / 60) % 60), (unsigned)(s % 60),
             (unsigned)ESP.getCpuFreqMHz(), (unsigned)ESP.getChipCores());
    v.text(1, 9, b, 0x70);
    snprintf(b, sizeof b, "Tasks   %u    Screen %u bytes", (unsigned)uxTaskGetNumberOfTasks(), (unsigned)(sizeof(Cell) * SCR_CELLS));
    v.text(1, 10, b, 0x70);
    snprintf(b, sizeof b, "Wi-Fi   %s  %s  viewers: %d", g_apMode ? "hotspot" : "station", netIP().c_str(), wsClientCount());
    v.text(1, 11, b, 0x70);
    snprintf(b, sizeof b, "Chip    %s rev %u", ESP.getChipModel(), (unsigned)ESP.getChipRevision());
    v.text(1, 12, b, 0x70);
  }
};

// ================= My Computer =================
class MyComputerApp : public App {
 public:
  MyComputerApp() { prefW = 46; prefH = 16; fixed = false; }
  const char* title() override { return "My Computer"; }

  void bar(View& v, int x, int y, int w, uint32_t used, uint32_t total) {
    int fill = total ? (int)((uint64_t)used * w / total) : 0;
    for (int i = 0; i < w; i++) v.put(x + i, y, i < fill ? G_FULL : G_LIGHT, 0x72);
  }

  void draw(View& v) override {
    v.fill(0x70);
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    v.text(1, 0, "System Information & Drives", 0x1F);

    char b[64];
    v.text(1, 2, "Computer Hardware:", 0x71);
    snprintf(b, sizeof b, "CPU: ESP32 Dual-Core @ %u MHz", (unsigned)ESP.getCpuFreqMHz());
    v.text(2, 3, b, 0x70);
    snprintf(b, sizeof b, "RAM: %u KB Heap (%u KB free)", (unsigned)(ESP.getHeapSize()/1024), (unsigned)(ESP.getFreeHeap()/1024));
    v.text(2, 4, b, 0x70);

    v.text(1, 6, "Storage Devices:", 0x71);
    v.text(2, 7, "[C:] Local Disk (LittleFS Flash)", 0x70);
    uint32_t ft = LittleFS.totalBytes(), fu = LittleFS.usedBytes();
    bar(v, 2, 8, v.w - 4, fu, ft);
    snprintf(b, sizeof b, "%u KB used of %u KB total (%u KB free)",
             (unsigned)(fu / 1024), (unsigned)(ft / 1024), (unsigned)((ft - fu) / 1024));
    v.text(2, 9, b, 0x78);

    v.text(2, 11, "[A:] 3.5\" Floppy Disk (1.44 MB Virtual)", 0x78);

    v.fillRect(2, 13, 22, 1, ' ', 0x30);
    v.text(3, 13, "[ Open Drive C: ]", 0x3F);
    v.text(26, 13, "(or press Enter)", 0x78);
  }

  void key(uint8_t k) override {
    if (k == KEY_ENTER || k == 'c' || k == 'C') desktopLaunch("files", "/");
    else if (k == KEY_ESC) closeReq = true;
  }

  void mouse(int cx, int cy, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (edge & 1) {
      if (cy >= 7 && cy <= 14) desktopLaunch("files", "/");
    }
  }
};

// ================= Recycle Bin =================
class RecycleBinApp : public App {
 public:
  enum { MAX_TRASH = 32 };
  FsEntry entries[MAX_TRASH];
  int count = 0;
  uint32_t totalBytes = 0;
  char msg[48] = "";

  RecycleBinApp() {
    prefW = 46; prefH = 15; fixed = false;
    refresh();
  }
  const char* title() override { return "Recycle Bin"; }

  void refresh() {
    count = 0;
    totalBytes = 0;
    if (LittleFS.exists("/TRASH")) {
      count = fsList("/TRASH", entries, MAX_TRASH);
      if (count < 0) count = 0;
      for (int i = 0; i < count; i++) totalBytes += entries[i].size;
    }
  }

  void emptyTrash() {
    if (!LittleFS.exists("/TRASH")) {
      snprintf(msg, sizeof(msg), "Recycle Bin is empty.");
      return;
    }
    File dir = LittleFS.open("/TRASH");
    if (dir && dir.isDirectory()) {
      File file = dir.openNextFile();
      while (file) {
        char p[PATH_LEN];
        fsJoin("/TRASH", file.name(), p, sizeof(p));
        file.close();
        LittleFS.remove(p);
        file = dir.openNextFile();
      }
      dir.close();
    }
    refresh();
    snprintf(msg, sizeof(msg), "Recycle Bin emptied!");
  }

  void draw(View& v) override {
    v.fill(0x70);
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    v.text(1, 0, "Recycle Bin Contents", 0x1F);

    char buf[64];
    snprintf(buf, sizeof(buf), "Status: %d items (%u bytes in /TRASH)", count, (unsigned)totalBytes);
    v.text(1, 2, buf, count > 0 ? 0x74 : 0x78);

    int rows = v.h - 7;
    if (count == 0) {
      v.text(2, 4, "(The Recycle Bin is empty)", 0x78);
      v.text(2, 5, "Deleted files will appear here.", 0x78);
    } else {
      for (int i = 0; i < count && i < rows; i++) {
        snprintf(buf, sizeof(buf), " - %-18s %6u B", entries[i].name, (unsigned)entries[i].size);
        v.text(2, 4 + i, buf, 0x70);
      }
    }

    v.fillRect(2, v.h - 3, 23, 1, ' ', 0x30);
    v.text(3, v.h - 3, "[ Empty Recycle Bin ]", 0x3F);

    if (msg[0]) v.text(2, v.h - 2, msg, 0x72);
    else v.text(2, v.h - 2, "Press E or click to empty trash", 0x78);
  }

  void key(uint8_t k) override {
    if (k == 'e' || k == 'E') emptyTrash();
    else if (k == KEY_ESC) closeReq = true;
  }

  void mouse(int cx, int cy, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (edge & 1) {
      if (cy >= 11 && cy <= 13 && cx >= 2 && cx <= 26) emptyTrash();
    }
  }
};

// ================= Date/Time Properties =================
class TimeSetApp : public App {
 public:
  int h = 12, m = 0;
  char msg[32] = "";

  TimeSetApp() {
    prefW = 40; prefH = 14; fixed = false;
    h = g_rtcHour; m = g_rtcMin;
  }
  const char* title() override { return "Date / Time Properties"; }

  void draw(View& v) override {
    v.fill(0x70);
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    v.text(1, 0, "Set System Real-Time Clock", 0x1F);

    char buf[48];
    v.text(3, 2, "Current Clock:", 0x70);
    snprintf(buf, sizeof(buf), "[  %02d : %02d : %02d  ]", g_rtcHour, g_rtcMin, g_rtcSec);
    v.text(6, 4, buf, 0x1F);

    v.text(3, 6, "Adjust Time:", 0x70);
    snprintf(buf, sizeof(buf), "Hour: %02d    [+H]  [-H]", h);
    v.text(4, 7, buf, 0x70);
    snprintf(buf, sizeof(buf), "Min:  %02d    [+M]  [-M]", m);
    v.text(4, 9, buf, 0x70);

    // Buttons
    v.fillRect(4, 11, 10, 1, ' ', 0x30);
    v.text(5, 11, "[ Apply ]", 0x3F);
    v.fillRect(18, 11, 10, 1, ' ', 0x78);
    v.text(19, 11, "[ Close ]", 0x70);

    if (msg[0]) v.text(4, 12, msg, 0x72);
  }

  void key(uint8_t k) override {
    if (k == KEY_UP) { m = (m + 1) % 60; }
    else if (k == KEY_DOWN) { m = (m + 59) % 60; }
    else if (k == KEY_RIGHT) { h = (h + 1) % 24; }
    else if (k == KEY_LEFT) { h = (h + 23) % 24; }
    else if (k == KEY_ENTER) { apply(); }
    else if (k == KEY_ESC) { closeReq = true; }
  }

  void apply() {
    rtcSet(h, m, 0);
    snprintf(msg, sizeof(msg), "Time updated!");
  }

  void mouse(int cx, int cy, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;
    if (cy == 7) {
      if (cx >= 14 && cx <= 18) h = (h + 1) % 24;
      if (cx >= 20 && cx <= 24) h = (h + 23) % 24;
    }
    if (cy == 9) {
      if (cx >= 14 && cx <= 18) m = (m + 1) % 60;
      if (cx >= 20 && cx <= 24) m = (m + 59) % 60;
    }
    if (cy == 11 && cx >= 4 && cx <= 14) apply();
    if (cy == 11 && cx >= 18 && cx <= 28) closeReq = true;
  }
};

#include "apps/hwapps.h"
#include "apps/paint.h"
#include "net/netmgr.h"
#include "net/web_browser.h"


#endif // TENSILE_APPS_APPS_H
