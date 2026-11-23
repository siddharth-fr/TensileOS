#ifndef TENSILE_DISPLAY_DESKTOP_WM_H
#define TENSILE_DISPLAY_DESKTOP_WM_H

#pragma once
// TensileOS Desktop Window Manager running 100% on ESP32 Silicon (320x240 Pixel Framebuffer)
// Bridges all 16 real TensileOS applications into authentic 3D beveled windows.
#include "display/pixel_gfx.h"
#include "display/sprites.h"
#include "core/sound.h"
#include "fs/tensilefs.h"
#include "core/ui.h"
#include "shell/editor.h"
#include "apps/apps.h"
#include "games/games.h"
#include "games/doom.h"
#include "apps/hwapps.h"
#include "apps/paint.h"
#include "net/netmgr.h"
#include "net/web_browser.h"

#define MAX_WINDOWS 4

// Software Real-Time Clock
int g_rtcHour = 12;
int g_rtcMin = 0;
int g_rtcSec = 0;
static uint32_t g_rtcLastMillis = 0;

void rtcSet(int h, int m, int s) {
  g_rtcHour = (h % 24 + 24) % 24;
  g_rtcMin = (m % 60 + 60) % 60;
  g_rtcSec = (s % 60 + 60) % 60;
  g_rtcLastMillis = millis();
}

static void rtcTick() {
  uint32_t now = millis();
  if (g_rtcLastMillis == 0) g_rtcLastMillis = now;
  while (now - g_rtcLastMillis >= 1000) {
    g_rtcLastMillis += 1000;
    if (++g_rtcSec >= 60) {
      g_rtcSec = 0;
      if (++g_rtcMin >= 60) {
        g_rtcMin = 0;
        if (++g_rtcHour >= 24) {
          g_rtcHour = 0;
        }
      }
    }
  }
}

// Forward declaration
void desktopLaunch(const char* id, const char* arg);

// Interactive Command Prompt inside a window
class ShellWin : public App {
 public:
  enum { MAX_LINES = 16, LINE_LEN = 48 };
  char lines[MAX_LINES][LINE_LEN];
  int nLines = 0;
  char inBuf[LINE_LEN];
  int inLen = 0;

  ShellWin() {
    prefW = 46; prefH = 15;
    inBuf[0] = 0;
    addLine("TensileOS v0.2.0 Command Prompt");
    addLine("(C) Copyright TensileOS Contributors.");
    addLine("");
  }
  const char* title() override { return "Command Prompt"; }

  void addLine(const char* s) {
    if (nLines < MAX_LINES) {
      strncpy(lines[nLines++], s, LINE_LEN - 1);
      lines[nLines - 1][LINE_LEN - 1] = 0;
    } else {
      for (int i = 0; i < MAX_LINES - 1; i++) strcpy(lines[i], lines[i + 1]);
      strncpy(lines[MAX_LINES - 1], s, LINE_LEN - 1);
      lines[MAX_LINES - 1][LINE_LEN - 1] = 0;
    }
  }

  void draw(View& v) override {
    v.fillRect(0, 0, v.w, v.h, ' ', 0x07);
    int r = 0;
    int start = nLines > (v.h - 1) ? (nLines - (v.h - 1)) : 0;
    for (int i = start; i < nLines && r < v.h - 1; i++, r++) {
      v.text(0, r, lines[i], 0x07);
    }
    char prompt[LINE_LEN];
    snprintf(prompt, sizeof(prompt), "C:\\> %s", inBuf);
    v.text(0, r, prompt, 0x0F);
    v.cursor(5 + inLen, r);
  }

  void key(uint8_t k) override {
    if (k == 13 || k == 10) {
      char cmdLine[LINE_LEN];
      snprintf(cmdLine, sizeof(cmdLine), "C:\\> %s", inBuf);
      addLine(cmdLine);
      exec(inBuf);
      inBuf[0] = 0; inLen = 0;
    } else if (k == 8 || k == 127) {
      if (inLen > 0) inBuf[--inLen] = 0;
    } else if (k >= 32 && k < 127 && inLen < LINE_LEN - 8) {
      inBuf[inLen++] = k;
      inBuf[inLen] = 0;
    }
  }

