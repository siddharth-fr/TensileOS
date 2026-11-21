#ifndef TENSILE_CORE_SOUND_H
#define TENSILE_CORE_SOUND_H

#pragma once
// TensileOS Sound Engine: WebAudio IBM PC Speaker Synthesizer & MML Melody Player
#include <Arduino.h>
#include <ctype.h>

void wsBroadcastAudio(uint16_t freq, uint16_t dur); // defined in webserver.h

static inline void soundTone(uint16_t freq, uint16_t dur) {
  wsBroadcastAudio(freq, dur);
}

static inline void soundBeep() {
  soundTone(880, 80);
}

static inline void soundClick() {
  soundTone(1600, 12);
}

static inline void soundError() {
  soundTone(220, 200);
}

// Vintage 4-note ascending boot chime: C5, E5, G5, C6
static void soundChimeBoot() {
  soundTone(523, 100); vTaskDelay(pdMS_TO_TICKS(110));
  soundTone(659, 100); vTaskDelay(pdMS_TO_TICKS(110));
  soundTone(784, 100); vTaskDelay(pdMS_TO_TICKS(110));
  soundTone(1046, 260);
}

// Game sound effects
static inline void soundEat() {
  soundTone(987, 30); vTaskDelay(pdMS_TO_TICKS(35));
  soundTone(1318, 50);
}

static inline void soundRotate() {
  soundTone(600, 20);
}

static inline void soundLineClear() {
  soundTone(523, 50); vTaskDelay(pdMS_TO_TICKS(55));
  soundTone(659, 50); vTaskDelay(pdMS_TO_TICKS(55));
  soundTone(784, 50); vTaskDelay(pdMS_TO_TICKS(55));
  soundTone(1046, 150);
}

static inline void soundExplosion() {
  for (int f = 300; f > 100; f -= 30) {
    soundTone(f, 25);
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Note frequencies calculation for MML
static const uint16_t g_noteFreqs[12] = {
  // C, C#, D, D#, E, F, F#, G, G#, A, A#, B  (Octave 4)
  262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494
};

// Play MML (Music Macro Language) string: e.g. "T160 O4 C D E F G A B > C"
static void soundPlay(const char* mml) {
  if (!mml) return;
  int octave = 4;
  int defaultLen = 4; // 1/4 note
  int tempo = 120;     // beats per minute

  const char* p = mml;
  while (*p) {
    while (*p == ' ') p++;
    if (!*p) break;
    char cmd = toupper((unsigned char)*p++);

    if (cmd == 'T') { // Tempo
      int t = 0;
      while (isdigit((unsigned char)*p)) t = t * 10 + (*p++ - '0');
      if (t >= 32 && t <= 300) tempo = t;
    } else if (cmd == 'O') { // Octave
      if (*p >= '1' && *p <= '7') octave = *p++ - '0';
    } else if (cmd == '>') { // Octave up
      if (octave < 7) octave++;
    } else if (cmd == '<') { // Octave down
      if (octave > 1) octave--;
    } else if (cmd == 'L') { // Default note length
      int l = 0;
      while (isdigit((unsigned char)*p)) l = l * 10 + (*p++ - '0');
      if (l > 0 && l <= 64) defaultLen = l;
    } else if (cmd == 'R' || cmd == 'P') { // Rest
      int l = defaultLen;
      if (isdigit((unsigned char)*p)) {
        l = 0;
        while (isdigit((unsigned char)*p)) l = l * 10 + (*p++ - '0');
      }
      uint32_t ms = (240000 / tempo) / (l ? l : 4);
      vTaskDelay(pdMS_TO_TICKS(ms));
    } else if (cmd >= 'A' && cmd <= 'G') { // Note
      int semi = 0;
      switch (cmd) {
        case 'C': semi = 0; break;
        case 'D': semi = 2; break;
        case 'E': semi = 4; break;
        case 'F': semi = 5; break;
        case 'G': semi = 7; break;
        case 'A': semi = 9; break;
        case 'B': semi = 11; break;
      }
      if (*p == '#' || *p == '+') { semi++; p++; }
      else if (*p == '-') { semi--; p++; }

      int l = defaultLen;
      if (isdigit((unsigned char)*p)) {
        l = 0;
        while (isdigit((unsigned char)*p)) l = l * 10 + (*p++ - '0');
      }
      bool dotted = false;
      if (*p == '.') { dotted = true; p++; }

      // Compute frequency for target octave
      float baseF = g_noteFreqs[(semi + 12) % 12];
      int octShift = octave - 4 + (semi < 0 ? -1 : (semi >= 12 ? 1 : 0));
      float freq = baseF;
      if (octShift > 0) while (octShift--) freq *= 2.0f;
      else if (octShift < 0) while (octShift++) freq /= 2.0f;

      uint32_t ms = (240000 / tempo) / (l ? l : 4);
      if (dotted) ms += ms / 2;

      soundTone((uint16_t)freq, (uint16_t)(ms * 0.9f));
      vTaskDelay(pdMS_TO_TICKS(ms));
    }
  }
}


#endif // TENSILE_CORE_SOUND_H
