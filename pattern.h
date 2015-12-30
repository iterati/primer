#ifndef __PATTERN_H
#define __PATTERN_H

#include <Arduino.h>
#include <EEPROM.h>
#include "palette.h"

#define PALETTE_SIZE    16
#define NUM_PATTERNS    62

#define P_RIBBON        0
#define P_HYPER         1
#define P_STROBE        2
#define P_NANO          3
#define P_DOPS          4
#define P_SLOW_STROBE   5
#define P_STROBIE       6
#define P_FAINT         7
#define P_SIGNAL        8
#define P_BLASTER       9
#define P_HEAVYBLASTER  10
#define P_AUTOBLASTER   11
#define P_STROBE2       12
#define P_HYPER3        13
#define P_DOPS3         14
#define P_BLASTER3      15
#define P_HEAVYBLASTER3 16
#define P_AUTOBLASTER3  17
#define P_TRACER        18
#define P_DASHDOPS      19
#define P_DOPSDASH      20
#define P_VEXING        21
#define P_VEXING3       22
#define P_RIBBONTRACER  23
#define P_DOTTED        24
#define P_FIREWORK      25
#define P_BOTTLEROCKET  26
#define P_GROW          27
#define P_SHRINK        28
#define P_STRETCH       29
#define P_WAVE          30
#define P_SHIFT         31
#define P_COMET         32
#define P_METEOR        33
#define P_EMBERS        34
#define P_INFLUX        35
#define P_SWORD         36
#define P_SWORD5        37
#define P_RAZOR         38
#define P_RAZOR5        39
#define P_BARBS         40
#define P_BARBS5        41
#define P_CYCLOPS       42
#define P_FADEIN        43
#define P_STROBEIN      44
#define P_FADEOUT       45
#define P_STROBEOUT     46
#define P_PULSE         47
#define P_PULSAR        48
#define P_SLOWMORPH     49
#define P_MORPH         50
#define P_DOPMORPH      51
#define P_STROBIEMORPH  52
#define P_STROBEMORPH   53
#define P_HYPERMORPH    54
#define P_DASHMORPH     55
#define P_FUSE          56
#define P_DOPFUSE       57
#define P_STROBEFUSE    58
#define P_STROBIEFUSE   59
#define P_HYPERFUSE     60
#define P_DASHFUSE      61


void renderPattern(
    uint8_t pattern, uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t& r, uint8_t& g, uint8_t& b, bool doit);

#endif
