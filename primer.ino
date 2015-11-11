/*
The MIT License (MIT)

Copyright (c) 2015 John Joseph Miller

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include "elapsedMillis.h"
#include "palette.h"
#include "pattern.h"

#define DEBUG
#define NUM_MODES 16
#define NUM_BUNDLES 4
#define EEPROM_VERSION 101

// 0 - 640      Modes
// 640 - 708    Bundles
// 720 - 816    Palette
// 820 - 948    Custom Patterns
// 1023         Version
#define VERSION_ADDR 1023
#define BUNDLE_EEPROM_ADDR 640
#define PALETTE_EEPROM_ADDR 720
#define PATTERNS_EEPROM_ADDR 820
uint16_t addrs[NUM_MODES] = {
  0, 40, 80, 120, 160, 200, 240, 280, 320, 360, 400, 440, 480, 520, 560, 600,
};

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
    Mode() : accel_mode(0), accel_sens(0), cur_variant(0), edit_color(0), acc_counter(0) {
      pattern[0] = Pattern();
      pattern[1] = Pattern();
    }

    void render(uint8_t& r, uint8_t& g, uint8_t& b) {
      if (cur_variant == 0) {
        pattern[1].render(r, g, b);
        pattern[0].render(r, g, b);
      } else {
        pattern[0].render(r, g, b);
        pattern[1].render(r, g, b);
      }
    }

    void init() {
      pattern[0].reset();
      pattern[1].reset();
      edit_color = 0;
    }

    void load(uint16_t addr) {
      accel_mode = EEPROM.read(addr);
      accel_sens = EEPROM.read(addr + 1);
      pattern[0].load(addr + 2);
      pattern[1].load(addr + 20);
    }

    void save(uint16_t addr) {
      EEPROM.update(addr, accel_mode);
      EEPROM.update(addr + 1, accel_sens);
      pattern[0].save(addr + 2);
      pattern[1].save(addr + 20);
    }

    void changeColor(int8_t v) {
      pattern[cur_variant].colors[edit_color] = (pattern[cur_variant].colors[edit_color] + v + NUM_COLORS) % NUM_COLORS;
    }

    void changeShade() {
      pattern[cur_variant].colors[edit_color] += 0x40;
    }

    void updateAccel(float fxg, float fyg, float fzg) {
      float pitch, level;
      uint8_t thresh;

      if (acc_counter < 0) acc_counter = 0;

      switch (accel_mode) {
        case AMODE_SPEED:
          if (accel_sens == ASENS_LOW) {
            level = 1.5; thresh = 25;
          } else if (accel_sens == ASENS_MEDIUM) {
            level = 1.4; thresh = 25;
          } else if (accel_sens == ASENS_HIGH) {
            level = 1.2; thresh = 5;
          }
          pitch = max(abs(fxg), max(abs(fyg), abs(fzg)));
          if (cur_variant == 0) {
            if (pitch > level) {
              acc_counter++;
            } else {
              acc_counter = 0;
            }
            if (acc_counter > thresh) {
              cur_variant = 1;
              acc_counter = thresh;
            }
          } else {
            if (pitch > 1.15) {
              acc_counter = thresh;
            } else {
              acc_counter--;
            }
            if (acc_counter <= 0) cur_variant = 0;
          }
          break;

        case AMODE_TILTX:
          pitch = (atan2(fxg, sqrt(fyg * fyg + fzg * fzg)) * 180.0) / M_PI;
          if (cur_variant == 0) {
            acc_counter += (pitch < -65) ? 1 : -1;
          } else {
            acc_counter += (pitch > 65) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        case AMODE_TILTY:
          pitch = (atan2(fyg, sqrt(fxg * fxg + fzg * fzg)) * 180.0) / M_PI;
          if (cur_variant == 0) {
            acc_counter += (pitch < -65) ? 1 : -1;
          } else {
            acc_counter += (pitch > 65) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        case AMODE_FLIPZ:
          if (cur_variant == 0) {
            acc_counter += (fzg < -0.85) ? 1 : -1;
          } else {
            acc_counter += (fzg > 0.85) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        default:
          break;
      }
    }

    Pattern pattern[2];
    uint8_t cur_variant;
    uint8_t accel_mode, accel_sens;
    uint8_t edit_color;
    int16_t acc_counter;
};

Mode mode00 = Mode();
Mode mode01 = Mode();
Mode mode02 = Mode();
Mode mode03 = Mode();
Mode mode04 = Mode();
Mode mode05 = Mode();
Mode mode06 = Mode();
Mode mode07 = Mode();
Mode mode08 = Mode();
Mode mode09 = Mode();
Mode mode10 = Mode();
Mode mode11 = Mode();
Mode mode12 = Mode();
Mode mode13 = Mode();
Mode mode14 = Mode();
Mode mode15 = Mode();

Mode *modes[NUM_MODES] = {
  &mode00, &mode01, &mode02, &mode03, &mode04, &mode05, &mode06, &mode07,
  &mode08, &mode09, &mode10, &mode11, &mode12, &mode13, &mode14, &mode15,
};
Mode *mode;

/* Pattern edit_pattern = Pattern(PATTERN_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); */
uint8_t edit_palette = 0;
uint8_t edit_shade = 0;

