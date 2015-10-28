#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "anim.h"

uint8_t _r, _g, _b;
uint8_t r0, g0, b0;
uint8_t r1, g1, b1;


const PROGMEM uint8_t color_palette[64][3] = {
  // Dims
  {  0,   0,   0}, // 0x00
  { 32,  32,  32}, // 0x01
  { 48,   0,   0}, // 0x02
  { 40,  40,   0}, // 0x03
  {  0,  48,   0}, // 0x04
  {  0,  40,  40}, // 0x05
  {  0,   0,  48}, // 0x06
  { 40,   0,  40}, // 0x07

  // Red -> green
  {255,   0,   0}, // 0x08
  {252,  64,   0}, // 0x09
  {248, 128,   0}, // 0x0A
  {244, 192,   0}, // 0x0B
  {240, 240,   0}, // 0x0C
  {192, 244,   0}, // 0x0D
  {128, 248,   0}, // 0x0E
  { 64, 252,   0}, // 0x0F

  // Green -> blue
  {  0, 255,   0}, // 0x10
  {  0, 252,  64}, // 0x11
  {  0, 248, 128}, // 0x12
  {  0, 244, 192}, // 0x13
  {  0, 240, 240}, // 0x14
  {  0, 192, 244}, // 0x15
  {  0, 128, 248}, // 0x16
  {  0,  64, 252}, // 0x17

  // Blue -> red
  {  0,   0, 255}, // 0x18
  { 64,   0, 252}, // 0x19
  {128,   0, 248}, // 0x1A
  {192,   0, 244}, // 0x1B
  {240,   0, 240}, // 0x1C
  {244,   0, 192}, // 0x1D
  {248,   0, 128}, // 0x1E
  {252,   0,  64}, // 0x1F

  // Red saturated
  {240,  32,  32}, // 0x20
  {216,  64,  64}, // 0x21
  {192,  96,  96}, // 0x22
  {160, 128, 128}, // 0x23

  // Yellow saturated
  {224, 224,  32}, // 0x24
  {200, 200,  64}, // 0x25
  {176, 176,  96}, // 0x26
  {152, 152, 128}, // 0x27

  // Green saturated
  { 32, 240,  32}, // 0x28
  { 64, 216,  64}, // 0x29
  { 96, 192,  96}, // 0x2A
  {128, 160, 128}, // 0x2B

  // Cyan saturated
  { 32, 224, 224}, // 0x2C
  { 64, 200, 200}, // 0x2D
  { 96, 176, 176}, // 0x2E
  {128, 152, 152}, // 0x2F

  // Blue saturated
  { 32,  32, 240}, // 0x30
  { 64,  64, 216}, // 0x31
  { 96,  96, 192}, // 0x32
  {128, 128, 160}, // 0x33

  // Magenta saturated
  {224,  32, 224}, // 0x34
  {200,  64, 200}, // 0x35
  {176,  96, 176}, // 0x36
  {152, 128, 152}, // 0x37

  // Whites
  {160, 160, 160}, // 0x38
  { 50, 114,  96}, // 0x39
  { 50, 147, 135}, // 0x3A
  {128,  50,  96}, // 0x3B
  { 96,  50, 128}, // 0x3C
  {128,  96,  50}, // 0x3D
  { 96, 128,  50}, // 0x3E
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
      if (tick >= 10 + 16) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 10) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_HYPER:
      if (tick >= 34 + 34) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 34) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_DOPS:
      if (tick >= 3 + 20) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 3) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_STROBIE:
      if (tick >= 6 + 46) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 6) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_PULSE:
      if (tick >= 200 + 50) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 100) {
        unpackColor(palette[cur_variant][cur_color], &r0, &g0, &b0);
        morphColor(tick, 100, 0, 0, 0, r0, g0, b0, &_r, &_g, &_b);
      } else if (tick < 200) {
        unpackColor(palette[cur_variant][cur_color], &r0, &g0, &b0);
        morphColor(tick - 100, 100, r0, g0, b0, 0, 0, 0, &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_SEIZURE:
      if (tick >= 10 + 190) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }

      if (tick < 10) {
        unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_TRACER:
      if (tick >= 6 + 46) {
        tick = 0;
        cur_color = (cur_color + 1) % (num_colors[cur_variant] - 1);
      }

      if (tick < 6) {
        unpackColor(palette[cur_variant][(cur_color + 1) % num_colors[cur_variant]], &_r, &_g, &_b);
      } else {
        unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
      }
      break;

    case PRIME_DASHDOPS:
      counter1 = num_colors[cur_variant] - 1;
      if (tick >= (counter1 * 22) + (23 * 7) + 20) {
        tick = 0;
      }

      if (tick < counter1 * 22) {
        unpackColor(palette[cur_variant][(tick / 22) + 1], &_r, &_g, &_b);
      } else {
        counter0 = tick - (counter1 * 22);
        if (counter0 % 23 > 19) {
          unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
        } else {
          _r = 0; _g = 0; _b = 0;
        }
      }
      break;

    case PRIME_BLINKE:
      if (tick >= (num_colors[cur_variant] * 10) + 100) {
        tick = 0;
      }

      if (tick < num_colors[cur_variant] * 10) {
        unpackColor(palette[cur_variant][tick / 10], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_EDGE:
      if (counter0 == 0) counter0 = num_colors[cur_variant] - 1;
      if (tick >= (counter0 * 8) + 16 + 40) {
        tick = 0;
      }

      if (tick < counter0 * 4) {
        unpackColor(palette[cur_variant][counter0 - (tick / 4)], &_r, &_g, &_b);
      } else if (tick < (counter0 * 4) + 16) {
        unpackColor(palette[cur_variant][0], &_r, &_g, &_b);
      } else if (tick < (counter0 * 8) + 16) {
        unpackColor(palette[cur_variant][((tick - ((counter0 * 4) + 16)) / 4) + 1], &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_LEGO:
      if (counter0 == 0) counter0 = getLegoTime();
      if (tick >= counter0 + 16) {
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
      if (tick >= 120) {
        tick = 0;
        counter0++;
        if (counter0 >= 4) {
          counter0 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        }
      }

      if (tick < 100) {
        counter1 = tick / 20;
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
      if (tick >= 34 + 34) {
        tick = 0;
        counter0++;
        if (counter0 >= 4) {
          counter0 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        }
      }

      if (tick < 34) {
        unpackColor(palette[cur_variant][cur_color],                    &r0, &g0, &b0);
        unpackColor(palette[cur_variant][(cur_color + 1) % num_colors[cur_variant]], &r1, &g1, &b1);
        morphColor(tick + (68 * counter0), 68 * 4, r0, g0, b0, r1, g1, b1, &_r, &_g, &_b);
      } else {
        _r = 0; _g = 0; _b = 0;
      }
      break;

    case PRIME_RIBBON:
      if (tick >= 22) {
        tick = 0;
        cur_color = (cur_color + 1) % num_colors[cur_variant];
      }
      unpackColor(palette[cur_variant][cur_color], &_r, &_g, &_b);
      break;

    case PRIME_COMET:
      if (tick >= 30 + 16) {
        tick = 0;
        counter0 += (counter1 == 0) ? 2 : -2;
        if (counter0 <= 0) {
          counter1 = 0;
          cur_color = (cur_color + 1) % num_colors[cur_variant];
        } else if (counter0 >= 30) {
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
      if (tick >= 10 + 16) {
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

      if (tick < 10) {
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
