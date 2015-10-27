#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "anim.h"

uint8_t _r, _g, _b;
uint8_t r0, g0, b0;
uint8_t r1, g1, b1;


const PROGMEM uint8_t color_palette[64][3] = {
  // Blank and whites
  {  0,   0,   0},  // 0x00
  {144, 144, 144},  // 0x01
  {192, 140, 140},  // 0x02
  {184, 184, 148},  // 0x03
  {140, 192, 140},  // 0x04
  {148, 184, 184},  // 0x05
  {140, 140, 192},  // 0x06
  {184, 148, 184},  // 0x07

  // Red - green
  {255,   0,   0},  // 0x08
  {252,  63,   0},  // 0x09
  {248, 127,   0},  // 0x0A
  {244, 191,   0},  // 0x0B
  {240, 240,   0},  // 0x0C
  {191, 244,   0},  // 0x0D
  {127, 248,   0},  // 0x0E
  { 63, 252,   0},  // 0x0F

  // Green - blue
  {  0, 255,   0},  // 0x10
  {  0, 252,  63},  // 0x11
  {  0, 248, 127},  // 0x12
  {  0, 244, 191},  // 0x13
  {  0, 240, 240},  // 0x14
  {  0, 191, 244},  // 0x15
  {  0, 127, 248},  // 0x16
  {  0,  63, 252},  // 0x17

  // Blue - red
  {  0,   0, 255},  // 0x18
  { 63,   0, 252},  // 0x19
  {127,   0, 248},  // 0x1A
  {191,   0, 244},  // 0x1B
  {240,   0, 240},  // 0x1C
  {244,   0, 191},  // 0x1D
  {248,   0, 127},  // 0x1E
  {252,   0,  63},  // 0x1F

  // Red - green saturated
  {224,  64,  64},  // 0x20
  {216,  96,  64},  // 0x21
  {208, 128,  64},  // 0x22
  {200, 160,  64},  // 0x23
  {192, 192,  64},  // 0x24
  {160, 200,  64},  // 0x25
  {128, 208,  64},  // 0x26
  {104, 216,  64},  // 0x27

  // Green - blue saturated
  { 64, 224,  64},  // 0x28
  { 64, 216,  96},  // 0x29
  { 64, 208, 128},  // 0x2A
  { 64, 200, 160},  // 0x2B
  { 64, 192, 192},  // 0x2C
  { 64, 160, 200},  // 0x2D
  { 64, 128, 208},  // 0x2E
  { 64, 104, 216},  // 0x2F

  // Blue - red saturated
  { 64,  64, 224},  // 0x30
  { 96,  64, 216},  // 0x31
  {128,  64, 208},  // 0x32
  {160,  64, 200},  // 0x33
  {192,  64, 192},  // 0x34
  {200,  64, 160},  // 0x35
  {208,  64, 128},  // 0x36
  {216,  64, 104},  // 0x37

  // Dim colors
  { 32,  32,  32},  // 0x38
  { 48,   0,   0},  // 0x39
  { 40,  40,   0},  // 0x3A
  {  0,  48,   0},  // 0x3B
  {  0,  40,  40},  // 0x3C
  {  0,   0,  48},  // 0x3D
  { 40,   0,  40},  // 0x3E
};

void unpackColor(uint8_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
  uint8_t shade = color >> 6;                                   // shade is first 2 bits
  uint8_t idx = color & 0b00111111;                             // palette index is last 6 bits
  *r = pgm_read_byte(&color_palette[idx][0]); *r = *r >> shade; // get red value and shade
  *g = pgm_read_byte(&color_palette[idx][1]); *g = *g >> shade; // get green value and shade
  *b = pgm_read_byte(&color_palette[idx][2]); *b = *b >> shade; // get blue value and shade
}

void morphColor(uint16_t tick, uint16_t morph_time,
    uint8_t r0, uint8_t g0, uint8_t b0,
    uint8_t r1, uint8_t g1, uint8_t b1,
    uint8_t *r, uint8_t *g, uint8_t *b)
{
  *r = r0 + (int)(r1 - r0) * (tick / (float)morph_time);
  *g = g0 + (int)(g1 - g0) * (tick / (float)morph_time);
  *b = b0 + (int)(b1 - b0) * (tick / (float)morph_time);
}

uint8_t getLegoTime() {
  switch (random(0, 3)) {
    case 0:
      return 2;
    case 1:
      return 8;
    default:
      return 16;
  }
}


