#ifndef __PATTERN_H
#define __PATTERN_H

#include <Arduino.h>
#include <EEPROM.h>
#include "palette.h"

#define PALETTE_SIZE    16
#define NUM_PATTERNS    60

#define P_RIBBON        0
#define P_HYPER         1
#define P_STROBE        2
#define P_NANO          3
#define P_DOPS          4
#define P_STROBIE       5
#define P_FAINT         6
#define P_SIGNAL        7
#define P_BLASTER       8
#define P_HEAVYBLASTER  9
#define P_AUTOBLASTER   10
#define P_STROBE2       11
#define P_HYPER3        12
#define P_DOPS3         13
#define P_BLASTER3      14
#define P_HEAVYBLASTER3 15
#define P_AUTOBLASTER3  16
#define P_TRACER        17
#define P_DASHDOPS      18
#define P_DOPSDASH      19
#define P_STROBETRACER  20
#define P_HYPERTRACER   21
#define P_RIBBONTRACER  22
#define P_DOTTED        23
#define P_FIREWORK      24
#define P_BOTTLEROCKET  25
#define P_GROW          26
#define P_SHRINK        27
#define P_SPRING        28
#define P_WAVE          29
#define P_SHAPESHIFT    30
#define P_COMET         31
#define P_METEOR        32
#define P_EMBERS        33
#define P_INFLUX        34
#define P_SWORD         35
#define P_SWORD5        36
#define P_RAZOR         37
#define P_RAZOR5        38
#define P_BARBS         39
#define P_BARBS5        40
#define P_CYCLOPS       41
#define P_FADEIN        42
#define P_STROBEIN      43
#define P_FADEOUT       44
#define P_STROBEOUT     45
#define P_PULSE         46
#define P_PULSAR        47
#define P_MORPH         48
#define P_DOPMORPH      49
#define P_STROBIEMORPH  50
#define P_STROBEMORPH   51
#define P_HYPERMORPH    52
#define P_DASHMORPH     53
#define P_FUSE          54
#define P_DOPFUSE       55
#define P_STROBEFUSE    56
#define P_STROBIEFUSE   57
#define P_HYPERFUSE     58
#define P_DASHFUSE      59


void renderPattern(
    uint8_t pattern, uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t& r, uint8_t& g, uint8_t& b, bool doit);

#endif
