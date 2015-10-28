#ifndef __ANIM_H
#define __ANIM_H

#include <Arduino.h>

#define PALETTE_SIZE      12

#define AMODE_OFF         0
#define AMODE_SPEED       1
#define AMODE_TILTX       2
#define AMODE_TILTY       3
#define AMODE_FLIPZ       4

#define ASENS_LOW         2
#define ASENS_MEDIUM      1
#define ASENS_HIGH        0

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


void unpackColor(uint8_t color, uint8_t *r, uint8_t *g, uint8_t *b);


class Mode {
  public:
    Mode(uint16_t user_eeprom_addr, uint8_t user_acc_mode, uint8_t user_acc_sensitivity,
        uint8_t prime0, uint8_t num_colors0,
        uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03, uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07, uint8_t c08, uint8_t c09, uint8_t c0A, uint8_t c0B,
        uint8_t prime1, uint8_t num_colors1,
        uint8_t c10, uint8_t c11, uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15, uint8_t c16, uint8_t c17, uint8_t c18, uint8_t c19, uint8_t c1A, uint8_t c1B);

    void init();
    void reset();
    void load();
    void save();

    void prime_strobe(uint8_t c_time, uint8_t b_time);
    void prime_pulse(uint8_t c_time, uint8_t b_time);
    void prime_tracer(uint8_t c_time, uint8_t b_time);
    void prime_dashdops(uint8_t c_time, uint8_t d_time, uint8_t b_time, uint8_t dops);
    void prime_blinke(uint8_t c_time, uint8_t b_time);
    void prime_edge(uint8_t c_time, uint8_t e_time, uint8_t b_time);
    void prime_lego(uint8_t b_time);
    void prime_chase(uint8_t c_time, uint8_t b_time, uint8_t steps);
    void prime_morph(uint8_t c_time, uint8_t b_time, uint8_t steps);
    void prime_comet(uint8_t c_time, uint8_t b_time, uint8_t per_step);
    void prime_candy(uint8_t c_time, uint8_t b_time, uint8_t pick, uint8_t repeat);

    void render(uint8_t *r, uint8_t *g, uint8_t *b);
    void updateAcc(float fxg, float fyg, float fzg);

    uint32_t tick;
    uint16_t eeprom_addr;
    uint8_t cur_color;
    int16_t counter0, counter1;

    uint8_t acc_mode;
    uint8_t acc_sensitivity;
    int16_t acc_counter;
    uint8_t cur_variant;

    uint8_t prime[2];
    uint8_t num_colors[2];
    uint8_t palette[2][PALETTE_SIZE];
};

#endif
