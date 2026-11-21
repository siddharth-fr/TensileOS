#ifndef TENSILE_DISPLAY_DESKTOP_H
#define TENSILE_DISPLAY_DESKTOP_H

#pragma once
// The desktop: wallpaper, icons, overlapping windows (drag, raise, maximise, close), Start menu, taskbar.
#include "shell/shell.h"

#define MAX_WIN 5

struct AppDef {
  const char* id; const char* label; const char* menu; uint8_t iattr; const char* glyph;
  App* (*make)(const char* arg);
};
static App* mkFiles(const char* a)  { return new FileManager(a && a[0] ? a : "/"); }
static App* mkNotepad(const char* a){ return new Editor(a ? a : "", false); }
static App* mkCalc(const char*)     { return new Calc(); }
static App* mkSysmon(const char*)   { return new Sysmon(); }
static App* mkSnake(const char*)    { return new Snake(); }
static App* mkMines(const char*)    { return new Mines(); }
static App* mkTetris(const char*)   { return new Tetris(); }
static App* mkSysBench(const char*) { return new SysBench(); }
static App* mkHwLab(const char*)    { return new HwLab(); }
static App* mkPaint(const char* a)   { return new AnsiPaint(a); }
static App* mkNetMgr(const char*)   { return new NetManager(); }
static App* mkWebBrowser(const char* a) { return new RetroWeb(a); }

static const AppDef g_apps[] = {
  {"terminal", "Terminal", "Terminal",       0x0A, ">_",    NULL},
  {"files",    "Files",    "File Manager",   0x6E, "[=]",   mkFiles},
  {"notepad",  "Notepad",  "Notepad",        0x7F, "ABC",   mkNotepad},
  {"calc",     "Calc",     "Calculator",     0x2F, "1+2",   mkCalc},
  {"sysmon",   "Monitor",  "System Monitor", 0x30, "~^~",   mkSysmon},
  {"snake",    "Snake",    "Snake",          0x2E, "s=O",   mkSnake},
  {"mines",    "Mines",    "Minesweeper",    0x78, "*1*",   mkMines},
  {"tetris",   "Tetris",   "Tetris",         0x5E, "\xDB\xDB\xDC", mkTetris},
  {"bench",    "Bench",    "SysBench",       0x4E, "[#]",   mkSysBench},
  {"hwlab",    "HW Lab",   "Hardware Lab",   0x3F, "[*]",   mkHwLab},
  {"paint",    "Paint",    "ANSI Paint",     0x5F, "[&]",   mkPaint},
  {"netmgr",   "Wi-Fi",    "Net Manager",    0x1E, "((o))", mkNetMgr},
  {"browser",  "Web",      "RetroWeb",       0x1B, "[@]",   mkWebBrowser},
};
#define NAPPS ((int)(sizeof(g_apps) / sizeof(g_apps[0])))

struct Win { App* app; int x, y, w, h, sx, sy, sw, sh; bool maxi; uint32_t lastTick; };
static Win g_win[MAX_WIN];
static int g_nwin = 0;                         // g_win[g_nwin-1] is on top and has focus
static int g_icon = 0;                         // selected desktop icon
static bool g_startOpen = false;
static int g_startSel = 0;
static bool g_dirty = true, g_dirtyTop = false, g_leave = false, g_dragging = false, g_deskFirst = true;
static int g_dragDX = 0, g_dragDY = 0, g_capture = -1;
static uint8_t g_prevBtn = 0;
static uint32_t g_iconClickT = 0; static int g_iconClickI = -1;

#define ICON_W 8
static void iconPos(int i, int& x, int& y) { x = 2 + (i / 4) * 11; y = 2 + (i % 4) * 5; }
static const int TASKBAR_Y = SCR_ROWS - 1;
static const int MENU_ITEMS = NAPPS + 1;       // apps + Reboot
static const int MENU_W = 20;

