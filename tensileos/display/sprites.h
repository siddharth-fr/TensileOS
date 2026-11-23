#ifndef TENSILE_DISPLAY_SPRITES_H
#define TENSILE_DISPLAY_SPRITES_H

#pragma once
// TensileOS Retro Pixel Art Sprites (16-Color Palette)
// Stored 100% in ESP32 Flash memory.
// Color indices: 0=Black, 1=Navy, 2=Green, 3=Teal, 7=Silver, 8=Grey, 9=Blue, 10=Lime, 12=Red, 13=Magenta(Transparent), 14=Yellow, 15=White.
#include <Arduino.h>
#include "display/pixel_gfx.h"

#define T 13 // Transparent (Magenta)
#define K 0  // Black
#define W 15 // White
#define S 7  // Silver
#define G 8  // Grey
#define Y 14 // Yellow
#define B 9  // Blue
#define N 1  // Navy
#define R 12 // Red
#define L 10 // Lime/Green

// Helper to pack two 4-bit nibbles into one byte
#define P(a, b) (uint8_t)(((a) << 4) | ((b) & 0x0F))

// 1. Mouse Cursor: Classic 11x17 Arrow with Drop Shadow
// Hotspot at (0, 0)
const uint8_t SPRITE_CURSOR[17 * 6] = {
  P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), // 0
  P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), // 1
  P(K,W), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), // 2
  P(K,W), P(W,K), P(T,T), P(T,T), P(T,T), P(T,T), // 3
  P(K,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T), // 4
  P(K,W), P(W,W), P(W,K), P(T,T), P(T,T), P(T,T), // 5
  P(K,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), // 6
  P(K,W), P(W,W), P(W,W), P(W,K), P(T,T), P(T,T), // 7
  P(K,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), // 8
  P(K,W), P(W,W), P(W,W), P(W,W), P(W,K), P(T,T), // 9
  P(K,W), P(W,W), P(W,K), P(K,K), P(K,K), P(T,T), // 10
  P(K,W), P(W,K), P(W,K), P(T,T), P(T,T), P(T,T), // 11
  P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), // 12
  P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), // 13
  P(T,T), P(T,T), P(T,K), P(W,K), P(T,T), P(T,T), // 14
  P(T,T), P(T,T), P(T,T), P(K,K), P(T,T), P(T,T), // 15
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)  // 16
};

// 2. Start Button TensileOS Badge Logo (12x12)
// Custom pixel art matching T-OS logo: silver frame, navy background, white 'T' with maroon shadow
const uint8_t SPRITE_TENSILE_LOGO[12 * 6] = {
  P(T,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,T),
  P(S,N), P(N,N), P(N,N), P(N,N), P(N,N), P(N,S),
  P(S,N), P(W,W), P(W,W), P(W,W), P(W,W), P(N,S),
  P(S,N), P(W,W), P(W,W), P(W,W), P(W,W), P(N,S),
  P(S,N), P(R,R), P(W,W), P(W,W), P(R,R), P(N,S),
  P(S,N), P(N,R), P(W,W), P(W,W), P(R,N), P(N,S),
  P(S,N), P(N,R), P(W,W), P(W,W), P(R,N), P(N,S),
  P(S,N), P(N,R), P(W,W), P(W,W), P(R,N), P(N,S),
  P(S,N), P(N,R), P(W,W), P(W,W), P(R,N), P(N,S),
  P(S,N), P(N,N), P(R,R), P(R,R), P(N,N), P(N,S),
  P(S,N), P(N,N), P(N,N), P(N,N), P(N,N), P(N,S),
  P(T,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,T)
};

