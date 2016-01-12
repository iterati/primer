#ifndef __PALETTE_H
#define __PALETTE_H

#include <Arduino.h>
#include <EEPROM.h>

#define NUM_COLORS 48

extern uint8_t color_palette[NUM_COLORS][3];

void initPalette();
void loadPalette(uint16_t addr);
void savePalette(uint16_t addr);

void unpackColor(uint8_t color, uint8_t& r, uint8_t& g, uint8_t& b);
void morphColor(uint16_t tick, uint16_t morph_time, uint8_t r0, uint8_t g0, uint8_t b0,
    uint8_t r1, uint8_t g1, uint8_t b1, uint8_t& r, uint8_t& g, uint8_t& b);


#endif
