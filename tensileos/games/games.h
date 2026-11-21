#ifndef TENSILE_GAMES_GAMES_H
#define TENSILE_GAMES_GAMES_H

#pragma once
// Games: Snake, Minesweeper, Tetris. All run on the 80x25 text screen (cells are 2 chars wide to look square).
#include "apps/apps.h"

// ================= Snake =================
class Snake : public App {
 public:
  enum { GW = 20, GH = 16 };
  int8_t bx[GW * GH], by[GW * GH];
  int len = 0, dir = 1, ndir = 1, fx = 0, fy = 0, score = 0, best = 0;
  bool over = false, paused = false;
  uint32_t last = 0;
  int lastBoardX = 0, lastBoardY = 0;

  Snake() { prefW = 30; prefH = 22; tickMs = 30; fixed = false; reset(); }
  const char* title() override { return "Snake"; }
  bool onSnake(int x, int y, int skip) { for (int i = 0; i < len - skip; i++) if (bx[i] == x && by[i] == y) return true; return false; }
  void food() {
    if (len >= GW * GH) return;
    do { fx = rnd(GW); fy = rnd(GH); } while (onSnake(fx, fy, 0));
  }
  void reset() {
    len = 3; for (int i = 0; i < 3; i++) { bx[i] = GW / 2 - i; by[i] = GH / 2; }
    dir = ndir = 1; score = 0; over = false; paused = false; last = millis(); food();
  }
  void step() {
    static const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
    dir = ndir;
    int nx = bx[0] + dx[dir], ny = by[0] + dy[dir];
    if (nx < 0 || ny < 0 || nx >= GW || ny >= GH || onSnake(nx, ny, 1)) {
      over = true;
      if (score > best) best = score;
      soundError();
      return;
    }
    bool eat = (nx == fx && ny == fy);
    if (eat && len < GW * GH) len++;
    for (int i = len - 1; i > 0; i--) { bx[i] = bx[i - 1]; by[i] = by[i - 1]; }
    bx[0] = nx; by[0] = ny;
    if (eat) { score += 10; food(); soundEat(); }
  }
  void tick(uint32_t now) override {
    if (!focused || paused || over) return;
    int iv = 150 - score; if (iv < 60) iv = 60;
    if (now - last < (uint32_t)iv) return;
    last = now; step();
  }
  void key(uint8_t k) override {
    int nd = -1;
    if (k == KEY_UP || k == 'w' || k == 'W') nd = 0;
    else if (k == KEY_RIGHT || k == 'd' || k == 'D') nd = 1;
    else if (k == KEY_DOWN || k == 's' || k == 'S') nd = 2;
    else if (k == KEY_LEFT || k == 'a' || k == 'A') nd = 3;
    if (nd >= 0 && (nd + 2) % 4 != dir) ndir = nd;
    else if (k == 'p' || k == 'P') paused = !paused;
    else if (k == 'r' || k == 'R') reset();
    else if (k == KEY_ESC) closeReq = true;
  }
  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;
    if (over) { reset(); return; }
    int hx = lastBoardX + bx[0] * 8 + 4;
    int hy = lastBoardY + by[0] * 8 + 4;
    int dx = px - hx, dy = py - hy;
    if (abs(dx) > abs(dy) * 2) {
      int nd = dx > 0 ? 1 : 3;
      if ((nd + 2) % 4 != dir) ndir = nd;
    } else {
      int nd = dy > 0 ? 2 : 0;
      if ((nd + 2) % 4 != dir) ndir = nd;
    }
  }
  void mouse(int, int, uint8_t, uint8_t) override {}

  void draw(View& v) override {
    if (!v.pixelMode) {
      v.fill(0x70);
      char b[64]; snprintf(b, sizeof b, " Score %d   Best %d    P pause  R restart", score, best);
      v.text(0, 0, b, 0x70);
      for (int y = 0; y < GH; y++) for (int x = 0; x < GW; x++) {
        char ch = (x == fx && y == fy) ? '*' : (onSnake(x, y, 0) ? 'O' : '.');
        v.put(x, 2 + y, ch, 0x78);
      }
      return;
    }

    int cw = v.pw > 0 ? v.pw : (v.w * 6);
    int ch = v.ph > 0 ? v.ph : (v.h * 8);

    gfx.fillRect(v.px, v.py, cw, ch, COL_SILVER);

    int boardW = GW * 8; // 160
    int boardH = GH * 8; // 128
    int boardX = (cw >= boardW) ? (v.px + (cw - boardW) / 2) : (v.px + 2);
    int startY = (ch >= 172) ? (v.py + (ch - 172) / 2) : (v.py + 2);
    int headerY = startY;
    int boardY = startY + 20;
    int infoY = boardY + boardH + 4;

    lastBoardX = boardX - v.px;
    lastBoardY = boardY - v.py;

    // Header: Score & Best (Sunken LED Display)
    gfx.fillRect(boardX, headerY, 74, 16, COL_BLACK);
    gfx.drawBevelBox(boardX, headerY, 74, 16, true);
    char scBuf[16]; snprintf(scBuf, sizeof(scBuf), "Score:%03d", score);
    gfx.drawText6x8(boardX + 6, headerY + 4, scBuf, COL_GREEN, COL_BLACK);

    gfx.fillRect(boardX + boardW - 74, headerY, 74, 16, COL_BLACK);
    gfx.drawBevelBox(boardX + boardW - 74, headerY, 74, 16, true);
    char bestBuf[16]; snprintf(bestBuf, sizeof(bestBuf), "Best:%03d", best);
    gfx.drawText6x8(boardX + boardW - 68, headerY + 4, bestBuf, COL_YELLOW, COL_BLACK);

    // Board Frame & Canvas
    gfx.drawBevelBox(boardX - 2, boardY - 2, boardW + 4, boardH + 4, true);
    gfx.fillRect(boardX, boardY, boardW, boardH, COL_BLACK);

    // Subtle grid dots
    for (int gy = 1; gy < GH; gy++) {
      for (int gx = 1; gx < GW; gx++) {
        gfx.setPixelFast(boardX + gx * 8, boardY + gy * 8, 0x08);
      }
    }

    // Food (3D Apple with green leaf and shine)
    int foodX = boardX + fx * 8;
    int foodY = boardY + fy * 8;
    gfx.fillRect(foodX + 1, foodY + 2, 6, 5, COL_RED);
    gfx.setPixelFast(foodX + 2, foodY + 1, COL_RED);
    gfx.setPixelFast(foodX + 5, foodY + 1, COL_RED);
    gfx.setPixelFast(foodX + 2, foodY + 3, COL_WHITE);
    gfx.drawVLine(foodX + 4, foodY, 2, 0x06);
    gfx.setPixelFast(foodX + 5, foodY, COL_GREEN);

    // Snake Body & Head
    for (int i = len - 1; i >= 0; i--) {
      int sx0 = boardX + bx[i] * 8;
      int sy0 = boardY + by[i] * 8;
      if (i == 0) {
        // Head: bright green with 3D bevel and eyes
        gfx.fillRect(sx0, sy0, 8, 8, COL_GREEN);
        gfx.drawHLine(sx0, sy0, 7, COL_WHITE);
        gfx.drawVLine(sx0, sy0, 7, COL_WHITE);
        gfx.drawHLine(sx0 + 1, sy0 + 7, 7, 0x02);
        gfx.drawVLine(sx0 + 7, sy0 + 1, 7, 0x02);
        if (dir == 1) { // Right
          gfx.setPixelFast(sx0 + 5, sy0 + 2, COL_BLACK);
          gfx.setPixelFast(sx0 + 5, sy0 + 5, COL_BLACK);
        } else if (dir == 3) { // Left
          gfx.setPixelFast(sx0 + 2, sy0 + 2, COL_BLACK);
          gfx.setPixelFast(sx0 + 2, sy0 + 5, COL_BLACK);
        } else if (dir == 0) { // Up
          gfx.setPixelFast(sx0 + 2, sy0 + 2, COL_BLACK);
          gfx.setPixelFast(sx0 + 5, sy0 + 2, COL_BLACK);
        } else { // Down
          gfx.setPixelFast(sx0 + 2, sy0 + 5, COL_BLACK);
          gfx.setPixelFast(sx0 + 5, sy0 + 5, COL_BLACK);
        }
      } else {
        // Body segment
        gfx.fillRect(sx0, sy0, 8, 8, 0x02);
        gfx.drawHLine(sx0, sy0, 7, COL_GREEN);
        gfx.drawVLine(sx0, sy0, 7, COL_GREEN);
        gfx.drawHLine(sx0 + 1, sy0 + 7, 7, COL_BLACK);
        gfx.drawVLine(sx0 + 7, sy0 + 1, 7, COL_BLACK);
      }
    }

    // Overlays
    if (over) {
      gfx.fillRect(boardX + 20, boardY + 48, 120, 32, COL_SILVER);
      gfx.drawBevelBox(boardX + 20, boardY + 48, 120, 32, false);
      gfx.drawText6x8(boardX + 38, boardY + 54, "GAME OVER!", COL_RED, COL_SILVER);
      gfx.drawText6x8(boardX + 26, boardY + 66, "Press [R] to retry", COL_BLACK, COL_SILVER);
    } else if (paused) {
      gfx.fillRect(boardX + 35, boardY + 52, 90, 24, COL_SILVER);
      gfx.drawBevelBox(boardX + 35, boardY + 52, 90, 24, false);
      gfx.drawText6x8(boardX + 50, boardY + 60, "PAUSED", COL_BLUE, COL_SILVER);
    }

    // Bottom Status & Controls
    gfx.fillRect(boardX - 2, infoY, boardW + 4, 20, COL_SILVER);
    gfx.drawBevelBox(boardX - 2, infoY, boardW + 4, 20, true);
    if (over) {
      gfx.drawText6x8(boardX + 4, infoY + 6, "* CRASH! * Press [R] to Restart", COL_RED, COL_SILVER);
    } else if (paused) {
      gfx.drawText6x8(boardX + 16, infoY + 6, "PAUSED - Press [P] to Resume", COL_BLUE, COL_SILVER);
    } else {
      gfx.drawText6x8(boardX + 4, infoY + 6, "Arrows/WASD: Move  [R] New", COL_BLACK, COL_SILVER);
    }
  }
};

