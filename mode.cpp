#include "mode.h"

Mode::Mode() {
  cur_variant = 0;
  edit_color = 0;
  accel_mode = 0;
  accel_sens = 0;
  accel_counter = 0;
  pattern[0] = Pattern();
  pattern[1] = Pattern();
}

void Mode::render(uint8_t& r, uint8_t& g, uint8_t& b) {
  pattern[0].render(r, g, b, cur_variant == 0);
  pattern[1].render(r, g, b, cur_variant == 1);
}

void Mode::init() {
  cur_variant = 0;
  edit_color = 0;
  accel_counter = 0;
  pattern[0].reset();
  pattern[1].reset();
}

void Mode::load(uint16_t addr) {
  accel_mode = EEPROM.read(addr);
  accel_sens = EEPROM.read(addr + 1);
  pattern[0].load(addr + 2);
  pattern[1].load(addr + 20);
}

void Mode::save(uint16_t addr) {
  EEPROM.update(addr, accel_mode);
  EEPROM.update(addr + 1, accel_sens);
  pattern[0].save(addr + 2);
  pattern[1].save(addr + 20);
}

void Mode::changeColor(int8_t v) {
  pattern[cur_variant].colors[edit_color] =
    (pattern[cur_variant].colors[edit_color] + v + NUM_COLORS) % NUM_COLORS;
}

void Mode::changeShade() {
  pattern[cur_variant].colors[edit_color] += 0x40;
}
