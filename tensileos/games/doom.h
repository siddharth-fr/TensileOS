#ifndef TENSILE_GAMES_DOOM_H
#define TENSILE_GAMES_DOOM_H

#pragma once
// TensileDoom: Native 2.5D Doom Raycasting Engine executing 100% on ESP32 Silicon
// Runs inside a standard 3D beveled TensileOS desktop window or maximized.
#include <Arduino.h>
#include <math.h>
#include "core/ui.h"
#include "core/sound.h"
#include "display/pixel_gfx.h"

#ifndef COL_BROWN
#define COL_BROWN COL_OLIVE
#endif

// ================= Sound Effects =================
static inline void soundDoomPistol() {
  soundTone(820, 25);
  soundTone(340, 45);
}

static inline void soundDoomShotgun() {
  soundTone(180, 70);
  soundTone(420, 35);
}

static inline void soundDoomDoor() {
  soundTone(300, 30);
  soundTone(400, 30);
  soundTone(500, 40);
}

static inline void soundDoomItem() {
  soundTone(659, 35);
  soundTone(987, 50);
}

static inline void soundDoomHurt() {
  soundTone(160, 90);
}

static inline void soundDoomImp() {
  soundTone(240, 60);
  soundTone(180, 80);
}

static inline void soundDoomVictory() {
  soundTone(523, 60);
  soundTone(659, 60);
  soundTone(784, 80);
  soundTone(1046, 160);
}

// ================= Map Definition (24x24 E1M1 Homage) =================
#define DOOM_MAP_W 24
#define DOOM_MAP_H 24

// Tile IDs:
// 0: Empty Floor
// 1: STARG1 (Techbase Silver Rivets)
// 2: BROWN96 (Weathered Brick)
// 3: COMP2 (Computer Terminal)
// 4: DOOR (Airlock Door)
// 5: SLIME_WALL (Corroded Metal)
// 6: EXIT_BUTTON (Level Finish)
// 7: TOXIC_SLIME (Floor Hazard Sector)
static const uint8_t DOOM_MAP[DOOM_MAP_H][DOOM_MAP_W] PROGMEM = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,1,0,0,0,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,4,0,0,0,1,2,0,0,0,2,1,0,0,0,0,0,0,0,1,1},
  {1,0,0,0,1,0,0,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,1,1},
  {1,1,4,1,1,1,0,1,1,2,0,0,0,2,1,0,0,0,0,0,0,0,1,1},
  {1,0,0,0,0,1,0,1,1,2,2,4,2,2,1,1,4,1,1,1,1,0,1,1},
  {1,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1},
  {1,0,0,0,0,1,0,1,1,3,3,0,3,3,1,1,0,1,1,1,1,0,1,1},
  {1,1,1,1,1,1,0,1,3,0,0,0,0,0,3,1,0,1,5,5,5,0,5,1},
  {2,2,2,2,2,1,0,1,3,0,3,0,3,0,3,1,0,4,7,7,7,0,5,1},
  {2,0,0,0,2,1,0,4,0,0,0,0,0,0,0,4,0,1,7,7,7,0,5,1},
  {2,0,0,0,4,0,0,1,3,0,3,0,3,0,3,1,0,1,7,7,7,0,5,1},
  {2,0,0,0,2,1,0,1,3,0,0,0,0,0,3,1,0,1,5,5,5,0,5,1},
  {2,2,4,2,2,1,0,1,1,3,3,4,3,3,1,1,0,1,1,1,1,0,1,1},
  {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1},
  {1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1},
  {1,0,0,0,0,4,0,0,0,0,1,0,1,0,0,0,0,0,4,0,1,0,1,1},
  {1,1,1,1,1,1,0,0,0,0,4,0,4,0,0,0,0,0,1,0,4,0,1,1},
  {1,0,0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,1},
  {1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,6,1},
  {1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// ================= Wall Textures (16x16 4-Bit Packed, 128 Bytes Each) =================
// Macro helper to pack two 4-bit nibbles
#define DP(a, b) (uint8_t)(((a) << 4) | ((b) & 0x0F))

// Texture 1: STARG1 (Techbase Silver Rivets)
static const uint8_t TEX_STARG1[16 * 8] PROGMEM = {
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,15),DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,15),DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(8,8), DP(8,8), DP(8,8), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(7,7), DP(7,7), DP(7,7), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(7,7), DP(7,7), DP(7,7), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(8,8), DP(8,8), DP(8,8), DP(8,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,0),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,15),DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,15),DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,0)
};

// Texture 2: BROWN96 (Weathered Brown Brick)
static const uint8_t TEX_BROWN96[16 * 8] PROGMEM = {
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(6,6), DP(6,8), DP(6,6), DP(6,6), DP(6,6), DP(6,8), DP(6,6), DP(6,6),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8)
};

// Texture 3: COMP2 (Computer Terminal Screens)
static const uint8_t TEX_COMP2[16 * 8] PROGMEM = {
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,0), DP(0,0), DP(0,0), DP(0,7), DP(7,0), DP(0,0), DP(0,0), DP(7,8),
  DP(7,0), DP(11,11),DP(11,0),DP(7,0),DP(10,10),DP(10,0),DP(0,7),DP(7,8),
  DP(7,0), DP(11,0), DP(11,0),DP(7,0),DP(10,0), DP(10,0),DP(0,7),DP(7,8),
  DP(7,0), DP(0,0), DP(0,0), DP(0,7), DP(7,0), DP(0,0), DP(0,0), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(7,14),DP(7,12),DP(7,10),DP(7,7), DP(7,14),DP(7,12),DP(7,10),DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,0), DP(0,0), DP(0,0), DP(0,7), DP(7,0), DP(0,0), DP(0,0), DP(7,8),
  DP(7,0), DP(9,9),  DP(9,0), DP(7,0), DP(12,12),DP(12,0),DP(0,7),DP(7,8),
  DP(7,0), DP(9,0),  DP(9,0), DP(7,0), DP(12,0), DP(12,0),DP(0,7),DP(7,8),
  DP(7,0), DP(0,0), DP(0,0), DP(0,7), DP(7,0), DP(0,0), DP(0,0), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,0)
};

