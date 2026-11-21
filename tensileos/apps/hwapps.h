#ifndef TENSILE_APPS_HWAPPS_H
#define TENSILE_APPS_HWAPPS_H

#pragma once
// Hardware & diagnostic utilities: SysBench and Hardware Lab.
#include "core/ui.h"
#include "fs/tensilefs.h"
#include "core/sound.h"
#include <math.h>

// ==========================================
// SysBench: Hardware Benchmark Suite
// ==========================================
class SysBench : public App {
 public:
  enum TestStage {
    STAGE_IDLE = 0,
    STAGE_CPU_INT,
    STAGE_CPU_FLOAT,
    STAGE_RAM,
    STAGE_FLASH,
    STAGE_RENDER,
    STAGE_DONE
  };

  TestStage stage = STAGE_IDLE;
  uint32_t t0 = 0;
  uint32_t intScore = 0;
  float floatMflops = 0.0f;
  float ramMbs = 0.0f;
  float flashKbs = 0.0f;
  uint32_t renderFps = 0;
  uint32_t totalScore = 0;
  float chipTemp = 0.0f;
  char statusMsg[56] = "Press ENTER or Click [Run] to start";

  SysBench() {
    prefW = 54; prefH = 17;
    fixed = false;
    tickMs = 40;
  }
  const char* title() override { return "SysBench v1.0"; }

  // --- Benchmark execution functions ---
  void runIntTest() {
    uint32_t start = millis();
    // 1. Sieve of Eratosthenes up to 40,000
    const int N = 40000;
    uint8_t* sieve = (uint8_t*)malloc(N);
    if (sieve) {
      memset(sieve, 1, N);
      for (int p = 2; p * p < N; p++) {
        if (sieve[p]) {
          for (int i = p * p; i < N; i += p) sieve[i] = 0;
        }
      }
      free(sieve);
    }
    // 2. Heavy integer calculation
    volatile uint32_t acc = 0;
    for (uint32_t i = 1; i <= 200000; i++) {
      acc = (acc * 1664525UL + 1013904223UL) ^ (i * 31);
    }
    uint32_t elapsed = millis() - start;
    if (elapsed < 1) elapsed = 1;
    intScore = (uint32_t)(320000.0f / (float)elapsed);
  }

  void runFloatTest() {
    uint32_t start = millis();
    volatile float sum = 1.0f;
    const int ITERS = 150000;
    for (int i = 1; i <= ITERS; i++) {
      float f = (float)i * 0.001f;
      sum += sinf(f) * cosf(f) + sqrtf(fabsf(sum));
      if (sum > 1000.0f) sum = 1.0f;
    }
    uint32_t elapsed = millis() - start;
    if (elapsed < 1) elapsed = 1;
    float ops = (float)ITERS * 7.0f;
    floatMflops = (ops / ((float)elapsed / 1000.0f)) / 1000000.0f;
  }

  void runRamTest() {
    uint32_t start = millis();
    const size_t BUF_SIZE = 16384;
    uint32_t* buf = (uint32_t*)malloc(BUF_SIZE);
    if (!buf) { ramMbs = 0.0f; return; }
    const int PASSES = 1200;
    for (int p = 0; p < PASSES; p++) {
      for (size_t i = 0; i < BUF_SIZE / 4; i++) buf[i] = (uint32_t)(i + p);
      volatile uint32_t chk = 0;
      for (size_t i = 0; i < BUF_SIZE / 4; i++) chk ^= buf[i];
    }
    free(buf);
    uint32_t elapsed = millis() - start;
    if (elapsed < 1) elapsed = 1;
    float totalMb = (float)(BUF_SIZE * PASSES * 2) / (1024.0f * 1024.0f);
    ramMbs = totalMb / ((float)elapsed / 1000.0f);
  }

  void runFlashTest() {
    uint32_t start = millis();
    const char* testPath = "/SYSBENCH.TMP";
    File f = LittleFS.open(testPath, "w");
    if (!f) { flashKbs = 0.0f; return; }
    uint8_t chunk[512];
    memset(chunk, 0xAA, sizeof(chunk));
    for (int i = 0; i < 64; i++) f.write(chunk, sizeof(chunk));
    f.close();

    f = LittleFS.open(testPath, "r");
    if (f) {
      while (f.read(chunk, sizeof(chunk)) > 0) {}
      f.close();
    }
    LittleFS.remove(testPath);
    uint32_t elapsed = millis() - start;
    if (elapsed < 1) elapsed = 1;
    flashKbs = (64.0f * 1000.0f) / (float)elapsed;
  }