  void exec(const char* cmd) {
    while (*cmd == ' ') cmd++;
    if (!*cmd) return;
    if (!strcasecmp(cmd, "ver")) {
      addLine("TensileOS v0.2.0 Workstation");
    } else if (!strcasecmp(cmd, "mem")) {
      char b[48];
      snprintf(b, sizeof(b), "Free Heap: %u KB", (unsigned)(ESP.getFreeHeap() / 1024));
      addLine(b);
    } else if (!strcasecmp(cmd, "cls")) {
      nLines = 0;
    } else if (!strcasecmp(cmd, "help") || !strcasecmp(cmd, "?")) {
      addLine("Commands: VER, MEM, DIR, TIME, CLS, HELP, REBOOT");
    } else if (!strncasecmp(cmd, "time", 4)) {
      const char* p = cmd + 4;
      while (*p == ' ') p++;
      if (*p) {
        int th = 0, tm = 0;
        if (sscanf(p, "%d:%d", &th, &tm) >= 1) {
          rtcSet(th, tm, 0);
          char b[48];
          snprintf(b, sizeof(b), "Time set to %02d:%02d:00", g_rtcHour, g_rtcMin);
          addLine(b);
        } else {
          addLine("Usage: TIME [HH:MM]");
        }
      } else {
        char b[48];
        snprintf(b, sizeof(b), "Current time is %02d:%02d:%02d", g_rtcHour, g_rtcMin, g_rtcSec);
        addLine(b);
      }
    } else if (!strcasecmp(cmd, "dir")) {
      FsEntry E[6];
      int c = fsList("/", E, 6);
      char b[48];
      snprintf(b, sizeof(b), "Directory of C:\\ (%d files):", c);
      addLine(b);
      for (int i = 0; i < c; i++) {
        snprintf(b, sizeof(b), "%-12s %6u", E[i].name, (unsigned)E[i].size);
        addLine(b);
      }
    } else if (!strcasecmp(cmd, "reboot")) {
      ESP.restart();
    } else if (!strcasecmp(cmd, "doom")) {
      addLine("Starting TensileDoom...");
      desktopLaunch("doom", "");
    } else {
      addLine("Bad command or file name");
    }
  }
};

struct WindowDef {
  App* app = NULL;
  int x = 20, y = 20, w = 220, h = 140;
  bool active = false;
  bool visible = true;
  bool maxi = false;
  int sx = 0, sy = 0, sw = 0, sh = 0;
  char title[32];
  uint32_t lastTick = 0;
};

class TensileDesktop {
 public:
  WindowDef wins[MAX_WINDOWS];
  int nWins = 0;
  int activeWin = -1;
  int mouseX = 160, mouseY = 120;
  uint8_t mouseBtn = 0;
  bool startMenuOpen = false;
  bool dragging = false;
  int dragOffX = 0, dragOffY = 0;
  bool showShutDown = false;
  bool needsRedraw = true;
  bool dirtyTaskbar = false;
  int dirtyWinApp = -1;
  int dragX = 0, dragY = 0;
  uint8_t* bgCache = NULL;
  bool bgCacheValid = false;

  void markDirty() {
    needsRedraw = true;
  }

  void markTaskbarDirty() {
    dirtyTaskbar = true;
  }

  void markWinDirty(int idx) {
    if (dirtyWinApp == -1) dirtyWinApp = idx;
    else if (dirtyWinApp != idx) needsRedraw = true;
  }