const PROGMEM uint8_t factory_bundles[NUM_BUNDLES][NUM_MODES] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 0, 0, 0, 0, 0},
  {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
  {12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
const PROGMEM uint8_t factory_bundle_slots[NUM_BUNDLES] = {16, 9, 16, 4};

uint8_t cur_mode_idx = 0;
uint8_t bundles[NUM_BUNDLES][NUM_MODES];
uint8_t bundle_slots[NUM_BUNDLES];
uint8_t cur_bundle = 0;
uint8_t bundle_idx = 0;

elapsedMicros limiter = 0;

uint8_t accel_counter = 0;
int8_t xg, yg, zg;
float fxg, fyg, fzg;

uint8_t button_state = 0;
uint8_t new_state = 0;
uint8_t config_state = 0;
uint32_t since_trans = 0;

bool conjure = false;
bool conjure_toggle = false;

bool bpm_enabled = false;
uint32_t bpm_tracker = 0;
uint32_t bpm_trigger = 64000;
uint8_t times_clicked = 0;
uint32_t bpm_pressed = 0;
uint32_t total_time = 0;

bool locked = false;

// ********************************************************************
// **** SETUP CODE ****************************************************
// ********************************************************************
#define PRESS_DELAY 100   // 0.05s
#define SHORT_HOLD  1000  // 0.5s
#define LONG_HOLD   2000  // 1.0s
#define FLASH_TIME  500   // 0.25s

#define S_PLAY_OFF                  0
#define S_PLAY_PRESSED              1
#define S_PLAY_SLEEP_WAIT           2
#define S_PLAY_CONJURE_WAIT         3
#define S_PLAY_CONFIG_WAIT          4

#define S_WAKEUP_WAIT               5
#define S_WAKEUP_BUNDLE_SELECT      6
#define S_WAKEUP_LOCK               7
#define S_WAKEUP_MASTER_RESET       8
#define S_WAKEUP_AFTER_LOCK         9

#define S_CONFIG_MENU_OFF           10
#define S_CONFIG_MENU_PRESSED       11
#define S_CONFIG_MENU_EDIT_WAIT     12
#define S_CONFIG_MENU_EXIT_WAIT     13

#define S_COLOR_SELECT_OFF          20
#define S_COLOR_SELECT_PRESSED      21
#define S_COLOR_SELECT_SHADE_WAIT   23

#define S_COLOR_CONFIRM_OFF         30
#define S_COLOR_CONFIRM_PRESSED     31
#define S_COLOR_CONFIRM_REJECT_WAIT 32
#define S_COLOR_CONFIRM_EXIT_WAIT   33

#define S_PATTERN_SELECT_OFF        40
#define S_PATTERN_SELECT_PRESSED    41
#define S_PATTERN_SELECT_WAIT       42

#define S_ACC_MODE_SELECT_OFF       50
#define S_ACC_MODE_SELECT_PRESSED   51
#define S_ACC_MODE_SELECT_WAIT      52

#define S_ACC_SENS_SELECT_OFF       60
#define S_ACC_SENS_SELECT_PRESSED   61
#define S_ACC_SENS_SELECT_WAIT      62

#define S_BUNDLE_SELECT_OFF         70
#define S_BUNDLE_SELECT_PRESSED     71
#define S_BUNDLE_SELECT_WAIT        72
#define S_BUNDLE_SELECT_EDIT        73
#define S_BUNDLE_SELECT_BPM         74
#define S_BPM_SET_OFF               75
#define S_BPM_SET_PRESSED           76

#define S_BUNDLE_EDIT_OFF           80
#define S_BUNDLE_EDIT_PRESSED       81
#define S_BUNDLE_EDIT_WAIT          82
#define S_BUNDLE_EDIT_SAVE          83

#define S_SAVE_MODE                 90
#define S_SAVE_BUNDLES              91

#define S_GUI_MODE                  100
#define S_GUI_BUNDLES               101
#define S_GUI_PALETTE               102
#define S_GUI_PATTERNS              103

#define PIN_R 9
#define PIN_G 6
#define PIN_B 5
#define PIN_BUTTON 2
#define PIN_LDO A3
#define MMA7660_ADDRESS 0x4C

#define CONFIG_PALETTE0    0
#define CONFIG_PALETTE1    1
#define CONFIG_PATTERN0    2
#define CONFIG_PATTERN1    3
#define CONFIG_ACC_MODE    4
#define CONFIG_ACC_SENS    5

#define FRAME_TICKS 32000

void clearMemory() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void saveModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->save(addrs[i]);
  EEPROM.update(VERSION_ADDR, EEPROM_VERSION);
}

void loadModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->load(addrs[i]);
}

void writeMode(uint8_t idx, uint8_t addr, uint8_t val) {
  if (addr == 0) {
    modes[idx]->accel_mode = val;
  } else if (addr == 1) {
    modes[idx]->accel_sens = val;
  } else if (addr == 2) {
    modes[idx]->pattern[0].pattern = val;
  } else if (addr == 3) {
    modes[idx]->pattern[0].num_colors = val;
  } else if (addr < 20) {
    modes[idx]->pattern[0].colors[addr - 4] = val;
  } else if (addr == 20) {
    modes[idx]->pattern[1].pattern = val;
  } else if (addr == 21) {
    modes[idx]->pattern[1].num_colors = val;
  } else if (addr < 38) {
    modes[idx]->pattern[1].colors[addr - 22] = val;
  }
}