Mode::Mode(uint16_t user_eeprom_addr, uint8_t user_acc_mode, uint8_t user_acc_sensitivity,
    uint8_t prime0, uint8_t num_colors0,
    uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03, uint8_t c04, uint8_t c05,
    uint8_t c06, uint8_t c07, uint8_t c08, uint8_t c09, uint8_t c0A, uint8_t c0B,
    uint8_t prime1, uint8_t num_colors1,
    uint8_t c10, uint8_t c11, uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15,
    uint8_t c16, uint8_t c17, uint8_t c18, uint8_t c19, uint8_t c1A, uint8_t c1B)
{
  eeprom_addr = user_eeprom_addr;
  acc_mode = user_acc_mode;
  acc_sensitivity = user_acc_sensitivity;

  prime[0] = prime0;
  num_colors[0] = num_colors0;
  palette[0][0] = c00;
  palette[0][1] = c01;
  palette[0][2] = c02;
  palette[0][3] = c03;
  palette[0][4] = c04;
  palette[0][5] = c05;
  palette[0][6] = c06;
  palette[0][7] = c07;
  palette[0][8] = c08;
  palette[0][9] = c09;
  palette[0][10] = c0A;
  palette[0][11] = c0B;

  prime[1] = prime1;
  num_colors[1] = num_colors1;
  palette[1][0] = c10;
  palette[1][1] = c11;
  palette[1][2] = c12;
  palette[1][3] = c13;
  palette[1][4] = c14;
  palette[1][5] = c15;
  palette[1][6] = c16;
  palette[1][7] = c17;
  palette[1][8] = c18;
  palette[1][9] = c19;
  palette[1][10] = c1A;
  palette[1][11] = c1B;
}