  void runRenderTest() {
    uint32_t start = millis();
    Cell simCells[SCR_CELLS];
    Cell simShadow[SCR_CELLS];
    memset(simShadow, 0, sizeof(simShadow));
    const int FRAMES = 350;
    for (int f = 0; f < FRAMES; f++) {
      for (int i = 0; i < SCR_CELLS; i++) {
        simCells[i].c = (uint8_t)('A' + ((i + f) % 26));
        simCells[i].a = (uint8_t)(0x10 | (f % 16));
      }
      int diffCount = 0;
      for (int i = 0; i < SCR_CELLS; i++) {
        if (simCells[i].c != simShadow[i].c || simCells[i].a != simShadow[i].a) diffCount++;
      }
      memcpy(simShadow, simCells, sizeof(simCells));
    }
    uint32_t elapsed = millis() - start;
    if (elapsed < 1) elapsed = 1;
    renderFps = (uint32_t)((float)FRAMES * 1000.0f / (float)elapsed);
  }

  void startBench() {
    stage = STAGE_CPU_INT;
    intScore = 0;
    floatMflops = 0.0f;
    ramMbs = 0.0f;
    flashKbs = 0.0f;
    renderFps = 0;
    totalScore = 0;
    snprintf(statusMsg, sizeof(statusMsg), "[1/5] Testing CPU Integer...");
  }

  void tick(uint32_t) override {
    chipTemp = temperatureRead();
    if (stage == STAGE_IDLE || stage == STAGE_DONE) return;

    switch (stage) {
      case STAGE_CPU_INT:
        runIntTest();
        stage = STAGE_CPU_FLOAT;
        snprintf(statusMsg, sizeof(statusMsg), "[2/5] Testing CPU Float MFLOPS...");
        break;
      case STAGE_CPU_FLOAT:
        runFloatTest();
        stage = STAGE_RAM;
        snprintf(statusMsg, sizeof(statusMsg), "[3/5] Testing 32-bit RAM Speed...");
        break;
      case STAGE_RAM:
        runRamTest();
        stage = STAGE_FLASH;
        snprintf(statusMsg, sizeof(statusMsg), "[4/5] Testing LittleFS Flash Disk...");
        break;
      case STAGE_FLASH:
        runFlashTest();
        stage = STAGE_RENDER;
        snprintf(statusMsg, sizeof(statusMsg), "[5/5] Testing Virtual Display GPU...");
        break;
      case STAGE_RENDER:
        runRenderTest();
        totalScore = intScore + (uint32_t)(floatMflops * 25.0f) + (uint32_t)(ramMbs * 3.5f) + (uint32_t)(flashKbs * 1.5f);
        stage = STAGE_DONE;
        snprintf(statusMsg, sizeof(statusMsg), "Benchmark Complete! Press R to rerun");
        break;
      default:
        break;
    }
  }

