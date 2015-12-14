#ifndef __PATTERN_H
#define __PATTERN_H

#include <Arduino.h>

#define PALETTE_SIZE 16

#define NUM_PATTERNS          48
#define PATTERN_STROBE_FAST   0
#define PATTERN_STROBE        1
#define PATTERN_STROBE_SLOW   2
#define PATTERN_NANODOPS      3
#define PATTERN_DOPS          4
#define PATTERN_STROBIE       5
#define PATTERN_SEIZURE       6
#define PATTERN_ULTRA         7
#define PATTERN_HYPER         8
#define PATTERN_MEGA          9
#define PATTERN_PULSE_STROBE  10
#define PATTERN_PULSE_FAST    11
#define PATTERN_PULSE         12
#define PATTERN_PULSE_SLOW    13
#define PATTERN_LASER         14
#define PATTERN_TRACER        15
#define PATTERN_TAZER         16
#define PATTERN_DASHDOPS2     17
#define PATTERN_DASHDOPS7     18
#define PATTERN_DASHSTROBE    19
#define PATTERN_DASHDASH      20
#define PATTERN_QUICKE        21
#define PATTERN_BLINKE        22
#define PATTERN_STRIBBON      23
#define PATTERN_RAZOR         24
#define PATTERN_EDGE          25
#define PATTERN_SWORD         26
#define PATTERN_BARBWIRE      27
#define PATTERN_LEGO_MINI     28
#define PATTERN_LEGO          29
#define PATTERN_LEGO_HUGE     30
#define PATTERN_CHASE_SHORT   31
#define PATTERN_CHASE         32
#define PATTERN_CHASE_LONG    33
#define PATTERN_MORPH         34
#define PATTERN_MORPH_SLOW    35
#define PATTERN_MORPH_STROBE  36
#define PATTERN_MORPH_HYPER   37
#define PATTERN_RIBBON5       38
#define PATTERN_RIBBON10      39
#define PATTERN_RIBBON20      40
#define PATTERN_COMET_SHORT   41
#define PATTERN_COMET         42
#define PATTERN_COMET_LONG    43
#define PATTERN_CANDY2        44
#define PATTERN_CANDY3        45
#define PATTERN_CANDOPS       46
#define PATTERN_CANDYCRUSH    47
#define PATTERN_USER_00       48
#define PATTERN_USER_01       49
#define PATTERN_USER_02       50
#define PATTERN_USER_03       51
#define PATTERN_USER_04       52
#define PATTERN_USER_05       53
#define PATTERN_USER_06       54
#define PATTERN_USER_07       55
#define PATTERN_USER_08       56
#define PATTERN_USER_09       57
#define PATTERN_USER_10       58
#define PATTERN_USER_11       59
#define PATTERN_USER_12       60
#define PATTERN_USER_13       61
#define PATTERN_USER_14       62
#define PATTERN_USER_15       63

#define BASE_STROBE           0
#define BASE_PULSE            1
#define BASE_TRACER           2
#define BASE_DASHDOPS         3
#define BASE_BLINKE           4
#define BASE_EDGE             5
#define BASE_LEGO             6
#define BASE_CHASE            7
#define BASE_MORPH            8
#define BASE_COMET            9
#define BASE_CANDY            10

extern uint8_t user_patterns[16][8];

void initPatterns();
void loadPatterns(uint16_t addr);
void savePatterns(uint16_t addr);

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
