#ifndef TENSILE_DISPLAY_SCREEN_H
#define TENSILE_DISPLAY_SCREEN_H

#pragma once
// Text-mode screen: 80x25 cells, 2 bytes per cell (CP437 char + VGA colour attr).
// The whole "GPU" is these 4000 bytes. Attr: high nibble = background, low = foreground.
#include <Arduino.h>
#include <string.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#define SCR_COLS 80
#define SCR_ROWS 25
#define SCR_CELLS (SCR_COLS * SCR_ROWS)
#define SCR_FRAME_MAX (6 + SCR_CELLS * 2 + 16)

// CP437 glyph codes (box drawing, blocks, symbols)
enum : uint8_t {
  G_V1 = 0xB3, G_H1 = 0xC4, G_TL1 = 0xDA, G_TR1 = 0xBF, G_BL1 = 0xC0, G_BR1 = 0xD9,
  G_V2 = 0xBA, G_H2 = 0xCD, G_TL2 = 0xC9, G_TR2 = 0xBB, G_BL2 = 0xC8, G_BR2 = 0xBC,
  G_FULL = 0xDB, G_LOWER = 0xDC, G_UPPER = 0xDF, G_LIGHT = 0xB0, G_MED = 0xB1, G_DARK = 0xB2
};

struct Cell { uint8_t c, a; };

class Screen {
 public:
  Cell cells[SCR_CELLS];
  Cell shadow[SCR_CELLS];            // what clients last received
  uint8_t curX = 0, curY = 1;
  uint8_t attr = 0x1F;               // default: bright white on blue
  uint8_t top = 1;                   // row 0 is the status bar; scrolling happens below it
  bool curVis = true;
  uint8_t sCurX = 255, sCurY = 255;
  bool sCurVis = false;
  uint32_t dirtyRows = 0x01FFFFFF;
  SemaphoreHandle_t mtx = NULL;

  void begin() { mtx = xSemaphoreCreateRecursiveMutex(); clearAll(); }
  void lock()   { if (mtx) xSemaphoreTakeRecursive(mtx, portMAX_DELAY); }
  void unlock() { if (mtx) xSemaphoreGiveRecursive(mtx); }

  // ---- clearing / cursor ----
  void clearAll() {
    lock();
    for (int i = 0; i < SCR_CELLS; i++) { cells[i].c = ' '; cells[i].a = attr; }
    curX = 0; curY = top;
    dirtyRows = 0x01FFFFFF;
    unlock();
  }
  void clear() {                     // clears everything below the status bar
    lock();
    for (int i = top * SCR_COLS; i < SCR_CELLS; i++) { cells[i].c = ' '; cells[i].a = attr; }
    curX = 0; curY = top;
    for (int r = top; r < SCR_ROWS; r++) dirtyRows |= (1UL << r);
    unlock();
  }
  void setCursor(int x, int y) {
    lock();
    curX = constrain(x, 0, SCR_COLS - 1);
    curY = constrain(y, 0, SCR_ROWS - 1);
    unlock();
  }
  void showCursor(bool v) { lock(); curVis = v; unlock(); }

  // ---- terminal-style output (moves the cursor, scrolls) ----
  void putc(uint8_t c) { lock(); put1(c); unlock(); }
  void print(const char* s) { lock(); while (*s) put1((uint8_t)*s++); unlock(); }
  void printf(const char* fmt, ...) {
    char b[200];
    va_list ap; va_start(ap, fmt); vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    print(b);
  }