// ================= Minesweeper =================
class Mines : public App {
 public:
  enum { W = 12, H = 10, M = 16 };
  uint8_t mine[W * H], num[W * H], st[W * H];   // st: 0 hidden, 1 open, 2 flag
  int cx = 0, cy = 0, flags = 0, opened = 0;
  bool over = false, won = false, first = true;
  uint32_t t0 = 0, t1 = 0;
  int lastFieldX = 0, lastFieldY = 0, lastHeaderY = 0, lastInfoY = 0;

  Mines() { prefW = 28; prefH = 22; tickMs = 500; fixed = false; reset(); }
  const char* title() override { return "Minesweeper"; }
  void reset() { memset(mine, 0, sizeof mine); memset(num, 0, sizeof num); memset(st, 0, sizeof st); flags = opened = 0; over = won = false; first = true; t0 = t1 = 0; }
  void place(int sx, int sy) {
    for (int m = 0; m < M;) {
      int x = rnd(W), y = rnd(H);
      if (mine[y * W + x] || (abs(x - sx) <= 1 && abs(y - sy) <= 1)) continue;
      mine[y * W + x] = 1; m++;
    }
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
      int c = 0;
      for (int j = -1; j <= 1; j++) for (int i = -1; i <= 1; i++) {
        int nx = x + i, ny = y + j;
        if (nx >= 0 && ny >= 0 && nx < W && ny < H) c += mine[ny * W + nx];
      }
      num[y * W + x] = c;
    }
  }
  void reveal(int x, int y) {
    if (over || won) return;
    int p = y * W + x;
    if (st[p] == 2) return;
    if (first) { first = false; place(x, y); t0 = millis(); }
    if (st[p] == 1) {
      if (num[p] > 0) {
        int fc = 0;
        for (int j = -1; j <= 1; j++) for (int i = -1; i <= 1; i++) {
          int nx = x + i, ny = y + j;
          if (nx >= 0 && ny >= 0 && nx < W && ny < H && st[ny * W + nx] == 2) fc++;
        }
        if (fc == num[p]) {
          for (int j = -1; j <= 1; j++) for (int i = -1; i <= 1; i++) {
            int nx = x + i, ny = y + j;
            if (nx >= 0 && ny >= 0 && nx < W && ny < H && st[ny * W + nx] == 0) reveal(nx, ny);
          }
        }
      }
      return;
    }
    if (mine[p]) { over = true; t1 = millis(); soundExplosion(); return; }
    int stk[W * H], sp = 0; stk[sp++] = p; st[p] = 1; opened++;
    while (sp) {
      int cur = stk[--sp];
      if (num[cur]) continue;
      int px = cur % W, py = cur / W;
      for (int j = -1; j <= 1; j++) for (int i = -1; i <= 1; i++) {
        int nx = px + i, ny = py + j;
        if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue;
        int q = ny * W + nx;
        if (st[q] == 0) { st[q] = 1; opened++; stk[sp++] = q; }
      }
    }
    if (opened == W * H - M) { won = true; t1 = millis(); soundLineClear(); }
    else soundClick();
  }
  void flag(int x, int y) {
    if (over || won) return;
    uint8_t& s = st[y * W + x];
    if (s == 0) { s = 2; flags++; soundClick(); } else if (s == 2) { s = 0; flags--; soundClick(); }
  }
  void key(uint8_t k) override {
    if (k == KEY_LEFT && cx > 0) cx--;
    else if (k == KEY_RIGHT && cx < W - 1) cx++;
    else if (k == KEY_UP && cy > 0) cy--;
    else if (k == KEY_DOWN && cy < H - 1) cy++;
    else if (k == ' ' || k == KEY_ENTER) reveal(cx, cy);
    else if (k == 'f' || k == 'F') flag(cx, cy);
    else if (k == 'r' || k == 'R') reset();
    else if (k == KEY_ESC) closeReq = true;
  }
  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!edge) return;
    if (px >= lastFieldX + 64 && px <= lastFieldX + 80 && py >= lastHeaderY && py <= lastHeaderY + 16) {
      reset();
      soundClick();
      return;
    }
    if ((over || won) && py >= lastInfoY && py <= lastInfoY + 24) {
      reset();
      soundClick();
      return;
    }
    int col = (px - lastFieldX) / 12;
    int row = (py - lastFieldY) / 12;
    if (col >= 0 && col < W && row >= 0 && row < H) {
      cx = col; cy = row;
      if (edge & 1) reveal(col, row);
      else if (edge & 2) flag(col, row);
    }
  }
  void mouse(int, int, uint8_t, uint8_t) override {}
  void draw(View& v) override {
    if (!v.pixelMode) {
      v.fill(0x70);
      uint32_t el = first ? 0 : ((over || won ? t1 : millis()) - t0) / 1000;
      char b[48]; snprintf(b, sizeof b, " Mines %2d  Time %3u  %s", M - flags, (unsigned)el, over ? "BOOM!" : won ? "WIN!" : "");
      v.text(0, 0, b, over ? 0x74 : 0x70);
      for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
        int p = y * W + x; char c1 = '#', c2 = '#'; uint8_t a = 0x78;
        if (st[p] == 1) { c1 = ' '; c2 = num[p] ? ('0' + num[p]) : ' '; a = 0x71; }
        else if (over && mine[p]) { c1 = '*'; c2 = ' '; a = 0x4F; }
        else if (st[p] == 2) { c1 = 'F'; c2 = ' '; a = 0x74; }
        if (x == cx && y == cy) a = 0x2F;
        v.put(1 + 2 * x, 2 + y, c1, a); v.put(2 + 2 * x, 2 + y, c2, a);
      }
      v.text(0, H + 2, " Mouse: [L]Open [R]Flag", 0x78);
      v.text(0, H + 3, " Restart: [R] or Click :)", 0x78);
      return;
    }

    int cw = v.pw > 0 ? v.pw : (v.w * 6);
    int ch = v.ph > 0 ? v.ph : (v.h * 8);

    gfx.fillRect(v.px, v.py, cw, ch, COL_SILVER);

    int fieldW = W * 12;
    int fieldH = H * 12;
    int fieldX = (cw >= fieldW) ? (v.px + (cw - fieldW) / 2) : (v.px + 2);
    int startY = (ch >= 171) ? (v.py + (ch - 171) / 2) : (v.py + 3);
    int headerY = startY;
    int fieldY = startY + 23;
    int infoY = fieldY + fieldH + 4;

    lastFieldX = fieldX - v.px;
    lastFieldY = fieldY - v.py;
    lastHeaderY = headerY - v.py;
    lastInfoY = infoY - v.py;

    gfx.drawBevelBox(fieldX - 2, headerY - 2, fieldW + 4, 20, true);

    gfx.fillRect(fieldX + 2, headerY + 1, 28, 14, COL_BLACK);
    int remM = M - flags;
    if (remM < 0) remM = 0;
    char mBuf[6]; snprintf(mBuf, sizeof(mBuf), "%03d", remM);
    gfx.drawText6x8(fieldX + 5, headerY + 4, mBuf, COL_RED, COL_BLACK);

    int smX = fieldX + 64;
    gfx.fillRect(smX, headerY, 16, 16, COL_SILVER);
    gfx.drawBevelBox(smX, headerY, 16, 16, false);
    gfx.fillRect(smX + 4, headerY + 2, 8, 12, COL_YELLOW);
    gfx.fillRect(smX + 2, headerY + 4, 12, 8, COL_YELLOW);
    if (won) {
      gfx.fillRect(smX + 3, headerY + 5, 10, 3, COL_BLACK);
      gfx.drawHLine(smX + 5, headerY + 11, 6, COL_BLACK);
    } else if (over) {
      gfx.setPixelFast(smX + 4, headerY + 4, COL_BLACK);
      gfx.setPixelFast(smX + 6, headerY + 4, COL_BLACK);
      gfx.setPixelFast(smX + 5, headerY + 5, COL_BLACK);
      gfx.setPixelFast(smX + 4, headerY + 6, COL_BLACK);
      gfx.setPixelFast(smX + 6, headerY + 6, COL_BLACK);

      gfx.setPixelFast(smX + 9, headerY + 4, COL_BLACK);
      gfx.setPixelFast(smX + 11, headerY + 4, COL_BLACK);
      gfx.setPixelFast(smX + 10, headerY + 5, COL_BLACK);
      gfx.setPixelFast(smX + 9, headerY + 6, COL_BLACK);
      gfx.setPixelFast(smX + 11, headerY + 6, COL_BLACK);
      gfx.drawHLine(smX + 6, headerY + 11, 4, COL_BLACK);
    } else {
      gfx.fillRect(smX + 4, headerY + 4, 2, 2, COL_BLACK);
      gfx.fillRect(smX + 10, headerY + 4, 2, 2, COL_BLACK);
      gfx.drawHLine(smX + 5, headerY + 11, 6, COL_BLACK);
      gfx.setPixelFast(smX + 4, headerY + 10, COL_BLACK);
      gfx.setPixelFast(smX + 11, headerY + 10, COL_BLACK);
    }

    gfx.fillRect(fieldX + fieldW - 30, headerY + 1, 28, 14, COL_BLACK);
    uint32_t el = first ? 0 : ((over || won ? t1 : millis()) - t0) / 1000;
    if (el > 999) el = 999;
    char tBuf[6]; snprintf(tBuf, sizeof(tBuf), "%03d", (unsigned)el);
    gfx.drawText6x8(fieldX + fieldW - 27, headerY + 4, tBuf, COL_RED, COL_BLACK);

    gfx.drawBevelBox(fieldX - 2, fieldY - 2, fieldW + 4, fieldH + 4, true);

    static const uint8_t numCols[9] = {
      COL_SILVER,
      COL_BLUE,   // 1: Blue
      COL_GREEN,  // 2: Green
      COL_RED,    // 3: Red
      COL_NAVY,   // 4: Navy
      0x06,       // 5: Maroon
      COL_CYAN,   // 6: Cyan
      COL_BLACK,  // 7: Black
      COL_GREY    // 8: Grey
    };

    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        int cx0 = fieldX + x * 12;
        int cy0 = fieldY + y * 12;
        int p = y * W + x;

        if (st[p] == 1) {
          if (mine[p]) {
            gfx.fillRect(cx0, cy0, 12, 12, COL_RED);
            gfx.drawRect(cx0, cy0, 12, 12, COL_BLACK);
            gfx.fillRect(cx0 + 4, cy0 + 4, 4, 4, COL_BLACK);
            gfx.drawHLine(cx0 + 2, cy0 + 5, 8, COL_BLACK);
            gfx.drawVLine(cx0 + 5, cy0 + 2, 8, COL_BLACK);
            gfx.setPixelFast(cx0 + 5, cy0 + 5, COL_WHITE);
          } else {
            gfx.fillRect(cx0, cy0, 12, 12, COL_SILVER);
            gfx.drawRect(cx0, cy0, 12, 12, COL_GREY);
            if (num[p] > 0) {
              gfx.drawChar6x8(cx0 + 3, cy0 + 2, '0' + num[p], numCols[num[p]], COL_SILVER);
            }
          }
        } else {
          gfx.fillRect(cx0, cy0, 12, 12, COL_SILVER);
          gfx.drawHLine(cx0, cy0, 11, COL_WHITE);
          gfx.drawVLine(cx0, cy0, 11, COL_WHITE);
          gfx.drawHLine(cx0 + 1, cy0 + 11, 11, COL_GREY);
          gfx.drawVLine(cx0 + 11, cy0 + 1, 11, COL_GREY);

          if (over && mine[p]) {
            gfx.fillRect(cx0 + 4, cy0 + 4, 4, 4, COL_BLACK);
            gfx.drawHLine(cx0 + 3, cy0 + 5, 6, COL_BLACK);
            gfx.drawVLine(cx0 + 5, cy0 + 3, 6, COL_BLACK);
          } else if (st[p] == 2) {
            gfx.drawVLine(cx0 + 7, cy0 + 2, 7, COL_BLACK);
            gfx.drawHLine(cx0 + 5, cy0 + 9, 5, COL_BLACK);
            gfx.fillRect(cx0 + 4, cy0 + 3, 3, 3, COL_RED);
            if (over && !mine[p]) {
              gfx.drawChar6x8(cx0 + 3, cy0 + 2, 'X', COL_RED, 0xFF);
            }
          }
        }

        if (x == cx && y == cy) {
          gfx.drawRect(cx0 + 1, cy0 + 1, 10, 10, COL_NAVY);
        }
      }
    }

    // 6. Bottom Controls & Instructions Status Panel (Classic Classic 3D Sunken Well)
    gfx.fillRect(fieldX - 2, infoY, fieldW + 4, 24, COL_SILVER);
    gfx.drawBevelBox(fieldX - 2, infoY, fieldW + 4, 24, true);

    if (over) {
      gfx.drawText6x8(fieldX + 2, infoY + 3, "* BOOM! Game Over *", COL_RED, COL_SILVER);
      gfx.drawText6x8(fieldX + 2, infoY + 13, "Restart: [R] or Click :)", COL_BLACK, COL_SILVER);
    } else if (won) {
      gfx.drawText6x8(fieldX + 2, infoY + 3, "* YOU WON! Clean run! *", COL_GREEN, COL_SILVER);
      gfx.drawText6x8(fieldX + 2, infoY + 13, "Play again: [R] or :)", COL_BLACK, COL_SILVER);
    } else {
      gfx.drawText6x8(fieldX + 2, infoY + 3, "Mouse: [L]Open  [R]Flag", COL_NAVY, COL_SILVER);
      gfx.drawText6x8(fieldX + 2, infoY + 13, "Restart: [R] or Click :)", COL_BLACK, COL_SILVER);
    }
  }
};