  void draw(View& v) override {
    v.fill(0x70);
    // Header
    v.fillRect(0, 0, v.w, 1, ' ', 0x1F);
    v.text(1, 0, "SysBench Hardware Performance Benchmark", 0x1F);

    char buf[64];
    snprintf(buf, sizeof(buf), "Chip: %s @ %u MHz   Temp: %.1f C", ESP.getChipModel(), (unsigned)ESP.getCpuFreqMHz(), chipTemp);
    v.text(1, 2, buf, 0x71);

    // Results table
    v.text(1, 4, "TEST METRIC            RESULT          RATING", 0x78);
    v.fillRect(1, 5, v.w - 2, 1, '-', 0x78);

    // 1. CPU Int
    if (stage > STAGE_CPU_INT || stage == STAGE_DONE) snprintf(buf, sizeof(buf), "%u pts", (unsigned)intScore);
    else if (stage == STAGE_CPU_INT) snprintf(buf, sizeof(buf), "testing...");
    else snprintf(buf, sizeof(buf), "-");
    v.text(1, 6, "CPU Integer Compute", 0x70);
    v.text(24, 6, buf, 0x71);

    // 2. CPU Float
    if (stage > STAGE_CPU_FLOAT || stage == STAGE_DONE) snprintf(buf, sizeof(buf), "%.1f MFLOPS", floatMflops);
    else if (stage == STAGE_CPU_FLOAT) snprintf(buf, sizeof(buf), "testing...");
    else snprintf(buf, sizeof(buf), "-");
    v.text(1, 7, "CPU Float (MFLOPS)", 0x70);
    v.text(24, 7, buf, 0x71);

    // 3. RAM
    if (stage > STAGE_RAM || stage == STAGE_DONE) snprintf(buf, sizeof(buf), "%.1f MB/s", ramMbs);
    else if (stage == STAGE_RAM) snprintf(buf, sizeof(buf), "testing...");
    else snprintf(buf, sizeof(buf), "-");
    v.text(1, 8, "RAM Memory Bandwidth", 0x70);
    v.text(24, 8, buf, 0x71);

    // 4. Flash
    if (stage > STAGE_FLASH || stage == STAGE_DONE) snprintf(buf, sizeof(buf), "%.1f KB/s", flashKbs);
    else if (stage == STAGE_FLASH) snprintf(buf, sizeof(buf), "testing...");
    else snprintf(buf, sizeof(buf), "-");
    v.text(1, 9, "Flash Disk (LittleFS)", 0x70);
    v.text(24, 9, buf, 0x71);

    // 5. Render
    if (stage == STAGE_DONE) snprintf(buf, sizeof(buf), "%u FPS", (unsigned)renderFps);
    else if (stage == STAGE_RENDER) snprintf(buf, sizeof(buf), "testing...");
    else snprintf(buf, sizeof(buf), "-");
    v.text(1, 10, "Text VRAM Renderer", 0x70);
    v.text(24, 10, buf, 0x71);

    v.fillRect(1, 11, v.w - 2, 1, '-', 0x78);

    if (stage == STAGE_DONE) {
      snprintf(buf, sizeof(buf), "TOTAL SCORE: %u pts", (unsigned)totalScore);
      v.text(1, 12, buf, 0x1E);
      float ratio486 = (float)totalScore / 950.0f;
      float ratioPent = (float)totalScore / 3200.0f;
      snprintf(buf, sizeof(buf), "Comparison: ~%.1fx Intel 486 DX2-66", ratio486);
      v.text(1, 13, buf, 0x72);
      snprintf(buf, sizeof(buf), "          : ~%.1fx Intel Pentium 100 MHz", ratioPent);
      v.text(1, 14, buf, 0x72);
    } else {
      v.fillRect(2, 12, v.w - 4, 1, ' ', 0x30);
      v.text(3, 12, stage == STAGE_IDLE ? "  [ RUN BENCHMARK (Enter) ]  " : "  RUNNING BENCHMARK...  ", 0x3F);
    }

    // Status bar
    v.fillRect(0, v.h - 1, v.w, 1, ' ', 0x30);
    v.text(1, v.h - 1, statusMsg, 0x30);
  }

  void key(uint8_t k) override {
    if (k == KEY_ENTER || k == 'r' || k == 'R') {
      if (stage == STAGE_IDLE || stage == STAGE_DONE) startBench();
    } else if (k == KEY_ESC) {
      closeReq = true;
    }
  }

  void mouse(int mx, int my, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if ((edge & 1) && (stage == STAGE_IDLE || stage == STAGE_DONE)) {
      if (my >= 12 && my <= 14 && mx >= 2 && mx <= 30) startBench();
    }
  }
};

// ==========================================
// Hardware Lab: Live GPIO & Virtual Oscilloscope
// ==========================================
class HwLab : public App {
 public:
  enum { HIST_LEN = 202 };
  int selPin = 0;
  uint8_t adcPin = 34; // GPIO 34 (ADC1_CH6 - safe with Wi-Fi)
  uint16_t adcHistory[HIST_LEN];
  int histIdx = 0;
  uint16_t currentMv = 0;
  uint16_t vMax = 0;
  uint16_t vMin = 3300;
  uint16_t vPp = 0;
  float chipTemp = 0.0f;
  bool paused = false;