  void renderFrame() {
    if (needsRedraw) {
      needsRedraw = false;
      dirtyWinApp = -1;
      dirtyTaskbar = false;
      redrawAll();
      return;
    }
    if (dirtyWinApp >= 0 && dirtyWinApp < nWins) {
      int idx = dirtyWinApp;
      dirtyWinApp = -1;
      if (wins[idx].visible && idx == activeWin && !startMenuOpen && !showShutDown && !dragging) {
        gfx.lock();
        drawWinInstance(wins[idx]);
        gfx.markDirtyRange(wins[idx].y, wins[idx].y + wins[idx].h - 1);
        gfx.unlock();
      } else {
        redrawAll();
      }
    }
    if (dirtyTaskbar) {
      dirtyTaskbar = false;
      gfx.lock();
      drawTaskbar();
      gfx.markDirtyRange(216, 239);
      gfx.unlock();
    }
  }

  void rebuildBgCache() {
    if (!bgCache) bgCache = (uint8_t*)malloc(GFX_PITCH * 216);
    if (!bgCache) return;

    uint8_t* origFb = gfx.fb;
    gfx.fb = bgCache;
    gfx.fillRect(0, 0, GFX_W, 216, COL_TEAL);

    // Column 0 (x = 12)
    drawIcon(12, 8,   SPRITE_MY_COMPUTER, "My", "Computer");
    drawIcon(12, 58,  SPRITE_RECYCLE,     "Recycle", "Bin");
    drawIcon(12, 108, SPRITE_FOLDER,      "File", "Manager");
    drawIcon(12, 158, SPRITE_BROWSER,     "Web", "Browser");

    // Column 1 (x = 84)
    drawIcon(84, 8,   SPRITE_DOCUMENT,    "Notepad");
    drawIcon(84, 58,  SPRITE_CMD,         "Command", "Prompt");
    drawIcon(84, 108, SPRITE_PAINT,       "Paint");
    drawIcon(84, 158, SPRITE_CALC,        "Calculator");

    // Column 2 (x = 156)
    drawIcon(156, 8,   SPRITE_HWLAB,      "Hardware", "Lab");
    drawIcon(156, 58,  SPRITE_NET,        "Network");
    drawIcon(156, 108, SPRITE_MY_COMPUTER,"SysMon");
    drawIcon(156, 158, SPRITE_FLOPPY,     "Time /", "Date");

    // Column 3 (x = 228)
    drawIcon(228, 8,   SPRITE_DOOM,       "Tensile", "Doom");

    gfx.fb = origFb;
    bgCacheValid = true;
  }

  void begin() {
    gfx.begin();
    nWins = 0;
    activeWin = -1;
    startMenuOpen = false;
    showShutDown = false;
    rebuildBgCache();
    // Launch default welcome app: real Editor with /README.TXT
    launch("notepad", "/README.TXT");
    redrawAll();
  }

