#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"
#include "palette.h"
#include "pattern.h"

#define EEPROM_VERSION          202

#define ACCEL_BINS              16
#define NUM_MODES               16
#define NUM_BUNDLES             4

#define PIN_R                   9
#define PIN_G                   6
#define PIN_B                   5
#define PIN_BUTTON              2
#define PIN_LDO                 A3
#define MMA7660_ADDRESS         0x4C
#define MMA8652_ADDRESS         0x1D

#define ADDR_MODES              0    // 640
#define ADDR_BUNDLES            640  // 68
#define ADDR_PALETTE            740  // 144

#define ADDR_VERSION            1020
#define ADDR_CUR_BUNDLE         1021
#define ADDR_LOCKED             1022
#define ADDR_SLEEPING           1023

#define PRESS_DELAY             100
#define SHORT_HOLD              1000
#define LONG_HOLD               2000
#define VERY_LONG_HOLD          6000

#define S_PLAY_OFF              0
#define S_PLAY_PRESSED          1
#define S_PLAY_SLEEP_WAIT       2
#define S_PLAY_CONFIG_WAIT      3
#define S_PLAY_CONJURE_WAIT     4
#define S_PLAY_LOCK_WAIT        5

#define S_CONFIG_OFF            10
#define S_CONFIG_PRESSED        11
#define S_CONFIG_EDIT_WAIT      12
#define S_CONFIG_EXIT_WAIT      13

#define S_COLOR_OFF             20
#define S_COLOR_PRESSED         21
#define S_COLOR_SHADE           22

#define S_CONFIRM_OFF           30
#define S_CONFIRM_PRESSED       31
#define S_CONFIRM_REJECT_WAIT   32
#define S_CONFIRM_SAVE_WAIT     33

#define S_PATTERN_OFF           40
#define S_PATTERN_PRESSED       41
#define S_PATTERN_SELECT_WAIT   42

#define S_ACCMODE_OFF           50
#define S_ACCMODE_PRESSED       51
#define S_ACCMODE_SENS          52

#define S_BUNDLE_OFF            60
#define S_BUNDLE_PRESSED        61
#define S_BUNDLE_SELECT_WAIT    62
#define S_BUNDLE_EDIT_WAIT      63

#define S_BUNDLE_EDIT_OFF       70
#define S_BUNDLE_EDIT_PRESSED   71
#define S_BUNDLE_EDIT_SET_WAIT  72
#define S_BUNDLE_EDIT_SAVE_WAIT 73

#define S_SLEEP_WAKE            100
#define S_SLEEP_BUNDLE_WAIT     101
#define S_SLEEP_RESET_WAIT      102
#define S_SLEEP_WAIT_TOO_LONG   103

#define S_RESET_OFF             110
#define S_RESET_PRESSED         111

#define S_HELD                  120

#define S_SAVE_MODE             200
#define S_SAVE_BUNDLES          201

#define S_GUI_MODE              210
#define S_GUI_BUNDLES           211
#define S_GUI_PALETTE           212

#define CONFIG_COLORSA          0
#define CONFIG_COLORSB          1
#define CONFIG_PATTERNA         2
#define CONFIG_PATTERNB         3
#define CONFIG_MODE             4

#define AMODE_OFF               0
#define AMODE_SPEED             1
#define AMODE_TILTX             2
#define AMODE_TILTY             3
#define AMODE_FLIPZ             4

#define ASENS_LOW               0
#define ASENS_MEDIUM            1
#define ASENS_HIGH              2

#define V1_ACCEL_ADDR 0x4C
#define V2_ACCEL_ADDR 0x1D

elapsedMicros limiter = 0;
uint8_t led_r, led_g, led_b;
bool conjure = false;
bool conjure_toggle = false;

uint8_t gui_color, gui_shade, edit_color;
uint8_t button_state, new_state, config_state;
uint64_t since_trans = 0;

uint8_t accel_model;
uint8_t accel_addr;
uint8_t accel_counts;
uint8_t accel_count_wrap;
uint8_t accel_tick = 0;
int16_t xg, yg, zg;
int16_t lxg = 0;
int16_t lyg = 0;
int16_t lzg = 0;
float fxg, fyg, fzg;
float a_mag, a_pitch, a_roll;
uint8_t a_speed;

float thresh_bins_p[2][ACCEL_BINS] = {
  {1.1, 1.195, 1.29, 1.385, 1.48, 1.575, 1.67, 1.765, 1.86, 1.955, 2.05, 2.145, 2.24, 2.335, 2.43, 2.525},
  {1.1, 1.255, 1.41, 1.565, 1.72, 1.875, 2.03, 2.185, 2.34, 2.495, 2.65, 2.805, 2.96, 3.115, 3.27, 3.425},
};
float thresh_bins_n[ACCEL_BINS] = {0.845, 0.79, 0.735, 0.68, 0.625, 0.57, 0.515, 0.46, 0.405, 0.35, 0.295, 0.24, 0.185, 0.13, 0.075, 0.02};
uint8_t thresh_last[ACCEL_BINS];
uint8_t thresh_cnts[ACCEL_BINS];
uint8_t thresh_timings[3] = {30, 18, 6};
uint8_t thresh_falloff;
uint8_t thresh_target;

