#ifndef TENSILE_DISPLAY_SCREENSAVER_H
#define TENSILE_DISPLAY_SCREENSAVER_H

#pragma once
// TensileOS Screensaver Suite: Matrix Digital Rain, 3D Warp Starfield, Bouncing Logo
#include "display/screen.h"
#include "core/input.h"

enum SaverMode {
  SAVER_MATRIX = 0,
  SAVER_STARFIELD = 1,
  SAVER_BOUNCE = 2,
  SAVER_OFF = 99
};

static int g_saverMode = SAVER_MATRIX;
static uint32_t g_saverTimeoutMs = 60000; // 60 seconds idle

// ================= 1. Matrix Digital Rain =================
struct MatrixCol {
  int y;
  int len;
  int speed;
  int tick;
};

static void runMatrixRain(Cell* saved) {
  MatrixCol cols[SCR_COLS];
  for (int x = 0; x < SCR_COLS; x++) {
    cols[x].y = -(rnd(SCR_ROWS));
    cols[x].len = 4 + rnd(10);
    cols[x].speed = 1 + rnd(3);
    cols[x].tick = 0;
  }

  screen.clearAll();
  screen.showCursor(false);

  for (;;) {
    uint8_t k = readKey(40);
    if (k != 0) break; // any key or mouse dismisses

    screen.lock();
    for (int x = 0; x < SCR_COLS; x++) {
      cols[x].tick++;
      if (cols[x].tick < cols[x].speed) continue;
      cols[x].tick = 0;

      int y = cols[x].y;
      // Head
      if (y >= 0 && y < SCR_ROWS) {
        uint8_t glyph = 0x21 + rnd(94); // printable ASCII / CP437
        screen.put(x, y, glyph, 0x0F); // bright white
      }
      // Trail
      for (int t = 1; t <= cols[x].len; t++) {
        int ty = y - t;
        if (ty >= 0 && ty < SCR_ROWS) {
          uint8_t a = (t < 3) ? 0x0A : (t < cols[x].len - 2 ? 0x02 : 0x08);
          screen.cells[ty * SCR_COLS + x].a = a;
        }
      }
      // Erase tail end
      int ey = y - cols[x].len - 1;
      if (ey >= 0 && ey < SCR_ROWS) {
        screen.put(x, ey, ' ', 0x00);
      }

      cols[x].y++;
      if (cols[x].y - cols[x].len > SCR_ROWS) {
        cols[x].y = 0;
        cols[x].len = 4 + rnd(10);
        cols[x].speed = 1 + rnd(3);
      }
    }
    screen.unlock();
  }
}

// ================= 2. 3D Warp Starfield =================
struct Star3D {
  float x, y, z;
};

static void runStarfield(Cell* saved) {
  enum { NSTARS = 80 };
  Star3D stars[NSTARS];
  for (int i = 0; i < NSTARS; i++) {
    stars[i].x = (rnd(1600) - 800) / 10.0f;
    stars[i].y = (rnd(1000) - 500) / 10.0f;
    stars[i].z = 1.0f + rnd(100) / 10.0f;
  }

  screen.clearAll();
  screen.showCursor(false);

  for (;;) {
    uint8_t k = readKey(33); // ~30 fps
    if (k != 0) break;

    screen.lock();
    screen.clearAll();
    int cx = SCR_COLS / 2;
    int cy = SCR_ROWS / 2;

    for (int i = 0; i < NSTARS; i++) {
      stars[i].z -= 0.35f;
      if (stars[i].z <= 0.1f) {
        stars[i].x = (rnd(1600) - 800) / 10.0f;
        stars[i].y = (rnd(1000) - 500) / 10.0f;
        stars[i].z = 10.0f;
      }

      int sx = cx + (int)(stars[i].x / stars[i].z);
      int sy = cy + (int)(stars[i].y / stars[i].z);

      if (sx >= 0 && sx < SCR_COLS && sy >= 0 && sy < SCR_ROWS) {
        uint8_t glyph = '.';
        uint8_t a = 0x08; // dark grey
        if (stars[i].z < 3.0f) { glyph = '*'; a = 0x0F; } // bright white
        else if (stars[i].z < 6.0f) { glyph = '+'; a = 0x07; } // light grey
        else if (stars[i].z < 8.0f) { glyph = (uint8_t)0xFA; a = 0x08; }
        screen.put(sx, sy, glyph, a);
      } else {
        stars[i].z = 0; // reset next frame
      }
    }
    screen.unlock();
  }
}

// ================= 3. Bouncing DVD-style Logo =================
static void runBouncingLogo(Cell* saved) {
  int x = 20, y = 8;
  int vx = 1, vy = 1;
  const int bw = 16, bh = 5;
  uint8_t colorIdx = 1;
  static const uint8_t colors[] = { 0x1F, 0x2F, 0x4F, 0x5F, 0x6F, 0x3F };

  screen.clearAll();
  screen.showCursor(false);

  for (;;) {
    uint8_t k = readKey(50);
    if (k != 0) break;

    screen.lock();
    screen.fillRect(x, y, bw, bh, ' ', 0x00);

    x += vx;
    y += vy;
    bool bounce = false;
    if (x <= 0 || x + bw >= SCR_COLS) { vx = -vx; bounce = true; }
    if (y <= 1 || y + bh >= SCR_ROWS) { vy = -vy; bounce = true; }
    if (bounce) {
      colorIdx = (colorIdx + 1) % (sizeof(colors) / sizeof(colors[0]));
    }
    x = constrain(x, 0, SCR_COLS - bw);
    y = constrain(y, 1, SCR_ROWS - bh);

    uint8_t a = colors[colorIdx];
    screen.box(x, y, bw, bh, a, true);
    screen.text(x + 3, y + 2, "TensileOS", a);
    screen.unlock();
  }
}

static void launchScreensaver(int mode = -1) {
  if (mode < 0) mode = g_saverMode;
  if (mode == SAVER_OFF) return;

  Cell* saved = (Cell*)malloc(sizeof(Cell) * SCR_CELLS);
  if (saved) screen.snapshot(saved);

  if (mode == SAVER_MATRIX) runMatrixRain(saved);
  else if (mode == SAVER_STARFIELD) runStarfield(saved);
  else if (mode == SAVER_BOUNCE) runBouncingLogo(saved);

  if (saved) {
    screen.restore(saved);
    free(saved);
  }
}


#endif // TENSILE_DISPLAY_SCREENSAVER_H
