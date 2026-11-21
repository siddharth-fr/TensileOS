#ifndef TENSILE_APPS_PAINT_H
#define TENSILE_APPS_PAINT_H

#pragma once
// TensileOS 2D Paint: 16-color VGA palette, pixel drawing canvas, tools & swatches
#include "core/ui.h"
#include "fs/tensilefs.h"
#include "core/sound.h"

class AnsiPaint : public App {
 public:
  enum { CW = 210, CH = 118 };
  uint8_t canvasData[(CW * CH) / 2]; // 4-bit nibbles: 12,390 bytes

  enum ToolType {
    TOOL_PEN = 0,
    TOOL_BRUSH,
    TOOL_ERASER,
    TOOL_FILL,
    TOOL_LINE,
    NUM_TOOLS
  };

  int curTool = TOOL_PEN;
  uint8_t curFg = COL_BLACK; // Black pen
  uint8_t curBg = COL_WHITE; // White canvas / secondary
  int brushWidth = 3;        // 1, 3, 5 px
  int lastPx = -1, lastPy = -1;
  bool isDragging = false;
  int lineStartX = -1, lineStartY = -1;
  char filename[32] = "/ART1.DAT";

  AnsiPaint(const char* file = NULL) {
    prefW = 48; prefH = 22;
    fixed = false;
    clearCanvas(COL_WHITE);
    if (file && file[0]) {
      strncpy(filename, file, sizeof(filename) - 1);
      filename[sizeof(filename) - 1] = 0;
      loadArt();
    }
  }

  const char* title() override { return "Paint"; }

  void clearCanvas(uint8_t col) {
    uint8_t byteVal = ((col & 0x0F) << 4) | (col & 0x0F);
    memset(canvasData, byteVal, sizeof(canvasData));
  }

  inline void setPixel(int x, int y, uint8_t c) {
    if (x < 0 || x >= CW || y < 0 || y >= CH) return;
    int idx = (y * CW + x) >> 1;
    if (x & 1) canvasData[idx] = (canvasData[idx] & 0xF0) | (c & 0x0F);
    else       canvasData[idx] = (canvasData[idx] & 0x0F) | ((c & 0x0F) << 4);
  }

  inline uint8_t getPixel(int x, int y) {
    if (x < 0 || x >= CW || y < 0 || y >= CH) return 0;
    int idx = (y * CW + x) >> 1;
    return (x & 1) ? (canvasData[idx] & 0x0F) : (canvasData[idx] >> 4);
  }

  void drawDot(int cx, int cy, uint8_t c, int r) {
    if (r <= 1) {
      setPixel(cx, cy, c);
    } else {
      int half = r / 2;
      for (int dy = -half; dy <= half; dy++) {
        for (int dx = -half; dx <= half; dx++) {
          setPixel(cx + dx, cy + dy, c);
        }
      }
    }
  }