// Texture 4: DOOR3 (Hydraulic Airlock Door with Yellow Chevrons)
static const uint8_t TEX_DOOR3[16 * 8] PROGMEM = {
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(8,14),DP(14,0),DP(0,14),DP(14,0),DP(0,14),DP(14,0),DP(0,14),DP(14,8),
  DP(8,0), DP(14,14),DP(0,0),DP(14,14),DP(0,0),DP(14,14),DP(0,0),DP(14,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(8,8), DP(8,8), DP(8,8), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(7,7), DP(15,15),DP(7,7),DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(7,7), DP(15,15),DP(7,7),DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(8,8), DP(8,8), DP(8,8), DP(8,7), DP(7,7), DP(7,8),
  DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(8,14),DP(14,0),DP(0,14),DP(14,0),DP(0,14),DP(14,0),DP(0,14),DP(14,8),
  DP(8,0), DP(14,14),DP(0,0),DP(14,14),DP(0,0),DP(14,14),DP(0,0),DP(14,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8)
};

// Texture 5: SLIME_WALL (Corroded Industrial Metal)
static const uint8_t TEX_SLIME_WALL[16 * 8] PROGMEM = {
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,10),DP(7,7), DP(7,7), DP(7,10),DP(7,7), DP(7,8),
  DP(8,7), DP(7,10),DP(10,10),DP(7,7),DP(7,10),DP(10,10),DP(7,7),DP(7,8),
  DP(8,7), DP(7,10),DP(2,10), DP(7,7), DP(7,10),DP(2,10), DP(7,7), DP(7,8),
  DP(8,7), DP(7,2), DP(2,2),  DP(7,7), DP(7,2), DP(2,2),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,2), DP(2,2),  DP(7,7), DP(7,2), DP(2,2),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(2,2),  DP(7,7), DP(7,7), DP(2,2),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,2),  DP(7,7), DP(7,7), DP(7,2),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,7), DP(7,7),  DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8),  DP(8,8), DP(8,8), DP(8,8),  DP(8,8), DP(8,8),
  DP(0,0), DP(0,0), DP(0,0),  DP(0,0), DP(0,0), DP(0,0),  DP(0,0), DP(0,0)
};

// Texture 6: EXIT_BUTTON (Red Switch Panel)
static const uint8_t TEX_EXIT[16 * 8] PROGMEM = {
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,0), DP(0,0), DP(0,0), DP(0,0), DP(0,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,0), DP(12,12),DP(12,12),DP(12,12),DP(0,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,0), DP(12,15),DP(12,12),DP(12,12),DP(0,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,0), DP(12,12),DP(12,12),DP(12,12),DP(0,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,0), DP(0,0), DP(0,0), DP(0,0), DP(0,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,14),DP(14,14),DP(14,14),DP(14,14),DP(7,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,14),DP(0,0), DP(0,0), DP(14,14),DP(7,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,14),DP(14,14),DP(14,14),DP(14,14),DP(7,7),DP(7,7),DP(7,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,7), DP(7,8),
  DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8), DP(8,8),
  DP(0,0), DP(0,0), DP(0,0), DP(0,0), DP(0,0), DP(0,0), DP(0,0), DP(0,0)
};

static inline uint8_t sampleWallTex(int texId, int u, int v) {
  u = constrain(u, 0, 15);
  v = constrain(v, 0, 15);
  int idx = v * 8 + (u >> 1);
  const uint8_t* p = NULL;
  switch (texId) {
    case 1: p = TEX_STARG1; break;
    case 2: p = TEX_BROWN96; break;
    case 3: p = TEX_COMP2; break;
    case 4: p = TEX_DOOR3; break;
    case 5: p = TEX_SLIME_WALL; break;
    case 6: p = TEX_EXIT; break;
    default: p = TEX_STARG1; break;
  }
  uint8_t byteVal = pgm_read_byte(p + idx);
  return (u & 1) ? (byteVal & 0x0F) : (byteVal >> 4);
}

// 16-Color Distance Shading LUT: 4 levels of darkness (0=Full, 1=Medium, 2=Dark, 3=Deep Shadow)
static const uint8_t SHADE_LUT[4][16] PROGMEM = {
  // Near (Full Vibrancy)
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  // Medium (Slightly dimmed)
  {0, 1, 2, 1, 4, 1, 6, 8, 0, 1, 2, 3, 4, 13, 6, 7},
  // Far (Dark shadow)
  {0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1, 0, 13, 0, 8},
  // Deep (Near black)
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0}
};

static inline uint8_t applyShade(uint8_t col, float dist, bool side) {
  int level = 0;
  if (dist > 8.0f) level = 3;
  else if (dist > 5.0f) level = 2;
  else if (dist > 2.8f) level = 1;
  if (side && level < 3) level++; // North/South wall side shadow
  return pgm_read_byte(&SHADE_LUT[level][col & 0x0F]);
}