// 3. Desktop Icon: My Computer (24x24)
const uint8_t SPRITE_MY_COMPUTER[24 * 12] = {
  P(T,T), P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,B), P(B,B), P(B,B), P(B,B), P(B,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,B), P(W,B), P(B,B), P(B,B), P(B,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,B), P(B,B), P(W,B), P(B,B), P(B,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,B), P(B,B), P(B,B), P(B,B), P(B,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,S), P(S,G), P(S,S), P(S,S), P(S,S), P(S,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(K,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(K,S), P(S,S), P(S,K), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,S), P(K,K), P(K,K), P(K,K), P(K,K), P(S,W), P(W,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,S), P(S,S), P(S,S), P(K,K), P(S,S), P(S,W), P(W,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 4. Desktop Icon: Manila Folder (24x24)
const uint8_t SPRITE_FOLDER[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(Y,Y), P(Y,Y), P(Y,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(W,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(Y,K), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(W,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(W,W), P(Y,K), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(Y,K), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(W,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,T),
  P(K,Y), P(W,W), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,T),
  P(K,Y), P(W,W), P(W,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(W,W), P(W,W), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(W,W), P(W,W), P(W,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T),
  P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 5. Desktop Icon: Text Document (24x24)
const uint8_t SPRITE_DOCUMENT[24 * 12] = {
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,K), P(K,K), P(K,K), P(K,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 6. Desktop Icon: 3.5" Floppy Disk (24x24)
const uint8_t SPRITE_FLOPPY[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(S,S), P(S,K), P(S,S), P(S,S), P(S,S), P(S,S), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,S), P(S,S), P(S,K), P(S,S), P(S,S), P(S,S), P(S,S), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,S), P(S,S), P(S,K), P(S,S), P(S,S), P(S,S), P(S,S), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,S), P(S,S), P(S,K), P(S,S), P(S,S), P(S,S), P(S,S), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,S), P(S,S), P(S,K), P(S,S), P(S,S), P(S,S), P(S,S), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 7. Desktop Icon: Recycle Bin (24x24)
const uint8_t SPRITE_RECYCLE[24 * 12] = {
  P(T,T), P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(S,S), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(L,L), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,L), P(L,L), P(L,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(L,L), P(S,S), P(L,L), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(S,S), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,L), P(L,L), P(L,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(L,S), P(S,S), P(S,L), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(S,S), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(S,S), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,W), P(S,S), P(S,S), P(S,S), P(S,W), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 8. Desktop Icon: ANSI Paint Palette & Brush (24x24)
const uint8_t SPRITE_PAINT[24 * 12] = {
  P(T,T), P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(Y,Y), P(Y,Y), P(R,R), P(Y,Y), P(Y,Y), P(Y,Y), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(Y,Y), P(R,R), P(R,R), P(R,Y), P(B,B), P(Y,Y), P(Y,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(Y,Y), P(Y,R), P(R,Y), P(Y,B), P(B,B), P(B,Y), P(Y,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(B,B), P(Y,Y), P(Y,K), P(T,T), P(T,T), P(T,K), P(K,T),
  P(K,Y), P(L,L), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(T,T), P(T,K), P(N,N), P(K,T),
  P(K,Y), P(L,L), P(L,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(W,W), P(Y,K), P(T,T), P(K,N), P(N,K), P(T,T),
  P(K,Y), P(Y,L), P(L,Y), P(Y,Y), P(Y,Y), P(W,W), P(W,W), P(W,K), P(T,K), P(N,N), P(K,T), P(T,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(W,W), P(K,K), P(W,K), P(K,N), P(N,K), P(T,T), P(T,T),
  P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,W), P(K,K), P(W,K), P(N,N), P(K,T), P(T,T), P(T,T),
  P(T,K), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(W,W), P(K,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,Y), P(K,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(Y,Y), P(Y,Y), P(Y,Y), P(Y,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(Y,Y), P(Y,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 9. Desktop Icon: Calculator (24x24)
const uint8_t SPRITE_CALC[24 * 12] = {
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,L), P(L,L), P(L,L), P(L,L), P(L,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,L), P(K,L), P(L,K), P(L,K), P(L,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(B,B), P(W,B), P(B,W), P(B,B), P(W,R), P(R,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(B,B), P(W,B), P(B,W), P(B,B), P(W,R), P(R,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(W,K), P(K,W), P(K,K), P(W,B), P(B,W), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 10. Desktop Icon: Hardware Lab (Microchip & Scope) (24x24)
const uint8_t SPRITE_HWLAB[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,T), P(K,S), P(T,K), P(S,T), P(K,S), P(T,K), P(S,T), P(K,S), P(T,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,K), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(K,T), P(T,T), P(T,T),
  P(K,S), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(G,G), P(K,S), P(T,T), P(T,T),
  P(T,T), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(G,G), P(T,T), P(T,T), P(T,T),
  P(K,S), P(G,K), P(K,K), P(L,K), P(K,K), P(K,L), P(K,K), P(K,K), P(G,G), P(K,S), P(T,T), P(T,T),
  P(T,T), P(G,K), P(K,L), P(K,L), P(K,K), P(L,K), P(L,K), P(K,K), P(G,G), P(T,T), P(T,T), P(T,T),
  P(K,S), P(G,K), P(L,K), P(K,K), P(L,L), P(K,K), P(K,L), P(K,K), P(G,G), P(K,S), P(T,T), P(T,T),
  P(T,T), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(L,K), P(G,G), P(T,T), P(T,T), P(T,T),
  P(K,S), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(G,G), P(K,S), P(T,T), P(T,T),
  P(T,T), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(G,G), P(T,T), P(T,T), P(T,T),
  P(K,S), P(G,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(G,G), P(K,S), P(T,T), P(T,T),
  P(T,K), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(K,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,T), P(K,S), P(T,K), P(S,T), P(K,S), P(T,K), P(S,T), P(K,S), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 11. Desktop Icon: Network Neighborhood (24x24)
const uint8_t SPRITE_NET[24 * 12] = {
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,K), P(K,K), P(K,W), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,S), P(K,B), P(B,B), P(B,K), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,S), P(K,B), P(B,B), P(B,K), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(K,S), P(W,K), P(K,K), P(K,W), P(S,S), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(S,S), P(K,K), P(T,T), P(T,T), P(T,T), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(K,T), P(T,T), P(K,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T),
  P(K,S), P(S,S), P(K,K), P(S,S), P(S,K), P(T,K), P(S,W), P(K,K), P(K,W), P(S,S), P(K,T), P(T,T),
  P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,K), P(S,K), P(B,B), P(B,K), P(S,S), P(K,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(T,T), P(T,T), P(T,K), P(S,K), P(B,B), P(B,K), P(S,S), P(K,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(T,T), P(T,T), P(T,K), P(S,W), P(K,K), P(K,W), P(S,S), P(K,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(T,T), P(T,T), P(T,T), P(K,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(T,T), P(T,T), P(T,T), P(T,K), P(K,S), P(S,K), P(K,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,S), P(S,S), P(S,S), P(K,T), P(T,T), P(T,T),
  P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,S), P(S,K), P(K,S), P(S,K), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 12. Desktop Icon: World Globe / Web Browser (24x24)
const uint8_t SPRITE_BROWSER[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(B,B), P(B,B), P(B,B), P(B,B), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(B,B), P(L,L), P(B,B), P(B,B), P(B,B), P(B,B), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(B,B), P(L,L), P(L,L), P(L,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,T), P(T,T), P(T,T),
  P(K,B), P(B,L), P(L,L), P(L,L), P(B,B), P(B,W), P(B,B), P(B,B), P(B,B), P(B,K), P(T,T), P(T,T),
  P(K,B), P(B,B), P(L,L), P(L,B), P(B,W), P(B,B), P(W,B), P(B,B), P(B,B), P(B,K), P(T,T), P(T,T),
  P(K,B), P(B,B), P(B,B), P(B,B), P(W,B), P(B,B), P(B,W), P(B,B), P(L,L), P(B,K), P(T,T), P(T,T),
  P(K,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,W), P(W,K), P(T,T), P(T,T),
  P(K,B), P(B,B), P(B,B), P(B,B), P(W,B), P(B,B), P(B,W), P(L,L), P(L,B), P(B,K), P(T,T), P(T,T),
  P(K,B), P(B,B), P(B,B), P(B,B), P(B,W), P(B,B), P(W,B), P(B,L), P(L,B), P(B,K), P(T,T), P(T,T),
  P(K,B), P(B,B), P(B,B), P(B,B), P(B,B), P(W,W), P(B,B), P(B,B), P(B,B), P(B,K), P(T,T), P(T,T),
  P(T,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,T), P(T,T), P(T,T),
  P(T,T), P(K,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(K,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(B,B), P(B,B), P(B,B), P(B,B), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(K,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(K,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(K,K), P(S,S), P(S,S), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 13. Desktop Icon: Terminal / Command Prompt (24x24)
const uint8_t SPRITE_CMD[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(K,W), P(K,W), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(K,K), P(K,W), P(K,W), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(K,K), P(K,K), P(W,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(K,W), P(K,W), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,W), P(W,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,W), P(W,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,W), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(K,T), P(T,T), P(T,T),
  P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T),
  P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

// 14. Desktop Icon: TensileDoom Helmet (24x24)
const uint8_t SPRITE_DOOM[24 * 12] = {
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(K,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,L), P(W,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(L,W), P(W,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(K,T), P(T,T), P(T,T),
  P(T,K), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(L,L), P(K,T), P(T,T), P(T,T),
  P(T,K), P(L,L), P(S,S), P(S,S), P(S,S), P(S,S), P(S,S), P(S,L), P(L,L), P(K,T), P(T,T), P(T,T),
  P(T,K), P(L,S), P(S,K), P(K,K), P(K,K), P(K,K), P(K,K), P(S,S), P(L,L), P(K,T), P(T,T), P(T,T),
  P(K,L), P(S,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,K), P(S,L), P(L,K), P(T,T), P(T,T),
  P(K,L), P(S,K), P(B,W), P(B,B), P(B,B), P(B,B), P(B,B), P(B,K), P(S,L), P(L,K), P(T,T), P(T,T),
  P(K,L), P(S,K), P(B,B), P(W,B), P(B,B), P(B,B), P(B,B), P(B,K), P(S,L), P(L,K), P(T,T), P(T,T),
  P(K,L), P(S,K), P(B,B), P(B,B), P(B,B), P(B,B), P(B,B), P(B,K), P(S,L), P(L,K), P(T,T), P(T,T),
  P(T,K), P(L,S), P(K,B), P(B,B), P(B,B), P(B,B), P(B,K), P(S,L), P(L,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(L,L), P(S,S), P(K,K), P(K,K), P(K,K), P(S,S), P(L,L), P(L,K), P(T,T), P(T,T), P(T,T),
  P(T,K), P(L,L), P(L,S), P(G,G), P(G,G), P(G,G), P(S,L), P(L,L), P(L,K), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,L), P(L,L), P(S,K), P(S,K), P(S,L), P(L,L), P(L,K), P(K,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(K,L), P(L,L), P(S,K), P(S,K), P(S,L), P(L,L), P(L,K), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(L,L), P(S,S), P(S,S), P(S,L), P(L,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,K), P(K,K), P(K,K), P(K,K), P(K,K), P(K,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(K,G), P(G,G), P(G,G), P(G,G), P(G,K), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T),
  P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T), P(T,T)
};

#undef T
#undef K
#undef W
#undef S
#undef G
#undef Y
#undef B
#undef N
#undef R
#undef L
#undef P


#endif // TENSILE_DISPLAY_SPRITES_H