void readMode(uint8_t idx, uint8_t addr) {
  Serial.write(idx);
  Serial.write(addr);
  if (addr == 0) {
    Serial.write(modes[idx]->accel_mode);
  } else if (addr == 1) {
    Serial.write(modes[idx]->accel_sens);
  } else if (addr == 2) {
    Serial.write(modes[idx]->pattern[0].pattern);
  } else if (addr == 3) {
    Serial.write(modes[idx]->pattern[0].num_colors);
  } else if (addr < 20) {
    Serial.write(modes[idx]->pattern[0].colors[addr - 4]);
  } else if (addr == 20) {
    Serial.write(modes[idx]->pattern[1].pattern);
  } else if (addr == 21) {
    Serial.write(modes[idx]->pattern[1].num_colors);
  } else if (addr < 38) {
    Serial.write(modes[idx]->pattern[1].colors[addr - 22]);
  }
}

void dumpMode(uint8_t idx) {
  for (uint8_t i = 0; i < 38; i++) {
    readMode(idx, i);
  }
}

//******************************************************************************
void saveBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    EEPROM.update(BUNDLE_EEPROM_ADDR + 64 + b, bundle_slots[b]);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      EEPROM.update(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i, bundles[b][i]);
    }
  }
}

void loadBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = EEPROM.read(BUNDLE_EEPROM_ADDR + 64 + b);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      bundles[b][i] = EEPROM.read(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i);
    }
  }
}

void dumpBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    Serial.write(16);
    Serial.write(b * 20);
    Serial.write(bundle_slots[b]);

    for (uint8_t i = 0; i < NUM_MODES; i++) {
      Serial.write(16);
      Serial.write((b * 20) + i + 1);
      Serial.write(bundles[b][i]);
    }
  }
}

void writeBundle(uint8_t addr, uint8_t val) {
  uint8_t b = addr / 20;
  uint8_t i = addr % 20;
  if (i == 0) {
    bundle_slots[b] = val;
  } else {
    bundles[b][i - 1] = val;
  }
}

void readBundle(uint8_t addr) {
  uint8_t b = addr / 20;
  uint8_t i = addr % 20;
  Serial.write(16);
  Serial.write(addr);
  if (addr % 20 == 0) {
    Serial.write(bundle_slots[b]);
  } else {
    Serial.write(bundles[b][i - 1]);
  }
}


//******************************************************************************
void dumpPalette() {
  for (uint8_t c = 0; c < NUM_COLORS; c++) {
    for (uint8_t i = 0; i < 3; i++) {
      Serial.write(17);
      Serial.write((c * 4) + i);
      Serial.write(color_palette[c][i]);
    }
  }
}

void writePalette(uint8_t addr, uint8_t val) {
  uint8_t c = addr / 4;
  uint8_t i = addr % 4;
  color_palette[c][i] = val;
}

void readPalette(uint8_t addr) {
  uint8_t c = addr / 4;
  uint8_t i = addr % 4;
  Serial.write(17);
  Serial.write(addr);
  Serial.write(color_palette[c][i]);
}


//******************************************************************************
void dumpPatterns() {
  for (uint8_t p = 0; p < 16; p++) {
    for (uint8_t i = 0; i < 8; i++) {
      Serial.write(18);
      Serial.write((p * 8) + i);
      Serial.write(user_patterns[p][i]);
    }
  }
}

void writePatterns(uint8_t addr, uint8_t val) {
  uint8_t p = addr / 8;
  uint8_t i = addr % 8;
  user_patterns[p][i] = val;
}

void readPatterns(uint8_t addr) {
  uint8_t p = addr / 8;
  uint8_t i = addr % 8;
  Serial.write(18);
  Serial.write(addr);
  Serial.write(user_patterns[p][i]);
}


//******************************************************************************
void cmdDump(uint8_t target) {
  Serial.write(200); Serial.write(target); Serial.write(cur_mode_idx);
  if (target == 16) {
    dumpBundles();
  } else if (target == 17) {
    dumpPalette();
  } else if (target == 18) {
    dumpPatterns();
  } else if (target == 99) {
    dumpPalette();
    dumpMode(0);
    dumpMode(1);
    dumpMode(2);
    dumpMode(3);
    dumpMode(4);
    dumpMode(5);
    dumpMode(6);
    dumpMode(7);
    dumpMode(8);
    dumpMode(9);
    dumpMode(10);
    dumpMode(11);
    dumpMode(12);
    dumpMode(13);
    dumpMode(14);
    dumpMode(15);
    dumpBundles();
    dumpPatterns();
  } else if (target < 16) {
    dumpMode(target);
  }
  Serial.write(201); Serial.write(target); Serial.write(cur_mode_idx);
}

void cmdLoad(uint8_t target) {
  flash(0, 128, 0, 5);
  if (target < 16) {
    modes[target]->load(addrs[target]);
  } else if (target == 16) {
    loadBundles();
  } else if (target == 17) {
    loadPalette(PALETTE_EEPROM_ADDR);
  } else if (target == 18) {
    loadPatterns(PATTERNS_EEPROM_ADDR);
  } else if (target == 99) {
    loadModes();
    loadBundles();
    loadPalette(PALETTE_EEPROM_ADDR);
    loadPatterns(PATTERNS_EEPROM_ADDR);
  }
}