// ================= Entity Definition =================
enum EntType {
  ENT_NONE = 0,
  ENT_ZOMBIE = 1,
  ENT_IMP = 2,
  ENT_BARREL = 3,
  ENT_SHOTGUN = 4,
  ENT_AMMO = 5,
  ENT_MEDIKIT = 6,
  ENT_ARMOR = 7,
  ENT_KEY_BLUE = 8,
  ENT_FIREBALL = 9
};

enum EntState {
  STATE_IDLE = 0,
  STATE_CHASE = 1,
  STATE_ATTACK = 2,
  STATE_PAIN = 3,
  STATE_DEAD = 4
};

struct DoomEntity {
  float x, y;
  float vx, vy;
  EntType type;
  EntState state;
  int16_t hp;
  uint32_t timer;
  float dist;
  bool active;
};

// ================= TensileDoom Application Class =================
class TensileDoom : public App {
 public:
  // Player state
  float posX = 2.5f, posY = 2.5f;
  float dirX = 1.0f, dirY = 0.0f;
  float planeX = 0.0f, planeY = 0.66f; // 66 deg FOV
  int health = 100;
  int armor = 50;
  int ammoBullets = 50;
  int ammoShells = 8;
  int weapon = 2; // 1 = Pistol, 2 = Shotgun
  bool hasShotgun = true;
  bool hasBlueKey = false;
  bool levelFinished = false;
  bool gameOver = false;

  // Weapon Animation
  int gunAnimFrame = 0;
  uint32_t lastGunTick = 0;
  bool gunFiring = false;

  // Hurt indicator
  uint32_t hurtFlashTimer = 0;

  // Doors
  float doorOpenFraction = 0.0f;
  int activeDoorX = -1, activeDoorY = -1;
  uint32_t doorTimer = 0;
  bool doorOpening = false;

  // Doomguy face state
  int faceLook = 0; // 0=Straight, 1=Left, 2=Right
  uint32_t faceTimer = 0;

  // Entities (Max 14)
  enum { MAX_ENT = 14 };
  DoomEntity ents[MAX_ENT];

  // Raycast Z-Buffer for sprite occlusion
  float zBuffer[256];

  TensileDoom() {
    prefW = 38;
    prefH = 20;
    tickMs = 35; // ~28-30 FPS tick
    fixed = false;
    resetGame();
  }

  const char* title() override {
    return levelFinished ? "TensileDoom - Victory!" : (gameOver ? "TensileDoom - Dead" : "TensileDoom (E1M1)");
  }

  void resetGame() {
    posX = 2.5f; posY = 2.5f;
    dirX = 1.0f; dirY = 0.0f;
    planeX = 0.0f; planeY = 0.66f;
    health = 100; armor = 50;
    ammoBullets = 50; ammoShells = 8;
    weapon = 2; hasShotgun = true; hasBlueKey = false;
    levelFinished = false; gameOver = false;
    gunAnimFrame = 0; gunFiring = false;
    doorOpenFraction = 0.0f; activeDoorX = -1; activeDoorY = -1;
    doorOpening = false;

    // Spawn entities
    for (int i = 0; i < MAX_ENT; i++) ents[i].active = false;

    // Zombiemen
    spawnEnt(0, ENT_ZOMBIE, 6.5f, 2.5f, 30);
    spawnEnt(1, ENT_ZOMBIE, 10.5f, 2.5f, 30);
    spawnEnt(2, ENT_ZOMBIE, 13.5f, 6.5f, 30);
    spawnEnt(3, ENT_ZOMBIE, 10.5f, 10.5f, 30);

    // Imps
    spawnEnt(4, ENT_IMP, 13.5f, 10.5f, 60);
    spawnEnt(5, ENT_IMP, 16.5f, 6.5f, 60);
    spawnEnt(6, ENT_IMP, 18.5f, 10.5f, 60);
    spawnEnt(7, ENT_IMP, 21.5f, 18.5f, 60);

    // Pickups
    spawnEnt(8, ENT_MEDIKIT, 2.5f, 5.5f, 1);
    spawnEnt(9, ENT_ARMOR, 1.5f, 1.5f, 1);
    spawnEnt(10, ENT_AMMO, 10.5f, 6.5f, 1);
    spawnEnt(11, ENT_KEY_BLUE, 18.5f, 2.5f, 1);
    spawnEnt(12, ENT_BARREL, 6.5f, 6.5f, 20);
    spawnEnt(13, ENT_BARREL, 12.5f, 14.5f, 20);
  }

  void spawnEnt(int idx, EntType t, float x, float y, int16_t hp) {
    if (idx < 0 || idx >= MAX_ENT) return;
    ents[idx].active = true;
    ents[idx].type = t;
    ents[idx].x = x; ents[idx].y = y;
    ents[idx].vx = 0; ents[idx].vy = 0;
    ents[idx].hp = hp;
    ents[idx].state = STATE_IDLE;
    ents[idx].timer = millis();
    ents[idx].dist = 0;
  }

  // ================= Movement and Actions =================
  void movePlayer(float fwd, float strafe) {
    if (gameOver || levelFinished) return;
    float moveSpeed = 0.14f;
    float rotSpeed = 0.08f;

    // Forward/backward
    if (fabs(fwd) > 0.001f) {
      float nx = posX + dirX * fwd * moveSpeed;
      float ny = posY + dirY * fwd * moveSpeed;
      if (canWalk(nx, posY)) posX = nx;
      if (canWalk(posX, ny)) posY = ny;
    }

    // Strafe
    if (fabs(strafe) > 0.001f) {
      float nx = posX + planeX * strafe * moveSpeed;
      float ny = posY + planeY * strafe * moveSpeed;
      if (canWalk(nx, posY)) posX = nx;
      if (canWalk(posX, ny)) posY = ny;
    }
  }