static void focusWin(int i) {                  // raise window i to the top
  if (i < 0 || i >= g_nwin || i == g_nwin - 1) return;
  Win w = g_win[i];
  for (int j = i; j < g_nwin - 1; j++) g_win[j] = g_win[j + 1];
  g_win[g_nwin - 1] = w;
  g_dirty = true;
}
static void closeWin(int i) {
  if (i < 0 || i >= g_nwin) return;
  soundClick();
  delete g_win[i].app;
  for (int j = i; j < g_nwin - 1; j++) g_win[j] = g_win[j + 1];
  g_nwin--; g_capture = -1; g_dragging = false; g_dirty = true;
}
static void toggleMax(int i) {
  Win& w = g_win[i];
  if (w.app->fixed) return;
  if (!w.maxi) { w.sx = w.x; w.sy = w.y; w.sw = w.w; w.sh = w.h; w.x = 0; w.y = 1; w.w = SCR_COLS; w.h = SCR_ROWS - 2; w.maxi = true; }
  else { w.x = w.sx; w.y = w.sy; w.w = w.sw; w.h = w.sh; w.maxi = false; }
  g_dirty = true;
}
static void legacyDesktopLaunch(const char* id, const char* arg) {
  for (int i = 0; i < NAPPS; i++) {
    if (strcmp(id, g_apps[i].id)) continue;
    if (!g_apps[i].make) { g_leave = true; return; }              // Terminal: leave the desktop
    if (!strcmp(id, "notepad") && arg && arg[0])                   // already open? just bring it forward
      for (int j = 0; j < g_nwin; j++) {
        const char* op = g_win[j].app->openPath();
        if (op && !strcmp(op, arg)) { focusWin(j); g_startOpen = false; g_dirty = true; return; }
      }
    if (g_nwin >= MAX_WIN) return;
    soundClick();
    App* a = g_apps[i].make(arg);
    Win w; w.app = a; w.maxi = false; w.lastTick = millis();
    w.w = a->prefW; w.h = a->prefH;
    if (w.w > SCR_COLS - 2) w.w = SCR_COLS - 2;
    if (w.h > SCR_ROWS - 2) w.h = SCR_ROWS - 2;
    w.x = imax(1, imin(22 + 3 * g_nwin, SCR_COLS - w.w - 2));
    w.y = imin(2 + g_nwin, SCR_ROWS - 1 - w.h);
    if (w.y < 1) w.y = 1;
    w.sx = w.x; w.sy = w.y; w.sw = w.w; w.sh = w.h;
    g_win[g_nwin++] = w;
    g_startOpen = false; g_dirty = true;
    return;
  }
}

static void drawDesktop() {
  screen.lock();
  screen.curVis = false;
  screen.fillRect(0, 1, SCR_COLS, SCR_ROWS - 2, G_MED, 0x19);
  for (int i = 0; i < NAPPS; i++) {
    int x, y; iconPos(i, x, y);
    const AppDef& d = g_apps[i];
    screen.fillRect(x, y, ICON_W, 3, ' ', d.iattr);
    screen.text(x + (ICON_W - (int)strlen(d.glyph)) / 2, y + 1, d.glyph, d.iattr);
    bool sel = (i == g_icon && g_nwin == 0);
    screen.fillRect(x, y + 3, ICON_W, 1, ' ', sel ? 0x70 : 0x1F);
    int l = strlen(d.label);
    screen.text(x + (ICON_W - l) / 2, y + 3, d.label, sel ? 0x70 : 0x1F);
  }
  for (int i = 0; i < g_nwin; i++) {
    Win& w = g_win[i];
    bool top = (i == g_nwin - 1);
    w.app->focused = top;
    drawFrame(w.x, w.y, w.w, w.h, w.app->title(), top, w.maxi);
    View v = {w.x + 1, w.y + 1, w.w - 2, w.h - 2, top};
    w.app->draw(v);
  }
  if (g_startOpen) {
    int mx = 0, my = TASKBAR_Y - MENU_ITEMS - 2;
    screen.box(mx, my, MENU_W, MENU_ITEMS + 2, 0x70, false);
    for (int i = 0; i < MENU_ITEMS; i++) {
      const char* t = i < NAPPS ? g_apps[i].menu : "Reboot";
      uint8_t a = (i == g_startSel) ? 0x1F : 0x70;
      screen.fillRect(mx + 1, my + 1 + i, MENU_W - 2, 1, ' ', a);
      screen.text(mx + 2, my + 1 + i, t, a);
    }
  }
  screen.fillRect(0, TASKBAR_Y, SCR_COLS, 1, ' ', 0x70);
  screen.text(0, TASKBAR_Y, " Start ", g_startOpen ? 0x1F : 0x2F);
  int tx = 9;
  for (int i = 0; i < g_nwin && tx < SCR_COLS - 12; i++) {
    char t[20]; snprintf(t, sizeof t, " %s ", g_win[i].app->title());
    if (strlen(t) > 16) { t[15] = ' '; t[16] = 0; }
    bool top = (i == g_nwin - 1);
    screen.text(tx, TASKBAR_Y, t, top ? 0x1F : 0x78);
    tx += strlen(t) + 1;
  }
  screen.unlock();
}

