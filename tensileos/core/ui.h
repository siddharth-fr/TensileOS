#ifndef TENSILE_CORE_UI_H
#define TENSILE_CORE_UI_H

#pragma once
// App framework: every app draws into a View (a clipped rectangle) and reacts to key/mouse/tick events.
// The same app runs inside a desktop window or full-screen from the terminal.
#include <vector>
#include <string>
#include <math.h>
#include "fs/tensilefs.h"
#include "display/pixel_gfx.h"

struct View {
  int x, y, w, h; bool focused;
  int px = 0, py = 0; // pixel offset on 320x240 screen
  int pw = 0, ph = 0; // pixel width and height of client area
  bool pixelMode = false;

  void put(int cx, int cy, uint8_t c, uint8_t a) {
    if (cx < 0 || cy < 0 || cx >= w || cy >= h) return;
    if (pixelMode) {
      uint8_t fg = a & 0x0F;
      uint8_t bg = (a >> 4) & 0x0F;
      gfx.drawChar6x8(px + cx * 6, py + cy * 8, (char)c, fg, bg);
    } else {
      screen.put(x + cx, y + cy, c, a);
    }
  }

  void text(int cx, int cy, const char* s, uint8_t a) {
    for (; *s; s++, cx++) put(cx, cy, (uint8_t)*s, a);
  }

  void textr(int cxRight, int cy, const char* s, uint8_t a) {
    text(cxRight - (int)strlen(s), cy, s, a);
  }

  void fillRect(int cx, int cy, int cw, int ch, uint8_t c, uint8_t a) {
    if (pixelMode) {
      uint8_t bg = (a >> 4) & 0x0F;
      uint8_t fg = a & 0x0F;
      gfx.fillRect(px + cx * 6, py + cy * 8, cw * 6, ch * 8, bg);
      if (c != ' ' && c != 0) {
        for (int j = 0; j < ch; j++) {
          for (int i = 0; i < cw; i++) {
            gfx.drawChar6x8(px + (cx + i) * 6, py + (cy + j) * 8, (char)c, fg, bg);
          }
        }
      }
    } else {
      for (int j = 0; j < ch; j++)
        for (int i = 0; i < cw; i++)
          put(cx + i, cy + j, c, a);
    }
  }

  void fill(uint8_t a) { fillRect(0, 0, w, h, ' ', a); }

  void cursor(int cx, int cy) {
    if (focused && cx >= 0 && cy >= 0 && cx < w && cy < h) {
      if (pixelMode) {
        gfx.drawHLine(px + cx * 6, py + cy * 8 + 6, 6, COL_BLACK);
        gfx.drawHLine(px + cx * 6, py + cy * 8 + 7, 6, COL_BLACK);
      } else {
        screen.setCursor(x + cx, y + cy);
        screen.showCursor(true);
      }
    }
  }
};

struct App {
  int prefW = 40, prefH = 14;   // preferred OUTER window size (0,0 = fill the screen)
  uint32_t tickMs = 0;          // >0: tick() is called about this often
  bool fixed = false;           // true: window can't be maximised
  bool closeReq = false;        // app sets this to close itself
  bool focused = false;
  virtual ~App() {}
  virtual const char* title() = 0;
  virtual void draw(View& v) = 0;
  virtual void key(uint8_t k) {}
  virtual void mouse(int cx, int cy, uint8_t btn, uint8_t prev) {}   // client character coords; press edge = btn & ~prev
  virtual void mousePixel(int px, int py, uint8_t btn, uint8_t prev) {} // client pixel coords (0..cw-1, 0..ch-1)
  virtual void tick(uint32_t now) {}
  virtual const char* openPath() { return NULL; }   // editors report the file they have open
};

// ---- window frame (retro grey window, blue title bar, [^] maximise, [X] close, drop shadow) ----
static void drawFrame(int x, int y, int w, int h, const char* title, bool focus, bool maxi) {
  screen.lock();
  if (!maxi) {
    screen.fillRect(x + w, y + 1, 2, h, ' ', 0x00);
    screen.fillRect(x + 2, y + h, w, 1, ' ', 0x00);
  }
  screen.box(x, y, w, h, 0x70, false);
  uint8_t ta = focus ? 0x1F : 0x78;
  screen.fillRect(x + 1, y, w - 2, 1, ' ', ta);
  char t[48]; snprintf(t, sizeof t, " %s", title);
  int maxt = w - 12; if (maxt < 4) maxt = 4;
  if ((int)strlen(t) > maxt) t[maxt] = 0;
  screen.text(x + 1, y, t, ta);
  if (w >= 16) {
    screen.put(x + w - 8, y, ' ', 0x2F); screen.put(x + w - 7, y, 0x1E, 0x2F); screen.put(x + w - 6, y, ' ', 0x2F);
    screen.put(x + w - 5, y, ' ', 0x4F); screen.put(x + w - 4, y, 'X', 0x4F);  screen.put(x + w - 3, y, ' ', 0x4F);
  }
  screen.unlock();
}
enum { HIT_NONE = 0, HIT_TITLE, HIT_CLOSE, HIT_MAX, HIT_CLIENT };
static int frameHit(int x, int y, int w, int h, int mx, int my) {
  if (mx < x || my < y || mx >= x + w || my >= y + h) return HIT_NONE;
  if (my == y) {
    if (w >= 16 && mx >= x + w - 5 && mx <= x + w - 3) return HIT_CLOSE;
    if (w >= 16 && mx >= x + w - 8 && mx <= x + w - 6) return HIT_MAX;
    return HIT_TITLE;
  }
  if (mx > x && mx < x + w - 1 && my < y + h - 1) return HIT_CLIENT;
  return HIT_TITLE;       // borders behave like the title bar (drag handle)
}

// Run one app modally (used by terminal commands: edit, snake, ...). Returns when the app closes itself.
static void runApp(App* a) {
  bool framed = a->prefW > 0;
  int w = framed ? a->prefW : SCR_COLS, h = framed ? a->prefH : SCR_ROWS - 1;
  int x = (SCR_COLS - w) / 2, y = 1 + (SCR_ROWS - 1 - h) / 2;
  View v; v.focused = true;
  if (framed) { v.x = x + 1; v.y = y + 1; v.w = w - 2; v.h = h - 2; }
  else        { v.x = 0; v.y = 1; v.w = SCR_COLS; v.h = SCR_ROWS - 1; }
  a->focused = true;
  uint8_t prev = 0;
  uint32_t lastTick = millis();
  bool first = true;
  while (!a->closeReq) {
    screen.lock();
    if (first) { screen.fillRect(0, 1, SCR_COLS, SCR_ROWS - 1, 0xB1, 0x19); first = false; }
    screen.showCursor(false);
    if (framed) drawFrame(x, y, w, h, a->title(), true, false);
    a->draw(v);
    screen.unlock();
    uint8_t k = readKey(a->tickMs ? a->tickMs : 1000);
    if (k == KEY_MOUSE) { a->mouse(g_mx - v.x, g_my - v.y, g_mb, prev); prev = g_mb; }
    else if (k) a->key(k);
    if (a->tickMs && millis() - lastTick >= a->tickMs) { lastTick = millis(); a->tick(lastTick); }
  }
  screen.showCursor(true);
}


#endif // TENSILE_CORE_UI_H