// ================= Tetris =================
class Tetris : public App {
 public:
  enum { BW = 10, BH = 20 };
  uint8_t b[BH][BW];                 // 0 empty, else colour attr
  uint8_t shape[7][4][4][4];         // [piece][rotation][row][col]
  int sz[7] = {4, 2, 3, 3, 3, 3, 3};
  uint8_t pcol[7] = {0x0B, 0x0E, 0x0D, 0x0A, 0x0C, 0x09, 0x06};
  int cur, nxt, rot, px, py, score, lines, level;
  bool over, paused; uint32_t last;
  int lastBoardX = 0, lastBoardY = 0;

  Tetris() { prefW = 30; prefH = 23; tickMs = 30; fixed = false; build(); reset(); }
  const char* title() override { return "Tetris"; }
  void build() {
    static const char* base[7][4] = {
      {"....", "IIII", "....", "...."}, {"OO", "OO", 0, 0}, {".T.", "TTT", "...", 0}, {".SS", "SS.", "...", 0},
      {"ZZ.", ".ZZ", "...", 0}, {"J..", "JJJ", "...", 0}, {"..L", "LLL", "...", 0}};
    memset(shape, 0, sizeof shape);
    for (int p = 0; p < 7; p++) {
      int n = sz[p];
      for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) shape[p][0][i][j] = base[p][i][j] != '.';
      for (int r = 1; r < 4; r++)
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) shape[p][r][i][j] = shape[p][r - 1][n - 1 - j][i];
    }
  }
  bool fits(int p, int r, int x, int y) {
    for (int i = 0; i < sz[p]; i++) for (int j = 0; j < sz[p]; j++) if (shape[p][r][i][j]) {
      int gx = x + j, gy = y + i;
      if (gx < 0 || gx >= BW || gy >= BH) return false;
      if (gy >= 0 && b[gy][gx]) return false;
    }
    return true;
  }
  void spawn() {
    cur = nxt; nxt = rnd(7); rot = 0; px = (BW - sz[cur]) / 2; py = cur == 0 ? -1 : 0;
    if (!fits(cur, rot, px, py)) over = true;
  }
  void reset() {
    memset(b, 0, sizeof b); score = lines = 0; level = 1; over = paused = false;
    nxt = rnd(7); spawn(); last = millis();
  }
  void lockPiece() {
    for (int i = 0; i < sz[cur]; i++) for (int j = 0; j < sz[cur]; j++) if (shape[cur][rot][i][j]) {
      int gy = py + i, gx = px + j;
      if (gy >= 0) b[gy][gx] = pcol[cur];
    }
    int cleared = 0;
    for (int y = BH - 1; y >= 0; y--) {
      bool full = true;
      for (int x = 0; x < BW; x++) if (!b[y][x]) full = false;
      if (full) {
        memmove(b[1], b[0], BW * y); memset(b[0], 0, BW);
        cleared++; y++;
      }
    }
    static const int pts[5] = {0, 100, 300, 500, 800};
    score += pts[cleared] * level; lines += cleared; level = 1 + lines / 10;
    if (cleared > 0) soundLineClear(); else soundClick();
    spawn();
    if (over) soundError();
  }
  bool move(int dx, int dy) { if (fits(cur, rot, px + dx, py + dy)) { px += dx; py += dy; return true; } return false; }
  void tick(uint32_t now) override {
    if (!focused || paused || over) return;
    int iv = 700 - level * 55; if (iv < 90) iv = 90;
    if (now - last < (uint32_t)iv) return;
    last = now;
    if (!move(0, 1)) lockPiece();
  }
  void key(uint8_t k) override {
    if (k == 'r' || k == 'R') { reset(); return; }
    if (k == KEY_ESC) { closeReq = true; return; }
    if (k == 'p' || k == 'P') { paused = !paused; return; }
    if (over || paused) return;
    if (k == KEY_LEFT || k == 'a') move(-1, 0);
    else if (k == KEY_RIGHT || k == 'd') move(1, 0);
    else if (k == KEY_DOWN || k == 's') { if (move(0, 1)) score++; last = millis(); }
    else if (k == KEY_UP || k == 'w') {
      int nr = (rot + 1) & 3;
      static const int kick[5] = {0, -1, 1, -2, 2};
      for (int i = 0; i < 5; i++) if (fits(cur, nr, px + kick[i], py)) { px += kick[i]; rot = nr; soundRotate(); break; }
    } else if (k == ' ') { while (move(0, 1)) score += 2; lockPiece(); last = millis(); }
  }
  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;
    if (over) { reset(); return; }
    int bx0 = lastBoardX;
    int by0 = lastBoardY;
    if (px < bx0 + 28) move(-1, 0);
    else if (px > bx0 + 52 && px < bx0 + 80) move(1, 0);
    else if (py < by0 + 50) {
      int nr = (rot + 1) & 3;
      static const int kick[5] = {0, -1, 1, -2, 2};
      for (int i = 0; i < 5; i++) if (fits(cur, nr, this->px + kick[i], this->py)) { this->px += kick[i]; rot = nr; soundRotate(); break; }
    } else {
      if (move(0, 1)) score++; last = millis();
    }
  }
  void mouse(int, int, uint8_t, uint8_t) override {}

  void draw(View& v) override {
    if (!v.pixelMode) {
      v.fill(0x70);
      char b[64]; snprintf(b, sizeof b, " Score %d  Lines %d", score, lines);
      v.text(0, 0, b, 0x70);
      for (int y = 0; y < BH; y++) {
        for (int x = 0; x < BW; x++) {
          v.put(x, 1 + y, this->b[y][x] ? '#' : '.', 0x78);
        }
      }
      return;
    }

    int cw = v.pw > 0 ? v.pw : (v.w * 6);
    int ch = v.ph > 0 ? v.ph : (v.h * 8);

    gfx.fillRect(v.px, v.py, cw, ch, COL_SILVER);

    int boardW = BW * 8; // 80
    int boardH = BH * 8; // 160
    int totalW = boardW + 8 + 74; // 162
    int boardX = (cw >= totalW) ? (v.px + (cw - totalW) / 2) : (v.px + 2);
    int boardY = (ch >= boardH) ? (v.py + (ch - boardH) / 2) : (v.py + 2);
    int sideX = boardX + boardW + 8;

    lastBoardX = boardX - v.px;
    lastBoardY = boardY - v.py;

    // 1. Playfield Frame & Background (Sunken 3D Well)
    gfx.drawBevelBox(boardX - 2, boardY - 2, boardW + 4, boardH + 4, true);
    gfx.fillRect(boardX, boardY, boardW, boardH, COL_BLACK);

    // Subtle grid dots
    for (int gy = 1; gy < BH; gy++) {
      for (int gx = 1; gx < BW; gx++) {
        gfx.setPixelFast(boardX + gx * 8, boardY + gy * 8, 0x08);
      }
    }

    // Fixed locked blocks with 3D beveled styling
    for (int y = 0; y < BH; y++) {
      for (int x = 0; x < BW; x++) {
        if (this->b[y][x]) {
          int bx0 = boardX + x * 8;
          int by0 = boardY + y * 8;
          uint8_t c = this->b[y][x];
          gfx.fillRect(bx0, by0, 8, 8, c);
          gfx.drawHLine(bx0, by0, 7, COL_WHITE);
          gfx.drawVLine(bx0, by0, 7, COL_WHITE);
          gfx.drawHLine(bx0 + 1, by0 + 7, 7, COL_BLACK);
          gfx.drawVLine(bx0 + 7, by0 + 1, 7, COL_BLACK);
        }
      }
    }

    // Active falling piece with 3D beveled blocks
    if (!over) {
      for (int i = 0; i < sz[cur]; i++) {
        for (int j = 0; j < sz[cur]; j++) {
          if (shape[cur][rot][i][j]) {
            int gx = this->px + j;
            int gy = this->py + i;
            if (gy >= 0 && gx >= 0 && gx < BW && gy < BH) {
              int bx0 = boardX + gx * 8;
              int by0 = boardY + gy * 8;
              uint8_t c = pcol[cur];
              gfx.fillRect(bx0, by0, 8, 8, c);
              gfx.drawHLine(bx0, by0, 7, COL_WHITE);
              gfx.drawVLine(bx0, by0, 7, COL_WHITE);
              gfx.drawHLine(bx0 + 1, by0 + 7, 7, COL_BLACK);
              gfx.drawVLine(bx0 + 7, by0 + 1, 7, COL_BLACK);
            }
          }
        }
      }
    }

    // Game Over / Pause Dialogs on the board
    if (over) {
      gfx.fillRect(boardX + 5, boardY + 60, 70, 34, COL_SILVER);
      gfx.drawBevelBox(boardX + 5, boardY + 60, 70, 34, false);
      gfx.drawText6x8(boardX + 11, boardY + 66, "GAME OVER", COL_RED, COL_SILVER);
      gfx.drawText6x8(boardX + 9, boardY + 78, "Press [R]", COL_BLACK, COL_SILVER);
    } else if (paused) {
      gfx.fillRect(boardX + 10, boardY + 66, 60, 22, COL_SILVER);
      gfx.drawBevelBox(boardX + 10, boardY + 66, 60, 22, false);
      gfx.drawText6x8(boardX + 17, boardY + 73, "PAUSED", COL_BLUE, COL_SILVER);
    }

    // 2. Side Panel
    // SCORE
    gfx.drawText6x8(sideX, boardY, "SCORE", COL_NAVY, COL_SILVER);
    gfx.fillRect(sideX, boardY + 9, 70, 14, COL_BLACK);
    gfx.drawBevelBox(sideX, boardY + 9, 70, 14, true);
    char scBuf[16]; snprintf(scBuf, sizeof(scBuf), "%05d", score);
    gfx.drawText6x8(sideX + 6, boardY + 12, scBuf, COL_GREEN, COL_BLACK);

    // LINES
    gfx.drawText6x8(sideX, boardY + 27, "LINES", COL_NAVY, COL_SILVER);
    gfx.fillRect(sideX, boardY + 36, 70, 14, COL_BLACK);
    gfx.drawBevelBox(sideX, boardY + 36, 70, 14, true);
    char lnBuf[16]; snprintf(lnBuf, sizeof(lnBuf), "%03d", lines);
    gfx.drawText6x8(sideX + 6, boardY + 39, lnBuf, COL_YELLOW, COL_BLACK);

    // LEVEL
    gfx.drawText6x8(sideX, boardY + 54, "LEVEL", COL_NAVY, COL_SILVER);
    gfx.fillRect(sideX, boardY + 63, 70, 14, COL_BLACK);
    gfx.drawBevelBox(sideX, boardY + 63, 70, 14, true);
    char lvBuf[16]; snprintf(lvBuf, sizeof(lvBuf), "%02d", level);
    gfx.drawText6x8(sideX + 6, boardY + 66, lvBuf, COL_CYAN, COL_BLACK);

    // NEXT PIECE PREVIEW
    gfx.drawText6x8(sideX, boardY + 81, "NEXT", COL_NAVY, COL_SILVER);
    gfx.fillRect(sideX, boardY + 90, 70, 36, COL_BLACK);
    gfx.drawBevelBox(sideX, boardY + 90, 70, 36, true);
    int nsz = sz[nxt];
    int npx = sideX + (70 - nsz * 8) / 2;
    int npy = boardY + 92 + (32 - nsz * 8) / 2;
    for (int i = 0; i < nsz; i++) {
      for (int j = 0; j < nsz; j++) {
        if (shape[nxt][0][i][j]) {
          int bx0 = npx + j * 8;
          int by0 = npy + i * 8;
          uint8_t c = pcol[nxt];
          gfx.fillRect(bx0, by0, 8, 8, c);
          gfx.drawHLine(bx0, by0, 7, COL_WHITE);
          gfx.drawVLine(bx0, by0, 7, COL_WHITE);
          gfx.drawHLine(bx0 + 1, by0 + 7, 7, COL_BLACK);
          gfx.drawVLine(bx0 + 7, by0 + 1, 7, COL_BLACK);
        }
      }
    }

    // CONTROLS INSTRUCTIONS
    gfx.drawText6x8(sideX, boardY + 130, "< >: Move", COL_BLACK, COL_SILVER);
    gfx.drawText6x8(sideX, boardY + 140, " ^ : Rotate", COL_BLACK, COL_SILVER);
    gfx.drawText6x8(sideX, boardY + 150, "SPC: Drop", COL_BLACK, COL_SILVER);
  }
};


#endif // TENSILE_GAMES_GAMES_H