uint8_t cur_mode_idx = 0;
uint8_t cur_bundle = 0;
uint8_t bundle_idx = 0;
uint8_t bundle_slots[NUM_BUNDLES];
uint8_t bundles[NUM_BUNDLES][NUM_MODES];

uint8_t cur_variant = 0;
int16_t accel_counter = 0;
uint16_t tick[2];
uint8_t cur_color[2];
int16_t cntr[2];

typedef struct Mode {
  uint8_t accel_mode, accel_sens;
  uint8_t pattern[2];
  uint8_t num_colors[2];
  uint8_t colors[2][PALETTE_SIZE];
} Mode;

Mode modes[NUM_MODES];
Mode* mode;


const PROGMEM uint8_t factory_modes[NUM_MODES][38] = {
  {AMODE_SPEED, ASENS_HIGH,
    P_HYPER3, 6, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_DOPS3, 6, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_MEDIUM,
    P_BLASTER, 7, 0x01, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBIEFUSE, 3, 0x48, 0x50, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_LOW,
    P_STROBIE, 2, 0xce, 0x1a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_VEXING, 2, 0xce, 0x1a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_LOW,
    P_DASHDOPS, 7, 0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_DOPSDASH, 7, 0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW,
    P_RAZOR5, 15, 0x08, 0x0a, 0x0c, 0x08, 0x10,  0x18, 0x1a, 0x1c, 0x18, 0x08,  0x10, 0x12, 0x14, 0x10, 0x18,  0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_BLASTER3, 9, 0x01, 0xc8, 0xc8, 0x01, 0xd0, 0xd0, 0x01, 0xd8, 0xd8, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_WAVE, 4, 0x18, 0x14, 0x16, 0x12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_STRETCH, 3, 0x1e, 0x1a, 0x15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW,
    P_DASHMORPH, 3, 0x21, 0x22, 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_PULSAR, 3, 0x1f, 0x1c, 0x19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_DOTTED, 13, 0xc1, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_BOTTLEROCKET, 7, 0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW,
    P_BLASTER3, 6, 0x1f, 0xd4, 0xd4, 0x14, 0xdf, 0xdf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_BLASTER3, 6, 0x26, 0xec, 0xec, 0x2c, 0xe6, 0xe6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_INFLUX, 3, 0x21, 0x24, 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW,
    P_GROW, 6, 0x28, 0, 0x2b, 0, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    P_STROBE, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
};
const PROGMEM uint8_t factory_bundle_slots[NUM_BUNDLES] = {16, 4, 4, 4};
const PROGMEM uint8_t factory_bundles[NUM_BUNDLES][NUM_MODES] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {8, 9, 10, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


void setup() {
  Wire.begin();
  Serial.begin(57600);

  button_state = new_state = S_PLAY_OFF;

  pinMode(PIN_BUTTON, INPUT);

  attachInterrupt(0, pushInterrupt, FALLING);

  if (EEPROM.read(ADDR_SLEEPING)) {
    EEPROM.update(ADDR_SLEEPING, 0);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    button_state = new_state = S_SLEEP_WAKE;
  }
  detachInterrupt(0);

  if (EEPROM_VERSION != EEPROM.read(ADDR_VERSION)) {
    resetMemory();
    EEPROM.update(ADDR_VERSION, EEPROM_VERSION);
  } else {
    loadModes();
    loadBundles();
    loadPalette(ADDR_PALETTE);
  }

  cur_bundle = EEPROM.read(ADDR_CUR_BUNDLE);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  detectAccelModel();
  accelInit();

  noInterrupts();
  ADCSRA = 0; // Disable ADC
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  delay(40);
  changeMode(0);
  Serial.write(100); Serial.write(cur_mode_idx); Serial.write(0);
  limiter = 0;
}

void loop() {
  handlePress(digitalRead(PIN_BUTTON) == LOW);
  handleSerial();

  if (button_state == S_PLAY_OFF || button_state == S_BUNDLE_OFF || button_state >= 210) {
    handleAccel();
  }

  handleRender();
  writeFrame(led_r, led_g, led_b);
}

void resetMode() {
  mode = &modes[cur_mode_idx];
  cur_variant = 0;
  accel_counter = 0;
  tick[0] = cur_color[0] = cntr[0] = 0;
  tick[1] = cur_color[1] = cntr[1] = 0;
}

void renderMode() {
  renderPattern(
      mode->pattern[0], mode->num_colors[0], mode->colors[0],
      tick[0], cur_color[0], cntr[0], led_r, led_g, led_b, cur_variant == 0);
  renderPattern(
      mode->pattern[1], mode->num_colors[1], mode->colors[1],
      tick[1], cur_color[1], cntr[1], led_r, led_g, led_b, cur_variant == 1);
}

void handleRender() {
  if (button_state == 0) {
    renderMode();
  } else if (button_state == 10) {
    if (config_state == 0) {        led_r = 192; led_g =   0; led_b =   0;
    } else if (config_state == 1) { led_r =   0; led_g =   0; led_b = 192;
    } else if (config_state == 2) { led_r = 128; led_g =   0; led_b = 128;
    } else if (config_state == 3) { led_r =   0; led_g = 128; led_b = 128;
    } else {                        led_r =   0; led_g = 192; led_b =   0;
    }
  } else if (button_state >= 20 && button_state < 30) {
    unpackColor(mode->colors[cur_variant][edit_color], led_r, led_g, led_b);
  } else if (button_state == 30) {
    renderMode();
  } else if (button_state == 40) {
    renderMode();
  } else if (button_state >= 50 && button_state < 60) {
    if (mode->accel_mode == AMODE_OFF) {            led_r =  96; led_g =  96; led_b =  96;
    } else {
      if (mode->accel_mode == AMODE_SPEED) {        led_r =  48; led_g =   0; led_b =   0;
      } else if (mode->accel_mode == AMODE_TILTX) { led_r =   0; led_g =   0; led_b =  48;
      } else if (mode->accel_mode == AMODE_TILTY) { led_r =  40; led_g =  40; led_b =   0;
      } else {                                      led_r =   0; led_g =  48; led_b =   0;
      }
      led_r = led_r << mode->accel_sens;
      led_g = led_g << mode->accel_sens;
      led_b = led_b << mode->accel_sens;
    }
  } else if (button_state == 60) {
    renderMode();
  } else if (button_state == 70) {
    renderMode();
  } else if (button_state == 110) {
    led_r = 128; led_g = led_b = 0;
  } else if (button_state == 210 || button_state == 211) {
    renderMode();
  } else if (button_state == 212) {
    led_r = color_palette[gui_color][0] >> gui_shade;
    led_g = color_palette[gui_color][1] >> gui_shade;
    led_b = color_palette[gui_color][2] >> gui_shade;
  } else {
    led_r = led_g = led_b = 0;
  }

  if (conjure && conjure_toggle) {
    if (since_trans > 2000 * 180) {
      enterSleep();
    }
    led_r = led_g = led_b = 0;
  }
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  if (limiter > 32000) { Serial.println(accel_counter); }
  while (limiter < 32000) {}
  limiter = 0;

  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t f = 0; f < flashes; f++) {
    for (uint8_t t = 0; t < 100; t++) {
        if (t < 50) { writeFrame(r, g, b); 
        } else {      writeFrame(0, 0, 0); }
    }
  }
  since_trans += flashes * 100;
}


void changeMode(uint8_t v) {
  if (v == 0) {
    bundle_idx = 0;
  } else {
    bundle_idx = (bundle_idx + 1) % bundle_slots[cur_bundle];
  }

  cur_mode_idx = bundles[cur_bundle][bundle_idx];
  resetMode();
}


void handlePress(bool pressed) {
  switch (button_state) {
    case S_PLAY_OFF:
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_PLAY_PRESSED;
      }
      break;

    case S_PLAY_PRESSED:
      if (conjure) {
        if (!pressed) {
          conjure_toggle = !conjure_toggle;
        }
      } else {
        if (since_trans >= SHORT_HOLD) {
          new_state = S_PLAY_SLEEP_WAIT;
        } else if (!pressed) {
          changeMode(1);
          new_state = S_PLAY_OFF;
        }
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
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        config_state = CONFIG_COLORSA;
        new_state = S_CONFIG_OFF;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_LOCK_WAIT;
      } else if (!pressed) {
        conjure = !conjure;
        conjure_toggle = false;
        new_state = S_PLAY_OFF;
      }

    case S_PLAY_LOCK_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (since_trans > LONG_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        EEPROM.update(ADDR_LOCKED, 1);
        enterSleep();
      }
      break;


    // Config menu
    case S_CONFIG_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_CONFIG_PRESSED;
      }
      break;

    case S_CONFIG_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_EDIT_WAIT;
      } else if (!pressed) {
        config_state = (config_state + 1) % 5;
        new_state = S_CONFIG_OFF;
      }
      break;

    case S_CONFIG_EDIT_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_EXIT_WAIT;
      } else if (!pressed) {
        // enter different config modes
        if (config_state < 2) {
          cur_variant = config_state % 2;
          new_state = S_COLOR_OFF;
        } else if (config_state < 4) {
          cur_variant = config_state % 2;
          new_state = S_PATTERN_OFF;
        } else {
          new_state = S_ACCMODE_OFF;
        }
      }
      break;

    case S_CONFIG_EXIT_WAIT:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (!pressed) {
        new_state = S_PLAY_OFF;
      }
      break;

    // Color config
    case S_COLOR_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_COLOR_PRESSED;
      }
      break;

    case S_COLOR_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_COLOR_SHADE;
      } else if (!pressed) {
        mode->colors[cur_variant][edit_color] =
          (mode->colors[cur_variant][edit_color] + 1) % NUM_COLORS;
        new_state = S_COLOR_OFF;
      }
      break;

    case S_COLOR_SHADE:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (since_trans > LONG_HOLD) {
        mode->colors[cur_variant][edit_color] += 0x40;
        since_trans = 0;
      } else if (!pressed) {
        mode->num_colors[cur_variant] = edit_color + 1;
        new_state = S_CONFIRM_OFF;
      }
      break;

    case S_CONFIRM_OFF:
      if (pressed && since_trans > PRESS_DELAY) new_state = S_CONFIRM_PRESSED;
      break;

    case S_CONFIRM_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_CONFIRM_REJECT_WAIT;
      } else if (!pressed) {
        edit_color = edit_color + 1;
        if (edit_color >= PALETTE_SIZE) { new_state = S_SAVE_MODE;
        } else {                          new_state = S_COLOR_OFF;
        }
      }
      break;

    case S_CONFIRM_REJECT_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (since_trans > LONG_HOLD) {
        new_state = S_CONFIRM_SAVE_WAIT;
      } else if (!pressed) {
        new_state = S_COLOR_OFF;
      }
      break;

    case S_CONFIRM_SAVE_WAIT:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (!pressed) new_state = S_SAVE_MODE;
      break;


    // Pattern config
    case S_PATTERN_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_PATTERN_PRESSED;
      }
      break;

    case S_PATTERN_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_PATTERN_SELECT_WAIT;
      } else if (!pressed) {
        mode->pattern[cur_variant] = (mode->pattern[cur_variant] + 1) % NUM_PATTERNS;
        resetMode();
        new_state = S_PATTERN_OFF;
      }
      break;

    case S_PATTERN_SELECT_WAIT:
      if (!pressed) {
        new_state = S_SAVE_MODE;
      }
      break;


    // Accel mode and sensitivity
    case S_ACCMODE_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_ACCMODE_PRESSED;
      }
      break;

    case S_ACCMODE_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_ACCMODE_SENS;
      } else if (!pressed) {
        mode->accel_mode = (mode->accel_mode + 1) % 5;
        new_state = S_ACCMODE_OFF;
      }
      break;

    case S_ACCMODE_SENS:
      if (since_trans == 0) flash (128, 128, 128, 5);
      if (since_trans > LONG_HOLD) {
        mode->accel_sens = (mode->accel_sens + 1) % 3;
        since_trans = 0;
      } else if (!pressed) {
        new_state = S_SAVE_MODE;
      }
      break;

    // Bundle select
    case S_BUNDLE_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_BUNDLE_PRESSED;
      }
      break;

    case S_BUNDLE_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_WAIT;
      } else if (!pressed) {
        cur_bundle = (cur_bundle + 1) % NUM_BUNDLES;
        changeMode(0);
        new_state = S_BUNDLE_OFF;
      }
      break;

    case S_BUNDLE_SELECT_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (since_trans > LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_WAIT;
      } else if (!pressed) {
        flash(128, 128, 128, 5);
        new_state = S_PLAY_OFF;
      }
      break;

    case S_BUNDLE_EDIT_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans > LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_WAIT;
      } else if (!pressed) {
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;


    // Bundle slot edit
    case S_BUNDLE_EDIT_OFF:
      if (since_trans == 0) {
        cur_mode_idx = bundles[cur_bundle][bundle_idx];
        resetMode();
      }
      if (pressed && since_trans > PRESS_DELAY) new_state = S_BUNDLE_EDIT_PRESSED;
      break;

    case S_BUNDLE_EDIT_PRESSED:
      if (since_trans > LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_SET_WAIT;
      } else if (!pressed) {
        bundles[cur_bundle][bundle_idx] = (bundles[cur_bundle][bundle_idx] + 1) % NUM_MODES;
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;

    case S_BUNDLE_EDIT_SET_WAIT:
      if (since_trans == 0) flash(128, 0, 128, 5);
      if (since_trans > LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_SAVE_WAIT;
      } else if (!pressed) {
        bundle_idx++;
        bundle_slots[cur_bundle] = bundle_idx;
        if (bundle_idx >= NUM_MODES) {
          new_state = S_SAVE_BUNDLES;
        } else {
          new_state = S_BUNDLE_EDIT_OFF;
        }
      }
      break;

    case S_BUNDLE_EDIT_SAVE_WAIT:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (!pressed) {
        new_state = S_SAVE_BUNDLES;
      }
      break;


    // On Wake
    case S_SLEEP_WAKE:
      if (EEPROM.read(ADDR_LOCKED)) {
        if (since_trans == VERY_LONG_HOLD) flash(0, 128, 0, 5);
        if (!pressed) {
          if (since_trans > VERY_LONG_HOLD) {
            EEPROM.update(ADDR_LOCKED, 0);
            new_state = S_PLAY_OFF;
          } else {
            enterSleep();
          }
        }
      } else {
        if (since_trans >= LONG_HOLD) {
          new_state = S_SLEEP_BUNDLE_WAIT;
        } else if (!pressed) {
          new_state = S_PLAY_OFF;
        }
      }
      break;

    case S_SLEEP_BUNDLE_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (since_trans > VERY_LONG_HOLD) {
        new_state = S_SLEEP_RESET_WAIT;
      } else if (!pressed) {
        new_state = S_BUNDLE_OFF;
      }
      break;

    case S_SLEEP_RESET_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (since_trans > VERY_LONG_HOLD) {
        new_state = S_SLEEP_WAIT_TOO_LONG;
      } else if (!pressed) {
        new_state = S_RESET_OFF;
      }
      break;

    case S_SLEEP_WAIT_TOO_LONG:
      if (!pressed) {
        enterSleep();
      }
      break;


    // Reset
    case S_RESET_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_RESET_PRESSED;
      }
      break;

    case S_RESET_PRESSED:
      if (since_trans >= VERY_LONG_HOLD) {
        flash(128, 0, 0, 1);
        flash(0, 0, 128, 1);
        flash(128, 0, 0, 1);
        flash(0, 0, 128, 1);
        flash(128, 0, 0, 1);
        resetMemory();
        new_state = S_HELD;
      } else if (!pressed) {
        enterSleep();
      }
      break;


    // Save states
    case S_SAVE_MODE:
      flash(128, 128, 128, 1);
      flash(128, 0, 0, 1);
      flash(128, 128, 128, 1);
      flash(0, 128, 0, 1);
      flash(128, 128, 128, 1);
      saveMode(cur_mode_idx);
      new_state = S_PLAY_OFF;
      break;

    case S_SAVE_BUNDLES:
      flash(128, 128, 128, 1);
      flash(128, 0, 0, 1);
      flash(128, 128, 128, 1);
      flash(0, 0, 128, 1);
      flash(128, 128, 128, 1);
      saveBundles();
      new_state = S_PLAY_OFF;
      break;


    // GUI states
    case S_GUI_MODE:
    case S_GUI_BUNDLES:
    case S_GUI_PALETTE:
      break;

    case S_HELD:
      if (!pressed) new_state = S_PLAY_OFF;
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
}