  void launch(const char* id, const char* arg = NULL) {
    // If already open, bring forward
    for (int i = 0; i < nWins; i++) {
      if (wins[i].app && wins[i].app->openPath() && arg && !strcmp(wins[i].app->openPath(), arg)) {
        focusWindow(i);
        return;
      }
    }
    if (nWins >= MAX_WINDOWS) return;

    App* a = NULL;
    if (!strcmp(id, "notepad"))      a = new Editor(arg ? arg : "", false);
    else if (!strcmp(id, "files"))   a = new FileManager(arg ? arg : "/");
    else if (!strcmp(id, "mycomp"))  a = new MyComputerApp();
    else if (!strcmp(id, "recycle")) a = new RecycleBinApp();
    else if (!strcmp(id, "timeset")) a = new TimeSetApp();
    else if (!strcmp(id, "calc"))    a = new Calc();
    else if (!strcmp(id, "sysmon"))  a = new Sysmon();
    else if (!strcmp(id, "snake"))   a = new Snake();
    else if (!strcmp(id, "mines"))   a = new Mines();
    else if (!strcmp(id, "tetris"))  a = new Tetris();
    else if (!strcmp(id, "bench"))   a = new SysBench();
    else if (!strcmp(id, "hwlab"))   a = new HwLab();
    else if (!strcmp(id, "paint"))   a = new AnsiPaint(arg);
    else if (!strcmp(id, "netmgr"))  a = new NetManager();
    else if (!strcmp(id, "browser")) a = new RetroWeb(arg);
    else if (!strcmp(id, "terminal"))a = new ShellWin();
    else if (!strcmp(id, "doom"))    a = new TensileDoom();

    if (!a) return;

    int idx = nWins++;
    WindowDef& w = wins[idx];
    w.app = a;

    int pw = a->prefW > 0 ? (a->prefW * 6 + 10) : 220;
    int ph = a->prefH > 0 ? (a->prefH * 8 + 30) : 140;
    if (pw > GFX_W - 8) pw = GFX_W - 8;
    if (ph > 214) ph = 214;
    w.w = pw;
    w.h = ph;
    w.x = constrain(36 + idx * 16, 2, GFX_W - w.w - 4);
    w.y = constrain(20 + idx * 12, 2, 216 - w.h - 4);
    w.maxi = false;
    w.active = true;
    w.visible = true;
    w.lastTick = millis();
    strncpy(w.title, a->title(), sizeof(w.title) - 1);
    w.title[sizeof(w.title) - 1] = 0;

    focusWindow(idx);
    soundClick();
  }

  void focusWindow(int idx) {
    if (idx < 0 || idx >= nWins) return;
    for (int i = 0; i < nWins; i++) {
      wins[i].active = (i == idx);
      if (wins[i].app) wins[i].app->focused = (i == idx);
    }
    activeWin = idx;
    markDirty();
  }

  void closeWindow(int idx) {
    if (idx < 0 || idx >= nWins) return;
    soundClick();
    if (wins[idx].app) delete wins[idx].app;
    for (int i = idx; i < nWins - 1; i++) wins[i] = wins[i + 1];
    nWins--;
    activeWin = (nWins > 0) ? (nWins - 1) : -1;
    if (activeWin >= 0) {
      wins[activeWin].active = true;
      if (wins[activeWin].app) wins[activeWin].app->focused = true;
    }
    markDirty();
  }

  void minimizeWindow(int idx) {
    if (idx < 0 || idx >= nWins) return;
    soundClick();
    wins[idx].visible = false;
    wins[idx].active = false;
    if (wins[idx].app) wins[idx].app->focused = false;
    activeWin = -1;
    for (int i = nWins - 1; i >= 0; i--) {
      if (wins[i].visible) {
        focusWindow(i);
        return;
      }
    }
    markDirty();
  }

  void toggleMax(int idx) {
    if (idx < 0 || idx >= nWins) return;
    WindowDef& w = wins[idx];
    soundClick();
    if (!w.maxi) {
      w.sx = w.x; w.sy = w.y; w.sw = w.w; w.sh = w.h;
      w.x = 0; w.y = 0; w.w = GFX_W; w.h = 216;
      w.maxi = true;
    } else {
      w.x = w.sx; w.y = w.sy; w.w = w.sw; w.h = w.sh;
      w.maxi = false;
    }
    focusWindow(idx);
    markDirty();
  }