  void rotatePlayer(float angle) {
    if (gameOver || levelFinished) return;
    float oldDirX = dirX;
    dirX = dirX * cos(angle) - dirY * sin(angle);
    dirY = oldDirX * sin(angle) + dirY * cos(angle);
    float oldPlaneX = planeX;
    planeX = planeX * cos(angle) - planeY * sin(angle);
    planeY = oldPlaneX * sin(angle) + planeY * cos(angle);
  }

  bool canWalk(float x, float y) {
    int mx = int(x);
    int my = int(y);
    if (mx < 0 || my < 0 || mx >= DOOM_MAP_W || my >= DOOM_MAP_H) return false;
    uint8_t tile = pgm_read_byte(&DOOM_MAP[my][mx]);
    if (tile == 0 || tile == 7) return true; // Empty or slime
    if (tile == 4) { // Door
      return (mx == activeDoorX && my == activeDoorY && doorOpenFraction > 0.7f);
    }
    return false;
  }

  void tryInteract() {
    if (gameOver || levelFinished) return;
    // Check 1.2 units directly in front of player
    float tx = posX + dirX * 1.2f;
    float ty = posY + dirY * 1.2f;
    int mx = int(tx);
    int my = int(ty);
    if (mx < 0 || my < 0 || mx >= DOOM_MAP_W || my >= DOOM_MAP_H) return;
    uint8_t tile = pgm_read_byte(&DOOM_MAP[my][mx]);

    if (tile == 4) { // Airlock door
      if (mx == 20 && my == 17 && !hasBlueKey) {
        soundError(); // Locked!
        return;
      }
      activeDoorX = mx;
      activeDoorY = my;
      doorOpening = true;
      doorTimer = millis();
      soundDoomDoor();
    } else if (tile == 6) { // Exit switch
      levelFinished = true;
      soundDoomVictory();
    }
  }

  void shootWeapon() {
    if (gameOver || levelFinished || gunFiring) return;
    if (weapon == 1) { // Pistol
      if (ammoBullets <= 0) { soundError(); return; }
      ammoBullets--;
      gunFiring = true;
      gunAnimFrame = 1;
      lastGunTick = millis();
      soundDoomPistol();
      hitscanAttack(1, 15);
    } else if (weapon == 2) { // Shotgun
      if (ammoShells <= 0) { soundError(); return; }
      ammoShells--;
      gunFiring = true;
      gunAnimFrame = 1;
      lastGunTick = millis();
      soundDoomShotgun();
      // 5 pellets
      for (int i = 0; i < 5; i++) {
        hitscanAttack(2, 12);
      }
    }
  }

  void hitscanAttack(int wepType, int damage) {
    // Cast center ray or slight spread
    float spread = (wepType == 2) ? ((float(random(-10, 11)) / 100.0f)) : 0.0f;
    float rayDX = dirX + planeX * spread;
    float rayDY = dirY + planeY * spread;

    // Check entity hits
    int hitIdx = -1;
    float hitDist = 999.0f;
    for (int i = 0; i < MAX_ENT; i++) {
      if (!ents[i].active || ents[i].state == STATE_DEAD) continue;
      float ex = ents[i].x - posX;
      float ey = ents[i].y - posY;
      float d = sqrt(ex * ex + ey * ey);
      if (d < 0.2f || d > 12.0f) continue;
      // Dot product to check if in front
      float dot = (ex * rayDX + ey * rayDY) / d;
      if (dot > 0.94f && d < hitDist) {
        hitDist = d;
        hitIdx = i;
      }
    }

    if (hitIdx >= 0) {
      damageEntity(hitIdx, damage);
    }
  }

  void damageEntity(int idx, int dmg) {
    DoomEntity& e = ents[idx];
    e.hp -= dmg;
    if (e.hp <= 0) {
      e.state = STATE_DEAD;
      if (e.type == ENT_ZOMBIE || e.type == ENT_IMP) {
        soundDoomImp();
        // Drop ammo/armor
        if (random(0, 2) == 0) spawnEnt(10, ENT_AMMO, e.x, e.y, 1);
      } else if (e.type == ENT_BARREL) {
        soundExplosion();
        // Area damage to nearby entities and player
        for (int j = 0; j < MAX_ENT; j++) {
          if (ents[j].active && j != idx) {
            float dx = ents[j].x - e.x, dy = ents[j].y - e.y;
            if (sqrt(dx * dx + dy * dy) < 2.5f) damageEntity(j, 40);
          }
        }
        float pdx = posX - e.x, pdy = posY - e.y;
        if (sqrt(pdx * pdx + pdy * pdy) < 2.5f) hurtPlayer(35);
      }
    } else {
      e.state = STATE_PAIN;
      e.timer = millis();
    }
  }

  void hurtPlayer(int dmg) {
    if (gameOver || levelFinished) return;
    int absorbed = (armor * dmg) / 100;
    if (absorbed > armor) absorbed = armor;
    armor -= absorbed;
    health -= (dmg - absorbed);
    hurtFlashTimer = millis() + 250;
    soundDoomHurt();
    if (health <= 0) {
      health = 0;
      gameOver = true;
      soundError();
    }
  }

