#include <EEPROM.h>
#include "palette.h"
#include "pattern.h"

uint8_t _r, _g, _b;
uint8_t r0, g0, b0;
uint8_t r1, g1, b1;

uint8_t user_patterns[16][8];

void initPatterns() {
  /* user_patterns[0]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[1]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[2]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[3]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[4]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[5]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[6]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[7]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[8]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[9]  = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[10] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[11] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[12] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[13] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[14] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
  /* user_patterns[15] = {BASE_STROBE, 0, 20, 0, 200, 0, 0, 0, 0}; */
}

void loadPatterns(uint16_t addr) {
  for (uint8_t p; p < 16; p++) {
    for (uint8_t i; i < 8; i++) {
      user_patterns[p][i] = EEPROM.read(addr + (16 * p) + i);
    }
  }
}

void savePatterns(uint16_t addr) {
  for (uint8_t p; p < 16; p++) {
    for (uint8_t i; i < 8; i++) {
      EEPROM.update(addr + (16 * p) + i, user_patterns[p][i]);
    }
  }
}


/*******************************************************************************
 ** BASE ANIMATIONS
 ******************************************************************************/
void _strobe(Pattern* pattern, uint16_t c_time, uint16_t b_time) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
  }

  if (pattern->tick < c_time) {
    unpackColor(pattern->colors[pattern->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _pulse(Pattern* pattern, uint16_t c_time, uint16_t b_time) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
  }

  if (pattern->tick < c_time / 2) {
    unpackColor(pattern->colors[pattern->cur_color], r0, g0, b0);
    morphColor(pattern->tick, c_time / 2,
               0, 0, 0, r0, g0, b0, _r, _g, _b);
  } else if (pattern->tick < c_time) {
    unpackColor(pattern->colors[pattern->cur_color], r0, g0, b0);
    morphColor(pattern->tick - (c_time / 2), c_time - (c_time / 2),
               r0, g0, b0, 0, 0, 0, _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _tracer(Pattern* pattern, uint16_t c_time, uint16_t b_time) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->cur_color = (pattern->cur_color + 1) % (pattern->num_colors - 1);
  }

  if (pattern->tick < c_time) {
    unpackColor(pattern->colors[(pattern->cur_color + 1) % pattern->num_colors], _r, _g, _b);
  } else {
    unpackColor(pattern->colors[0], _r, _g, _b);
  }
}

void _dashdops(Pattern* pattern, uint16_t c_time, uint16_t d_time, uint16_t b_time, uint8_t dops) {
  pattern->counter1 = pattern->num_colors - 1;
  if (pattern->tick >= (pattern->counter1 * c_time) + ((d_time + b_time) * dops) + b_time) {
    pattern->tick = 0;
  }

  if (pattern->tick < pattern->counter1 * c_time) {
    unpackColor(pattern->colors[(pattern->tick / c_time) + 1], _r, _g, _b);
  } else {
    pattern->counter0 = pattern->tick - (pattern->counter1 * c_time);
    if (pattern->counter0 % (d_time + b_time) >= b_time) {
      unpackColor(pattern->colors[0], _r, _g, _b);
    } else {
      _r = 0; _g = 0; _b = 0;
    }
  }
}

void _blinke(Pattern* pattern, uint16_t c_time, uint16_t b_time) {
  if (pattern->tick >= (pattern->num_colors * c_time) + b_time) {
    pattern->tick = 0;
  }

  if (pattern->tick < pattern->num_colors * c_time) {
    unpackColor(pattern->colors[pattern->tick / c_time], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _edge(Pattern* pattern, uint16_t c_time, uint16_t e_time, uint16_t b_time) {
  pattern->counter0 = pattern->num_colors - 1;
  if (pattern->tick >= (pattern->counter0 * c_time * 2) + e_time + b_time) {
    pattern->tick = 0;
  }

  if (pattern->tick < pattern->counter0 * c_time) {
    unpackColor(pattern->colors[pattern->counter0 - (pattern->tick / c_time)], _r, _g, _b);
  } else if (pattern->tick < (pattern->counter0 * c_time) + e_time) {
    unpackColor(pattern->colors[0], _r, _g, _b);
  } else if (pattern->tick < (pattern->counter0 * c_time * 2) + e_time) {
    pattern->counter1 = ((pattern->tick - ((pattern->counter0 * c_time) + e_time)) / c_time) + 1;
    unpackColor(pattern->colors[pattern->counter1], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

uint16_t getLegoTime(uint8_t mult) {
  switch (random(0, 3)) {
    case 0:
      return 2 * mult;
    case 1:
      return 8 * mult;
    default:
      return 16 * mult;
  }
}

void _lego(Pattern* pattern, uint16_t b_time, uint8_t mult) {
  if (pattern->counter0 == 0) pattern->counter0 = getLegoTime(mult);
  if (pattern->tick >= pattern->counter0 + b_time) {
    pattern->tick = 0;
    pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
    pattern->counter0 = getLegoTime(mult);
  }

  if (pattern->tick < pattern->counter0) {
    unpackColor(pattern->colors[pattern->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _chase(Pattern* pattern, uint16_t c_time, uint16_t b_time, uint8_t steps) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->counter0++;
    if (pattern->counter0 >= steps - 1) {
      pattern->counter0 = 0;
      pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
    }
  }

  if (pattern->tick < c_time) {
    pattern->counter1 = pattern->tick / (c_time / steps);
    if (pattern->counter0 == 0) {
      unpackColor(pattern->colors[pattern->cur_color], _r, _g, _b);
    } else {
      if (pattern->counter1 < pattern->counter0) {
        unpackColor(pattern->colors[(pattern->cur_color + 1) % pattern->num_colors], _r, _g, _b);
      } else if (pattern->counter1 == pattern->counter0) {
        _r = 0; _g = 0; _b = 0;
      } else {
        unpackColor(pattern->colors[pattern->cur_color], _r, _g, _b);
      }
    }
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _morph(Pattern* pattern, uint16_t c_time, uint16_t b_time, uint8_t steps) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->counter0++;
    if (pattern->counter0 >= steps) {
      pattern->counter0 = 0;
      pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
    }
  }

  if (pattern->tick < c_time) {
    unpackColor(pattern->colors[pattern->cur_color], r0, g0, b0);
    unpackColor(pattern->colors[(pattern->cur_color + 1) % pattern->num_colors], r1, g1, b1);
    morphColor(pattern->tick + ((c_time + b_time) * pattern->counter0), (c_time + b_time) * steps,
               r0, g0, b0, r1, g1, b1, _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _comet(Pattern* pattern, uint16_t c_time, uint16_t b_time, uint8_t per_step) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->counter0 += (pattern->counter1 == 0) ? per_step : -1 * per_step;
    pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
    if (pattern->counter0 <= 0) {
      pattern->counter1 = 0;
    } else if (pattern->counter0 >= c_time) {
      pattern->counter1 = 1;
    }
  }

  if (pattern->tick <= pattern->counter0) {
    unpackColor(pattern->colors[pattern->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _candy(Pattern* pattern, uint16_t c_time, uint16_t b_time, uint8_t pick, uint8_t repeat) {
  if (pattern->tick >= c_time + b_time) {
    pattern->tick = 0;
    pattern->counter0++;
    if (pattern->counter0 >= pick) {
      pattern->counter0 = 0;
      pattern->counter1++;
      if (pattern->counter1 >= repeat) {
        pattern->counter1 = 0;
        pattern->cur_color = (pattern->cur_color + 1) % pattern->num_colors;
      }
    }
  }

  if (pattern->tick < c_time) {
    unpackColor(pattern->colors[(pattern->cur_color + pattern->counter0) % pattern->num_colors], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _user_pattern(Pattern* pattern, uint8_t idx) {
  switch (user_patterns[idx][0]) {
    case BASE_STROBE:
      _strobe(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4]);
      break;
    case BASE_PULSE:
      _pulse(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4]);
      break;
    case BASE_TRACER:
      _tracer(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4]);
      break;
    case BASE_DASHDOPS:
      _dashdops(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          (user_patterns[idx][5] << 8) + user_patterns[idx][6],
          user_patterns[idx][7]);
      break;
    case BASE_BLINKE:
      _blinke(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4]);
      break;
    case BASE_EDGE:
      _edge(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          (user_patterns[idx][5] << 8) + user_patterns[idx][6]);
      break;
    case BASE_LEGO:
      _lego(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          user_patterns[idx][3]);
      break;
    case BASE_CHASE:
      _chase(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          user_patterns[idx][5]);
      break;
    case BASE_MORPH:
      _morph(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          user_patterns[idx][5]);
      break;
    case BASE_COMET:
      _comet(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          user_patterns[idx][5]);
      break;
    case BASE_CANDY:
      _candy(pattern,
          (user_patterns[idx][1] << 8) + user_patterns[idx][2],
          (user_patterns[idx][3] << 8) + user_patterns[idx][4],
          user_patterns[idx][5],
          user_patterns[idx][6]);
      break;
    default:
      break;
  }
}


/*******************************************************************************
 ** PATTERN CLASS
 ******************************************************************************/
Pattern::Pattern(uint8_t _pattern, uint8_t _num_colors,
             uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
             uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
             uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
             uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15) {
  pattern = _pattern;
  num_colors = _num_colors;
  colors[0]  = c00;
  colors[1]  = c01;
  colors[2]  = c02;
  colors[3]  = c03;
  colors[4]  = c04;
  colors[5]  = c05;
  colors[6]  = c06;
  colors[7]  = c07;
  colors[8]  = c08;
  colors[9]  = c09;
  colors[10] = c10;
  colors[11] = c11;
  colors[12] = c12;
  colors[13] = c13;
  colors[14] = c14;
  colors[15] = c15;
}

void Pattern::load(uint16_t addr) {
  pattern    = EEPROM.read(addr + 0);
  num_colors = EEPROM.read(addr + 1);
  colors[0]  = EEPROM.read(addr + 2);
  colors[1]  = EEPROM.read(addr + 3);
  colors[2]  = EEPROM.read(addr + 4);
  colors[3]  = EEPROM.read(addr + 5);
  colors[4]  = EEPROM.read(addr + 6);
  colors[5]  = EEPROM.read(addr + 7);
  colors[6]  = EEPROM.read(addr + 8);
  colors[7]  = EEPROM.read(addr + 9);
  colors[8]  = EEPROM.read(addr + 10);
  colors[9]  = EEPROM.read(addr + 11);
  colors[10] = EEPROM.read(addr + 12);
  colors[11] = EEPROM.read(addr + 13);
  colors[12] = EEPROM.read(addr + 14);
  colors[13] = EEPROM.read(addr + 15);
  colors[14] = EEPROM.read(addr + 16);
  colors[15] = EEPROM.read(addr + 17);
}

void Pattern::save(uint16_t addr) {
  EEPROM.update(addr +  0, pattern);
  EEPROM.update(addr +  1, num_colors);
  EEPROM.update(addr +  2, colors[0]);
  EEPROM.update(addr +  3, colors[1]);
  EEPROM.update(addr +  4, colors[2]);
  EEPROM.update(addr +  5, colors[3]);
  EEPROM.update(addr +  6, colors[4]);
  EEPROM.update(addr +  7, colors[5]);
  EEPROM.update(addr +  8, colors[6]);
  EEPROM.update(addr +  9, colors[7]);
  EEPROM.update(addr + 10, colors[8]);
  EEPROM.update(addr + 11, colors[9]);
  EEPROM.update(addr + 12, colors[10]);
  EEPROM.update(addr + 13, colors[11]);
  EEPROM.update(addr + 14, colors[12]);
  EEPROM.update(addr + 15, colors[13]);
  EEPROM.update(addr + 16, colors[14]);
  EEPROM.update(addr + 17, colors[15]);
}

void Pattern::render(uint8_t& r, uint8_t& g, uint8_t& b) {
  switch (pattern) {
    case PATTERN_STROBE_FAST:
      _strobe(this, 5, 8);
      break;
    case PATTERN_STROBE:
      _strobe(this, 10, 15);
      break;
    case PATTERN_STROBE_SLOW:
      _strobe(this, 20, 30);
      break;
    case PATTERN_NANODOPS:
      _strobe(this, 1, 10);
      break;
    case PATTERN_DOPS:
      _strobe(this, 3, 20);
      break;
    case PATTERN_STROBIE:
      _strobe(this, 6, 40);
      break;
    case PATTERN_SEIZURE:
      _strobe(this, 12, 160);
      break;
    case PATTERN_ULTRA:
      _strobe(this, 15, 15);
      break;
    case PATTERN_HYPER:
      _strobe(this, 33, 32);
      break;
    case PATTERN_MEGA:
      _strobe(this, 68, 67);
      break;
    case PATTERN_PULSE_STROBE:
      _pulse(this, 25, 15);
      break;
    case PATTERN_PULSE_FAST:
      _pulse(this, 100, 25);
      break;
    case PATTERN_PULSE:
      _pulse(this, 200, 50);
      break;
    case PATTERN_PULSE_SLOW:
      _pulse(this, 400, 100);
      break;
    case PATTERN_LASER:
      _tracer(this, 3, 20);
      break;
    case PATTERN_TRACER:
      _tracer(this, 6, 44);
      break;
    case PATTERN_TAZER:
      _tracer(this, 17, 34);
      break;
    case PATTERN_DASHDOPS2:
      _dashdops(this, 3, 3, 20, 2);
      break;
    case PATTERN_DASHDOPS7:
      _dashdops(this, 8, 3, 20, 7);
      break;
    case PATTERN_DASHSTROBE:
      _dashdops(this, 5, 10, 16, 4);
      break;
    case PATTERN_DASHDASH:
      _dashdops(this, 5, 40, 20, 1);
      break;
    case PATTERN_QUICKE:
      _blinke(this, 5, 50);
      break;
    case PATTERN_BLINKE:
      _blinke(this, 10, 100);
      break;
    case PATTERN_STRIBBON:
      _blinke(this, 20, 100);
      break;
    case PATTERN_RAZOR:
      _edge(this, 2, 6, 20);
      break;
    case PATTERN_EDGE:
      _edge(this, 4, 16, 40);
      break;
    case PATTERN_SWORD:
      _edge(this, 8, 20, 60);
      break;
    case PATTERN_BARBWIRE:
      _edge(this, 2, 6, 60);
      break;
    case PATTERN_LEGO_MINI:
      _lego(this, 8, 1);
      break;
    case PATTERN_LEGO:
      _lego(this, 16, 2);
      break;
    case PATTERN_LEGO_HUGE:
      _lego(this, 50, 5);
      break;
    case PATTERN_CHASE_SHORT:
      _chase(this, 50, 10, 5);
      break;
    case PATTERN_CHASE:
      _chase(this, 100, 20, 5);
      break;
    case PATTERN_CHASE_LONG:
      _chase(this, 500, 50, 10);
      break;
    case PATTERN_MORPH:
      _morph(this, 500, 0, 1);
      break;
    case PATTERN_MORPH_SLOW:
      _morph(this, 1000, 0, 1);
      break;
    case PATTERN_MORPH_STROBE:
      _morph(this, 10, 16, 8);
      break;
    case PATTERN_MORPH_HYPER:
      _morph(this, 34, 34, 4);
      break;
    case PATTERN_RIBBON5:
      _strobe(this, 10, 0);
      break;
    case PATTERN_RIBBON10:
      _strobe(this, 20, 0);
      break;
    case PATTERN_RIBBON20:
      _strobe(this, 40, 0);
      break;
    case PATTERN_COMET_SHORT:
      _comet(this, 30, 10, 2);
      break;
    case PATTERN_COMET:
      _comet(this, 100, 20, 5);
      break;
    case PATTERN_COMET_LONG:
      _comet(this, 250, 20, 10);
      break;
    case PATTERN_CANDY2:
      _candy(this, 10, 16, 2, 12);
      break;
    case PATTERN_CANDY3:
      _candy(this, 10, 16, 3, 8);
      break;
    case PATTERN_CANDOPS:
      _candy(this, 3, 20, 3, 8);
      break;
    case PATTERN_CANDYCRUSH:
      _candy(this, 34, 34, 3, 4);
      break;
    case PATTERN_USER_00:
    case PATTERN_USER_01:
    case PATTERN_USER_02:
    case PATTERN_USER_03:
    case PATTERN_USER_04:
    case PATTERN_USER_05:
    case PATTERN_USER_06:
    case PATTERN_USER_07:
    case PATTERN_USER_08:
    case PATTERN_USER_09:
    case PATTERN_USER_10:
    case PATTERN_USER_11:
    case PATTERN_USER_12:
    case PATTERN_USER_13:
    case PATTERN_USER_14:
    case PATTERN_USER_15:
      _user_pattern(this, pattern - 48);
      break;
    default:
      break;
  }
  tick++;
  r = _r; g = _g; b = _b;
}

void Pattern::reset() { tick = cur_color = counter0 = counter1 = 0; }