void cmdSave(uint8_t target) {
  flash(128, 0, 0, 5);
  if (target < 16) {
    modes[target]->save(addrs[target]);
  } else if (target == 16) {
    saveBundles();
  } else if (target == 17) {
    savePalette(PALETTE_EEPROM_ADDR);
  } else if (target == 18) {
    savePatterns(PATTERNS_EEPROM_ADDR);
  } else if (target == 99) {
    saveModes();
    saveBundles();
    savePalette(PALETTE_EEPROM_ADDR);
    savePatterns(PATTERNS_EEPROM_ADDR);
  }
}

void cmdRead(uint8_t target, uint8_t addr) {
  if (target < 16) {
    readMode(target, addr);
  } else if (target == 16) {
    readBundle(addr);
  } else if (target == 17) {
    readPalette(addr);
  } else if (target == 18) {
    readPatterns(addr);
  }
}

void cmdWrite(uint8_t target, uint8_t addr, uint8_t val) {
  if (target < 16) {
    writeMode(target, addr, val);
  } else if (target == 16) {
    writeBundle(addr, val);
  } else if (target == 17) {
    writePalette(addr, val);
  } else if (target == 18) {
    writePatterns(addr, val);
  }
  cmdRead(target, addr);
}

void cmdExecute(uint8_t action, uint8_t arg0, uint8_t arg1) {
  switch (action) {
    case 0:
      cur_mode_idx = (cur_mode_idx + 15) % 16;
      _modeChanged();
      break;
    case 1:
      cur_mode_idx = (cur_mode_idx + 1) % 16;
      _modeChanged();
      break;
    case 2:
      cur_mode_idx = arg0;
      _modeChanged();
      break;
    case 10:
      switch (arg0) {
        case 0:
          new_state = S_GUI_MODE;
          break;
        case 1:
          new_state = S_GUI_PALETTE;
          break;
        case 2:
          new_state = S_GUI_BUNDLES;
          break;
        case 3:
          new_state = S_GUI_PATTERNS;
          break;
        default:
          new_state = S_PLAY_OFF;
      }
    case 20:
      edit_palette = arg0;
      edit_shade = arg1;
      break;
    default:
      break;
  }
}