  // ================= Engine Tick & AI =================
  void tick(uint32_t now) override {
    if (!focused) return;

    // Face look timer
    if (now - faceTimer > 2000) {
      faceTimer = now;
      faceLook = random(0, 3);
    }

    // Weapon animation progression
    if (gunFiring && now - lastGunTick > 90) {
      lastGunTick = now;
      gunAnimFrame++;
      if (weapon == 1 && gunAnimFrame > 2) { // Pistol: 2 frames
        gunAnimFrame = 0;
        gunFiring = false;
      } else if (weapon == 2 && gunAnimFrame > 4) { // Shotgun: 4 frames
        gunAnimFrame = 0;
        gunFiring = false;
      }
    }

    // Door animation
    if (activeDoorX >= 0) {
      if (doorOpening) {
        doorOpenFraction += 0.08f;
        if (doorOpenFraction >= 1.0f) {
          doorOpenFraction = 1.0f;
          doorOpening = false;
        }
      } else if (now - doorTimer > 3500) { // Stay open 3.5s
        // Close if player not standing inside
        if (int(posX) != activeDoorX || int(posY) != activeDoorY) {
          doorOpenFraction -= 0.06f;
          if (doorOpenFraction <= 0.0f) {
            doorOpenFraction = 0.0f;
            activeDoorX = -1;
            activeDoorY = -1;
          }
        }
      }
    }

    // Toxic slime floor check (Tile 7)
    int px = int(posX), py = int(posY);
    if (px >= 0 && py >= 0 && px < DOOM_MAP_W && py < DOOM_MAP_H) {
      if (pgm_read_byte(&DOOM_MAP[py][px]) == 7) {
        static uint32_t lastSlime = 0;
        if (now - lastSlime > 800) {
          lastSlime = now;
          hurtPlayer(4);
        }
      }
    }

    // Entity AI and projectile physics
    for (int i = 0; i < MAX_ENT; i++) {
      if (!ents[i].active) continue;
      DoomEntity& e = ents[i];

      // Fireball physics
      if (e.type == ENT_FIREBALL) {
        e.x += e.vx; e.y += e.vy;
        float pdx = posX - e.x, pdy = posY - e.y;
        if (sqrt(pdx * pdx + pdy * pdy) < 0.6f) {
          hurtPlayer(15);
          e.active = false;
        } else if (!canWalk(e.x, e.y)) {
          e.active = false; // Explode on wall
        }
        continue;
      }

      // Pickups check
      if (e.type >= ENT_SHOTGUN && e.type <= ENT_KEY_BLUE) {
        float dx = posX - e.x, dy = posY - e.y;
        if (sqrt(dx * dx + dy * dy) < 0.8f) {
          if (e.type == ENT_MEDIKIT && health < 100) {
            health = min(100, health + 25);
            e.active = false; soundDoomItem();
          } else if (e.type == ENT_ARMOR && armor < 100) {
            armor = min(100, armor + 50);
            e.active = false; soundDoomItem();
          } else if (e.type == ENT_AMMO) {
            ammoBullets += 20; ammoShells += 4;
            e.active = false; soundDoomItem();
          } else if (e.type == ENT_KEY_BLUE) {
            hasBlueKey = true;
            e.active = false; soundDoomItem();
          }
        }
        continue;
      }

      // Enemy AI (Zombie, Imp)
      if (e.state == STATE_DEAD) continue;
      if (e.state == STATE_PAIN) {
        if (now - e.timer > 300) e.state = STATE_CHASE;
        continue;
      }

      float dx = posX - e.x, dy = posY - e.y;
      float dist = sqrt(dx * dx + dy * dy);

      // Line of sight check
      if (dist < 9.0f) {
        e.state = STATE_CHASE;
      }

      if (e.state == STATE_CHASE) {
        // Move towards player
        float step = (e.type == ENT_ZOMBIE ? 0.035f : 0.045f);
        float nx = e.x + (dx / dist) * step;
        float ny = e.y + (dy / dist) * step;
        if (canWalk(nx, e.y)) e.x = nx;
        if (canWalk(e.x, ny)) e.y = ny;

        // Attack if in range
        if (dist < (e.type == ENT_ZOMBIE ? 6.0f : 7.0f) && now - e.timer > 1400) {
          e.state = STATE_ATTACK;
          e.timer = now;
        }
      } else if (e.state == STATE_ATTACK) {
        if (now - e.timer > 400) {
          if (e.type == ENT_ZOMBIE) {
            soundDoomPistol();
            if (random(0, 10) > 3) hurtPlayer(8);
          } else if (e.type == ENT_IMP) {
            soundDoomImp();
            // Launch fireball
            for (int k = 0; k < MAX_ENT; k++) {
              if (!ents[k].active) {
                ents[k].active = true;
                ents[k].type = ENT_FIREBALL;
                ents[k].x = e.x; ents[k].y = e.y;
                float fdist = sqrt(dx * dx + dy * dy);
                ents[k].vx = (dx / fdist) * 0.22f;
                ents[k].vy = (dy / fdist) * 0.22f;
                break;
              }
            }
          }
          e.state = STATE_CHASE;
          e.timer = now;
        }
      }
    }
  }

  // ================= Input Handlers =================
  void key(uint8_t k) override {
    if (k == 'w' || k == 'W' || k == KEY_UP) movePlayer(1.0f, 0.0f);
    else if (k == 's' || k == 'S' || k == KEY_DOWN) movePlayer(-1.0f, 0.0f);
    else if (k == 'a' || k == 'A') movePlayer(0.0f, -1.0f);
    else if (k == 'd' || k == 'D') movePlayer(0.0f, 1.0f);
    else if (k == KEY_LEFT) rotatePlayer(-0.10f);
    else if (k == KEY_RIGHT) rotatePlayer(0.10f);
    else if (k == ' ' || k == 'e' || k == 'E') tryInteract();
    else if (k == '1') { weapon = 1; soundClick(); }
    else if (k == '2' && hasShotgun) { weapon = 2; soundClick(); }
    else if (k == 'f' || k == 'F') shootWeapon();
    else if (k == 'r' || k == 'R') { if (gameOver || levelFinished) resetGame(); }
    else if (k == KEY_ESC) closeReq = true;
  }

