#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"
#include "palette.h"
#include "pattern.h"

#define EEPROM_VERSION          42

#define ACCEL_BINS              12
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
#define SHORT_HOLD              500
#define LONG_HOLD               1000
#define VERY_LONG_HOLD          3000

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
int accel_addr;
uint8_t accel_counts;
uint8_t accel_count_wrap;
uint8_t accel_tick = 0;
int16_t xg, yg, zg;
float fxg, fyg, fzg;
float fxg2, fyg2, fzg2;
float a_mag, a_pitch, a_roll;
uint8_t a_speed;

float thresh_bins_p[ACCEL_BINS] = {1.1, 1.233, 1.367, 1.5, 1.633, 1.767, 1.9, 2.033, 2.167, 2.3, 2.433, 2.567};
uint8_t thresh_last[ACCEL_BINS];
uint8_t thresh_cnts[ACCEL_BINS];
uint8_t thresh_timings[3] = {12, 6, 3};
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
uint8_t cidx[2];
int16_t cntr[2];

typedef struct Mode {
  uint8_t accel_mode, accel_sens;   // 2B
  uint8_t pattern[2];               // 2B
  uint8_t num_colors[2];            // 2B
  uint8_t colors[2][PALETTE_SIZE];  // 32B
} Mode;                             // 38B

#define MODE_SIZE 38
typedef union PackedMode {
  Mode m;
  uint8_t d[MODE_SIZE];
} PackedMode;

PackedMode pmodes[NUM_MODES];
Mode* mode;


const PROGMEM uint8_t factory_modes[NUM_MODES][MODE_SIZE] = {
  {AMODE_SPEED, ASENS_HIGH, P_HYPER3, P_DOPS3, 6, 6,
    0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_MEDIUM, P_BLASTER, P_STROBIEFUSE, 7, 3,
    0x01, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x48, 0x50, 0x58, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_LOW, P_STROBIE, P_VEXING, 2, 2,
    0xce, 0x1a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0xce, 0x1a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_SPEED, ASENS_LOW, P_DASHDOPS, P_DOPSDASH, 7, 7,
    0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW, P_RAZOR5, P_STROBE, 15, 1,
    0x08, 0x0a, 0x0c, 0x08, 0x10,  0x18, 0x1a, 0x1c, 0x18, 0x08,  0x10, 0x12, 0x14, 0x10, 0x18,  0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_BLASTER3, P_STROBE, 9, 1,
    0x01, 0xc8, 0xc8, 0x01, 0xd0, 0xd0, 0x01, 0xd8, 0xd8, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_WAVE, P_STROBE, 4, 1,
    0x18, 0x14, 0x16, 0x12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_STRETCH, P_STROBE, 3, 1,
    0x1e, 0x1a, 0x15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW, P_DASHMORPH, P_STROBE, 3, 1,
    0x21, 0x22, 0x23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_PULSAR, P_STROBE, 3, 1,
    0x1f, 0x1c, 0x19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_DOTTED, P_STROBE, 13, 1,
    0xc1, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_BOTTLEROCKET, P_STROBE, 7, 1,
    0x01, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },

  {AMODE_OFF, ASENS_LOW, P_BLASTER3, P_STROBE, 6, 1,
    0x1f, 0xd4, 0xd4, 0x14, 0xdf, 0xdf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_BLASTER3, P_STROBE, 6, 1,
    0x26, 0xec, 0xec, 0x2c, 0xe6, 0xe6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_INFLUX, P_STROBE, 3, 1,
    0x21, 0x24, 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  },
  {AMODE_OFF, ASENS_LOW, P_GROW, P_STROBE, 6, 1,
    0x28, 0, 0x2b, 0, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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

  button_state = new_state = S_PLAY_OFF;
  pinMode(PIN_BUTTON, INPUT);
  attachInterrupt(0, pushInterrupt, FALLING);

  if (EEPROM.read(ADDR_SLEEPING)) {
    while (!eeprom_is_ready());
    EEPROM.update(ADDR_SLEEPING, 0);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
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

  Serial.begin(57600);
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
  wdt_enable(WDTO_15MS);
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
  mode = &pmodes[cur_mode_idx].m;
  cur_variant = 0;
  accel_counter = 0;
  tick[0] = cidx[0] = cntr[0] = 0;
  tick[1] = cidx[1] = cntr[1] = 0;
}

void renderMode() {
  renderPattern(
      mode->pattern[0], mode->num_colors[0], mode->colors[0],
      tick[0], cidx[0], cntr[0],
      led_r, led_g, led_b, cur_variant == 0);
  renderPattern(
      mode->pattern[1], mode->num_colors[1], mode->colors[1],
      tick[1], cidx[1], cntr[1],
      led_r, led_g, led_b, cur_variant == 1);
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
    if (since_trans > (2000 * 180)) {
      enterSleep();
    }
    led_r = led_g = led_b = 0;
  }
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  /* if (limiter > 64000) { Serial.print(limiter); Serial.print(F("\t")); Serial.println(accel_tick); } */
  while (limiter < 64000) {}
  limiter = 0;

  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
  wdt_reset();
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t f = 0; f < flashes; f++) {
    for (uint8_t t = 0; t < 50; t++) {
        if (t < 25) writeFrame(r, g, b);
        else        writeFrame(0, 0, 0);
    }
  }
  since_trans += flashes * 50;
}


