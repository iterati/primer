#ifndef __MODE_H
#define __MODE_H

#include <Arduino.h>
#include "pattern.h"

#define AMODE_OFF   0
#define AMODE_SPEED 1
#define AMODE_TILTX 2
#define AMODE_TILTY 3
#define AMODE_FLIPZ 4

#define ASENS_LOW     0
#define ASENS_MEDIUM  1
#define ASENS_HIGH    2

class Mode {
  public:
    Mode();

    void render(uint8_t& r, uint8_t& g, uint8_t& b);
    void init();
    void load(uint16_t addr);
    void save(uint16_t addr);
    void changeColor(int8_t v);
    void changeShade();

    Pattern pattern[2];
    uint8_t accel_mode, accel_sens;
    uint8_t cur_variant;
    uint8_t edit_color;
    int16_t accel_counter;
};

#endif
