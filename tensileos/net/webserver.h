#ifndef TENSILE_NET_WEBSERVER_H
#define TENSILE_NET_WEBSERVER_H

#pragma once
// HTTP + WebSocket server (ESP-IDF esp_http_server, built into the Arduino ESP32 core).
//   GET /    -> the browser "monitor" page (webui.h)
//   GET /ws  -> WebSocket: screen frames out, key presses in
#include "esp_http_server.h"
#include <unistd.h>
#include <lwip/sockets.h>
#include <netinet/tcp.h>
#include "display/screen.h"
#include "core/input.h"
#include "net/webui.h"
#include "display/pixel_gfx.h"
#include "display/desktop_wm.h"

#ifndef CONFIG_HTTPD_WS_SUPPORT
#error "This core was built without WebSocket support. Use arduino-esp32 core 2.0.6 or newer (3.x is fine)."
#endif

#define WS_MAX_CLIENTS 4
#define FLUSH_MS 16                      // ~60 frames/s max, only sent when something changed

static httpd_handle_t g_httpd = NULL;
static int  g_cliFd[WS_MAX_CLIENTS];
static bool g_cliNew[WS_MAX_CLIENTS];
static SemaphoreHandle_t g_cliMtx = NULL;

struct AudioTone { uint16_t freq, dur; };
static QueueHandle_t g_audioQ = NULL;

void wsBroadcastAudio(uint16_t freq, uint16_t dur) {
  if (!g_audioQ) return;
  AudioTone t = {freq, dur};
  xQueueSend(g_audioQ, &t, 0);
}

static bool clientAdd(int fd) {
  int nodelay = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
  bool ok = false;
  xSemaphoreTake(g_cliMtx, portMAX_DELAY);
  for (int i = 0; i < WS_MAX_CLIENTS && !ok; i++)
    if (g_cliFd[i] == fd) { g_cliNew[i] = true; ok = true; }
  for (int i = 0; i < WS_MAX_CLIENTS && !ok; i++)
    if (g_cliFd[i] < 0) { g_cliFd[i] = fd; g_cliNew[i] = true; ok = true; }
  xSemaphoreGive(g_cliMtx);
  return ok;
}
static void clientRemove(int fd) {
  xSemaphoreTake(g_cliMtx, portMAX_DELAY);
  for (int i = 0; i < WS_MAX_CLIENTS; i++) if (g_cliFd[i] == fd) g_cliFd[i] = -1;
  xSemaphoreGive(g_cliMtx);
}
int wsClientCount() {
  int n = 0;
  if (!g_cliMtx) return 0;
  xSemaphoreTake(g_cliMtx, portMAX_DELAY);
  for (int i = 0; i < WS_MAX_CLIENTS; i++) if (g_cliFd[i] >= 0) n++;
  xSemaphoreGive(g_cliMtx);
  return n;
}

static void onSockClose(httpd_handle_t, int fd) { clientRemove(fd); }

static esp_err_t indexHandler(httpd_req_t* req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  return httpd_resp_send(req, WEB_UI_HTML, HTTPD_RESP_USE_STRLEN);
}
static esp_err_t faviconHandler(httpd_req_t* req) {
  httpd_resp_set_status(req, "204 No Content");
  return httpd_resp_send(req, NULL, 0);
}