  void mousePixel(int px, int py, uint8_t btn, uint8_t prev) override {
    uint8_t edge = btn & ~prev;
    if (edge & 1) shootWeapon(); // Left click = shoot
    if (edge & 2) tryInteract(); // Right click = use/open door
  }

  // ================= Rendering Pipeline =================
  void draw(View& v) override {
    int cw = v.pw > 0 ? v.pw : (v.w * 6);
    int ch = v.ph > 0 ? v.ph : (v.h * 8);

    int hudH = 26;
    int gw = cw;
    int gh = ch - hudH;
    if (gh < 40) gh = 40;
    if (gw > 256) gw = 256;

    // 1. Ceiling & Floor Drawing (Fast SIMD Memset Fills)
    int halfH = gh / 2;
    gfx.fillRect(v.px, v.py, gw, halfH, COL_BLACK); // Ceiling
    gfx.fillRect(v.px, v.py + halfH, gw, gh - halfH, COL_GREY); // Floor

    // 2. DDA Column Raycasting
    for (int x = 0; x < gw; x++) {
      float cameraX = 2.0f * x / float(gw) - 1.0f;
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;

      int mapX = int(posX);
      int mapY = int(posY);

      float deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1.0f / rayDirX);
      float deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1.0f / rayDirY);

      float sideDistX, sideDistY;
      int stepX, stepY;
      int hit = 0;
      int side = 0; // 0=X, 1=Y
      int hitTile = 1;

      if (rayDirX < 0) {
        stepX = -1; sideDistX = (posX - mapX) * deltaDistX;
      } else {
        stepX = 1; sideDistX = (mapX + 1.0f - posX) * deltaDistX;
      }
      if (rayDirY < 0) {
        stepY = -1; sideDistY = (posY - mapY) * deltaDistY;
      } else {
        stepY = 1; sideDistY = (mapY + 1.0f - posY) * deltaDistY;
      }

      // Step DDA
      while (hit == 0) {
        if (sideDistX < sideDistY) {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        } else {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        if (mapX < 0 || mapY < 0 || mapX >= DOOM_MAP_W || mapY >= DOOM_MAP_H) {
          hit = 1; hitTile = 1; break;
        }
        uint8_t t = pgm_read_byte(&DOOM_MAP[mapY][mapX]);
        if (t > 0 && t != 7) {
          if (t == 4) { // Door
            if (mapX == activeDoorX && mapY == activeDoorY) {
              if (doorOpenFraction < 0.95f) { hit = 1; hitTile = t; }
            } else {
              hit = 1; hitTile = t;
            }
          } else {
            hit = 1; hitTile = t;
          }
        }
      }

      float perpWallDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
      if (perpWallDist < 0.1f) perpWallDist = 0.1f;
      zBuffer[x] = perpWallDist;

      int lineHeight = int(gh / perpWallDist);
      int drawStart = -lineHeight / 2 + gh / 2;
      int drawEnd = lineHeight / 2 + gh / 2;

      // Texture coordinate calculation
      float wallX = (side == 0) ? (posY + perpWallDist * rayDirY) : (posX + perpWallDist * rayDirX);
      wallX -= floor(wallX);
      int texX = int(wallX * 16.0f);
      if (side == 0 && rayDirX > 0) texX = 15 - texX;
      if (side == 1 && rayDirY < 0) texX = 15 - texX;

      // Vertical span drawing
      float step = 16.0f / float(lineHeight);
      float texPos = (drawStart - gh / 2 + lineHeight / 2) * step;

      int clampStart = max(0, drawStart);
      int clampEnd = min(gh - 1, drawEnd);

      for (int y = clampStart; y <= clampEnd; y++) {
        int texY = int(texPos) & 15;
        texPos += step;
        uint8_t col = sampleWallTex(hitTile, texX, texY);
        uint8_t shaded = applyShade(col, perpWallDist, side == 1);
        gfx.setPixelFast(v.px + x, v.py + y, shaded);
      }
    }

    // 3. Distance-Sorted Billboard Sprites
    renderSprites(v, gw, gh);

    // 4. Weapon Viewmodel (Shotgun / Pistol)
    renderWeapon(v, gw, gh);

    // 5. Hurt Screen Flash Overlay
    if (millis() < hurtFlashTimer) {
      gfx.drawRect(v.px, v.py, gw, gh, COL_RED);
      gfx.drawRect(v.px + 1, v.py + 1, gw - 2, gh - 2, COL_RED);
    }