void changeMode(uint8_t v) {
  if (v == 0) bundle_idx = 0;
  else        bundle_idx = (bundle_idx + 1) % bundle_slots[cur_bundle];

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
        if (since_trans == VERY_LONG_HOLD) flash(0, 0, 128, 5);
        if (!pressed) {
          conjure_toggle = !conjure_toggle;
          new_state = S_PLAY_OFF;
        } else if (since_trans >= VERY_LONG_HOLD) {
          conjure = conjure_toggle = false;
          new_state = S_PLAY_OFF;
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
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        enterSleep();
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONFIG_WAIT;
      } else if (!pressed) {
        conjure = true;
        conjure_toggle = false;
        new_state = S_PLAY_OFF;
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_LOCK_WAIT;
      } else if (!pressed) {
        config_state = CONFIG_COLORSA;
        new_state = S_CONFIG_OFF;
      }
      break;

    case S_PLAY_LOCK_WAIT:
      if (since_trans == 0) flash(128, 0, 0, 5);
      if (since_trans > LONG_HOLD) {
        flash(128, 128, 128, 5);
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        while (!eeprom_is_ready());
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
        while (!eeprom_is_ready());
        EEPROM.update(ADDR_CUR_BUNDLE, cur_bundle);
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
            while (!eeprom_is_ready());
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
        wdt_disable();
        resetMemory();
        wdt_enable(WDTO_15MS);
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
      while (!eeprom_is_ready());
      saveMode(cur_mode_idx);
      new_state = S_PLAY_OFF;
      break;

    case S_SAVE_BUNDLES:
      flash(128, 128, 128, 1);
      flash(128, 0, 0, 1);
      flash(128, 128, 128, 1);
      flash(0, 0, 128, 1);
      flash(128, 128, 128, 1);
      while (!eeprom_is_ready());
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
  accelStandby();
  while (!eeprom_is_ready());
  EEPROM.update(ADDR_SLEEPING, 1);
  digitalWrite(PIN_LDO, LOW);
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
    for (uint8_t b = 0; b < MODE_SIZE; b++) {
      pmodes[m].d[b] = pgm_read_byte(&factory_modes[m][b]);
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
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    EEPROM.update(ADDR_MODES + (idx * 40) + b, pmodes[idx].d[b]);
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
  for (uint8_t b = 0; b < MODE_SIZE; b++) {
    pmodes[idx].d[b] = EEPROM.read(ADDR_MODES + (idx * 40) + b);
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
  while (!eeprom_is_ready());
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
  Serial.write(idx);
  Serial.write(addr);
  if (addr < MODE_SIZE) Serial.write(pmodes[idx].d[addr]);
  else                  Serial.write(0);
}

void cmdReadBundles(uint8_t addr) {
  uint8_t b = addr / 20;
  uint8_t s = addr % 20;
  Serial.write(16);
  Serial.write(addr);
  if (s == 0) Serial.write(bundle_slots[b]);
  else        Serial.write(bundles[b][s - 1]);
}

void cmdReadPalette(uint8_t addr) {
  Serial.write(17);
  Serial.write(addr);
  Serial.write(color_palette[addr / 3][addr % 3]);
}

void cmdRead(uint8_t target, uint8_t addr) {
  if      (target < 16)  cmdReadMode(target, addr);
  else if (target == 16) cmdReadBundles(addr);
  else if (target == 17) cmdReadPalette(addr);
}

void cmdWriteMode(uint8_t idx, uint8_t addr, uint8_t val) {
  if (addr < MODE_SIZE) pmodes[idx].d[addr] = val;
  if (idx == cur_mode_idx) resetMode();
}

void cmdWriteBundles(uint8_t addr, uint8_t val) {
  if (addr % 20 == 0) bundle_slots[addr / 20] = val;
  else                bundles[addr / 20][(addr % 20) - 1] = val;
}

void cmdWritePalette(uint8_t addr, uint8_t val) {
  color_palette[addr / 3][addr % 3] = val;
}

void cmdWrite(uint8_t target, uint8_t addr, uint8_t val) {
  if      (target < 16)  cmdWriteMode(target, addr, val);
  else if (target == 16) cmdWriteBundles(addr, val);
  else if (target == 17) cmdWritePalette(addr, val);
}

void cmdDumpMode(uint8_t idx) {
  for (uint8_t i = 0; i < MODE_SIZE; i++) cmdReadMode(idx, i);
}

void cmdDumpBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    for (uint8_t i = 0; i < (1 + NUM_MODES); i++) cmdReadBundles(i + (b * 20));
  }
}

void cmdDumpPalette() {
  for (uint8_t c = 0; c < (NUM_COLORS * 3); c++) {
    cmdReadPalette(c);
  }
}

void cmdDump(uint8_t target) {
  Serial.write(200); Serial.write(target); Serial.write(cur_mode_idx);
  if (target < 16) {
    cmdDumpMode(target);
  } else if (target == 16) {
    cmdDumpBundles();
  } else if (target == 17) {
    cmdDumpPalette();
  } else if (target == 99) {
    cmdDumpPalette();
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      cmdDumpMode(i);
    }
    cmdDumpBundles();
  }
  Serial.write(201); Serial.write(target); Serial.write(cur_mode_idx);
}