void enterSleep() {
  writeFrame(0, 0, 0);
  EEPROM.update(ADDR_SLEEPING, 1);
  digitalWrite(PIN_LDO, LOW);

  delay(6400);
  wdt_enable(WDTO_15MS);
  delay(64000);
}

void pushInterrupt() {}


void clearMemory() {
  for (int i = 0; i < 1024; i++) {
    EEPROM.update(i, 0);
  }
}

void initModes() {
  for (uint8_t m = 0; m < NUM_MODES; m++) {
    modes[m].accel_mode = pgm_read_byte(&factory_modes[m][0]);
    modes[m].accel_sens = pgm_read_byte(&factory_modes[m][1]);
    modes[m].pattern[0] = pgm_read_byte(&factory_modes[m][2]);
    modes[m].num_colors[0] = pgm_read_byte(&factory_modes[m][3]);
    modes[m].pattern[1] = pgm_read_byte(&factory_modes[m][20]);
    modes[m].num_colors[1] = pgm_read_byte(&factory_modes[m][21]);
    for (uint8_t c = 0; c < 16; c++) {
      modes[m].colors[0][c] = pgm_read_byte(&factory_modes[m][c + 4]);
      modes[m].colors[1][c] = pgm_read_byte(&factory_modes[m][c + 22]);
    }
  }
}

void initBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = pgm_read_byte(&factory_bundle_slots[b]);
    for (uint8_t s = 0; s < NUM_MODES; s++) {
      bundles[b][s] = pgm_read_byte(&factory_bundles[b][s]);
    }
  }
}

void saveMode(uint8_t idx) {
  EEPROM.update(ADDR_MODES + (idx * 40) + 0, modes[idx].accel_mode);
  EEPROM.update(ADDR_MODES + (idx * 40) + 1, modes[idx].accel_sens);
  EEPROM.update(ADDR_MODES + (idx * 40) + 2, modes[idx].pattern[0]);
  EEPROM.update(ADDR_MODES + (idx * 40) + 3, modes[idx].num_colors[0]);
  EEPROM.update(ADDR_MODES + (idx * 40) + 20, modes[idx].pattern[1]);
  EEPROM.update(ADDR_MODES + (idx * 40) + 21, modes[idx].num_colors[1]);
  for (uint8_t c = 0; c < 16; c++) {
    EEPROM.update(ADDR_MODES + (idx * 40) + c + 4, modes[idx].colors[0][c]);
    EEPROM.update(ADDR_MODES + (idx * 40) + c + 22, modes[idx].colors[1][c]);
  }
}

void saveModes() {
  for (uint8_t m = 0; m < NUM_MODES; m++) saveMode(m);
}

void saveBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    EEPROM.update(ADDR_BUNDLES + (b * 20), bundle_slots[b]);
    for (uint8_t s = 0; s < NUM_MODES; s++) {
      EEPROM.update(ADDR_BUNDLES + (b * 20) + s + 1, bundles[b][s]);
    }
  }
}

void loadMode(uint8_t idx) {
  modes[idx].accel_mode = EEPROM.read(ADDR_MODES + (idx * 40) + 0);
  modes[idx].accel_sens = EEPROM.read(ADDR_MODES + (idx * 40) + 1);
  modes[idx].pattern[0] = EEPROM.read(ADDR_MODES + (idx * 40) + 2);
  modes[idx].num_colors[0] = EEPROM.read(ADDR_MODES + (idx * 40) + 3);
  modes[idx].pattern[1] = EEPROM.read(ADDR_MODES + (idx * 40) + 20);
  modes[idx].num_colors[1] = EEPROM.read(ADDR_MODES + (idx * 40) + 21);
  for (uint8_t c = 0; c < 16; c++) {
    modes[idx].colors[0][c] = EEPROM.read(ADDR_MODES + (idx * 40) + c + 4);
    modes[idx].colors[1][c] = EEPROM.read(ADDR_MODES + (idx * 40) + c + 22);
  }
}

