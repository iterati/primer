#include "palette.h"

const PROGMEM uint8_t color_palette[NUM_COLORS][3] = {
  {  0,   0,   0}, // 0x00 blank, white, dims
  { 56,  60,  60},
  { 24,   0,   0},
  { 16,  16,   0},
  {  0,  24,   0},
  {  0,  16,  16},
  {  0,   0,  24},
  { 16,   0,  16},

  // Red -> green
  {255,   0,   0}, // 0x08
  {192,   8,   0}, // 0x09
  {140,  16,   0}, // 0x0A
  { 96,  32,   0}, // 0x0B
  { 60,  60,   0}, // 0x0C
  { 32,  96,   0}, // 0x0D
  { 16, 140,   0}, // 0x0E
  {  8, 192,   0}, // 0x0F

  // Green -> blue
  {  0, 255,   0}, // 0x10
  {  0, 192,   8}, // 0x11
  {  0, 140,  16}, // 0x12
  {  0,  96,  32}, // 0x13
  {  0,  60,  60}, // 0x14
  {  0,  32,  96}, // 0x15
  {  0,  16, 140}, // 0x16
  {  0,   8, 192}, // 0x17

  // Blue -> red
  {  0,   0, 255}, // 0x18
  {  8,   0, 192}, // 0x19
  { 16,   0, 140}, // 0x1A
  { 32,   0,  96}, // 0x1B
  { 60,   0,  60}, // 0x1C
  { 96,   0,  32}, // 0x1D
  {140,   0,  16}, // 0x1E
  {192,   0,   8}, // 0x1F
};

void unpackColor(uint8_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
  uint8_t shade = color >> 6;
  uint8_t idx = color & 0b00111111;
  r = pgm_read_byte(&color_palette[idx][0]);
  g = pgm_read_byte(&color_palette[idx][1]);
  b = pgm_read_byte(&color_palette[idx][2]);
  r = r >> shade;
  g = g >> shade;
  b = b >> shade;
}

void morphColor(uint16_t tick, uint16_t morph_time, uint8_t r0, uint8_t g0, uint8_t b0,
                uint8_t r1, uint8_t g1, uint8_t b1, uint8_t& r, uint8_t& g, uint8_t& b) {
  r = r0 + (int)(r1 - r0) * (tick / (float)morph_time);
  g = g0 + (int)(g1 - g0) * (tick / (float)morph_time);
  b = b0 + (int)(b1 - b0) * (tick / (float)morph_time);
}
