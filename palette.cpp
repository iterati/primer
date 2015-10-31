#include "palette.h"

const PROGMEM uint8_t color_palette[NUM_COLORS][3] = {
  {  0,   0,   0}, // 0x00 // Dims
  { 32,  32,  32}, // 0x01
  { 48,   0,   0}, // 0x02
  { 40,  40,   0}, // 0x03
  {  0,  48,   0}, // 0x04
  {  0,  40,  40}, // 0x05
  {  0,   0,  48}, // 0x06
  { 40,   0,  40}, // 0x07
  {255,   0,   0}, // 0x08 // Red -> green
  {252,  64,   0}, // 0x09
  {248, 128,   0}, // 0x0A
  {244, 192,   0}, // 0x0B
  {240, 240,   0}, // 0x0C
  {192, 244,   0}, // 0x0D
  {128, 248,   0}, // 0x0E
  { 64, 252,   0}, // 0x0F
  {  0, 255,   0}, // 0x10 // Green -> blue
  {  0, 252,  64}, // 0x11
  {  0, 248, 128}, // 0x12
  {  0, 244, 192}, // 0x13
  {  0, 240, 240}, // 0x14
  {  0, 192, 244}, // 0x15
  {  0, 128, 248}, // 0x16
  {  0,  64, 252}, // 0x17
  {  0,   0, 255}, // 0x18 // Blue -> red
  { 64,   0, 252}, // 0x19
  {128,   0, 248}, // 0x1A
  {192,   0, 244}, // 0x1B
  {240,   0, 240}, // 0x1C
  {244,   0, 192}, // 0x1D
  {248,   0, 128}, // 0x1E
  {252,   0,  64}, // 0x1F
  {240,  32,  32}, // 0x20 // Red saturated
  {216,  64,  64}, // 0x21
  {192,  96,  96}, // 0x22
  {160, 128, 128}, // 0x23
  {224, 224,  32}, // 0x24 // Yellow saturated
  {200, 200,  64}, // 0x25
  {176, 176,  96}, // 0x26
  {152, 152, 128}, // 0x27
  { 32, 240,  32}, // 0x28 // Green saturated
  { 64, 216,  64}, // 0x29
  { 96, 192,  96}, // 0x2A
  {128, 160, 128}, // 0x2B
  { 32, 224, 224}, // 0x2C // Cyan saturated
  { 64, 200, 200}, // 0x2D
  { 96, 176, 176}, // 0x2E
  {128, 152, 152}, // 0x2F
  { 32,  32, 240}, // 0x30 // Blue saturated
  { 64,  64, 216}, // 0x31
  { 96,  96, 192}, // 0x32
  {128, 128, 160}, // 0x33
  {224,  32, 224}, // 0x34 // Magenta saturated
  {200,  64, 200}, // 0x35
  {176,  96, 176}, // 0x36
  {152, 128, 152}, // 0x37
  {160, 160, 160}, // 0x38 // Whites
  { 50, 114,  96}, // 0x39
  { 50, 147, 135}, // 0x3A
  {128,  50,  96}, // 0x3B
  { 96,  50, 128}, // 0x3C
  {128,  96,  50}, // 0x3D
  { 96, 128,  50}, // 0x3E
};

void unpackColor(uint8_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
  uint8_t shade = color >> 6;                                // shade is first 2 bits
  uint8_t idx = color & 0b00111111;                          // palette index is last 6 bits
  r = pgm_read_byte(&color_palette[idx][0]); r = r >> shade; // get red value and shade
  g = pgm_read_byte(&color_palette[idx][1]); g = g >> shade; // get green value and shade
  b = pgm_read_byte(&color_palette[idx][2]); b = b >> shade; // get blue value and shade
}

void morphColor(uint16_t tick, uint16_t morph_time, uint8_t r0, uint8_t g0, uint8_t b0,
                uint8_t r1, uint8_t g1, uint8_t b1, uint8_t& r, uint8_t& g, uint8_t& b) {
  r = r0 + (int)(r1 - r0) * (tick / (float)morph_time);
  g = g0 + (int)(g1 - g0) * (tick / (float)morph_time);
  b = b0 + (int)(b1 - b0) * (tick / (float)morph_time);
}