void loadModes() {
  for (uint8_t m = 0; m < NUM_MODES; m++) loadMode(m);
}

void loadBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = EEPROM.read(ADDR_BUNDLES + (b * 20));
    for (uint8_t s = 0; s < NUM_MODES; s++) {
      bundles[b][s] = EEPROM.read(ADDR_BUNDLES + (b * 20) + s + 1);
    }
  }
}

void resetMemory() {
  clearMemory();
  initModes();
  saveModes();
  initBundles();
  saveBundles();
  initPalette();
  savePalette(ADDR_PALETTE);
}

void loadMemory() {
  loadModes();
  loadBundles();
  loadPalette(ADDR_PALETTE);
}


void cmdReadMode(uint8_t idx, uint8_t addr) {
  uint8_t v, i;
  Serial.write(idx);
  Serial.write(addr);
  if (addr == 0) {
    Serial.write(modes[idx].accel_mode);
  } else if (addr == 1) {
    Serial.write(modes[idx].accel_sens);
  } else if (addr < 38) {
    v = (addr - 2) / 18;
    i = (addr - 2) % 18;
    if (i == 0) {
      Serial.write(modes[idx].pattern[v]);
    } else if (i == 1) {
      Serial.write(modes[idx].num_colors[v]);
    } else {
      Serial.write(modes[idx].colors[v][i - 2]);
    }
  } else {
    Serial.write(0);
  }
}

void cmdReadBundles(uint8_t addr) {
  uint8_t b = addr / 20;
  uint8_t s = addr % 20;
  Serial.write(16);
  Serial.write(addr);
  if (s == 0) { Serial.write(bundle_slots[b]);
  } else {      Serial.write(bundles[b][s - 1]);
  }
}

void cmdReadPalette(uint8_t addr) {
  Serial.write(17);
  Serial.write(addr);
  Serial.write(color_palette[addr / 3][addr % 3]);
}

void cmdRead(uint8_t target, uint8_t addr) {
  if (target < 16) {
    cmdReadMode(target, addr);
  } else if (target == 16) {
    cmdReadBundles(addr);
  } else if (target = 17) {
    cmdReadPalette(addr);
  }
}

void cmdWriteMode(uint8_t idx, uint8_t addr, uint8_t val) {
  uint8_t v, i;
  if (addr == 0) {
    modes[idx].accel_mode = val;
  } else if (addr == 1) {
    modes[idx].accel_sens = val;
  } else if (addr < 38) {
    v = (addr - 2) / 18;
    i = (addr - 2) % 18;
    if (i == 0) {
      modes[idx].pattern[v] = val;
    } else if (i == 1) {
      if (val == 100) {
        modes[idx].num_colors[v] = constrain(modes[idx].num_colors[v] + 1, 1, 16);
      } else if (val == 99) {
        modes[idx].num_colors[v] = constrain(modes[idx].num_colors[v] - 1, 1, 16);
      } else {
        modes[idx].num_colors[v] = val;
      }
    } else {
      modes[idx].colors[v][i - 2] = val;
    }
  }
  if (idx == cur_mode_idx) resetMode();
}

void cmdWriteBundles(uint8_t addr, uint8_t val) {
  if (addr % 20 == 0) { bundle_slots[addr / 20] = val;
  } else {              bundles[addr / 20][(addr % 20) - 1] = val;
  }
}

void cmdWritePalette(uint8_t addr, uint8_t val) {
  color_palette[addr / 3][addr % 3] = val;
}

void cmdWrite(uint8_t target, uint8_t addr, uint8_t val) {
  if (target < 16) {
    cmdWriteMode(target, addr, val);
  } else if (target == 16) {
    cmdWriteBundles(addr, val);
  } else if (target = 17) {
    cmdWritePalette(addr, val);
  }
  cmdRead(target, addr);
}

