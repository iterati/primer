#include <EEPROM.h>
#include "palette.h"

uint8_t color_palette[NUM_COLORS][3];

void initPalette() {
  color_palette[0x00][0] = 0; color_palette[0x00][1] = 0; color_palette[0x00][2] = 0;
  color_palette[0x01][0] = 56; color_palette[0x01][1] = 60; color_palette[0x01][2] = 60;
  color_palette[0x02][0] = 24; color_palette[0x02][1] = 0; color_palette[0x02][2] = 0;
  color_palette[0x03][0] = 16; color_palette[0x03][1] = 16; color_palette[0x03][2] = 0;
  color_palette[0x04][0] = 0; color_palette[0x04][1] = 24; color_palette[0x04][2] = 0;
  color_palette[0x05][0] = 0; color_palette[0x05][1] = 16; color_palette[0x05][2] = 16;
  color_palette[0x06][0] = 0; color_palette[0x06][1] = 0; color_palette[0x06][2] = 24;
  color_palette[0x07][0] = 16; color_palette[0x07][1] = 0; color_palette[0x07][2] = 16;

  color_palette[0x08][0] = 255; color_palette[0x08][1] = 0; color_palette[0x08][2] = 0;
  color_palette[0x09][0] = 192; color_palette[0x09][1] = 8; color_palette[0x09][2] = 0;
  color_palette[0x0A][0] = 140; color_palette[0x0A][1] = 16; color_palette[0x0A][2] = 0;
  color_palette[0x0B][0] = 96; color_palette[0x0B][1] = 32; color_palette[0x0B][2] = 0;
  color_palette[0x0C][0] = 60; color_palette[0x0C][1] = 60; color_palette[0x0C][2] = 0;
  color_palette[0x0D][0] = 32; color_palette[0x0D][1] = 96; color_palette[0x0D][2] = 0;
  color_palette[0x0E][0] = 16; color_palette[0x0E][1] = 140; color_palette[0x0E][2] = 0;
  color_palette[0x0F][0] = 8; color_palette[0x0F][1] = 192; color_palette[0x0F][2] = 0;

  color_palette[0x10][0] = 0; color_palette[0x10][1] = 255; color_palette[0x10][2] = 0;
  color_palette[0x11][0] = 0; color_palette[0x11][1] = 192; color_palette[0x11][2] = 8;
  color_palette[0x12][0] = 0; color_palette[0x12][1] = 140; color_palette[0x12][2] = 16;
  color_palette[0x13][0] = 0; color_palette[0x13][1] = 96; color_palette[0x13][2] = 32;
  color_palette[0x14][0] = 0; color_palette[0x14][1] = 60; color_palette[0x14][2] = 60;
  color_palette[0x15][0] = 0; color_palette[0x15][1] = 32; color_palette[0x15][2] = 96;
  color_palette[0x16][0] = 0; color_palette[0x16][1] = 16; color_palette[0x16][2] = 140;
  color_palette[0x17][0] = 0; color_palette[0x17][1] = 8; color_palette[0x17][2] = 192;

  color_palette[0x18][0] = 0; color_palette[0x18][1] = 0; color_palette[0x18][2] = 255;
  color_palette[0x19][0] = 8; color_palette[0x19][1] = 0; color_palette[0x19][2] = 192;
  color_palette[0x1A][0] = 16; color_palette[0x1A][1] = 0; color_palette[0x1A][2] = 140;
  color_palette[0x1B][0] = 32; color_palette[0x1B][1] = 0; color_palette[0x1B][2] = 96;
  color_palette[0x1C][0] = 60; color_palette[0x1C][1] = 0; color_palette[0x1C][2] = 60;
  color_palette[0x1D][0] = 96; color_palette[0x1D][1] = 0; color_palette[0x1D][2] = 32;
  color_palette[0x1E][0] = 140; color_palette[0x1E][1] = 0; color_palette[0x1E][2] = 16;
  color_palette[0x1F][0] = 192; color_palette[0x1F][1] = 0; color_palette[0x1F][2] = 8;
};

void savePalette(uint16_t addr) {
  for (uint8_t c = 0; c < 32; c++) {
    EEPROM.write((c * 3) + addr + 0, color_palette[c][0]);
    EEPROM.write((c * 3) + addr + 1, color_palette[c][1]);
    EEPROM.write((c * 3) + addr + 2, color_palette[c][2]);
  }
}

void loadPalette(uint16_t addr) {
  for (uint8_t c = 0; c < 32; c++) {
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