  void drawLine(int x0, int y0, int x1, int y1, uint8_t c, int r) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
      drawDot(x0, y0, c, r);
      if (x0 == x1 && y0 == y1) break;
      e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
    }
  }

  void floodFill(int startX, int startY, uint8_t targetCol, uint8_t fillCol) {
    if (targetCol == fillCol) return;
    if (startX < 0 || startX >= CW || startY < 0 || startY >= CH) return;
    if (getPixel(startX, startY) != targetCol) return;

    struct Pt { int16_t x, y; };
    const int STACK_CAP = 384;
    Pt* stack = (Pt*)malloc(STACK_CAP * sizeof(Pt));
    if (!stack) return;
    int top = 0;
    stack[top++] = {(int16_t)startX, (int16_t)startY};

    while (top > 0) {
      Pt p = stack[--top];
      if (p.x < 0 || p.x >= CW || p.y < 0 || p.y >= CH) continue;
      if (getPixel(p.x, p.y) != targetCol) continue;

      int x1 = p.x;
      while (x1 > 0 && getPixel(x1 - 1, p.y) == targetCol) x1--;
      int x2 = p.x;
      while (x2 < CW - 1 && getPixel(x2 + 1, p.y) == targetCol) x2++;

      for (int x = x1; x <= x2; x++) {
        setPixel(x, p.y, fillCol);
        if (p.y > 0 && getPixel(x, p.y - 1) == targetCol && top < STACK_CAP - 2) {
          stack[top++] = {(int16_t)x, (int16_t)(p.y - 1)};
        }
        if (p.y < CH - 1 && getPixel(x, p.y + 1) == targetCol && top < STACK_CAP - 2) {
          stack[top++] = {(int16_t)x, (int16_t)(p.y + 1)};
        }
      }
    }
    free(stack);
  }

  void saveArt() {
    File f = LittleFS.open(filename, "w");
    if (f) {
      f.write(canvasData, sizeof(canvasData));
      f.close();
    }
  }

  void loadArt() {
    if (!LittleFS.exists(filename)) return;
    File f = LittleFS.open(filename, "r");
    if (f) {
      f.read(canvasData, sizeof(canvasData));
      f.close();
    }
  }

  void draw(View& v) override {
    v.fill(0x70);
    int px = v.px;
    int py = v.py;

    // 1. Top Toolbar at py + 2, h = 18
    int ty = py + 2;
    gfx.drawButton(px + 2, ty, 32, 18, "New", false);
    gfx.drawButton(px + 36, ty, 32, 18, "Pen", curTool == TOOL_PEN);
    gfx.drawButton(px + 70, ty, 38, 18, "Brush", curTool == TOOL_BRUSH);
    gfx.drawButton(px + 110, ty, 44, 18, "Eraser", curTool == TOOL_ERASER);
    gfx.drawButton(px + 156, ty, 36, 18, "Fill", curTool == TOOL_FILL);
    gfx.drawButton(px + 194, ty, 36, 18, "Line", curTool == TOOL_LINE);
    gfx.drawButton(px + 232, ty, 42, 18, "Clear", false);

    // 2. Canvas at px + 4, py + 22, w = CW = 210, h = CH = 118
    int cx = px + 4;
    int cy = py + 22;
    gfx.drawBevelBox(cx - 2, cy - 2, CW + 4, CH + 4, true);

    // Blit canvas pixels using fast row copy
    for (int y = 0; y < CH; y++) {
      int screenY = cy + y;
      if (screenY < 0 || screenY >= GFX_H) continue;
      int rowOffset = (y * CW) >> 1;
      if ((cx & 1) == 0 && cx >= 0 && (cx + CW) <= GFX_W) {
        int byteStart = screenY * GFX_PITCH + (cx >> 1);
        memcpy(gfx.fb + byteStart, canvasData + rowOffset, CW / 2);
        gfx.markDirty(screenY);
      } else {
        for (int x = 0; x < CW; x += 2) {
          uint8_t byteVal = canvasData[rowOffset + (x >> 1)];
          gfx.setPixelFast(cx + x, screenY, byteVal >> 4);
          gfx.setPixelFast(cx + x + 1, screenY, byteVal & 0x0F);
        }
      }
    }

    // 3. Side Tool Info / Size Panel at px + 222, cy - 2, w = 68, h = 122
    int sx = px + 222;
    int sy = cy - 2;
    gfx.drawBevelBox(sx, sy, 68, CH + 4, true);
    gfx.fillRect(sx + 2, sy + 2, 64, CH, COL_SILVER);

    gfx.drawText6x8(sx + 4, sy + 6, "TOOL:", COL_BLACK, COL_SILVER);
    const char* toolNames[] = { "PEN", "BRUSH", "ERASER", "FILL", "LINE" };
    gfx.drawText6x8(sx + 4, sy + 18, toolNames[curTool], COL_NAVY, COL_SILVER);

    gfx.drawText6x8(sx + 4, sy + 38, "WIDTH:", COL_BLACK, COL_SILVER);
    gfx.drawButton(sx + 4, sy + 50, 60, 16, "1 px", brushWidth == 1);
    gfx.drawButton(sx + 4, sy + 68, 60, 16, "3 px", brushWidth == 3);
    gfx.drawButton(sx + 4, sy + 86, 60, 16, "5 px", brushWidth == 5);

    // 4. Bottom Palette Bar at py + 144 to 174
    int palY = py + 144;
    // Active Dual-Color indicator
    gfx.drawBevelBox(px + 4, palY, 26, 26, true);
    gfx.fillRect(px + 6, palY + 2, 22, 22, COL_SILVER);

    // Background color swatch (bottom right)
    gfx.drawBevelBox(px + 14, palY + 12, 13, 13, true);
    gfx.fillRect(px + 15, palY + 13, 11, 11, curBg);

    // Foreground color swatch (top left)
    gfx.drawBevelBox(px + 6, palY + 4, 13, 13, false);
    gfx.fillRect(px + 7, palY + 5, 11, 11, curFg);

    // 16 VGA Swatches (2 rows of 8)
    const uint8_t palRow0[8] = { 0, 8, 4, 6, 2, 3, 1, 5 };
    const uint8_t palRow1[8] = { 7, 15, 12, 14, 10, 11, 9, 13 };

    for (int i = 0; i < 8; i++) {
      int swX = px + 34 + i * 15;
      // Row 0
      gfx.drawBevelBox(swX, palY, 14, 12, true);
      gfx.fillRect(swX + 1, palY + 1, 12, 10, palRow0[i]);
      // Row 1
      gfx.drawBevelBox(swX, palY + 14, 14, 12, true);
      gfx.fillRect(swX + 1, palY + 15, 12, 10, palRow1[i]);
    }

    // Status instructions
    gfx.drawText6x8(px + 160, palY + 4, "L-Click : Paint (FG)", COL_BLACK, COL_SILVER);
    gfx.drawText6x8(px + 160, palY + 14, "R-Click : Erase (BG)", COL_BLACK, COL_SILVER);
  }

  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override {
    int cx = 4;
    int cy = 22;

    // 1. Toolbar clicks
    if (py >= 2 && py <= 20) {
      uint8_t edge = b & ~prev;
      if (!(edge & 1)) return;
      if (px >= 2 && px <= 34) { clearCanvas(COL_WHITE); soundClick(); }
      else if (px >= 36 && px <= 68) { curTool = TOOL_PEN; soundClick(); }
      else if (px >= 70 && px <= 108) { curTool = TOOL_BRUSH; soundClick(); }
      else if (px >= 110 && px <= 154) { curTool = TOOL_ERASER; soundClick(); }
      else if (px >= 156 && px <= 192) { curTool = TOOL_FILL; soundClick(); }
      else if (px >= 194 && px <= 230) { curTool = TOOL_LINE; soundClick(); }
      else if (px >= 232 && px <= 274) { clearCanvas(COL_WHITE); soundClick(); }
      return;
    }

    // 2. Width selector buttons
    if (px >= 226 && px <= 286 && py >= 70 && py <= 126) {
      uint8_t edge = b & ~prev;
      if (!(edge & 1)) return;
      int sy = cy - 2;
      if (py >= sy + 50 && py <= sy + 66) { brushWidth = 1; soundClick(); }
      else if (py >= sy + 68 && py <= sy + 84) { brushWidth = 3; soundClick(); }
      else if (py >= sy + 86 && py <= sy + 102) { brushWidth = 5; soundClick(); }
      return;
    }

    // 3. Palette Swatches (py >= 144 && py <= 170)
    if (py >= 144 && py <= 170 && px >= 34 && px < 34 + 8 * 15) {
      int colIdx = (px - 34) / 15;
      if (colIdx >= 0 && colIdx < 8) {
        const uint8_t palRow0[8] = { 0, 8, 4, 6, 2, 3, 1, 5 };
        const uint8_t palRow1[8] = { 7, 15, 12, 14, 10, 11, 9, 13 };
        uint8_t selectedCol = (py < 156) ? palRow0[colIdx] : palRow1[colIdx];
        if (b & 1) { curFg = selectedCol; soundClick(); }
        else if (b & 2) { curBg = selectedCol; soundClick(); }
      }
      return;
    }

    // 4. Canvas interaction
    if (px >= cx && px < cx + CW && py >= cy && py < cy + CH) {
      int canvX = px - cx;
      int canvY = py - cy;
      uint8_t drawCol = (b & 2) ? curBg : curFg;

      if (b & 3) {
        if (curTool == TOOL_FILL) {
          uint8_t edge = b & ~prev;
          if (edge & 3) {
            floodFill(canvX, canvY, getPixel(canvX, canvY), drawCol);
            soundClick();
          }
        } else if (curTool == TOOL_PEN) {
          if (isDragging && lastPx >= 0) {
            drawLine(lastPx, lastPy, canvX, canvY, drawCol, 1);
          } else {
            setPixel(canvX, canvY, drawCol);
          }
          lastPx = canvX; lastPy = canvY; isDragging = true;
        } else if (curTool == TOOL_BRUSH) {
          if (isDragging && lastPx >= 0) {
            drawLine(lastPx, lastPy, canvX, canvY, drawCol, brushWidth);
          } else {
            drawDot(canvX, canvY, drawCol, brushWidth);
          }
          lastPx = canvX; lastPy = canvY; isDragging = true;
        } else if (curTool == TOOL_ERASER) {
          if (isDragging && lastPx >= 0) {
            drawLine(lastPx, lastPy, canvX, canvY, curBg, 6);
          } else {
            drawDot(canvX, canvY, curBg, 6);
          }
          lastPx = canvX; lastPy = canvY; isDragging = true;
        } else if (curTool == TOOL_LINE) {
          uint8_t edge = b & ~prev;
          if (edge & 3) {
            lineStartX = canvX;
            lineStartY = canvY;
          }
          lastPx = canvX; lastPy = canvY;
        }
      }
    }

    // Button released
    if (!b && prev) {
      if (curTool == TOOL_LINE && lineStartX >= 0 && lastPx >= 0) {
        drawLine(lineStartX, lineStartY, lastPx, lastPy, (prev & 2) ? curBg : curFg, brushWidth);
        lineStartX = -1; lineStartY = -1;
      }
      isDragging = false;
      lastPx = -1;
      lastPy = -1;
    }
  }

  void mouse(int, int, uint8_t, uint8_t) override {}

  void key(uint8_t k) override {
    if (k == 'c' || k == 'C') {
      clearCanvas(COL_WHITE);
      soundClick();
    } else if (k == 'p' || k == 'P') {
      curTool = TOOL_PEN; soundClick();
    } else if (k == 'b' || k == 'B') {
      curTool = TOOL_BRUSH; soundClick();
    } else if (k == 'e' || k == 'E') {
      curTool = TOOL_ERASER; soundClick();
    } else if (k == 'f' || k == 'F') {
      curTool = TOOL_FILL; soundClick();
    } else if (k == 'l' || k == 'L') {
      curTool = TOOL_LINE; soundClick();
    } else if (k == '1') {
      brushWidth = 1; soundClick();
    } else if (k == '3') {
      brushWidth = 3; soundClick();
    } else if (k == '5') {
      brushWidth = 5; soundClick();
    } else if (k == KEY_CTRL_S) {
      saveArt(); soundClick();
    } else if (k == KEY_CTRL_O) {
      loadArt(); soundClick();
    } else if (k == KEY_ESC) {
      closeReq = true;
    }
  }
};


#endif // TENSILE_APPS_PAINT_H
