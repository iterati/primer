#ifndef __PALETTE_H
#define __PALETTE_H

#include <Arduino.h>
#include <avr/pgmspace.h>

#define NUM_COLORS 63

void unpackColor(uint8_t color, uint8_t& r, uint8_t& g, uint8_t& b);
void morphColor(uint16_t tick, uint16_t morph_time, uint8_t r0, uint8_t g0, uint8_t b0,
                uint8_t r1, uint8_t g1, uint8_t b1, uint8_t& r, uint8_t& g, uint8_t& b);

#endif