  void redrawAll() {
    gfx.lock();
    // 1. Blit cached Classic Teal desktop background + icons (ultra-fast memcpy)
    if (!bgCacheValid || !bgCache) rebuildBgCache();
    if (bgCache) {
      memcpy(gfx.fb, bgCache, GFX_PITCH * 216);
    } else {
      gfx.fillRect(0, 0, GFX_W, 216, COL_TEAL);
      // Fallback icon drawing
      drawIcon(12, 8,   SPRITE_MY_COMPUTER, "My", "Computer");
      drawIcon(12, 58,  SPRITE_RECYCLE,     "Recycle", "Bin");
      drawIcon(12, 108, SPRITE_FOLDER,      "File", "Manager");
      drawIcon(12, 158, SPRITE_BROWSER,     "Web", "Browser");
      drawIcon(84, 8,   SPRITE_DOCUMENT,    "Notepad");
      drawIcon(84, 58,  SPRITE_CMD,         "Command", "Prompt");
      drawIcon(84, 108, SPRITE_PAINT,       "Paint");
      drawIcon(84, 158, SPRITE_CALC,        "Calculator");
      drawIcon(156, 8,   SPRITE_HWLAB,      "Hardware", "Lab");
      drawIcon(156, 58,  SPRITE_NET,        "Network");
      drawIcon(156, 108, SPRITE_MY_COMPUTER,"SysMon");
      drawIcon(156, 158, SPRITE_FLOPPY,     "Time /", "Date");
      drawIcon(228, 8,   SPRITE_DOOM,       "Tensile", "Doom");
    }

    // 2. Draw Windows (inactive first, active last)
    for (int i = 0; i < nWins; i++) {
      if (i != activeWin && wins[i].visible) drawWinInstance(wins[i]);
    }
    if (activeWin >= 0 && wins[activeWin].visible) {
      drawWinInstance(wins[activeWin]);
    }

    // 2b. Draw Smooth Dithered Wireframe Drag Outline
    if (dragging && activeWin >= 0 && wins[activeWin].visible) {
      gfx.drawDitherRect(dragX, dragY, wins[activeWin].w, wins[activeWin].h);
    }

    // 3. Draw Start Menu if open
    if (startMenuOpen) {
      drawStartMenu();
    }

    // 4. Draw Shut Down confirmation dialog if open
    if (showShutDown) {
      drawShutDownDialog();
    }

    // 5. Draw TensileOS Taskbar (y = 216..239)
    drawTaskbar();

    gfx.markDirtyRange(0, GFX_H - 1);
    gfx.unlock();
  }

  void drawIcon(int x, int y, const uint8_t* sprite, const char* line1, const char* line2 = NULL) {
    gfx.drawSprite(x + 4, y, 24, 24, sprite);
    int w1 = strlen(line1) * 6;
    int tx1 = constrain(x + (32 - w1) / 2, 0, GFX_W - w1);
    gfx.drawText6x8(tx1 + 1, y + 26, line1, COL_BLACK, 0xFF);
    gfx.drawText6x8(tx1, y + 25, line1, COL_WHITE, 0xFF);
    if (line2) {
      int w2 = strlen(line2) * 6;
      int tx2 = constrain(x + (32 - w2) / 2, 0, GFX_W - w2);
      gfx.drawText6x8(tx2 + 1, y + 35, line2, COL_BLACK, 0xFF);
      gfx.drawText6x8(tx2, y + 34, line2, COL_WHITE, 0xFF);
    }
  }

  void drawWinInstance(WindowDef& w) {
    gfx.drawWindow(w.x, w.y, w.w, w.h, w.title, w.active, w.maxi);
    if (!w.app) return;

    int cx = w.x + 4, cy = w.y + 23;
    int cw = w.w - 8, ch = w.h - 27;
    int cols = cw / 6;
    int rows = ch / 8;
    if (cols < 1 || rows < 1) return;

    // Hardware clip strictly to client area!
    gfx.setClip(cx, cy, cx + cw - 1, cy + ch - 1);

    View v;
    v.x = 0; v.y = 0; v.w = cols; v.h = rows;
    v.px = cx; v.py = cy;
    v.pw = cw; v.ph = ch;
    v.pixelMode = true;
    v.focused = w.active;

    w.app->focused = w.active;
    w.app->draw(v);

    gfx.resetClip();
  }