void initModes() {
  mode00.accel_mode = AMODE_SPEED;
  mode00.accel_sens = ASENS_LOW;
  mode00.pattern[0] = Pattern(PATTERN_DOPS,      12, 0x01, 0x08, 0x01, 0x0C, 0x01, 0x10, 0x01, 0x14, 0x01, 0x18, 0x01, 0x1C, 0x00, 0x00, 0x00, 0x00);
  mode00.pattern[1] = Pattern(PATTERN_DASHDOPS2, 13, 0x01, 0x9E, 0x9C, 0x9A, 0x98, 0x96, 0x94, 0x92, 0x90, 0x8E, 0x8C, 0x8A, 0x88, 0x00, 0x00, 0x00);

  mode01.accel_mode = AMODE_SPEED;
  mode01.accel_sens = ASENS_MEDIUM;
  mode01.pattern[0] = Pattern(PATTERN_BLINKE,     7, 0x01, 0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode01.pattern[1] = Pattern(PATTERN_STROBIE,    8, 0x0A, 0x0D, 0x10, 0x13, 0x16, 0x19, 0x1C, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode02.accel_mode = AMODE_SPEED;
  mode02.accel_sens = ASENS_HIGH;
  mode02.pattern[0] = Pattern(PATTERN_HYPER,      3, 0x0E, 0x1E, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode02.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode03.accel_mode = AMODE_TILTX;
  mode03.accel_sens = ASENS_LOW;
  mode03.pattern[0] = Pattern(PATTERN_EDGE,      12, 0x1E, 0x16, 0x19, 0x88, 0x9F, 0x9E, 0x9D, 0x9C, 0xDB, 0xDA, 0xDA, 0xDA, 0x00, 0x00, 0x00, 0x00);
  mode03.pattern[1] = Pattern(PATTERN_EDGE,      12, 0x16, 0x5E, 0x5B, 0x94, 0x95, 0x96, 0x97, 0x98, 0xD9, 0xDA, 0xDA, 0xDA, 0x00, 0x00, 0x00, 0x00);

  mode04.accel_mode = AMODE_TILTX;
  mode04.accel_sens = ASENS_MEDIUM;
  mode04.pattern[0] = Pattern(PATTERN_TRACER,     9, 0xC2, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode04.pattern[1] = Pattern(PATTERN_TRACER,     9, 0xC6, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode05.accel_mode = AMODE_TILTX;
  mode05.accel_sens = ASENS_HIGH;
  mode05.pattern[0] = Pattern(PATTERN_CANDY3,     8, 0x08, 0x0B, 0x08, 0x1D, 0x08, 0x0E, 0x08, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode05.pattern[1] = Pattern(PATTERN_CANDY3,     8, 0x18, 0x1B, 0x18, 0x15, 0x18, 0x1E, 0x18, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode06.accel_mode = AMODE_TILTY;
  mode06.accel_sens = ASENS_LOW;
  mode06.pattern[0] = Pattern(PATTERN_CHASE,      5, 0x1A, 0x1D, 0x08, 0x0B, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode06.pattern[1] = Pattern(PATTERN_CHASE,      5, 0x12, 0x15, 0x18, 0x1B, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode07.accel_mode = AMODE_TILTY;
  mode07.accel_sens = ASENS_MEDIUM;
  mode07.pattern[0] = Pattern(PATTERN_RIBBON10,  12, 0x08, 0x0A, 0x00, 0x00, 0x10, 0x12, 0x00, 0x00, 0x18, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode07.pattern[1] = Pattern(PATTERN_RIBBON10,  12, 0x08, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x14, 0x00, 0x18, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode08.accel_mode = AMODE_TILTY;
  mode08.accel_sens = ASENS_HIGH;
  mode08.pattern[0] = Pattern(PATTERN_COMET,      5, 0x1E, 0x1F, 0x08, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode08.pattern[1] = Pattern(PATTERN_COMET,      5, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode09.accel_mode = AMODE_FLIPZ;
  mode09.accel_sens = ASENS_LOW;
  mode09.pattern[0] = Pattern(PATTERN_PULSE,      6, 0x56, 0x00, 0x58, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode09.pattern[1] = Pattern(PATTERN_PULSE,      6, 0x5E, 0x00, 0x48, 0x00, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode10.accel_mode = AMODE_FLIPZ;
  mode10.accel_sens = ASENS_MEDIUM;
  mode10.pattern[0] = Pattern(PATTERN_LEGO,       4, 0x48, 0x4A, 0x4C, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode10.pattern[1] = Pattern(PATTERN_LEGO,       4, 0x58, 0x5A, 0x5C, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode11.accel_mode = AMODE_FLIPZ;
  mode11.accel_sens = ASENS_HIGH;
  mode11.pattern[0] = Pattern(PATTERN_MORPH,      4, 0x10, 0x18, 0x08, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode11.pattern[1] = Pattern(PATTERN_MORPH,      4, 0x10, 0x08, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode12.accel_mode = AMODE_OFF;
  mode12.accel_sens = ASENS_LOW;
  mode12.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x08, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode12.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode13.accel_mode = AMODE_OFF;
  mode13.accel_sens = ASENS_LOW;
  mode13.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x18, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode13.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode14.accel_mode = AMODE_OFF;
  mode14.accel_sens = ASENS_LOW;
  mode14.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x1A, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode14.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode15.accel_mode = AMODE_OFF;
  mode15.accel_sens = ASENS_LOW;
  mode15.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x0E, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode15.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

void initBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = pgm_read_byte(&factory_bundle_slots[b]);
    for (uint8_t s = 0; s < NUM_MODES; s++) {
      bundles[b][s] = pgm_read_byte(&factory_bundles[b][s]);
    }
  }
}

void resetMemory() {
  clearMemory();

  initModes();
  initBundles();
  initPalette();
  initPatterns();

  saveModes();
  saveBundles();
  savePalette(PALETTE_EEPROM_ADDR);
  savePatterns(PATTERNS_EEPROM_ADDR);

  EEPROM.update(VERSION_ADDR, EEPROM_VERSION);
}

void handleSerial() {
  uint8_t in0, in1, in2, in3;
  if (Serial.available() >= 4) {
    in0 = Serial.read();
    in1 = Serial.read();
    in2 = Serial.read();
    in3 = Serial.read();
    switch (in0) {
      case 'D':  // Dump
        cmdDump(in1);
        break;
      case 'L':  // Load
        cmdLoad(in1);
        break;
      case 'S':  // Save
        cmdSave(in1);
        break;
      case 'R':  // Read
        cmdRead(in1, in2);
        break;
      case 'W':  // Write
        cmdWrite(in1, in2, in3);
        break;
      case 'X':  // eXecute
        cmdExecute(in1, in2, in3);
        break;
      default:
        break;
    }
  }
}

void setup() {
  power_spi_disable();

  Serial.begin(57600);
  randomSeed(analogRead(0));

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  if (EEPROM_VERSION != EEPROM.read(VERSION_ADDR)) {
    resetMemory();
  } else {
    loadModes();
    loadBundles();
    loadPalette(PALETTE_EEPROM_ADDR);
    loadPatterns(PATTERNS_EEPROM_ADDR);
  }
  resetMode();
  accInit();

  noInterrupts();
  ADCSRA = 0; // Disable ADC
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  delay(4000);
  limiter = 0;
  Serial.write(100); Serial.write(cur_mode_idx); Serial.write(0);
}

void loop() {
  uint8_t r, g, b;
  handleSerial();
  handlePress(digitalRead(PIN_BUTTON) == LOW);
  if (accel_counter >= 20) accel_counter = 0;
  if (button_state == S_PLAY_OFF || button_state == S_BUNDLE_SELECT_OFF) {
    if (accel_counter == 0) {
      accUpdate();
    } else if (accel_counter == 1) {
      fxg = translateAccel(xg);
    } else if (accel_counter == 2) {
      fyg = translateAccel(yg);
    } else if (accel_counter == 3) {
      fzg = translateAccel(zg);
    } else if (accel_counter == 4) {
      mode->updateAccel(fxg, fyg, fzg);
    }
  }
  accel_counter++;

  if (button_state == S_PLAY_OFF) {
    mode->render(r, g, b);
  } else if (button_state < 10) {
    r = 0; g = 0; b = 0;
  } else if (button_state < 20) {
    switch (config_state) {
      case CONFIG_PALETTE0:
        r = 255; g = 0; b = 0;
        break;
      case CONFIG_PALETTE1:
        r = 0; g = 0; b = 255;
        break;
      case CONFIG_PATTERN0:
        r = 255; g = 0; b = 255;
        break;
      case CONFIG_PATTERN1:
        r = 0; g = 255; b = 255;
        break;
      case CONFIG_ACC_MODE:
        r = 0; g = 255; b = 0;
        break;
      default:
        r = 255; g = 255; b = 0;
        break;
    }
  } else if (button_state < 30) {
    unpackColor(mode->pattern[mode->cur_variant].colors[mode->edit_color], r, g, b);
  } else if (button_state < 50) {
    mode->render(r, g, b);
  } else if (button_state < 60) {
    switch (mode->accel_mode) {
      case AMODE_SPEED:
        r = 255; g = 0; b = 0;
        break;
      case AMODE_TILTX:
        r = 0; g = 0; b = 255;
        break;
      case AMODE_TILTY:
        r = 255; g = 255; b = 0;
        break;
      case AMODE_FLIPZ:
        r = 0; g = 255; b = 0;
        break;
      default: // case AMODE_OFF
        r = 64; g = 64; b = 64;
        break;
    }
  } else if (button_state < 70) {
    switch (mode->accel_sens) {
      case ASENS_LOW:
        r = 0; g = 0; b = 255;
        break;
      case ASENS_MEDIUM:
        r = 255; g = 0; b = 255;
        break;
      default: // case ASENS_HIGH:
        r = 255; g = 0; b = 0;
        break;
    }
  } else if (button_state < 102) {
    mode->render(r, g, b);
  } else if (button_state == S_GUI_PALETTE) {
    r = color_palette[edit_palette][0] >> edit_shade;
    g = color_palette[edit_palette][1] >> edit_shade;
    b = color_palette[edit_palette][2] >> edit_shade;
  }

  if (conjure && conjure_toggle) {
    r = 0; g = 0; b = 0;
  }

  if (button_state == S_PLAY_OFF && bpm_enabled && bpm_tracker > bpm_trigger) {
    incMode();
  }

  writeFrame(r, g, b);
  bpm_tracker++;
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t i = 0; i < flashes; i++) {
    for (uint8_t j = 0; j < 100; j++) {
      if (j < 50) { writeFrame(r, g, b); }
      else {        writeFrame(0, 0, 0); }
    }
  }
  since_trans += flashes * 100;
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  while (limiter < FRAME_TICKS) {}
  limiter = 0;

  analogWrite(PIN_R, (r >> 1) + (r >> 2));
  analogWrite(PIN_G, (g >> 1) + (g >> 2));
  analogWrite(PIN_B, (b >> 1) + (b >> 2));
}

void _modeChanged() {
  mode = modes[cur_mode_idx];
  bpm_tracker = 0;
  mode->init();
  mode->cur_variant = 0;
  fxg = fyg = fzg = 0.0;
  if (button_state >= 100) {
    Serial.write(250); Serial.write(cur_mode_idx); Serial.write(button_state);
  }
}

void resetMode() {
  bundle_idx = 0;
  cur_mode_idx = bundles[cur_bundle][bundle_idx];
  _modeChanged();
}

void incMode() {
  bundle_idx = (bundle_idx + 1) % bundle_slots[cur_bundle];
  cur_mode_idx = bundles[cur_bundle][bundle_idx];
  _modeChanged();
}


// ********************************************************************
// **** ACCEL CODE ****************************************************
// ********************************************************************
void accInit() {
  Wire.begin();
  accSend(0x07, 0x00);
  accSend(0x06, 0x10);
  accSend(0x08, 0x00);
  accSend(0x07, 0x01);
}

void accStandby() {
  Wire.begin();
  accSend(0x07, 0x10);
}

void accSend(uint8_t reg_address, uint8_t data) {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}

float translateAccel(int8_t g) {
  if (g >= 64) {
    // Out of bounds, don't alter fg
    return 0.0;
  } else if (g >= 32) {
    // Translate 32  - 63 to -32 - -1
    g = -64 + g;
  }

  return (g / 20.0);
}

void accUpdate() {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 3);

  if (Wire.available()) {
    xg = Wire.read();
    yg = Wire.read();
    zg = Wire.read();
  }
}


// ********************************************************************
// **** BUTTON CODE ***************************************************
// ********************************************************************
void handlePress(bool pressed) {
  switch (button_state) {
    case S_PLAY_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_PLAY_PRESSED;
      }
      break;

    case S_PLAY_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        if (conjure) {
          conjure_toggle = !conjure_toggle;
        } else {
          incMode();
        }
        new_state = S_PLAY_OFF;
      }
      break;

    case S_PLAY_SLEEP_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONFIG_WAIT;
      } else if (!pressed) {
        enterSleep();
        new_state = S_WAKEUP_WAIT;
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        mode->edit_color = 0;
        config_state = CONFIG_PALETTE0;
        new_state = S_CONFIG_MENU_OFF;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) {
        flash(0, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        conjure = !conjure;
        conjure_toggle = false;
        new_state = S_PLAY_OFF;
      }
      break;

    case S_WAKEUP_WAIT:
      if (locked) {
        if (!pressed) {
          enterSleep();
          since_trans = 0;
        } else if (since_trans >= 6000) {
          locked = false;
          new_state = S_WAKEUP_AFTER_LOCK;
        }
      } else {
        if (!pressed) {
          new_state = S_PLAY_OFF;
        } else if (since_trans >= LONG_HOLD) {
          new_state = S_WAKEUP_BUNDLE_SELECT;
        }
      }
      break;

    case S_WAKEUP_BUNDLE_SELECT:
      if (since_trans == 0) {
        flash(0, 0, 128, 5);
      }
      if (!pressed) {
        bpm_enabled = false;
        new_state = S_BUNDLE_SELECT_OFF;
      } else if (since_trans > 4000) {
        new_state = S_WAKEUP_LOCK;
      }
      break;

    case S_WAKEUP_LOCK:
      if (since_trans == 0) {
        flash(0, 128, 0, 5);
      }
      if (!pressed) {
        locked = true;
        enterSleep();
        new_state = S_WAKEUP_WAIT;
      } else if (since_trans >= 4000) {
        new_state = S_WAKEUP_MASTER_RESET;
      }
      break;

    case S_WAKEUP_MASTER_RESET:
      if (since_trans == 0) {
        flash(128, 0, 0, 5);
      }
      if (!pressed) {
        resetMemory();
        resetMode();
        new_state = S_PLAY_OFF;
      }
      break;

    case S_WAKEUP_AFTER_LOCK:
      if (since_trans == 0) {
        flash(64, 64, 64, 5);
      }
      if (!pressed) {
        new_state = S_PLAY_OFF;
      }
      break;

    case S_CONFIG_MENU_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_CONFIG_MENU_PRESSED;
      }
      break;

    case S_CONFIG_MENU_PRESSED:
      if (!pressed) {
        config_state = (config_state + 1) % 6;
        new_state = S_CONFIG_MENU_OFF;
      } else if (since_trans >= SHORT_HOLD) {
        new_state = S_CONFIG_MENU_EDIT_WAIT;
      }
      break;

    case S_CONFIG_MENU_EDIT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 0, 5);
      }
      if (!pressed) {
        if (config_state < 2) {
          mode->edit_color = 0;
          mode->cur_variant = config_state % 2;
          new_state = S_COLOR_SELECT_OFF;
        } else if (config_state < 4) {
          mode->cur_variant = config_state % 2;
          new_state = S_PATTERN_SELECT_OFF;
        } else if (config_state == 4) {
          new_state = S_ACC_MODE_SELECT_OFF;
        } else {
          new_state = S_ACC_SENS_SELECT_OFF;
        }
        mode->init();
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_MENU_EXIT_WAIT;
      }
      break;

    case S_CONFIG_MENU_EXIT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        mode->init();
        new_state = S_PLAY_OFF;
      }
      break;

    case S_COLOR_SELECT_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_COLOR_SELECT_PRESSED;
      }
      break;

    case S_COLOR_SELECT_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        flash(64, 64, 64, 5);
        new_state = S_COLOR_SELECT_SHADE_WAIT;
      } else if (!pressed) {
        mode->changeColor(1);
        new_state = S_COLOR_SELECT_OFF;
      }
      break;

    case S_COLOR_SELECT_SHADE_WAIT:
      if (since_trans == 0) {
        flash(64, 64, 64, 5);
      }
      if (since_trans >= LONG_HOLD) {
        mode->changeShade();
        since_trans = 0;
      } else if (!pressed) {
        mode->pattern[mode->cur_variant].num_colors = mode->edit_color + 1;
        new_state = S_COLOR_CONFIRM_OFF;
      }
      break;

    case S_COLOR_CONFIRM_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_COLOR_CONFIRM_PRESSED;
      }
      break;

    case S_COLOR_CONFIRM_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_COLOR_CONFIRM_REJECT_WAIT;
      } else if (!pressed) {
        mode->edit_color++;
        if (mode->edit_color == PALETTE_SIZE) {
          flash(128, 128, 128, 5);
          new_state = S_SAVE_MODE;
        } else {
          new_state = S_COLOR_SELECT_OFF;
        }
      }
      break;

    case S_COLOR_CONFIRM_REJECT_WAIT:
      if (since_trans == 0) {
        flash(128, 0, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_COLOR_CONFIRM_EXIT_WAIT;
      } else if (!pressed) {
        if (mode->edit_color == 0) {
          new_state = S_COLOR_SELECT_OFF;
        } else {
          mode->edit_color--;
          mode->pattern[mode->cur_variant].num_colors--;
          new_state = S_COLOR_CONFIRM_OFF;
        }
      }
      break;

    case S_COLOR_CONFIRM_EXIT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        mode->pattern[mode->cur_variant].num_colors = mode->edit_color + 1;
        new_state = S_SAVE_MODE;
      }
      break;


    case S_PATTERN_SELECT_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_PATTERN_SELECT_PRESSED;
      }
      break;

    case S_PATTERN_SELECT_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_PATTERN_SELECT_WAIT;
      } else if (!pressed) {
        mode->pattern[mode->cur_variant].pattern = (mode->pattern[mode->cur_variant].pattern + 1) % NUM_PATTERNS;
        mode->init();
        new_state = S_PATTERN_SELECT_OFF;
      }
      break;

    case S_PATTERN_SELECT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        new_state = S_SAVE_MODE;
      }
      break;


    case S_ACC_MODE_SELECT_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_ACC_MODE_SELECT_PRESSED;
      }
      break;

    case S_ACC_MODE_SELECT_PRESSED:
      if (!pressed) {
        mode->accel_mode = (mode->accel_mode + 1) % 5;
        new_state = S_ACC_MODE_SELECT_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_ACC_MODE_SELECT_WAIT;
      }
      break;

    case S_ACC_MODE_SELECT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        new_state = S_SAVE_MODE;
      }
      break;


    case S_ACC_SENS_SELECT_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_ACC_SENS_SELECT_PRESSED;
      }
      break;

    case S_ACC_SENS_SELECT_PRESSED:
      if (!pressed) {
        mode->accel_sens = (mode->accel_sens + 1) % 3;
        new_state = S_ACC_SENS_SELECT_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_ACC_SENS_SELECT_WAIT;
      }
      break;

    case S_ACC_SENS_SELECT_WAIT:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        new_state = S_SAVE_MODE;
      }
      break;


    case S_BUNDLE_SELECT_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_BUNDLE_SELECT_PRESSED;
      }
      break;

    case S_BUNDLE_SELECT_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        new_state = S_BUNDLE_SELECT_WAIT;
      } else if (!pressed) {
        cur_bundle = (cur_bundle + 1) % NUM_BUNDLES;
        resetMode();
        new_state = S_BUNDLE_SELECT_OFF;
      }
      break;

    case S_BUNDLE_SELECT_WAIT:
      if (since_trans == 0) {
        flash(0, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_EDIT;
      } else if (!pressed) {
        new_state = S_PLAY_OFF;
      }
      break;

    case S_BUNDLE_SELECT_EDIT:
      if (since_trans == 0) {
        flash(128, 128, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_BPM;
      } else if (!pressed) {
        resetMode();
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;

    case S_BUNDLE_SELECT_BPM:
      if (since_trans == 0) {
        flash(0, 128, 0, 5);
      }
      if (!pressed) {
        total_time = 0;
        times_clicked = 0;
        bpm_pressed = 0;
        bpm_trigger = 0;
        new_state = S_BPM_SET_OFF;
      }
      break;

    case S_BPM_SET_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        if (times_clicked != 0) {
          bpm_trigger += bpm_pressed;
        }
        if (times_clicked >= 4) {
          bpm_trigger *= 4;
        }
        times_clicked++;
        bpm_pressed = 0;
        new_state = S_BPM_SET_PRESSED;
      } else if (since_trans > 20000) {
        bpm_trigger = 64000;
        new_state = S_PLAY_OFF;
      }
      break;

    case S_BPM_SET_PRESSED:
      if (!pressed) {
        if (times_clicked >= 5) {
          bpm_tracker = 0;
          bpm_enabled = true;
          new_state = S_PLAY_OFF;
        } else {
          new_state = S_BPM_SET_OFF;
        }
      }
      break;


    case S_BUNDLE_EDIT_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_BUNDLE_EDIT_PRESSED;
      }
      break;

    case S_BUNDLE_EDIT_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_WAIT;
      } else if (!pressed) {
        bundles[cur_bundle][bundle_idx] = (bundles[cur_bundle][bundle_idx] + 1) % NUM_MODES;
        mode = modes[bundles[cur_bundle][bundle_idx]];
        mode->init();
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;

    case S_BUNDLE_EDIT_WAIT:
      if (since_trans == 0) {
        flash(128, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_SAVE;
      } else if (!pressed) {
        bundle_idx++;
        if (bundle_idx == NUM_MODES) {
          bundle_slots[cur_bundle] = bundle_idx;
          new_state = S_SAVE_BUNDLES;;
        } else {
          mode = modes[bundles[cur_bundle][bundle_idx]];
          mode->init();
          new_state = S_BUNDLE_EDIT_OFF;
        }
      }
      break;

    case S_BUNDLE_EDIT_SAVE:
      if (since_trans == 0) {
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        bundle_slots[cur_bundle] = bundle_idx + 1;
        new_state = S_SAVE_BUNDLES;
      }
      break;


    case S_SAVE_MODE:
      mode->save(addrs[bundles[cur_bundle][bundle_idx]]);
      mode->init();
      new_state = S_PLAY_OFF;
      break;

    case S_SAVE_BUNDLES:
      saveBundles();
      resetMode();
      new_state = S_PLAY_OFF;
      break;


    case S_GUI_MODE:
      bpm_enabled = false;
      break;

    case S_GUI_BUNDLES:
      bpm_enabled = false;
      break;

    case S_GUI_PALETTE:
      bpm_enabled = false;
      break;

    case S_GUI_PATTERNS:
      bpm_enabled = false;
      break;


    default:
      new_state = S_PLAY_OFF;
      break;
  }

  if (button_state != new_state) {
    button_state = new_state;
    since_trans = 0;
  } else {
    since_trans++;
  }
  bpm_pressed++;
}


// ********************************************************************
// **** SLEEP CODE ****************************************************
// ********************************************************************
void enterSleep() {
  writeFrame(0, 0, 0);
  accStandby();
  digitalWrite(PIN_LDO, LOW);
  delay(4000);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  noInterrupts();
  attachInterrupt(0, pushInterrupt, FALLING);
  EIFR = bit(INTF0);
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS);
  interrupts();
  sleep_cpu();

  // Wait until button is releaed
  digitalWrite(PIN_LDO, HIGH);
  accInit();
  resetMode();
  conjure = conjure_toggle = false;
  delay(4000);
}

void pushInterrupt() {
  sleep_disable();
  detachInterrupt(0);
}
