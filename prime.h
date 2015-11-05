#ifndef __PRIME_H
#define __PRIME_H

#include <Arduino.h>

//#define PALETTE_12
#define PALETTE_16

#ifdef PALETTE_12
#define PALETTE_SIZE 12
#endif

#ifdef PALETTE_16
#define PALETTE_SIZE 16
#endif


#define NUM_PRIMES        16
#define PRIME_STROBE      0
#define PRIME_HYPER       1
#define PRIME_DOPS        2
#define PRIME_STROBIE     3
#define PRIME_PULSE       4
#define PRIME_SEIZURE     5
#define PRIME_TRACER      6
#define PRIME_DASHDOPS    7
#define PRIME_BLINKE      8
#define PRIME_EDGE        9
#define PRIME_LEGO        10
#define PRIME_CHASE       11
#define PRIME_MORPH       12
#define PRIME_RIBBON      13
#define PRIME_COMET       14
#define PRIME_CANDY       15


class Prime {
  public:
    Prime() {}

#ifdef PALETTE_12
    Prime(uint8_t pattern, uint8_t num_colors,
          uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
          uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
          uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11);
#endif

#ifdef PALETTE_16
    Prime(uint8_t pattern, uint8_t num_colors,
          uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
          uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
          uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
          uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15);
#endif

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