  void drawTaskbar() {
    gfx.fillRect(0, 216, GFX_W, 24, COL_SILVER);
    gfx.drawHLine(0, 216, GFX_W, COL_WHITE);

    // Start Button (at x = 2, y = 218, w = 56, h = 20)
    gfx.drawBevelBox(2, 218, 56, 20, startMenuOpen);
    gfx.drawSprite(5, 222, 12, 12, SPRITE_TENSILE_LOGO);
    gfx.drawText6x8(21, 224, "Start", COL_BLACK, COL_SILVER);

    // Taskbar Tabs
    int tabX = 62;
    for (int i = 0; i < nWins; i++) {
      int tabW = 60;
      bool act = (i == activeWin && wins[i].visible);
      gfx.drawBevelBox(tabX, 219, tabW, 19, act);
      char label[9];
      strncpy(label, wins[i].title, 8);
      label[8] = 0;
      gfx.drawText6x8(tabX + (act ? 5 : 4), 225 + (act ? 1 : 0), label, COL_BLACK, COL_SILVER);
      tabX += tabW + 2;
    }

    // System Tray (Sunken well at x = 258, y = 219, w = 58, h = 18)
    gfx.drawBevelBox(258, 219, 58, 18, true);
    // Live Real-Time Clock (Cleanly centered)
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", g_rtcHour, g_rtcMin);
    gfx.drawText6x8(272, 224, timeStr, COL_BLACK, COL_SILVER);
  }

  void drawStartMenu() {
    int mx = 2, my = 18, mw = 126, mh = 198;
    gfx.fillRect(mx, my, mw, mh, COL_SILVER);
    gfx.drawBevelBox(mx, my, mw, mh, false);

        // Navy Blue vertical banner with TensileOS branding
    gfx.fillRect(mx + 2, my + 2, 18, mh - 4, COL_NAVY);
    const char* brand = "TENSILE";
    for (int bi = 0; brand[bi]; bi++) {
      char chStr[2] = {brand[bi], 0};
      gfx.drawText6x8(mx + 6, my + 14 + bi * 12, chStr, COL_WHITE, COL_NAVY);
    }
    gfx.drawText6x8(mx + 4, my + mh - 22, "OS", COL_YELLOW, COL_NAVY);

    const char* items[] = {
      "File Manager", "Notepad", "ANSI Paint", "Calculator",
      "Web Browser", "Hardware Lab", "Network Manager",
      "Command Prompt", "System Monitor", "Date / Time",
      "TensileDoom", "Snake Game", "Minesweeper", "Tetris Game", "Shut Down..."
    };
    for (int i = 0; i < 15; i++) {
      int iy = my + 4 + i * 13;
      gfx.drawText6x8(mx + 24, iy + 2, items[i], (i == 14 ? COL_RED : COL_BLACK), COL_SILVER);
    }
  }

  void drawShutDownDialog() {
    int dx = 60, dy = 60, dw = 200, dh = 90;
    gfx.drawWindow(dx, dy, dw, dh, "Shut Down TensileOS", true);
    gfx.drawText6x8(dx + 12, dy + 32, "Are you sure you want to", COL_BLACK, COL_WHITE);
    gfx.drawText6x8(dx + 12, dy + 44, "restart your computer?", COL_BLACK, COL_WHITE);
    gfx.drawButton(dx + 25, dy + 62, 65, 18, "Restart", false);
    gfx.drawButton(dx + 110, dy + 62, 65, 18, "Cancel", false);
  }

