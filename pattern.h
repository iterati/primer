#ifndef __PATTERN_H
#define __PATTERN_H

#include <Arduino.h>
#include <EEPROM.h>
#include "palette.h"

#define PALETTE_SIZE 16
#define NUM_PATTERNS 48
#define P_RIBBON        0
#define P_HYPER         1
#define P_STROBE        2
#define P_DOPS          3
#define P_SPAZ          4
#define P_SIGNAL        5
#define P_BLASTER       6
#define P_STUTTER       7
#define P_STROBE2       8
#define P_HYPER3        9
#define P_DOPS3         10
#define P_BLASTER3      11
#define P_STUTTER3      12
#define P_TRACER        13
#define P_DASHDOPS      14
#define P_DOPSDASH      15
#define P_SANDWICH      16
#define P_HYPENATED     17
#define P_DASHED        18
#define P_DOTTED        19
#define P_FIREWORK      20
#define P_BOTTLEROCKET  21
#define P_STRETCH       22
#define P_DOPWAVE       23
#define P_SHAPESHIFT    24
#define P_COMET         25
#define P_METEOR        26
#define P_EMBERS        27
#define P_INFLUX        28
#define P_SWORD         29
#define P_SWORD3        30
#define P_BARBS         31
#define P_BARBS3        32
#define P_CYCLOPS       33
#define P_FADEIN        34
#define P_STROBEIN      35
#define P_FADEOUT       36
#define P_STROBEOUT     37
#define P_PULSE         38
#define P_PULSAR        39
#define P_MORPH         40
#define P_DOPMORPH      41
#define P_SPAZMORPH     42
#define P_STROBEMORPH   43
#define P_HYPERMORPH    44
#define P_DASHMORPH     45
#define P_FUSE          46
#define P_DOPFUSE       47
#define P_STROBEFUSE    48
#define P_SPAZFUSE      49
#define P_HYPERFUSE     50
#define P_DASHFUSE      51


class Pattern {
  public:
    Pattern() {}
    Pattern(uint8_t pattern, uint8_t num_colors,
          uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
          uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
          uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
          uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15);

    void render(uint8_t& r, uint8_t& g, uint8_t& b, bool doit);
    void reset();
    void load(uint16_t addr);
    void save(uint16_t addr);

    uint8_t pattern;
    uint8_t num_colors;
    uint8_t colors[PALETTE_SIZE];

    uint16_t tick;
    uint8_t cur_color;
    int16_t cntr;
};

#endif