  enum SignalSource {
    SRC_ADC34 = 0,
    SRC_ADC35,
    SRC_SINE,
    SRC_SQUARE,
    SRC_TRIANGLE,
    SRC_ECG,
    NUM_SOURCES
  };
  int curSource = SRC_ADC34;
  float simPhase = 0.0f;
  int timebaseIdx = 1; // 0: 20ms, 1: 40ms, 2: 80ms, 3: 150ms
  const uint32_t tickIntervals[4] = { 20, 40, 80, 150 };

  struct PinInfo {
    uint8_t pin;
    const char* label;
    bool isOutput;
    bool state;
  };

  PinInfo pins[6] = {
    {2,  "GPIO 02", true, false},
    {4,  "GPIO 04", true, false},
    {18, "GPIO 18", true, false},
    {19, "GPIO 19", true, false},
    {21, "GPIO 21", true, false},
    {22, "GPIO 22", true, false}
  };

  HwLab() {
    prefW = 50; prefH = 22;
    fixed = false;
    tickMs = tickIntervals[timebaseIdx];
    memset(adcHistory, 0, sizeof(adcHistory));
    for (int i = 0; i < 6; i++) {
      pinMode(pins[i].pin, OUTPUT);
      digitalWrite(pins[i].pin, pins[i].state ? HIGH : LOW);
    }
    pinMode(adcPin, INPUT);
    pinMode(35, INPUT);
  }

  const char* title() override { return "Hardware Lab"; }

  void togglePin(int idx) {
    if (idx < 0 || idx >= 6) return;
    pins[idx].state = !pins[idx].state;
    digitalWrite(pins[idx].pin, pins[idx].state ? HIGH : LOW);
  }

  void cycleSource() {
    curSource = (curSource + 1) % NUM_SOURCES;
    if (curSource == SRC_ADC34) { adcPin = 34; pinMode(34, INPUT); }
    else if (curSource == SRC_ADC35) { adcPin = 35; pinMode(35, INPUT); }
    memset(adcHistory, 0, sizeof(adcHistory));
  }

  void cycleTimebase() {
    timebaseIdx = (timebaseIdx + 1) % 4;
    tickMs = tickIntervals[timebaseIdx];
  }

  void tick(uint32_t) override {
    chipTemp = temperatureRead();
    if (paused) return;

    uint16_t sampleMv = 0;
    if (curSource == SRC_ADC34) {
      uint32_t raw = analogRead(34);
      sampleMv = (uint16_t)((raw * 3300UL) / 4095UL);
    } else if (curSource == SRC_ADC35) {
      uint32_t raw = analogRead(35);
      sampleMv = (uint16_t)((raw * 3300UL) / 4095UL);
    } else if (curSource == SRC_SINE) {
      simPhase += 0.25f;
      if (simPhase > 6.283185f) simPhase -= 6.283185f;
      sampleMv = (uint16_t)(1650.0f + 1400.0f * sinf(simPhase));
    } else if (curSource == SRC_SQUARE) {
      simPhase += 0.25f;
      if (simPhase > 6.283185f) simPhase -= 6.283185f;
      sampleMv = (sinf(simPhase) >= 0.0f) ? 3100 : 200;
    } else if (curSource == SRC_TRIANGLE) {
      simPhase += 0.25f;
      if (simPhase > 6.283185f) simPhase -= 6.283185f;
      float norm = simPhase / 6.283185f;
      sampleMv = (uint16_t)(200.0f + (norm < 0.5f ? (norm * 2.0f * 2900.0f) : ((1.0f - norm) * 2.0f * 2900.0f)));
    } else if (curSource == SRC_ECG) {
      simPhase += 0.22f;
      if (simPhase > 6.283185f) simPhase -= 6.283185f;
      float p = simPhase;
      float val = 0.0f;
      if (p < 0.8f) val = 0.2f * sinf(p * 3.14159f / 0.8f);
      else if (p < 1.1f) val = -0.15f;
      else if (p < 1.35f) val = 1.0f;
      else if (p < 1.55f) val = -0.3f;
      else if (p < 2.5f && p > 1.9f) val = 0.35f * sinf((p - 1.9f) * 3.14159f / 0.6f);
      sampleMv = (uint16_t)(1200.0f + val * 1800.0f);
    }

    currentMv = sampleMv;
    adcHistory[histIdx] = sampleMv;
    histIdx = (histIdx + 1) % HIST_LEN;

    uint16_t mx = 0, mn = 3300;
    for (int i = 0; i < HIST_LEN; i++) {
      uint16_t v = adcHistory[i];
      if (v > mx) mx = v;
      if (v < mn) mn = v;
    }
    vMax = mx; vMin = mn;
    vPp = (mx > mn) ? (mx - mn) : 0;
  }

