// TensileOS v0.2.0 for ESP32 — 320x240 Pixel Retro Desktop running 100% on ESP32 silicon.
#include "config/config.h"
#include "net/net.h"
#include "display/screen.h"
#include "core/sound.h"
#include "core/input.h"
#include "fs/tensilefs.h"
#include "core/ui.h"
#include "shell/editor.h"
#include "apps/apps.h"
#include "games/games.h"
#include "display/screensaver.h"
#include "shell/shell.h"
#include "display/pixel_gfx.h"
#include "display/desktop_wm.h"
#include "net/webserver.h"

// Global Pixel Framebuffer & Tensile Desktop Manager instances
PixelGfx gfx;
TensileDesktop desktop;

static void osTask(void*) {
  fsBegin();
  fsSeed();
  if (fsExists("/AUTOEXEC.BAT")) batchRun("/AUTOEXEC.BAT");
  soundChimeBoot();
  desktop.begin();

  for (;;) {
    desktop.tick();
    desktop.renderFrame();
    vTaskDelay(pdMS_TO_TICKS(16)); // ~60 Hz OS frame loop
  }
}

void setup() {
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  delay(200);
  Serial.println("\n" OS_NAME " " OS_VERSION " booting...");
  screen.begin();
  inputBegin();
  startNetwork();
  xTaskCreatePinnedToCore(osTask, "os", 16384, NULL, 3, NULL, 1);
  webStart();
}

void loop() { delay(1000); }