void cmdDumpMode(uint8_t idx) {
  for (uint8_t i = 0; i < 38; i++) cmdReadMode(idx, i);
}

void cmdDumpBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    for (uint8_t i = 0; i < (1 + NUM_MODES); i++) cmdReadBundles(i + (b * 20));
  }
}

void cmdDumpPalette() {
  for (uint8_t c = 0; c < (NUM_COLORS * 3); c++) cmdReadPalette(c);
}

void cmdDump(uint8_t target) {
  Serial.write(200); Serial.write(target); Serial.write(cur_mode_idx);
  if (target < 16) {
    cmdDumpMode(target);
  } else if (target == 16) {
    cmdDumpBundles();
  } else if (target = 17) {
    cmdDumpPalette();
  } else if (target == 99) {
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      cmdDumpMode(i);
    }
    cmdDumpBundles();
    cmdDumpPalette();
  }
  Serial.write(201); Serial.write(target); Serial.write(cur_mode_idx);
}

void cmdLoad(uint8_t target) {
  if (target < 16) {
    loadMode(target);
  } else if (target == 16) {
    loadBundles();
  } else if (target = 17) {
    loadPalette(ADDR_PALETTE);
  } else if (target == 99) {
    loadModes();
    loadBundles();
    loadPalette(ADDR_PALETTE);
  }
}

void cmdSave(uint8_t target) {
  if (target < 16) {
    saveMode(target);
  } else if (target == 16) {
    saveBundles();
  } else if (target = 17) {
    savePalette(ADDR_PALETTE);
  } else if (target == 99) {
    saveModes();
    saveBundles();
    savePalette(ADDR_PALETTE);
  }
  flash(128, 128, 128, 4);
  flash(128, 0, 0, 4);
  flash(128, 128, 128, 4);
}

void cmdExecute(uint8_t action, uint8_t arg0, uint8_t arg1) {
  if (action == 0) {
    cur_mode_idx = (cur_mode_idx + NUM_MODES - 1) % NUM_MODES;
    resetMode();
  } else if (action == 1) {
    cur_mode_idx = (cur_mode_idx + 1) % NUM_MODES;
    resetMode();
  } else if (action == 2) {
    cur_mode_idx = arg0;
    resetMode();
  } else if (action == 10) {
    if (arg0 == 0) {        new_state = S_GUI_MODE;
    } else if (arg0 == 1) { new_state = S_GUI_PALETTE;
    } else if (arg0 == 2) { new_state = S_GUI_BUNDLES;
    } else {                new_state = S_PLAY_OFF;
    }
  } else if (action == 20) {
    gui_color = arg0;
    gui_shade = arg1;
  }
}

void handleSerial() {
  uint8_t in0, in1, in2, in3;
  while (Serial.available() >= 4) {
    in0 = Serial.read();
    in1 = Serial.read();
    in2 = Serial.read();
    in3 = Serial.read();

    if (in0 == 'D') {
      cmdDump(in1);
    } else if (in0 == 'L') {
      cmdLoad(in1);
    } else if (in0 == 'S') {
      cmdSave(in1);
    } else if (in0 == 'R') {
      cmdRead(in1, in2);
    } else if (in0 == 'W') {
      cmdWrite(in1, in2, in3);
    } else if (in0 == 'X') {
      cmdExecute(in1, in2, in3);
    }
  }
}


void handleAccel() {
  switch (accel_tick % accel_counts) {
    case 0:   // Get raw accel values
      accelReadXYZ();
      break;
    case 1:   // Normalize to Gs
      accelNormalize();
      break;
    case 2:   // Calculate the magnitude of all acceleration axes
      a_mag = sqrt((fxg * fxg) + (fyg * fyg) + (fzg * fzg));
      break;
    case 3:   // Track the acceleration by bins
      accelUpdateBins();
      break;
    case 4:   // Calculate vector of Y and Z axes for pitch calculation
      a_pitch = sqrt((fyg * fyg) + (fzg * fzg));
      break;
    case 5:   // Calculate pitch in radians
      a_pitch = atan2(-fxg, a_pitch);
      break;
    case 6:   // Convert pitch to degrees
      a_pitch = (a_pitch * 180) / M_PI;
      break;
    case 7:   // Calculate roll in radians
      a_roll = atan2(-fyg, fzg);
      break;
    case 8:   // Convert roll to degrees
      a_roll = (a_roll * 180.0) / M_PI;
      break;
    case 9:
      if (mode->accel_mode == AMODE_OFF) {
        // noop
      } else if (mode->accel_mode == AMODE_SPEED) {
        if (mode->accel_sens == ASENS_LOW) {
          if (cur_variant == 0 && a_speed > 15) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 11) cur_variant = 0;
        } else if (mode->accel_sens == ASENS_MEDIUM) {
          if (cur_variant == 0 && a_speed > 8) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 4) cur_variant = 0;
        } else {
          if (cur_variant == 0 && a_speed > 4) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 3) cur_variant = 0;
        }
      } else {
        if (cur_variant == 0 && a_speed < 3) {
          if (mode->accel_mode == AMODE_TILTX) {
            accel_counter += (a_pitch < -75) ? 1 : -accel_counter;
          } else if (mode->accel_mode == AMODE_TILTY) {
            accel_counter += (a_roll < -80 && a_roll > -100) ? 1 : -accel_counter;
          } else {
            accel_counter += (fzg < -0.9 && fzg > -1.1) ? 1 : -accel_counter;
          }
        } else if (cur_variant == 1 && a_speed < 3) {
          if (mode->accel_mode == AMODE_TILTX) {
            accel_counter += (a_pitch > 75) ? 1 : -accel_counter;
          } else if (mode->accel_mode == AMODE_TILTY) {
            accel_counter += (a_roll > 80 && a_roll < 100) ? 1 : -accel_counter;
          } else {
            accel_counter += (fzg > 0.9 && fzg < 1.1) ? 1 : -accel_counter;
          }
        }
        if (accel_counter > thresh_timings[mode->accel_sens]) {
          cur_variant = !cur_variant;
          mode->accel_sens = 0;
        }
      }
      break;

    // Can have no higher than case 15
    default:
      break;
  }

  accel_tick++;
  if (accel_tick >= accel_count_wrap) accel_tick = 0;
}