static void drawTopWin() {
  if (!g_nwin || g_startOpen) { drawDesktop(); return; }
  screen.lock();
  screen.curVis = false;
  Win& w = g_win[g_nwin - 1];
  w.app->focused = true;
  drawFrame(w.x, w.y, w.w, w.h, w.app->title(), true, w.maxi);
  View v = {w.x + 1, w.y + 1, w.w - 2, w.h - 2, true};
  w.app->draw(v);
  screen.unlock();
}

static void desktopMouse(int mx, int my, uint8_t btn) {
  uint8_t edge = btn & ~g_prevBtn, prev = g_prevBtn;
  g_prevBtn = btn;
  // Dragging a window by its title bar
  if (g_dragging) {
    if (btn & 1) {
      Win& w = g_win[g_nwin - 1];
      w.x = constrain(mx - g_dragDX, -(w.w - 6), SCR_COLS - 6);
      w.y = constrain(my - g_dragDY, 1, SCR_ROWS - 3);
    } else g_dragging = false;
    g_dirty = true;
    return;
  }
  // A press that started inside an app keeps going to that app until released (drag-select, etc.)
  if (g_capture >= 0 && g_capture < g_nwin) {
    Win& w = g_win[g_capture];
    w.app->mouse(mx - (w.x + 1), my - (w.y + 1), btn, prev);
    int cap = g_capture;
    if (!btn) g_capture = -1;
    if (cap == g_nwin - 1) g_dirtyTop = true;
    else g_dirty = true;
    return;
  }
  if (!edge) return;
  g_dirty = true;
  // Start menu
  if (g_startOpen) {
    int my0 = TASKBAR_Y - MENU_ITEMS - 2;
    if (mx >= 0 && mx < MENU_W && my > my0 && my <= my0 + MENU_ITEMS) {
      int i = my - my0 - 1;
      g_startOpen = false;
      if (i < NAPPS) legacyDesktopLaunch(g_apps[i].id, NULL); else ESP.restart();
      return;
    }
    g_startOpen = false;
    if (!(my == TASKBAR_Y && mx < 7)) return;
  }
  // Taskbar
  if (my == TASKBAR_Y) {
    if (mx < 7) { g_startOpen = !g_startOpen; g_startSel = 0; return; }
    int tx = 9;
    for (int i = 0; i < g_nwin && tx < SCR_COLS - 12; i++) {
      int wl = imin((int)strlen(g_win[i].app->title()) + 2, 16);
      if (mx >= tx && mx < tx + wl) { focusWin(i); return; }
      tx += wl + 1;
    }
    return;
  }
  // Windows, topmost first
  for (int i = g_nwin - 1; i >= 0; i--) {
    Win& w = g_win[i];
    int hit = frameHit(w.x, w.y, w.w, w.h, mx, my);
    if (hit == HIT_NONE) continue;
    focusWin(i);
    Win& t = g_win[g_nwin - 1];
    if (hit == HIT_CLOSE) { closeWin(g_nwin - 1); return; }
    if (hit == HIT_MAX) { toggleMax(g_nwin - 1); return; }
    if (hit == HIT_TITLE) {
      if (!t.maxi && (btn & 1)) { g_dragging = true; g_dragDX = mx - t.x; g_dragDY = my - t.y; }
      return;
    }
    g_capture = g_nwin - 1;
    t.app->mouse(mx - (t.x + 1), my - (t.y + 1), btn, prev);
    return;
  }
  // Desktop icons
  for (int i = 0; i < NAPPS; i++) {
    int x, y; iconPos(i, x, y);
    if (mx >= x && mx < x + ICON_W && my >= y && my < y + 4) {
      uint32_t now = millis();
      if (g_iconClickI == i && now - g_iconClickT < 600) { legacyDesktopLaunch(g_apps[i].id, NULL); g_iconClickI = -1; }
      else { g_icon = i; g_iconClickI = i; g_iconClickT = now; }
      return;
    }
  }
}