  void draw(View& v) override {
    v.fill(0x70);

    int px = v.px;
    int py = v.py;

    // Header strip
    gfx.fillRect(px, py, v.pw, 19, COL_NAVY);
    gfx.drawText6x8(px + 4, py + 2, "Hardware Lab - Oscilloscope & Logic Analyzer", COL_WHITE, COL_NAVY);
    char sub[64];
    snprintf(sub, sizeof(sub), "ESP32-D0WDQ6 @ 240MHz  Temp: %.1f C  VDD: 3.3V", chipTemp);
    gfx.drawText6x8(px + 4, py + 11, sub, COL_SILVER, COL_NAVY);

    // Oscilloscope CRT Screen at px + 4, py + 21, w = 210, h = 80
    int ox = px + 4;
    int oy = py + 21;
    int ow = 210;
    int oh = 80;
    gfx.drawBevelBox(ox, oy, ow, oh, true);
    gfx.fillRect(ox + 2, oy + 2, ow - 4, oh - 4, COL_BLACK);

    // Phosphor Grid lines (dotted green graticule)
    for (int gy = 1; gy <= 3; gy++) {
      int yGrid = oy + 2 + gy * (oh - 4) / 4;
      for (int x = ox + 4; x < ox + ow - 4; x += 4) {
        gfx.setPixelFast(x, yGrid, COL_GREEN);
      }
    }
    for (int gx = 1; gx <= 5; gx++) {
      int xGrid = ox + 2 + gx * (ow - 4) / 6;
      for (int y = oy + 4; y < oy + oh - 4; y += 4) {
        gfx.setPixelFast(xGrid, y, COL_GREEN);
      }
    }

    // Oscilloscope Waveform Trace (Continuous bright lime line)
    int traceW = ow - 8; // 202 pixels
    int prevX = -1, prevY = -1;
    for (int i = 0; i < traceW; i++) {
      int sIdx = (histIdx - traceW + i + HIST_LEN) % HIST_LEN;
      uint16_t val = adcHistory[sIdx];
      int curY = (oy + oh - 6) - (int)((val * (uint32_t)(oh - 10)) / 3300UL);
      curY = constrain(curY, oy + 4, oy + oh - 6);
      int curX = ox + 4 + i;
      if (prevX >= 0) {
        gfx.drawLine(prevX, prevY, curX, curY, COL_LIME);
      } else {
        gfx.setPixelFast(curX, curY, COL_LIME);
      }
      prevX = curX;
      prevY = curY;
    }

    // Side Digital Measurements Panel at px + 218, py + 21, w = 80, h = 80
    int mx = px + 218;
    int my = py + 21;
    int mw = 80;
    int mh = 80;
    gfx.drawBevelBox(mx, my, mw, mh, true);
    gfx.fillRect(mx + 2, my + 2, mw - 4, mh - 4, COL_BLACK);

    const char* srcNames[] = { "ADC 34", "ADC 35", "SINE 1k", "SQUARE", "TRIANGLE", "CARDIAC" };
    char dbuf[32];
    snprintf(dbuf, sizeof(dbuf), "SRC:%s", srcNames[curSource]);
    gfx.drawText6x8(mx + 4, my + 5, dbuf, COL_YELLOW, COL_BLACK);

    snprintf(dbuf, sizeof(dbuf), "%4u mV", currentMv);
    gfx.drawText6x8(mx + 4, my + 17, dbuf, COL_LIME, COL_BLACK);

    snprintf(dbuf, sizeof(dbuf), "Vp:%.2fV", (float)vPp / 1000.0f);
    gfx.drawText6x8(mx + 4, my + 29, dbuf, COL_LIME, COL_BLACK);

    snprintf(dbuf, sizeof(dbuf), "Mx:%.2fV", (float)vMax / 1000.0f);
    gfx.drawText6x8(mx + 4, my + 41, dbuf, COL_CYAN, COL_BLACK);

    snprintf(dbuf, sizeof(dbuf), "Mn:%.2fV", (float)vMin / 1000.0f);
    gfx.drawText6x8(mx + 4, my + 53, dbuf, COL_CYAN, COL_BLACK);

    const char* tbNames[] = { " 20ms", " 40ms", " 80ms", "150ms" };
    snprintf(dbuf, sizeof(dbuf), "T :%s", tbNames[timebaseIdx]);
    gfx.drawText6x8(mx + 4, my + 65, dbuf, COL_SILVER, COL_BLACK);

    // Control Buttons Strip at py + 104, h = 18
    int cy = py + 104;
    gfx.drawButton(px + 4, cy, 50, 18, paused ? "RUN" : "STOP", paused);

    char srcBtn[16];
    snprintf(srcBtn, sizeof(srcBtn), "%s", srcNames[curSource]);
    gfx.drawButton(px + 58, cy, 84, 18, srcBtn, false);

    char tbBtn[16];
    snprintf(tbBtn, sizeof(tbBtn), "T:%s", tbNames[timebaseIdx]);
    gfx.drawButton(px + 146, cy, 74, 18, tbBtn, false);

    gfx.drawButton(px + 224, cy, 74, 18, "CLEAR", false);

    // GPIO Controller Panel at py + 125 to 175
    gfx.drawText6x8(px + 4, py + 125, "Digital GPIO Outputs (Click button to toggle):", COL_BLACK, COL_SILVER);

    for (int i = 0; i < 6; i++) {
      int col = i % 3;
      int row = i / 3;
      int bx = px + 4 + col * 98;
      int by = py + 137 + row * 20;
      int bw = 94;
      int bh = 18;

      bool st = pins[i].state;
      gfx.drawBevelBox(bx, by, bw, bh, st);
      gfx.fillRect(bx + 2, by + 2, bw - 4, bh - 4, st ? COL_WHITE : COL_SILVER);

      // LED square indicator
      gfx.drawRect(bx + 4, by + 4, 10, 10, COL_BLACK);
      gfx.fillRect(bx + 5, by + 5, 8, 8, st ? COL_LIME : COL_GREY);

      // Pin text
      char ptxt[24];
      snprintf(ptxt, sizeof(ptxt), "%s:%s", pins[i].label, st ? "ON " : "OFF");
      gfx.drawText6x8(bx + 18, by + 5, ptxt, COL_BLACK, st ? COL_WHITE : COL_SILVER);
    }
  }

