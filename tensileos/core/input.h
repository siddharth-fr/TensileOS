#ifndef TENSILE_CORE_INPUT_H
#define TENSILE_CORE_INPUT_H

#pragma once
// Input layer: key/mouse events from the browser, status bar, blocking readKey().
#include "display/screen.h"
#include "net/net.h"

#define KEY_UP    0x80
#define KEY_DOWN  0x81
#define KEY_LEFT  0x82
#define KEY_RIGHT 0x83
#define KEY_HOME  0x84
#define KEY_END   0x85
#define KEY_DEL   0x86
#define KEY_PGUP  0x87
#define KEY_PGDN  0x88
#define KEY_INS   0x89
#define KEY_F1    0x90   // F1..F10 = 0x90..0x99 (F5 is left to the browser)
#define KEY_F2    0x91
#define KEY_F3    0x92
#define KEY_F4    0x93
#define KEY_F10   0x99
#define KEY_MOUSE 0xF0   // readKey() returns this; position/buttons are in g_mx,g_my,g_mb
#define KEY_ENTER 13
#define KEY_BKSP  8
#define KEY_TAB   9
#define KEY_ESC   27
#define KEY_CTRL_C 3
#define KEY_CTRL_K 11
#define KEY_CTRL_L 12
#define KEY_CTRL_O 15
#define KEY_CTRL_Q 17
#define KEY_CTRL_S 19
#define KEY_CTRL_U 21

struct Ev { uint8_t k, a, b, c; };
int wsClientCount();                      // defined in webserver.h

static QueueHandle_t g_keyQ = NULL;
static uint8_t g_mx = 0, g_my = 0, g_mb = 0;   // last mouse event: cell column/row, buttons (1=left, 2=right)
static const char* g_barHint = "";

static void inputBegin() { if (!g_keyQ) g_keyQ = xQueueCreate(48, sizeof(Ev)); }
static void shellPushKey(uint8_t k) {
  if (!g_keyQ) return;
  Ev e = {k, 0, 0, 0};
  xQueueSend(g_keyQ, &e, 0);
}
static void shellPushMouse(uint8_t x, uint8_t y, uint8_t b) {
  if (!g_keyQ) return;
  Ev e = {KEY_MOUSE, x, y, b};
  xQueueSend(g_keyQ, &e, b ? 0 : pdMS_TO_TICKS(20));   // a button-release must not be lost
}

static void statusBar() {
  static uint32_t lastSec = 0xFFFFFFFF;
  static uint32_t lastHeap = 0xFFFFFFFF;
  static int lastVc = -1;
  static const char* lastHint = NULL;

  uint32_t s = millis() / 1000;
  uint32_t fr = ESP.getFreeHeap() / 1024;
  int vc = wsClientCount();
  if (s == lastSec && fr == lastHeap && vc == lastVc && g_barHint == lastHint) return;
  lastSec = s; lastHeap = fr; lastVc = vc; lastHint = g_barHint;

  char r[56], l[64], b[SCR_COLS + 1];
  snprintf(r, sizeof r, "Heap %uK  Up %02u:%02u:%02u  Viewers %d ",
           (unsigned)fr, (unsigned)(s / 3600),
           (unsigned)((s / 60) % 60), (unsigned)(s % 60), vc);
  snprintf(l, sizeof l, " " OS_NAME " " OS_VERSION "%s", g_barHint);
  memset(b, ' ', SCR_COLS); b[SCR_COLS] = 0;
  size_t rl = strlen(r), ll = strlen(l);
  if (ll > SCR_COLS - rl - 1) ll = SCR_COLS - rl - 1;
  memcpy(b, l, ll);
  memcpy(b + SCR_COLS - rl, r, rl);
  screen.text(0, 0, b, 0x70);
}

// Wait for a key or mouse event. timeoutMs==0 waits forever. Returns 0 on timeout.
static uint8_t readKey(uint32_t timeoutMs) {
  static uint32_t lastBar = 0;
  uint32_t start = millis();
  for (;;) {
    uint32_t now = millis();
    if (now - lastBar >= 1000) { statusBar(); lastBar = now; }
    uint32_t el = now - start;
    if (timeoutMs && el >= timeoutMs) return 0;
    uint32_t slice = 1000;
    if (timeoutMs && timeoutMs - el < slice) slice = timeoutMs - el;
    Ev e;
    if (xQueueReceive(g_keyQ, &e, pdMS_TO_TICKS(slice)) == pdTRUE) {
      if (e.k == KEY_MOUSE) {
        g_mx = e.a; g_my = e.b; g_mb = e.c;
        // Coalesce rapid mouse movements of the same button state to eliminate queue backlog
        Ev next;
        while (xQueuePeek(g_keyQ, &next, 0) == pdTRUE && next.k == KEY_MOUSE && next.c == g_mb) {
          xQueueReceive(g_keyQ, &next, 0);
          g_mx = next.a; g_my = next.b; g_mb = next.c;
        }
      }
      return e.k;
    }
  }
}

static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline uint32_t rnd(uint32_t n) { return (uint32_t)random((long)n); }


#endif // TENSILE_CORE_INPUT_H
