#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "palette.h"

uint8_t color_palette[NUM_COLORS][3];
const PROGMEM uint8_t factory_color_palette[NUM_COLORS][3] = {
  {0, 0, 0},
  {56, 60, 60},
  {24, 0, 0},
  {16, 16, 0},
  {0, 24, 0},
  {0, 16, 16},
  {0, 0, 24},
  {16, 0, 16},
  {255, 0, 0},
  {192, 8, 0},
  {140, 16, 0},
  {96, 32, 0},
  {60, 60, 0},
  {32, 96, 0},
  {16, 140, 0},
  {8, 192, 0},
  {0, 255, 0},
  {0, 192, 8},
  {0, 140, 16},
  {0, 96, 32},
  {0, 60, 60},
  {0, 32, 96},
  {0, 16, 140},
  {0, 8, 192},
  {0, 0, 255},
  {8, 0, 192},
  {16, 0, 140},
  {32, 0, 96},
  {60, 0, 60},
  {96, 0, 32},
  {140, 0, 16},
  {192, 0, 8},
  /*
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}.
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}.
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}.
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0}.
  */
};


void initPalette() {
  for (uint8_t c = 0; c < NUM_COLORS; c++) {
    color_palette[c][0] = pgm_read_byte(&factory_color_palette[c][0]);
    color_palette[c][1] = pgm_read_byte(&factory_color_palette[c][1]);
    color_palette[c][2] = pgm_read_byte(&factory_color_palette[c][2]);
  }
}

void savePalette(uint16_t addr) {
  for (uint8_t c = 0; c < NUM_COLORS; c++) {
    EEPROM.write((c * 3) + addr + 0, color_palette[c][0]);
    EEPROM.write((c * 3) + addr + 1, color_palette[c][1]);
    EEPROM.write((c * 3) + addr + 2, color_palette[c][2]);
  }
}

void loadPalette(uint16_t addr) {
  for (uint8_t c = 0; c < NUM_COLORS; c++) {
    color_palette[c][0] = EEPROM.read((c * 3) + addr + 0);
    color_palette[c][1] = EEPROM.read((c * 3) + addr + 1);
    color_palette[c][2] = EEPROM.read((c * 3) + addr + 2);
  }
}

void unpackColor(uint8_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
  uint8_t shade = color >> 6;
  uint8_t idx = color & 0b00111111;
  r = color_palette[idx][0];
  g = color_palette[idx][1];
  b = color_palette[idx][2];
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
