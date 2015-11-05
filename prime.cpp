#include <EEPROM.h>
#include "palette.h"
#include "prime.h"

uint8_t _r, _g, _b;
uint8_t r0, g0, b0;
uint8_t r1, g1, b1;

/*******************************************************************************
 ** BASE ANIMATIONS
 ******************************************************************************/
void _strobe(Prime* prime, uint8_t c_time, uint8_t b_time) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
  }

  if (prime->tick < c_time) {
    unpackColor(prime->colors[prime->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _pulse(Prime* prime, uint8_t c_time, uint8_t b_time) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
  }

  if (prime->tick < c_time / 2) {
    unpackColor(prime->colors[prime->cur_color], r0, g0, b0);
    morphColor(prime->tick, c_time / 2,
               0, 0, 0, r0, g0, b0, _r, _g, _b);
  } else if (prime->tick < c_time) {
    unpackColor(prime->colors[prime->cur_color], r0, g0, b0);
    morphColor(prime->tick - (c_time / 2), c_time - (c_time / 2),
               r0, g0, b0, 0, 0, 0, _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _tracer(Prime* prime, uint8_t c_time, uint8_t b_time) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->cur_color = (prime->cur_color + 1) % (prime->num_colors - 1);
  }

  if (prime->tick < c_time) {
    unpackColor(prime->colors[(prime->cur_color + 1) % prime->num_colors], _r, _g, _b);
  } else {
    unpackColor(prime->colors[0], _r, _g, _b);
  }
}

void _dashdops(Prime* prime, uint8_t c_time, uint8_t d_time, uint8_t b_time, uint8_t dops) {
  prime->counter1 = prime->num_colors - 1;
  if (prime->tick >= (prime->counter1 * c_time) + ((d_time + b_time) * dops) + b_time) {
    prime->tick = 0;
  }

  if (prime->tick < prime->counter1 * c_time) {
    unpackColor(prime->colors[(prime->tick / c_time) + 1], _r, _g, _b);
  } else {
    prime->counter0 = prime->tick - (prime->counter1 * c_time);
    if (prime->counter0 % (d_time + b_time) >= b_time) {
      unpackColor(prime->colors[0], _r, _g, _b);
    } else {
      _r = 0; _g = 0; _b = 0;
    }
  }
}

void _blinke(Prime* prime, uint8_t c_time, uint8_t b_time) {
  if (prime->tick >= (prime->num_colors * c_time) + b_time) {
    prime->tick = 0;
  }

  if (prime->tick < prime->num_colors * c_time) {
    unpackColor(prime->colors[prime->tick / c_time], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _edge(Prime* prime, uint8_t c_time, uint8_t e_time, uint8_t b_time) {
  prime->counter0 = prime->num_colors - 1;
  if (prime->tick >= (prime->counter0 * c_time * 2) + e_time + b_time) {
    prime->tick = 0;
  }

  if (prime->tick < prime->counter0 * c_time) {
    unpackColor(prime->colors[prime->counter0 - (prime->tick / c_time)], _r, _g, _b);
  } else if (prime->tick < (prime->counter0 * c_time) + e_time) {
    unpackColor(prime->colors[0], _r, _g, _b);
  } else if (prime->tick < (prime->counter0 * c_time * 2) + e_time) {
    prime->counter1 = ((prime->tick - ((prime->counter0 * c_time) + e_time)) / c_time) + 1;
    unpackColor(prime->colors[prime->counter1], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

uint8_t getLegoTime() {
  switch (random(0, 3)) {
    case 0:
      return 4;
    case 1:
      return 16;
    default:
      return 32;
  }
}

void _lego(Prime* prime, uint8_t b_time) {
  if (prime->counter0 == 0) prime->counter0 = getLegoTime();
  if (prime->tick >= prime->counter0 + b_time) {
    prime->tick = 0;
    prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
    prime->counter0 = getLegoTime();
  }

  if (prime->tick < prime->counter0) {
    unpackColor(prime->colors[prime->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _chase(Prime* prime, uint8_t c_time, uint8_t b_time, uint8_t steps) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->counter0++;
    if (prime->counter0 >= steps - 1) {
      prime->counter0 = 0;
      prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
    }
  }

  if (prime->tick < c_time) {
    prime->counter1 = prime->tick / (c_time / steps);
    if (prime->counter0 == 0) {
      unpackColor(prime->colors[prime->cur_color], _r, _g, _b);
    } else {
      if (prime->counter1 < prime->counter0) {
        unpackColor(prime->colors[(prime->cur_color + 1) % prime->num_colors], _r, _g, _b);
      } else if (prime->counter1 == prime->counter0) {
        _r = 0; _g = 0; _b = 0;
      } else {
        unpackColor(prime->colors[prime->cur_color], _r, _g, _b);
      }
    }
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _morph(Prime* prime, uint8_t c_time, uint8_t b_time, uint8_t steps) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->counter0++;
    if (prime->counter0 >= steps) {
      prime->counter0 = 0;
      prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
    }
  }

  if (prime->tick < c_time) {
    unpackColor(prime->colors[prime->cur_color], r0, g0, b0);
    unpackColor(prime->colors[(prime->cur_color + 1) % prime->num_colors], r1, g1, b1);
    morphColor(prime->tick + ((c_time + b_time) * prime->counter0), (c_time + b_time) * steps,
               r0, g0, b0, r1, g1, b1, _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _comet(Prime* prime, uint8_t c_time, uint8_t b_time, uint8_t per_step) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->counter0 += (prime->counter1 == 0) ? per_step : -1 * per_step;
    prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
    if (prime->counter0 <= 0) {
      prime->counter1 = 0;
    } else if (prime->counter0 >= c_time) {
      prime->counter1 = 1;
    }
  }

  if (prime->tick <= prime->counter0) {
    unpackColor(prime->colors[prime->cur_color], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}

void _candy(Prime* prime, uint8_t c_time, uint8_t b_time, uint8_t pick, uint8_t repeat) {
  if (prime->tick >= c_time + b_time) {
    prime->tick = 0;
    prime->counter0++;
    if (prime->counter0 >= pick) {
      prime->counter0 = 0;
      prime->counter1++;
      if (prime->counter1 >= repeat) {
        prime->counter1 = 0;
        prime->cur_color = (prime->cur_color + 1) % prime->num_colors;
      }
    }
  }

  if (prime->tick < c_time) {
    unpackColor(prime->colors[(prime->cur_color + prime->counter0) % prime->num_colors], _r, _g, _b);
  } else {
    _r = 0; _g = 0; _b = 0;
  }
}


/*******************************************************************************
 ** PRIME CLASS
 ******************************************************************************/
#ifdef PALETTE_12
Prime::Prime(uint8_t _pattern, uint8_t _num_colors,
             uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
             uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
             uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11) {
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
}

void Prime::load(uint16_t addr) {
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
}

void Prime::save(uint16_t addr) {
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
}
#endif

#ifdef PALETTE_16
Prime::Prime(uint8_t _pattern, uint8_t _num_colors,
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

void Prime::load(uint16_t addr) {
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

void Prime::save(uint16_t addr) {
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
#endif

void Prime::render(uint8_t& r, uint8_t& g, uint8_t& b) {
  switch (pattern) {
    case PRIME_STROBE:
      _strobe(this, 10, 16);
      break;
    case PRIME_HYPER:
      _strobe(this, 34, 34);
      break;
    case PRIME_DOPS:
      _strobe(this, 3, 20);
      break;
    case PRIME_STROBIE:
      _strobe(this, 6, 40);
      break;
    case PRIME_PULSE:
      _pulse(this, 200, 50);
      break;
    case PRIME_SEIZURE:
      _strobe(this, 10, 190);
      break;
    case PRIME_TRACER:
      _tracer(this, 6, 40);
      break;
    case PRIME_DASHDOPS:
      _dashdops(this, 3, 3, 20, 2);
      break;
    case PRIME_BLINKE:
      _blinke(this, 10, 100);
      break;
    case PRIME_EDGE:
      _edge(this, 4, 16, 40);
      break;
    case PRIME_LEGO:
      _lego(this, 16);
      break;
    case PRIME_CHASE:
      _chase(this, 100, 20, 5);
      break;
    case PRIME_MORPH:
      _morph(this, 34, 34, 4);
      break;
    case PRIME_RIBBON:
      _strobe(this, 22, 0);
      break;
    case PRIME_COMET:
      _comet(this, 30, 16, 2);
      break;
    case PRIME_CANDY:
      _candy(this, 10, 16, 3, 8);
      break;
    default:
      break;
  }
  tick++;
  r = _r; g = _g; b = _b;
}

void Prime::reset() { tick = cur_color = counter0 = counter1 = 0; }