static esp_err_t wsHandler(httpd_req_t* req) {
  int fd = httpd_req_to_sockfd(req);
  if (req->method == HTTP_GET) {                  // WebSocket handshake finished
    return clientAdd(fd) ? ESP_OK : ESP_FAIL;     // too many viewers -> refuse
  }
  httpd_ws_frame_t pkt;
  memset(&pkt, 0, sizeof pkt);
  pkt.type = HTTPD_WS_TYPE_BINARY;
  esp_err_t r = httpd_ws_recv_frame(req, &pkt, 0);       // first call: get length
  if (r != ESP_OK) return r;
  if (pkt.len == 0) return ESP_OK;
  if (pkt.len > 64) return ESP_FAIL;                      // keys are tiny; drop abusive clients
  uint8_t buf[65];
  pkt.payload = buf;
  r = httpd_ws_recv_frame(req, &pkt, pkt.len);
  if (r != ESP_OK) return r;

  // Native TensileOS Mouse Events [0xF1, xHi, xLo, y, buttons]
  if (pkt.len == 5 && buf[0] == 0xF1) {
    int mx = (buf[1] << 8) | buf[2];
    int my = buf[3];
    uint8_t btn = buf[4];
    static uint8_t prevBtn = 0;
    desktop.onMouse(mx, my, btn, prevBtn);
    prevBtn = btn;
    return ESP_OK;
  }

  // Keep-alive heartbeat ping [0xFE]
  if (pkt.len == 1 && buf[0] == 0xFE) return ESP_OK;

  if (pkt.len == 4 && buf[0] == KEY_MOUSE) { shellPushMouse(buf[1], buf[2], buf[3]); return ESP_OK; }
  for (size_t i = 0; i < pkt.len; i++) {
    desktop.onKey(buf[i]);
    shellPushKey(buf[i]);
  }
  return ESP_OK;
}

static void flushTask(void*) {
  static uint8_t dbuf[SCR_FRAME_MAX], fbuf[SCR_FRAME_MAX];
  static uint8_t pixPkt[4 + 120 * GFX_PITCH]; // Up to 120 scanlines * 160 bytes = 19,204 bytes (max 2 pkts per full screen)
  static int s_failCount[WS_MAX_CLIENTS] = {0};

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(FLUSH_MS));
    int fds[WS_MAX_CLIENTS]; bool isNew[WS_MAX_CLIENTS]; int n = 0; bool anyNew = false;
    xSemaphoreTake(g_cliMtx, portMAX_DELAY);
    for (int i = 0; i < WS_MAX_CLIENTS; i++)
      if (g_cliFd[i] >= 0) {
        fds[n] = g_cliFd[i]; isNew[n] = g_cliNew[i];
        anyNew |= g_cliNew[i]; g_cliNew[i] = false; n++;
      }
    xSemaphoreGive(g_cliMtx);
    if (!n) continue;

    // 1. Option B: Native 320x240 Pixel Framebuffer Streaming (100% on ESP32)
    if (gfx.fb) {
      int y = 0;
      while (y < GFX_H) {
        int y0 = -1;
        int sliceLines = 0;

        gfx.lock();
        // Find next dirty line
        while (y < GFX_H && !anyNew && !(gfx.dirtyLines[y >> 5] & (1UL << (y & 31)))) {
          y++;
        }
        if (y < GFX_H) {
          y0 = y;
          while (y < GFX_H && sliceLines < 120) {
            if (anyNew || (gfx.dirtyLines[y >> 5] & (1UL << (y & 31)))) {
              sliceLines = (y - y0 + 1);
            }
            y++;
          }
          if (sliceLines > 0) {
            pixPkt[0] = 4; // Opcode 0x04: Pixel Framebuffer Slice
            pixPkt[1] = (uint8_t)(y0 >> 8);
            pixPkt[2] = (uint8_t)(y0 & 0xFF);
            pixPkt[3] = (uint8_t)sliceLines;
            memcpy(pixPkt + 4, gfx.fb + y0 * GFX_PITCH, sliceLines * GFX_PITCH);

            // Clear dirty flags for these scanlines immediately under lock
            for (int k = y0; k < y0 + sliceLines; k++) {
              gfx.dirtyLines[k >> 5] &= ~(1UL << (k & 31));
            }
          }
        }
        gfx.unlock(); // Unlock IMMEDIATELY after memcpy! osTask is never starved by network I/O

        if (sliceLines <= 0 || y0 < 0) break;

        httpd_ws_frame_t pf;
        memset(&pf, 0, sizeof pf);
        pf.final = true;
        pf.type = HTTPD_WS_TYPE_BINARY;
        pf.payload = pixPkt;
        pf.len = 4 + sliceLines * GFX_PITCH;

        for (int i = 0; i < n; i++) {
          if (httpd_ws_send_frame_async(g_httpd, fds[i], &pf) != ESP_OK) {
            s_failCount[i]++;
            if (s_failCount[i] > 60) { // Tolerates transient network latency (~1s) without closing socket
              clientRemove(fds[i]);
              httpd_sess_trigger_close(g_httpd, fds[i]);
              s_failCount[i] = 0;
            }
          } else {
            s_failCount[i] = 0;
          }
        }
        if (y < GFX_H) vTaskDelay(pdMS_TO_TICKS(1)); // Allow async queue to drain smoothly
      }
    } else {
      // Fallback: Text-mode diff encoding
      size_t dl = 0, fl = 0;
      bool diff = screen.encode(dbuf, &dl, fbuf, &fl, anyNew);
      for (int i = 0; i < n; i++) {
        const uint8_t* p; size_t len;
        if (isNew[i])  { p = fbuf; len = fl; }
        else if (diff) { p = dbuf; len = dl; }
        else continue;
        httpd_ws_frame_t f;
        memset(&f, 0, sizeof f);
        f.final = true; f.fragmented = false;
        f.type = HTTPD_WS_TYPE_BINARY;
        f.payload = (uint8_t*)p; f.len = len;
        if (httpd_ws_send_frame_async(g_httpd, fds[i], &f) != ESP_OK) {
          clientRemove(fds[i]);
          httpd_sess_trigger_close(g_httpd, fds[i]);
        }
      }
    }

    // 2. Audio playback (clicks, beeps)
    AudioTone at;
    while (g_audioQ && xQueueReceive(g_audioQ, &at, 0) == pdTRUE) {
      uint8_t abuf[5] = {3, (uint8_t)(at.freq >> 8), (uint8_t)(at.freq & 0xFF),
                            (uint8_t)(at.dur >> 8),  (uint8_t)(at.dur & 0xFF)};
      httpd_ws_frame_t af;
      memset(&af, 0, sizeof af);
      af.final = true; af.type = HTTPD_WS_TYPE_BINARY;
      af.payload = abuf; af.len = 5;
      for (int i = 0; i < n; i++) httpd_ws_send_frame_async(g_httpd, fds[i], &af);
    }
  }
}