void accelSend(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(accel_addr);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}

void accelInit() {
  if (accel_model == 0) {
    accelSend(0x07, 0x00);        // Standby to accept new settings
    accelSend(0x08, 0x00);        // Set 120 samples/sec (every 16 2/3 frames)
    accelSend(0x07, 0x01);        // Active mode
  } else {
    accelSend(0x2A, 0x00);        // Standby to accept new settings
    accelSend(0x0E, 0x00);        // Set +-2g range
    accelSend(0x2A, 0b00011001);  // Set 100 samples/sec (every 20 frames) and active
  }
}

void accelReadXYZ() {
  Wire.beginTransmission(accel_addr);

  // v1 is a 6 bit value from 
  if (accel_model == 0) {
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom((int)accel_addr, 3);

    if (Wire.available()) xg = Wire.read();
    xg = (xg >= 32) ? -64 + xg : xg;

    if (Wire.available()) yg = Wire.read();
    yg = (yg >= 32) ? -64 + yg : yg;

    if (Wire.available()) zg = Wire.read();
    zg = (zg >= 32) ? -64 + zg : zg;
  } else {
    Wire.write(0x01);
    Wire.endTransmission(false);
    Wire.requestFrom((int)accel_addr, 6);

    if (Wire.available()) xg = Wire.read() << 4;
    if (Wire.available()) xg |= Wire.read() >> 4;
    xg = (xg >= 2048) ? -4096 + xg : xg;

    if (Wire.available()) yg = Wire.read() << 4;
    if (Wire.available()) yg |= Wire.read() >> 4;
    yg = (yg >= 2048) ? -4096 + yg : yg;

    if (Wire.available()) zg = Wire.read() << 4;
    if (Wire.available()) zg |= Wire.read() >> 4;
    zg = (zg >= 2048) ? -4096 + zg : zg;
  }
}

void accelNormalize() {
  // Normalize accel values to gs
  float pg = (accel_model == 0) ? 21.0 : 1024.0;
  fxg = xg / pg; fyg = yg / pg; fzg = zg / pg;
}

void accelUpdateBins() {
  // Tracks the magnitude of acceleration
  // v1 max is ~ 2.55 - 2.64
  // v2 max is - 3.46
  a_speed = 0;
  for (uint8_t i = 0; i < ACCEL_BINS; i++) {
    if (a_mag > thresh_bins_p[accel_model][i] || a_mag < thresh_bins_n[i]) {
      thresh_last[i] = 0;
      thresh_cnts[i] = constrain(thresh_cnts[i] + 1, 0, 200);
    }
    thresh_last[i]++;
    if (thresh_last[i] >= thresh_falloff) thresh_cnts[i] = 0;
    if (thresh_cnts[i] > thresh_target) a_speed = i + 1;
  }
}

void detectAccelModel() {
  // Try to talk to the v2 sensor to get it's id
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(0x0d);
  Wire.endTransmission(false);

  // Read in the ID value if it's there
  Wire.requestFrom(V2_ACCEL_ADDR, 1);
  uint8_t v = 0;
  if (Wire.available()) v = Wire.read();

  // Set accelerometer properties based on the model id
  if (v == 0x4a || v == 0x5a) {
    // v2 updates at 100/s or every 20 frames
    accel_model = 1;
    accel_addr = V2_ACCEL_ADDR;
    accel_counts = 20;
    accel_count_wrap = 20;
    thresh_falloff = 10;
    thresh_target = 5;
  } else {
    // v1 updates 120/s or every 16 and 2/3 frames
    accel_model = 0;
    accel_addr = V1_ACCEL_ADDR;
    accel_counts = 17;
    accel_count_wrap = 50;
    thresh_falloff = 12;
    thresh_target = 6;
  }
}