  void onMouse(int x, int y, uint8_t btn, uint8_t prev) {
    mouseX = constrain(x, 0, GFX_W - 1);
    mouseY = constrain(y, 0, GFX_H - 1);
    uint8_t edge = btn & ~prev;

    // 1. Handle Titlebar dragging (Smooth Wireframe Ghost Drag)
    if (dragging) {
      if (btn & 1) {
        if (activeWin >= 0) {
          int newX = constrain(mouseX - dragOffX, -20, GFX_W - 40) & ~1;
          int newY = constrain(mouseY - dragOffY, 0, 216 - 20);
          if (dragX != newX || dragY != newY) {
            dragX = newX;
            dragY = newY;
            markDirty();
          }
        }
      } else {
        if (activeWin >= 0) {
          wins[activeWin].x = dragX;
          wins[activeWin].y = dragY;
        }
        dragging = false;
        soundClick();
        markDirty();
      }
      return;
    }

    // 2. Continuous drawing / dragging inside active window client area
    if ((btn & 1) && activeWin >= 0) {
      WindowDef& w = wins[activeWin];
      if (w.visible && mouseX >= w.x + 4 && mouseX < w.x + w.w - 4 && mouseY >= w.y + 23 && mouseY < w.y + w.h - 4) {
        int px = mouseX - (w.x + 4);
        int py = mouseY - (w.y + 23);
        int cx = px / 6;
        int cy = py / 8;
        if (w.app) {
          w.app->mouse(cx, cy, btn, prev);
          w.app->mousePixel(px, py, btn, prev);
          markWinDirty(activeWin);
        }
        return;
      }
    }

    if (!edge) return;

    // 3. Shut Down Dialog clicks
    if (showShutDown) {
      int dx = 60, dy = 60;
      if (mouseY >= dy + 62 && mouseY <= dy + 80) {
        if (mouseX >= dx + 25 && mouseX <= dx + 90) { soundClick(); ESP.restart(); }
        if (mouseX >= dx + 110 && mouseX <= dx + 175) { soundClick(); showShutDown = false; markDirty(); return; }
      }
      return;
    }

    // 4. Click on Start Button
    if (mouseY >= 218 && mouseX >= 2 && mouseX <= 58) {
      startMenuOpen = !startMenuOpen;
      soundClick();
      markDirty();
      return;
    }

    // 5. Click in Start Menu
    if (startMenuOpen) {
      int mx = 2, my = 18, mw = 126, mh = 198;
      if (mouseX >= mx && mouseX <= mx + mw && mouseY >= my && mouseY <= my + mh) {
        int item = (mouseY - (my + 4)) / 13;
        startMenuOpen = false;
        soundClick();
        if (item == 0) launch("files", "/");
        else if (item == 1) launch("notepad", "");
        else if (item == 2) launch("paint");
        else if (item == 3) launch("calc");
        else if (item == 4) launch("browser");
        else if (item == 5) launch("hwlab");
        else if (item == 6) launch("netmgr");
        else if (item == 7) launch("terminal");
        else if (item == 8) launch("sysmon");
        else if (item == 9) launch("timeset");
        else if (item == 10) launch("doom");
        else if (item == 11) launch("snake");
        else if (item == 12) launch("mines");
        else if (item == 13) launch("tetris");
        else if (item == 14) { showShutDown = true; markDirty(); return; }
        markDirty();
        return;
      }
      startMenuOpen = false;
      markDirty();
    }

    // 6. Click on System Tray Clock -> Launch Date/Time Properties
    if (mouseY >= 218 && mouseX >= 256) {
      launch("timeset");
      soundClick();
      return;
    }

    // 7. Taskbar Tabs
    if (mouseY >= 218 && mouseX >= 62 && mouseX < 256) {
      int tabIdx = (mouseX - 62) / 62;
      if (tabIdx >= 0 && tabIdx < nWins) {
        if (!wins[tabIdx].visible) {
          wins[tabIdx].visible = true;
          focusWindow(tabIdx);
        } else if (tabIdx == activeWin) {
          minimizeWindow(tabIdx);
        } else {
          focusWindow(tabIdx);
        }
        soundClick();
        return;
      }
    }

    // 8. Windows hit testing (topmost first)
    for (int i = nWins - 1; i >= 0; i--) {
      WindowDef& w = wins[i];
      if (!w.visible) continue;
      if (mouseX >= w.x && mouseX < w.x + w.w && mouseY >= w.y && mouseY < w.y + w.h) {
        focusWindow(i);

        // Control buttons (Minimize [_], Maximize / Restore [■] / [❐], Close [X])
        if (mouseY >= w.y + 4 && mouseY <= w.y + 18) {
          if (mouseX >= w.x + w.w - 18 && mouseX <= w.x + w.w - 4) {
            closeWindow(i);
            return;
          }
          if (mouseX >= w.x + w.w - 32 && mouseX <= w.x + w.w - 19) {
            toggleMax(i);
            return;
          }
          if (mouseX >= w.x + w.w - 46 && mouseX <= w.x + w.w - 33) {
            minimizeWindow(i);
            return;
          }
        }

        // Titlebar click: start dragging
        if (mouseY <= w.y + 22) {
          if (!w.maxi && (btn & 1)) {
            dragging = true;
            dragOffX = mouseX - w.x;
            dragOffY = mouseY - w.y;
            dragX = w.x;
            dragY = w.y;
            markDirty();
          }
          return;
        }

        // Client area click: forward to app
        int px = mouseX - (w.x + 4);
        int py = mouseY - (w.y + 23);
        int cx = px / 6;
        int cy = py / 8;
        if (w.app) {
          w.app->mouse(cx, cy, btn, prev);
          w.app->mousePixel(px, py, btn, prev);
        }
        markWinDirty(i);
        return;
      }
    }

    // 9. Desktop Icons Hit-Testing (Col 0, Col 1, Col 2)
    int row = -1;
    if (mouseY >= 8 && mouseY <= 54) row = 0;
    else if (mouseY >= 58 && mouseY <= 104) row = 1;
    else if (mouseY >= 108 && mouseY <= 154) row = 2;
    else if (mouseY >= 158 && mouseY <= 204) row = 3;

    if (row >= 0) {
      if (mouseX >= 4 && mouseX <= 64) { // Col 0
        if (row == 0) launch("mycomp");
        else if (row == 1) launch("recycle");
        else if (row == 2) launch("files", "/");
        else if (row == 3) launch("browser");
        return;
      }
      if (mouseX >= 76 && mouseX <= 136) { // Col 1
        if (row == 0) launch("notepad", "");
        else if (row == 1) launch("terminal");
        else if (row == 2) launch("paint");
        else if (row == 3) launch("calc");
        return;
      }
      if (mouseX >= 148 && mouseX <= 208) { // Col 2
        if (row == 0) launch("hwlab");
        else if (row == 1) launch("netmgr");
        else if (row == 2) launch("sysmon");
        else if (row == 3) launch("timeset");
        return;
      }
      if (mouseX >= 220 && mouseX <= 280) { // Col 3
        if (row == 0) launch("doom");
        return;
      }
    }
  }