    // 6. Classic DOOM Status Bar (HUD)
    renderHUD(v, cw, ch, hudH);
  }

  void renderSprites(View& v, int gw, int gh) {
    // Calculate entity distance
    for (int i = 0; i < MAX_ENT; i++) {
      if (ents[i].active) {
        float dx = posX - ents[i].x;
        float dy = posY - ents[i].y;
        ents[i].dist = dx * dx + dy * dy;
      }
    }

    // Simple sorting indices
    int order[MAX_ENT];
    for (int i = 0; i < MAX_ENT; i++) order[i] = i;
    for (int i = 0; i < MAX_ENT - 1; i++) {
      for (int j = i + 1; j < MAX_ENT; j++) {
        if (ents[order[i]].dist < ents[order[j]].dist) {
          int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }
      }
    }

    // Project and draw
    for (int i = 0; i < MAX_ENT; i++) {
      int idx = order[i];
      if (!ents[idx].active) continue;
      DoomEntity& e = ents[idx];

      float spriteX = e.x - posX;
      float spriteY = e.y - posY;

      float invDet = 1.0f / (planeX * dirY - dirX * planeY);
      float transformX = invDet * (dirY * spriteX - dirX * spriteY);
      float transformY = invDet * (-planeY * spriteX + planeX * spriteY);

      if (transformY <= 0.15f) continue; // Behind camera

      int spriteScreenX = int((gw / 2) * (1.0f + transformX / transformY));
      int spriteHeight = abs(int(gh / transformY));
      int spriteWidth = spriteHeight; // Square sprites

      int drawStartY = -spriteHeight / 2 + gh / 2;
      int drawEndY = spriteHeight / 2 + gh / 2;
      int drawStartX = -spriteWidth / 2 + spriteScreenX;
      int drawEndX = spriteWidth / 2 + spriteScreenX;

      int clampStartX = max(0, drawStartX);
      int clampEndX = min(gw - 1, drawEndX);
      int clampStartY = max(0, drawStartY);
      int clampEndY = min(gh - 1, drawEndY);

      // Render sprite vertical stripes
      for (int stripe = clampStartX; stripe <= clampEndX; stripe++) {
        if (transformY < zBuffer[stripe]) {
          int texX = int(256 * (stripe - drawStartX) * 16 / spriteWidth) / 256;
          for (int y = clampStartY; y <= clampEndY; y++) {
            int d = y * 256 - gh * 128 + spriteHeight * 128;
            int texY = ((d * 16) / spriteHeight) / 256;
            uint8_t col = sampleEntityPixel(e.type, e.state, texX, texY);
            if (col != 13 && col != 0) { // Magenta / Black key
              uint8_t shaded = applyShade(col, transformY, false);
              gfx.setPixelFast(v.px + stripe, v.py + y, shaded);
            }
          }
        }
      }
    }
  }

  uint8_t sampleEntityPixel(EntType t, EntState s, int x, int y) {
    x = constrain(x, 0, 15);
    y = constrain(y, 0, 15);

    // Procedural pixel sprites for entities
    if (t == ENT_ZOMBIE) {
      if (s == STATE_DEAD) {
        // Red blood splat and rifle on ground
        if (y > 10 && x > 2 && x < 14) return (x % 3 == 0) ? COL_RED : COL_MAROON;
        return 13;
      }
      // Soldier in green combat armor and brown pants
      if (y >= 1 && y <= 4 && x >= 6 && x <= 9) return COL_WHITE; // Face
      if (y >= 5 && y <= 9 && x >= 5 && x <= 10) return COL_GREEN; // Armor vest
      if (y >= 10 && y <= 14 && (x == 6 || x == 9)) return COL_BROWN; // Legs
      if (y == 7 && (x == 11 || x == 12)) return COL_GREY; // Gun barrel
      return 13;
    } else if (t == ENT_IMP) {
      if (s == STATE_DEAD) {
        if (y > 9 && x > 2 && x < 14) return (x % 2 == 0) ? COL_RED : COL_BROWN;
        return 13;
      }
      // Brown spiked demon with red eyes
      if (y >= 2 && y <= 5 && x >= 5 && x <= 10) {
        if (y == 3 && (x == 6 || x == 9)) return COL_RED; // Glowing red eyes
        return COL_BROWN;
      }
      if (y >= 6 && y <= 11 && x >= 4 && x <= 11) {
        if (x == 4 || x == 11) return COL_WHITE; // Spikes
        return COL_BROWN;
      }
      if (y >= 12 && y <= 15 && (x == 6 || x == 9)) return COL_BROWN; // Legs
      return 13;
    } else if (t == ENT_FIREBALL) {
      // Glowing orange/red fireball
      int distCenter = (x - 7) * (x - 7) + (y - 7) * (y - 7);
      if (distCenter <= 9) return COL_WHITE;
      if (distCenter <= 20) return COL_YELLOW;
      if (distCenter <= 36) return COL_RED;
      return 13;
    } else if (t == ENT_BARREL) {
      // Green toxic barrel with hazard yellow rim
      if (y == 2 || y == 14) {
        if (x >= 4 && x <= 11) return COL_YELLOW;
      }
      if (y >= 3 && y <= 13 && x >= 4 && x <= 11) {
        if (y >= 6 && y <= 8 && (x == 7 || x == 8)) return COL_LIME; // Slime leak
        return COL_GREEN;
      }
      return 13;
    } else if (t == ENT_MEDIKIT) {
      // White box with Red Cross
      if (y >= 6 && y <= 14 && x >= 4 && x <= 12) {
        if ((x >= 7 && x <= 9 && y >= 8 && y <= 12) || (x >= 5 && x <= 11 && y >= 9 && y <= 11)) {
          return COL_RED;
        }
        return COL_WHITE;
      }
      return 13;
    } else if (t == ENT_ARMOR) {
      // Green security vest
      if (y >= 5 && y <= 13 && x >= 4 && x <= 11) return (y % 2 == 0) ? COL_LIME : COL_GREEN;
      return 13;
    } else if (t == ENT_AMMO) {
      // Red ammo cartridge box
      if (y >= 7 && y <= 13 && x >= 5 && x <= 11) return COL_RED;
      return 13;
    } else if (t == ENT_KEY_BLUE) {
      // Glowing Blue keycard
      if (y >= 6 && y <= 12 && x >= 5 && x <= 10) return COL_CYAN;
      return 13;
    }
    return 13;
  }

  void renderWeapon(View& v, int gw, int gh) {
    int wx = v.px + gw / 2;
    int wy = v.py + gh;

    if (weapon == 1) { // Pistol
      int kick = (gunAnimFrame == 1) ? 5 : 0;
      // Gun barrel & hand
      gfx.fillRect(wx - 4, wy - 30 + kick, 8, 30, COL_GREY);
      gfx.fillRect(wx - 2, wy - 34 + kick, 4, 6, COL_BLACK);
      gfx.drawBevelBox(wx - 5, wy - 31 + kick, 10, 32, false);

      // Muzzle Flash
      if (gunAnimFrame == 1) {
        gfx.fillRect(wx - 6, wy - 46, 12, 10, COL_YELLOW);
        gfx.fillRect(wx - 3, wy - 44, 6, 6, COL_WHITE);
      }
    } else if (weapon == 2) { // Shotgun
      int kick = (gunAnimFrame == 1) ? 8 : (gunAnimFrame == 2 ? 4 : 0);
      // Twin Barrels
      gfx.fillRect(wx - 8, wy - 34 + kick, 6, 34, COL_GREY);
      gfx.fillRect(wx + 2, wy - 34 + kick, 6, 34, COL_GREY);
      gfx.drawRect(wx - 9, wy - 35 + kick, 18, 35, COL_BLACK);

      // Wooden foregrip pump
      int pumpY = wy - 18 + kick + (gunAnimFrame == 3 ? 5 : 0);
      gfx.fillRect(wx - 10, pumpY, 20, 10, COL_BROWN);

      // Muzzle Flash
      if (gunAnimFrame == 1) {
        gfx.fillRect(wx - 14, wy - 52, 28, 16, COL_YELLOW);
        gfx.fillRect(wx - 8, wy - 48, 16, 10, COL_WHITE);
      }
    }
  }

  void renderHUD(View& v, int cw, int ch, int hudH) {
    int hx = v.px;
    int hy = v.py + ch - hudH;

    // Outer HUD frame
    gfx.fillRect(hx, hy, cw, hudH, COL_SILVER);
    gfx.drawBevelBox(hx, hy, cw, hudH, false);

    // Left Panel: ARMOR
    gfx.fillRect(hx + 4, hy + 3, 56, hudH - 6, COL_BLACK);
    gfx.drawBevelBox(hx + 4, hy + 3, 56, hudH - 6, true);
    char armBuf[12]; snprintf(armBuf, sizeof(armBuf), "ARM:%d%%", armor);
    gfx.drawText6x8(hx + 8, hy + 8, armBuf, COL_GREEN, COL_BLACK);

    // Center Panel: DOOM GUY ANIMATED FACE (16x16)
    int fx = hx + cw / 2 - 12;
    int fy = hy + 3;
    gfx.fillRect(fx, fy, 24, hudH - 6, COL_BLACK);
    gfx.drawBevelBox(fx, fy, 24, hudH - 6, true);
    renderDoomguyFace(fx + 4, fy + 2);

    // Right Panel: HEALTH & AMMO
    int rx = hx + cw - 64;
    gfx.fillRect(rx, hy + 3, 60, hudH - 6, COL_BLACK);
    gfx.drawBevelBox(rx, hy + 3, 60, hudH - 6, true);
    char hpBuf[12]; snprintf(hpBuf, sizeof(hpBuf), "HP:%d%%", health);
    uint8_t hpCol = (health > 50) ? COL_GREEN : (health > 25 ? COL_YELLOW : COL_RED);
    gfx.drawText6x8(rx + 6, hy + 5, hpBuf, hpCol, COL_BLACK);

    char amBuf[12];
    if (weapon == 1) snprintf(amBuf, sizeof(amBuf), "B:%d", ammoBullets);
    else snprintf(amBuf, sizeof(amBuf), "S:%d", ammoShells);
    gfx.drawText6x8(rx + 6, hy + 14, amBuf, COL_YELLOW, COL_BLACK);

    // Keycard indicator
    if (hasBlueKey) {
      gfx.fillRect(hx + cw / 2 + 18, hy + 8, 8, 8, COL_CYAN);
    }
  }

  void renderDoomguyFace(int x, int y) {
    // 16x16 Face
    uint8_t skin = COL_WHITE;
    uint8_t hair = COL_BROWN;
    uint8_t eye = COL_BLUE;

    if (gameOver) {
      // Dead face with red eyes
      skin = COL_SILVER; eye = COL_RED;
    } else if (levelFinished) {
      // Victory grin
      skin = COL_YELLOW; eye = COL_LIME;
    }

    // Hair
    gfx.fillRect(x + 3, y + 1, 10, 3, hair);
    // Face skin
    gfx.fillRect(x + 3, y + 4, 10, 9, skin);
    // Eyes
    int eyeOff = (faceLook == 1 ? -1 : (faceLook == 2 ? 1 : 0));
    gfx.setPixelFast(x + 5 + eyeOff, y + 6, eye);
    gfx.setPixelFast(x + 9 + eyeOff, y + 6, eye);
    // Mouth
    if (health < 40) {
      // Bloody hurt grimace
      gfx.drawHLine(x + 5, y + 10, 5, COL_RED);
    } else if (levelFinished) {
      // Big grin
      gfx.drawHLine(x + 5, y + 10, 6, COL_BLACK);
    } else {
      gfx.drawHLine(x + 6, y + 10, 4, COL_BLACK);
    }
  }
};

#endif // TENSILE_GAMES_DOOM_H