  void mousePixel(int px, int py, uint8_t b, uint8_t prev) override {
    uint8_t edge = b & ~prev;
    if (!(edge & 1)) return;

    // Control buttons (py >= 104 && py <= 122)
    if (py >= 104 && py <= 122) {
      if (px >= 4 && px <= 54) {
        paused = !paused;
        soundClick();
        return;
      }
      if (px >= 58 && px <= 142) {
        cycleSource();
        soundClick();
        return;
      }
      if (px >= 146 && px <= 220) {
        cycleTimebase();
        soundClick();
        return;
      }
      if (px >= 224 && px <= 298) {
        memset(adcHistory, 0, sizeof(adcHistory));
        soundClick();
        return;
      }
    }

    // GPIO buttons (Row 0: 137..155, Row 1: 157..175)
    for (int i = 0; i < 6; i++) {
      int col = i % 3;
      int row = i / 3;
      int bx = 4 + col * 98;
      int by = 137 + row * 20;
      if (px >= bx && px <= bx + 94 && py >= by && py <= by + 18) {
        togglePin(i);
        soundClick();
        return;
      }
    }

    // Click on CRT screen toggles pause
    if (px >= 4 && px <= 214 && py >= 21 && py <= 101) {
      paused = !paused;
      soundClick();
    }
  }

  void mouse(int, int, uint8_t, uint8_t) override {}

  void key(uint8_t k) override {
    if (k == ' ' || k == 'p' || k == 'P') {
      paused = !paused;
      soundClick();
    } else if (k == 's' || k == 'S' || k == 'a' || k == 'A') {
      cycleSource();
      soundClick();
    } else if (k == 't' || k == 'T') {
      cycleTimebase();
      soundClick();
    } else if (k == 'c' || k == 'C') {
      memset(adcHistory, 0, sizeof(adcHistory));
      soundClick();
    } else if (k >= '1' && k <= '6') {
      togglePin(k - '1');
      soundClick();
    } else if (k == KEY_ESC) {
      closeReq = true;
    }
  }
};


#endif // TENSILE_APPS_HWAPPS_H