static void desktopKey(uint8_t k) {
  if (g_startOpen) {
    g_dirty = true;
    if (k == KEY_UP) g_startSel = (g_startSel + MENU_ITEMS - 1) % MENU_ITEMS;
    else if (k == KEY_DOWN) g_startSel = (g_startSel + 1) % MENU_ITEMS;
    else if (k == KEY_ENTER) { g_startOpen = false; if (g_startSel < NAPPS) legacyDesktopLaunch(g_apps[g_startSel].id, NULL); else ESP.restart(); }
    else if (k == KEY_ESC || k == KEY_F1) g_startOpen = false;
    return;
  }
  switch (k) {
    case KEY_F1: g_startOpen = true; g_startSel = 0; g_dirty = true; return;
    case KEY_F2: if (g_nwin > 1) { focusWin(0); } g_dirty = true; return;
    case KEY_F3: if (g_nwin) closeWin(g_nwin - 1); g_dirty = true; return;
    case KEY_F4: if (g_nwin) toggleMax(g_nwin - 1); g_dirty = true; return;
    case KEY_F10: g_leave = true; return;
  }
  if (g_nwin) {
    g_win[g_nwin - 1].app->key(k);
    g_dirtyTop = true;
    return;
  }
  g_dirty = true;
  // No windows: arrows pick an icon, Enter opens it
  if (k == KEY_UP && g_icon > 0) g_icon--;
  else if (k == KEY_DOWN && g_icon < NAPPS - 1) g_icon++;
  else if (k == KEY_LEFT && g_icon >= 4) g_icon -= 4;
  else if (k == KEY_RIGHT && g_icon + 4 < NAPPS) g_icon += 4;
  else if (k == KEY_ENTER) legacyDesktopLaunch(g_apps[g_icon].id, NULL);
}

static void desktopRun() {
  g_leave = false; g_barHint = "  F1 Menu  F2 Next  F3 Close";
  screen.setCursor(0, 1);
  if (g_deskFirst) { g_deskFirst = false; if (fsExists("/README.TXT")) legacyDesktopLaunch("notepad", "/README.TXT"); }
  g_dirty = true;
  uint32_t lastActivity = millis();
  while (!g_leave) {
    // close apps that asked to close themselves
    for (int i = g_nwin - 1; i >= 0; i--) if (g_win[i].app->closeReq) closeWin(i);
    if (g_dirty) { g_dirty = false; g_dirtyTop = false; drawDesktop(); }
    else if (g_dirtyTop) { g_dirtyTop = false; drawTopWin(); }
    uint32_t wait = 0, now = millis();
    for (int i = 0; i < g_nwin; i++) if (g_win[i].app->tickMs) {
      uint32_t due = g_win[i].lastTick + g_win[i].app->tickMs;
      uint32_t w = due > now ? due - now : 1;
      if (!wait || w < wait) wait = w;
    }
    uint8_t k = readKey(wait ? wait : (g_saverMode != SAVER_OFF ? 1000 : 0));
    now = millis();
    if (k == KEY_MOUSE) { lastActivity = now; desktopMouse(g_mx, g_my, g_mb); }
    else if (k) { lastActivity = now; desktopKey(k); }
    else if (g_saverMode != SAVER_OFF && now - lastActivity >= g_saverTimeoutMs) {
      launchScreensaver(g_saverMode);
      lastActivity = millis();
      g_dirty = true;
    }
    now = millis();
    for (int i = 0; i < g_nwin; i++) {
      Win& w = g_win[i];
      if (w.app->tickMs && now - w.lastTick >= w.app->tickMs) {
        w.lastTick = now;
        w.app->tick(now);
        if (i == g_nwin - 1) g_dirtyTop = true;
        else g_dirty = true;
      }
    }
  }
  screen.showCursor(true);
}


#endif // TENSILE_DISPLAY_DESKTOP_H