  // ---- positioned drawing (does not move the cursor) ----
  void put(int x, int y, uint8_t c, uint8_t a) { lock(); setCell(x, y, c, a); unlock(); }
  void text(int x, int y, const char* s, uint8_t a) {
    lock();
    for (; *s && x < SCR_COLS; s++, x++) setCell(x, y, (uint8_t)*s, a);
    unlock();
  }
  void fillRect(int x, int y, int w, int h, uint8_t c, uint8_t a) {
    lock();
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) setCell(x + i, y + j, c, a);
    unlock();
  }
  void box(int x, int y, int w, int h, uint8_t a, bool dbl) {
    uint8_t tl = dbl ? G_TL2 : G_TL1, tr = dbl ? G_TR2 : G_TR1;
    uint8_t bl = dbl ? G_BL2 : G_BL1, br = dbl ? G_BR2 : G_BR1;
    uint8_t hh = dbl ? G_H2 : G_H1,  vv = dbl ? G_V2 : G_V1;
    lock();
    fillRect(x, y, w, h, ' ', a);
    for (int i = 1; i < w - 1; i++) { setCell(x + i, y, hh, a); setCell(x + i, y + h - 1, hh, a); }
    for (int j = 1; j < h - 1; j++) { setCell(x, y + j, vv, a); setCell(x + w - 1, y + j, vv, a); }
    setCell(x, y, tl, a); setCell(x + w - 1, y, tr, a);
    setCell(x, y + h - 1, bl, a); setCell(x + w - 1, y + h - 1, br, a);
    unlock();
  }
  void recolor(uint8_t a) {          // repaint every cell below the status bar
    lock();
    attr = a;
    for (int i = top * SCR_COLS; i < SCR_CELLS; i++) cells[i].a = a;
    for (int r = top; r < SCR_ROWS; r++) dirtyRows |= (1UL << r);
    unlock();
  }
  void snapshot(Cell* out) { lock(); memcpy(out, cells, sizeof(cells)); unlock(); }
  void restore(const Cell* in) { lock(); memcpy(cells, in, sizeof(cells)); dirtyRows = 0x01FFFFFF; unlock(); }

  // ---- network encoding ----
  // Frame 0x01 FULL: [1, cols, rows, curX, curY, curVis, cells...]
  // Frame 0x02 DIFF: [2, curX, curY, curVis, nHi, nLo, (idxHi, idxLo, ch, attr) * n]
  // Returns true if dbuf holds a frame that must be broadcast. If wantFull, fbuf gets a full frame.
  bool encode(uint8_t* dbuf, size_t* dlen, uint8_t* fbuf, size_t* flen, bool wantFull) {
    bool need = false;
    lock();
    if (wantFull) encodeFull(fbuf, flen);
    bool curChg = (curX != sCurX || curY != sCurY || curVis != sCurVis);
    if (dirtyRows != 0 || curChg) {
      dbuf[0] = 2; dbuf[1] = curX; dbuf[2] = curY; dbuf[3] = curVis ? 1 : 0;
      size_t p = 6;
      int n = 0;
      bool overflow = false;
      uint32_t mask = dirtyRows;
      for (int y = 0; y < SCR_ROWS && !overflow; y++) {
        if (!(mask & (1UL << y))) continue;
        int base = y * SCR_COLS;
        for (int x = 0; x < SCR_COLS; x++) {
          int i = base + x;
          if (cells[i].c != shadow[i].c || cells[i].a != shadow[i].a) {
            n++;
            if (n > SCR_CELLS / 2) { overflow = true; break; }
            dbuf[p++] = i >> 8; dbuf[p++] = i & 0xFF; dbuf[p++] = cells[i].c; dbuf[p++] = cells[i].a;
            shadow[i] = cells[i];
          }
        }
      }
      if (overflow) {
        encodeFull(dbuf, dlen);
        memcpy(shadow, cells, sizeof(cells));
      } else {
        dbuf[4] = n >> 8; dbuf[5] = n & 0xFF;
        *dlen = p;
      }
      sCurX = curX; sCurY = curY; sCurVis = curVis;
      dirtyRows = 0;
      need = (n > 0 || curChg || overflow);
    }
    unlock();
    return need;
  }

 private:
  void setCell(int x, int y, uint8_t c, uint8_t a) {
    if (x < 0 || y < 0 || x >= SCR_COLS || y >= SCR_ROWS) return;
    int idx = y * SCR_COLS + x;
    if (cells[idx].c != c || cells[idx].a != a) {
      cells[idx].c = c; cells[idx].a = a;
      dirtyRows |= (1UL << y);
    }
  }
  void scrollUp() {
    memmove(&cells[top * SCR_COLS], &cells[(top + 1) * SCR_COLS],
            sizeof(Cell) * SCR_COLS * (SCR_ROWS - 1 - top));
    for (int x = 0; x < SCR_COLS; x++) {
      int idx = (SCR_ROWS - 1) * SCR_COLS + x;
      cells[idx].c = ' '; cells[idx].a = attr;
    }
    for (int r = top; r < SCR_ROWS; r++) dirtyRows |= (1UL << r);
  }
  void newline() { curX = 0; if (curY < SCR_ROWS - 1) curY++; else scrollUp(); }
  void put1(uint8_t c) {
    switch (c) {
      case '\n': newline(); break;
      case '\r': curX = 0; break;
      case '\b': if (curX > 0) curX--; break;
      case '\t': { int n = 8 - (curX % 8); while (n--) put1(' '); } break;
      default:
        setCell(curX, curY, c, attr);
        if (++curX >= SCR_COLS) newline();
    }
  }
  void encodeFull(uint8_t* o, size_t* len) {
    o[0] = 1; o[1] = SCR_COLS; o[2] = SCR_ROWS; o[3] = curX; o[4] = curY; o[5] = curVis ? 1 : 0;
    memcpy(o + 6, cells, sizeof(cells));
    *len = 6 + sizeof(cells);
  }
};

static Screen screen;


#endif // TENSILE_DISPLAY_SCREEN_H