void Mode::render(uint8_t *r, uint8_t *g, uint8_t *b) {
  switch (prime[cur_variant]) {
    case PRIME_STROBE:
      if (tick >= 5 + 8) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 5) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_HYPER:
      if (tick >= 17 + 17) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 17) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_DOPS:
      if (tick >= 1 + 10) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 1) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_STROBIE:
      if (tick >= 3 + 23) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 3) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_PULSE:
      if (tick >= 100 + 25) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 50) {
        unpackColor(palette[cur_variant][cur_color], &r0, &g0, &b0);
        morphColor(tick, 50, 0, 0, 0, r0, g0, b0, &_r, &_g, &_b);
      } else if (tick < 100) {
        unpackColor(palette[cur_variant][cur_color], &r0, &g0, &b0);
        morphColor(tick - 50, 50, r0, g0, b0, 0, 0, 0, &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_SEIZURE:
      if (tick >= 5 + 95) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 5) {
        unpackColor(palette[cur_variant][cur_color], &r0, &g0, &b0);
        morphColor(tick, 5, 0, 0, 0, r0, g0, b0, &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_TRACER:
      if (tick >= 3 + 23) {
        tick = 0;
        cur_color = (cur_color + 1) % (num_colors[cur_variant] - 1);
      }

      if (tick < 3) {
        unpackColor(palette[cur_variant][(cur_color + 1) % num_colors[cur_variant]], &_r, &_g, &_b);
      } else {
        unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
      }
      break;

    case PRIME_DASHDOPS:
      if (tick >= ((num_colors[cur_variant] - 1) * 11) + ((1 + 10) * 7) + 10) {
        tick = 0;
      }

      if (tick < (num_colors[cur_variant] - 1) * 11) {
        counter0 = (tick / 11) + 1;
        unpackColor(palette[cur_variant][counter0], &_r, &_g, &_b);
      } else {
        counter0 = tick - ((num_colors[cur_variant] - 1) * 11);
        if (counter0 % 11 == 10) {
          unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
        } else {
          _r = 0; _g = 0; _b = 0;
        }
      }
      break;

    case PRIME_BLINKE:
      if (tick >= (num_colors[cur_variant] * 5) + 50) {
        tick = 0;
      }

      if (tick < (num_colors[cur_variant] * 5)) {
        unpackColor(palette[cur_variant][tick / 5], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_EDGE:
      if (counter0 == 0) counter0 = num_colors[cur_variant] - 1;
      if (tick >= (counter0 * 4) + 5 + 20) {
        tick = 0;
      }

      if (tick < counter0 * 2) {
        unpackColor(palette[cur_variant][counter0 - (tick / 2)], &_r, &_g, &_b);
      } else if (tick < (counter0 * 2) + 5) {
        unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
      } else if (tick < (counter0 * 4) + 5) {
        unpackColor(palette[cur_variant][((tick - ((counter0 * 2) + 5)) / 2) + 1], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_LEGO:
      if (counter0 == 0) counter0 = getLegoTime();
      if (tick >= counter0 + 8) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
        counter0 = getLegoTime();
      }

      if (tick < counter0) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_CHASE:
      if (tick >= 60) {
        tick = 0;
        counter0++;
        if (counter0 >= 4) {
          counter0 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        }
      }

      if (tick < 50) {
        counter1 = tick / 10;
        if (counter0 == 0) {
          unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
        } else {
          if (counter1 < counter0) {
            unpackColor(palette[cur_variant][(cur_color + 1) % num_colors[cur_variant]], &_r, &_g, &_b);
          } else if (counter1 == counter0) {
            _r = 0; _g = 0; _b = 0;
          } else {
            unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
          }
        }
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_MORPH:
      if (tick >= 17 + 17) {
        tick = 0;
        counter0++;
        if (counter0 >= 4) {
          counter0 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        }
      }

      if (tick < 17) {
        unpackColor(palette[cur_variant][cur_color],                                 &r0, &g0, &b0);
        unpackColor(palette[cur_variant][(cur_color + 1) % num_colors[cur_variant]], &r1, &g1, &b1);
        morphColor(tick, 136, r0, g0, b0, r1, g1, b1, &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_RIBBON:
      if (tick >= 11) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }
      unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      break;

    case PRIME_COMET:
      if (tick >= 15 + 8) {
        tick = 0;
        counter0 += (counter1 == 0) ? 1 : -1;
        if (counter0 <= 0) {
          counter1 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        } else if (counter0 >= 15) {
          counter1 = 1;
        }
      }

      if (tick <= counter0) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_CANDY:
      if (tick >= 5 + 8) {
        tick = 0;
        counter0++;
        if (counter0 >= 3) {
          counter0 = 0;
          counter1++;
          if (counter1 >= 8) {
            counter1 = 0;
            cur_color = (cur_color + 1) % num_colors[cur_variant];
          }
        }
      }

      if (tick < 5) {
        unpackColor(palette[cur_variant][(cur_color + counter0) % num_colors[cur_variant]], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    default:
      break;
  }
  tick++;
  *r = _r; *g = _g; *b = _b;
}

void Mode::init() {
  tick = 0;
  cur_color = 0;
  cur_variant = 0;
  counter0 = 0;
  counter1 = 0;
  acc_counter = 0;
}

void Mode::reset() {
  tick = 0;
  cur_color = 0;
  counter0 = 0;
  counter1 = 0;
  acc_counter = 0;
}

void Mode::updateAcc(float fxg, float fyg, float fzg) {
  float pitch, level;
  uint8_t thresh;

  if (acc_counter < 0) acc_counter = 0;

  switch (acc_mode) {
    case AMODE_SPEED:
      if (acc_sensitivity == ASENS_LOW) {
        level = 2.0; thresh = 25;
      } else if (acc_sensitivity == ASENS_MEDIUM) {
        level = 1.7; thresh = 25;
      } else if (acc_sensitivity == ASENS_HIGH) {
        level = 1.0; thresh = 10;
      }

      pitch = abs(fxg) + abs(fyg) + abs(fzg) - 1.0;

      if (cur_variant == 0) {
        if (pitch > level) {
          acc_counter++;
        } else {
          acc_counter = 0;
        }
        if (acc_counter > thresh) {
          cur_variant = 1;
          acc_counter = 25;
        }
      } else {
        if (pitch > 1.0) {
          acc_counter = 25;
        } else {
          acc_counter--;
        }
        if (acc_counter <= 0) cur_variant = 0;
      }
      break;

    case AMODE_TILTX:
      pitch = (atan2(fxg, sqrt(fyg * fyg + fzg * fzg)) * 180.0) / M_PI;
      if (cur_variant == 0) {
        acc_counter += (pitch < -75) ? 1 : -1;
      } else {
        acc_counter += (pitch > 75) ? 1 : -1;
      }
      if (acc_counter > 15 << acc_sensitivity) {
        acc_counter = 0;
        cur_variant = (cur_variant == 0) ? 1 : 0;
      }
      break;

    case AMODE_TILTY:
      pitch = (atan2(fyg, sqrt(fxg * fxg + fzg * fzg)) * 180.0) / M_PI;
      if (cur_variant == 0) {
        acc_counter += (pitch < -75) ? 1 : -1;
      } else {
        acc_counter += (pitch > 75) ? 1 : -1;
      }
      if (acc_counter > 15 << acc_sensitivity) {
        acc_counter = 0;
        cur_variant = (cur_variant == 0) ? 1 : 0;
      }
      break;

    case AMODE_FLIPZ:
      if (cur_variant == 0) {
        acc_counter += (fzg < -0.95) ? 1 : -1;
      } else {
        acc_counter += (fzg > 0.95) ? 1 : -1;
      }
      if (acc_counter > 15 << acc_sensitivity) {
        acc_counter = 0;
        cur_variant = (cur_variant == 0) ? 1 : 0;
      }
      break;

    default:
      break;
  }
}

void Mode::save() {
  EEPROM.update(eeprom_addr +  0, acc_mode);
  EEPROM.update(eeprom_addr +  1, acc_sensitivity);

  EEPROM.update(eeprom_addr +  2, prime[0]);
  EEPROM.update(eeprom_addr +  3, num_colors[0]);
  EEPROM.update(eeprom_addr +  4, palette[0][0]);
  EEPROM.update(eeprom_addr +  5, palette[0][1]);
  EEPROM.update(eeprom_addr +  6, palette[0][2]);
  EEPROM.update(eeprom_addr +  7, palette[0][3]);
  EEPROM.update(eeprom_addr +  8, palette[0][4]);
  EEPROM.update(eeprom_addr +  9, palette[0][5]);
  EEPROM.update(eeprom_addr + 10, palette[0][6]);
  EEPROM.update(eeprom_addr + 11, palette[0][7]);
  EEPROM.update(eeprom_addr + 12, palette[0][8]);
  EEPROM.update(eeprom_addr + 13, palette[0][9]);
  EEPROM.update(eeprom_addr + 14, palette[0][10]);
  EEPROM.update(eeprom_addr + 15, palette[0][11]);

  EEPROM.update(eeprom_addr + 16, prime[1]);
  EEPROM.update(eeprom_addr + 17, num_colors[1]);
  EEPROM.update(eeprom_addr + 18, palette[1][0]);
  EEPROM.update(eeprom_addr + 19, palette[1][1]);
  EEPROM.update(eeprom_addr + 20, palette[1][2]);
  EEPROM.update(eeprom_addr + 21, palette[1][3]);
  EEPROM.update(eeprom_addr + 22, palette[1][4]);
  EEPROM.update(eeprom_addr + 23, palette[1][5]);
  EEPROM.update(eeprom_addr + 24, palette[1][6]);
  EEPROM.update(eeprom_addr + 25, palette[1][7]);
  EEPROM.update(eeprom_addr + 26, palette[1][8]);
  EEPROM.update(eeprom_addr + 27, palette[1][9]);
  EEPROM.update(eeprom_addr + 28, palette[1][10]);
  EEPROM.update(eeprom_addr + 29, palette[1][11]);
}

void Mode::load() {
  acc_mode        = EEPROM.read(eeprom_addr + 0);
  acc_sensitivity = EEPROM.read(eeprom_addr + 1);

  prime[0]        = EEPROM.read(eeprom_addr + 2);
  num_colors[0]   = EEPROM.read(eeprom_addr + 3);
  palette[0][0]   = EEPROM.read(eeprom_addr + 4);
  palette[0][1]   = EEPROM.read(eeprom_addr + 5);
  palette[0][2]   = EEPROM.read(eeprom_addr + 6);
  palette[0][3]   = EEPROM.read(eeprom_addr + 7);
  palette[0][4]   = EEPROM.read(eeprom_addr + 8);
  palette[0][5]   = EEPROM.read(eeprom_addr + 9);
  palette[0][6]   = EEPROM.read(eeprom_addr + 10);
  palette[0][7]   = EEPROM.read(eeprom_addr + 11);
  palette[0][8]   = EEPROM.read(eeprom_addr + 12);
  palette[0][9]   = EEPROM.read(eeprom_addr + 13);
  palette[0][10]  = EEPROM.read(eeprom_addr + 14);
  palette[0][11]  = EEPROM.read(eeprom_addr + 15);

  prime[1]        = EEPROM.read(eeprom_addr + 16);
  num_colors[1]   = EEPROM.read(eeprom_addr + 17);
  palette[1][0]   = EEPROM.read(eeprom_addr + 18);
  palette[1][1]   = EEPROM.read(eeprom_addr + 19);
  palette[1][2]   = EEPROM.read(eeprom_addr + 20);
  palette[1][3]   = EEPROM.read(eeprom_addr + 21);
  palette[1][4]   = EEPROM.read(eeprom_addr + 22);
  palette[1][5]   = EEPROM.read(eeprom_addr + 23);
  palette[1][6]   = EEPROM.read(eeprom_addr + 24);
  palette[1][7]   = EEPROM.read(eeprom_addr + 25);
  palette[1][8]   = EEPROM.read(eeprom_addr + 26);
  palette[1][9]   = EEPROM.read(eeprom_addr + 27);
  palette[1][10]  = EEPROM.read(eeprom_addr + 28);
  palette[1][11]  = EEPROM.read(eeprom_addr + 29);
}
