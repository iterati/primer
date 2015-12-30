#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "palette.h"

uint8_t color_palette[NUM_COLORS][3];
const PROGMEM uint8_t factory_color_palette[NUM_COLORS][3] = {
  {0, 0, 0},      // Blank
  {56, 64, 72},   // True White
  {24, 0, 0},     // Dim Red
  {16, 16, 0},    // Dim Yellow
  {0, 24, 0},     // Dim Green
  {0, 16, 16},    // Dim Cyan
  {0, 0, 24},     // Dum Blue
  {16, 0, 16},    // Dim Magenta
  {255, 0, 0},    // Red
  {224, 32, 0},
  {192, 64, 0},   // Orange
  {160, 96, 0},
  {128, 128, 0},  // Yellow
  {96, 160, 0},
  {64, 192, 0},   // Lime
  {32, 224, 0},
  {0, 255, 0},    // Green
  {0, 224, 32},
  {0, 192, 64},   // Sea
  {0, 160, 96},
  {0, 128, 128},  // Cyan
  {0, 96, 160},
  {0, 64, 192},   // Sky Blue
  {0, 32, 224},
  {0, 0, 255},    // Blue
  {32, 0, 224},
  {64, 0, 192},   // Purple
  {96, 0, 160},
  {128, 0, 128},  // Magneta
  {160, 0, 96},
  {192, 0, 64},   // Fushia
  {224, 0, 32},
  {64, 64, 64},   // Dumb White
  {160, 16, 16},  // Pastel Red
  {16, 160, 16},  // Pastel Green
  {16, 16, 160},  // Pastel Blue
  {128, 8, 48},   // Pastel Fushia
  {80, 48, 48},   // Light Pastel Red
  {128, 48, 8},   // Pastel Orange
  {80, 80, 8},    // Pastel Yellow
  {48, 128, 8},   // Pastel Lime
  {48, 80, 48},   // Light Pastel Green
  {8, 128, 48},   // Pastel Sea
  {8, 80, 80},    // Pastel Cyan
  {8, 48, 128},   // Pastel Sky Blue
  {48, 48, 80},   // Light Pastel Blue
  {48, 8, 128},   // Pastel Purplea
  {80, 8, 80},    // Pastel Magenta
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
