#ifndef __PATTERN_H
#define __PATTERN_H

#include <Arduino.h>

#define PALETTE_SIZE 16

#define NUM_PATTERNS      16
#define PATTERN_STROBE    0
#define PATTERN_HYPER     1
#define PATTERN_DOPS      2
#define PATTERN_STROBIE   3
#define PATTERN_PULSE     4
#define PATTERN_SEIZURE   5
#define PATTERN_TRACER    6
#define PATTERN_DASHDOPS  7
#define PATTERN_BLINKE    8
#define PATTERN_EDGE      9
#define PATTERN_LEGO      10
#define PATTERN_CHASE     11
#define PATTERN_MORPH     12
#define PATTERN_RIBBON    13
#define PATTERN_COMET     14
#define PATTERN_CANDY     15


class Pattern {
  public:
    Pattern() {}
    Pattern(uint8_t pattern, uint8_t num_colors,
          uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
          uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
          uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
          uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15);

    void render(uint8_t& r, uint8_t& g, uint8_t& b);
    void reset();
    void load(uint16_t addr);
    void save(uint16_t addr);

    uint8_t pattern;
    uint8_t num_colors;
    uint8_t colors[PALETTE_SIZE];

    uint16_t tick;
    uint8_t cur_color;
    int16_t counter0, counter1;
};

#endif