  void onKey(uint8_t k) {
    if (activeWin >= 0 && activeWin < nWins && wins[activeWin].app) {
      wins[activeWin].app->key(k);
      markWinDirty(activeWin);
    }
  }

  void tick() {
    uint32_t now = millis();
    rtcTick();
    bool needFullRedraw = false;
    for (int i = 0; i < nWins; i++) {
      if (wins[i].app) wins[i].app->focused = (i == activeWin);
      if (wins[i].app && wins[i].app->tickMs > 0) {
        if (now - wins[i].lastTick >= wins[i].app->tickMs) {
          wins[i].lastTick = now;
          wins[i].app->tick(now);
          if (i == activeWin && wins[i].visible && !startMenuOpen && !showShutDown && !dragging) {
            markWinDirty(i);
          } else {
            needFullRedraw = true;
          }
        }
      }
      if (wins[i].app && wins[i].app->closeReq) {
        closeWindow(i);
        return;
      }
    }
    static uint32_t lastClockSec = 0;
    if ((uint32_t)g_rtcSec != lastClockSec) {
      lastClockSec = g_rtcSec;
      markTaskbarDirty();
    }
    if (needFullRedraw) markDirty();
  }
};

extern TensileDesktop desktop;

void desktopLaunch(const char* id, const char* arg) {
  desktop.launch(id, arg);
}


#endif // TENSILE_DISPLAY_DESKTOP_WM_H