static void webStart() {
  if (!g_audioQ) g_audioQ = xQueueCreate(16, sizeof(AudioTone));
  g_cliMtx = xSemaphoreCreateMutex();
  for (int i = 0; i < WS_MAX_CLIENTS; i++) { g_cliFd[i] = -1; g_cliNew[i] = false; }

  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  cfg.server_port = 80;
  cfg.core_id = 0;
  cfg.max_open_sockets = 8;
  cfg.lru_purge_enable = true;
  cfg.stack_size = 8192;
  cfg.send_wait_timeout = 1;
  cfg.close_fn = onSockClose;
  if (httpd_start(&g_httpd, &cfg) != ESP_OK) { Serial.println("httpd_start failed"); return; }

  httpd_uri_t u;
  memset(&u, 0, sizeof u);
  u.uri = "/"; u.method = HTTP_GET; u.handler = indexHandler;
  httpd_register_uri_handler(g_httpd, &u);

  memset(&u, 0, sizeof u);
  u.uri = "/favicon.ico"; u.method = HTTP_GET; u.handler = faviconHandler;
  httpd_register_uri_handler(g_httpd, &u);

  memset(&u, 0, sizeof u);
  u.uri = "/ws"; u.method = HTTP_GET; u.handler = wsHandler;
  u.is_websocket = true;
  httpd_register_uri_handler(g_httpd, &u);

  xTaskCreatePinnedToCore(flushTask, "flush", 6144, NULL, 3, NULL, 0);
}


#endif // TENSILE_NET_WEBSERVER_H