void cmdLoad(uint8_t target) {
  if (target < 16) {
    loadMode(target);
  } else if (target == 16) {
    loadBundles();
  } else if (target == 17) {
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
  } else if (target == 17) {
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
    if (arg0 == 0) {
      wdt_disable();
      new_state = S_GUI_MODE;
    } else if (arg0 == 1) {
      wdt_disable();
      new_state = S_GUI_PALETTE;
    } else if (arg0 == 2) {
      wdt_disable();
      new_state = S_GUI_BUNDLES;
    } else {
      wdt_enable(WDTO_15MS);
      new_state = S_PLAY_OFF;
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
    case 0:
      Wire.beginTransmission(accel_addr);
      if (accel_model == 0) {
        Wire.write(0x00);
        Wire.endTransmission(false);
      } else {
        Wire.write(0x01);
        Wire.endTransmission(false);
      }
      break;
    case 1:
      if (accel_model == 0) Wire.requestFrom(accel_addr, 3);
      else                  Wire.requestFrom(accel_addr, 6);
      break;
    case 2:
      if (accel_model == 0) {
        if (Wire.available()) xg = Wire.read();
        xg = (xg >= 32) ? -64 + xg : xg;
        if (Wire.available()) yg = Wire.read();
        yg = (yg >= 32) ? -64 + yg : yg;
        if (Wire.available()) zg = Wire.read();
        zg = (zg >= 32) ? -64 + zg : zg;
      } else {
        if (Wire.available()) yg = Wire.read() << 4;
        if (Wire.available()) yg |= Wire.read() >> 4;
        yg = (yg >= 2048) ? -4096 + yg : yg;
        if (Wire.available()) xg = Wire.read() << 4;
        if (Wire.available()) xg |= Wire.read() >> 4;
        xg = (xg >= 2048) ? -4096 + xg : xg;
        if (Wire.available()) zg = Wire.read() << 4;
        if (Wire.available()) zg |= Wire.read() >> 4;
        zg = (zg >= 2048) ? -4096 + zg : zg;
      }
      break;
    case 3:
      accelNormalize();
      break;
    case 4:
      fxg2 = fxg * fxg; fyg2 = fyg * fyg; fzg2 = fzg * fzg;
      break;
    case 5:
      a_mag = sqrt(fxg2 + fyg2 + fzg2);
      break;
    case 6:
      accelUpdateBins();
      break;
    case 7:
      a_pitch = sqrt(fyg2 + fzg2);
      break;
    case 8:
      a_pitch = atan2(-fxg, a_pitch);
      break;
    case 9:
      a_pitch = (a_pitch * 180.0) / M_PI;
      break;
    case 10:
      a_roll = atan2(-fyg, fzg);
      break;
    case 11:
      a_roll = (a_roll * 180.0) / M_PI;
      break;
    case 12:
      if (mode->accel_mode == AMODE_OFF) {
        // noop
      } else if (mode->accel_mode == AMODE_SPEED) {
        if (mode->accel_sens == ASENS_LOW) {
          if (cur_variant == 0 && a_speed > 11) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 9) cur_variant = 0;
        } else if (mode->accel_sens == ASENS_MEDIUM) {
          if (cur_variant == 0 && a_speed > 7) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 6) cur_variant = 0;
        } else {
          if (cur_variant == 0 && a_speed > 1) cur_variant = 1;
          if (cur_variant == 1 && a_speed < 1) cur_variant = 0;
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
    accelSend(0x08, 0x01);        // Set 64 samples/sec (every 31.25 frames)
    accelSend(0x07, 0x01);        // Active mode
  } else {
    accelSend(0x2A, 0x00);        // Standby to accept new settings
    accelSend(0x0E, 0x00);        // Set +-2g range
    accelSend(0x2B, 0b00011000);  // Low Power SLEEP
    accelSend(0x2A, 0b00100001);  // Set 50 samples/sec (every 40 frames) and active
  }
}

void accelStandby() {
  if (accel_model == 0) {
    accelSend(0x07, 0x00);
  } else {
    accelSend(0x2A, 0x00);
  }
}

void accelNormalize() {
  float pg = (accel_model == 0) ? 21.0 : 1024.0;
  fxg = xg / pg; fyg = yg / pg; fzg = zg / pg;
}

void accelUpdateBins() {
  a_speed = 0;
  for (uint8_t i = 0; i < ACCEL_BINS; i++) {
    if (a_mag > thresh_bins_p[i]) {
      thresh_last[i] = 0;
      thresh_cnts[i] = max(thresh_cnts[i] + 1, 100);
    }
    thresh_last[i]++;
    if (thresh_last[i] >= thresh_falloff) thresh_cnts[i] = 0;
    if (thresh_cnts[i] > thresh_target) a_speed = i + 1;
  }
}

void detectAccelModel() {
  Wire.beginTransmission(V2_ACCEL_ADDR);
  Wire.write(0x0d);
  Wire.endTransmission(false);
  Wire.requestFrom(V2_ACCEL_ADDR, 1);
  uint8_t v = 0;
  if (Wire.available()) v = Wire.read();

  if (v == 0x4a || v == 0x5a) {
    // v2 updates at 50/s or every 20 frames
    accel_model = 1;
    accel_addr = V2_ACCEL_ADDR;
    accel_counts = 20;
    accel_count_wrap = 20;
    thresh_falloff = 10;
    thresh_target = 10;
  } else {
    // v1 updates 64/s or (15.625 frames)
    accel_model = 0;
    accel_addr = V1_ACCEL_ADDR;
    accel_counts = 16;
    accel_count_wrap = 125;
    thresh_falloff = 8;
    thresh_target = 8;
  }
}
