#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include "LowPower.h"
#include "elapsedMillis.h"
#include "MMA7660.h"
#include "mode.h"
#include "palette.h"
#include "pattern.h"

// Hardware
#define PIN_R 9
#define PIN_G 6
#define PIN_B 5
#define PIN_BUTTON 2
#define PIN_LDO A3
#define MMA7660_ADDRESS 0x4C
#define MMA8652_ADDRESS 0x1D

#define ADDR_MODES      0
#define ADDR_PALETTE    640
#define ADDR_BUNDLES    740
#define ADDR_VERSION    1020
#define ADDR_BUNDLE     1021
#define ADDR_LOCKED     1022
#define ADDR_SLEEPING   1023

#define PRESS_DELAY     100
#define SHORT_HOLD      1000
#define LONG_HOLD       2000
#define VERY_LONG_HOLD  6000

#define S_PLAY_OFF              0
#define S_PLAY_PRESSED          1
#define S_PLAY_SLEEP_WAIT       2
#define S_PLAY_CONFIG_WAIT      3
#define S_PLAY_CONJURE_WAIT     4

#define S_CONFIG_OFF            10
#define S_CONFIG_PRESSED        11
#define S_CONFIG_EDIT_WAIT      12
#define S_CONFIG_EXIT_WAIT      13

#define S_COLOR_OFF             20
#define S_COLOR_PRESSED         21
#define S_COLOR_SHADE           22
#define S_CONFIRM_OFF           23
#define S_CONFIRM_PRESSED       24
#define S_CONFIRM_REJECT_WAIT   25
#define S_CONFIRM_EXIT_WAIT     26

#define S_PATTERN_OFF           30
#define S_PATTERN_PRESSED       31
#define S_PATTERN_SELECT_WAIT   32
#define S_ACCMODE_OFF           33
#define S_ACCMODE_PRESSED       34
#define S_ACCMODE_SENS          35

#define S_BUNDLE_OFF            40
#define S_BUNDLE_PRESSED        41
#define S_BUNDLE_SELECT_WAIT    42
#define S_BUNDLE_EDIT_WAIT      43
#define S_BUNDLE_BPM_WAIT       44

#define S_BUNDLE_EDIT_OFF       50
#define S_BUNDLE_EDIT_PRESSED   51
#define S_BUNDLE_EDIT_SET_WAIT  52
#define S_BUNDLE_EDIT_EXIT_WAIT 53

#define S_SLEEP_WAKE            100
#define S_SLEEP_BUNDLE_WAIT     101
#define S_SLEEP_LOCK            102
#define S_SLEEP_WAKE_LOCK       103
#define S_SLEEP_RESET           104
#define S_RESET_OFF             105
#define S_RESET_PRESSED         106
#define S_RESET_CONFIRM_WAIT    107

#define S_SAVE_MODE             200
#define S_SAVE_BUNDLE           201

#define S_GUI_MODE              210
#define S_GUI_BUNDLE            211
#define S_GUI_PALETTE           212

#define THRESH_BINS 8
#define NUM_MODES   16
#define NUM_BUNDLES 4

// Button
uint8_t button_state, new_state;
uint16_t since_trans = 0;

// Accel
MMA7660 v1accel;
/* MMA8652 v2accel; */

uint8_t accel_model;
uint8_t accel_counts;
uint8_t accel_count_wrap;
uint8_t accel_tick;
uint8_t accel_high;
int16_t xg, yg, zg, lxg, lyg, lzg;
float fxg, fyg, fzg, mag_g, tiltx, tilty;
uint8_t thresh_last[THRESH_BINS];
uint8_t thresh_cnts[THRESH_BINS];
uint16_t thresh_test[2][THRESH_BINS] = {
  {24, 28, 32, 36, 40, 44, 48, 51},
  {19, 16, 13, 10, 8, 6, 4, 2},
};
uint8_t timing_thresh[3] = {30, 18, 6};

// Modes and Bundles
uint8_t cur_mode_idx, cur_bundle, bundle_idx;
uint8_t bundle_slots[NUM_BUNDLES] = {16, 4, 4, 4};
uint8_t bundles[NUM_BUNDLES][NUM_MODES] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {8, 9, 10, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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

// Core
elapsedMicros limiter = 0;
uint8_t led_r, led_g, led_b;
bool conjure = false;
bool conjure_toggle = false;
uint8_t cv = 0;
uint8_t cvc = 0;


void setup() {
  Serial.begin(57600);

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDO, OUTPUT);

  /*
  attachInterrupt(0, pushInterrupt, FALLING);
  locked = EEPROM.read(ADDR_LOCKED);
  cur_bundle = EEPROM.read(ADDR_BUNDLE);

  if (EEPROM.read(ADDR_SLEEPING)) {
    EEPROM.write(ADDR_SLEEPING, 0);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    button_state = new_state = S_SLEEP_WAKE;
  }
  detachInterrupt(0);

  if (EEPROM_VERSION != EEPROM.read(ADDR_VERSION)) {
    resetMemory();
  } else {
    loadSettings();
  }
  */

  digitalWrite(PIN_LDO, HIGH);
  accelSetup();

  noInterrupts();
  ADCSRA = 0; // Disable ADC
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  delay(40);
  limiter = accel_tick = cur_bundle = bundle_idx = lxg = lyg = lzg = 0;
  changeMode(0);
}

void accelSetup() {
  accel_model = 0;
  accel_counts = (accel_model == 0) ? 17 : 20;
  accel_count_wrap = (accel_model == 0) ? 50 : 20;
  accelInit();
}

void accelInit() {
  if (accel_model == 0) {
    v1accel.init();
  } else {
    // v2accel.init();
  }
}

void accelStandby() {
  if (accel_model == 0) {
    v1accel.standby();
  } else {
    /* v2accel.standby(); */
  }
}

void clearMemory() {
  for (int i = 0; i < 1024; i++) {
    EEPROM.write(i, 0);
  }
}

void resetMemory() {
  clearMemory();

  // Restore modes
  // Restore palette
  // Restore bundles
}

void loadSettings() {
  // Load modes
  // Load palette
  // Load bundles
}

void loop() {
  handlePress(digitalRead(PIN_BUTTON) == LOW);

  /* if (button_state == S_PLAY_OFF || */
  /*     button_state == S_BUNDLE_SELECT_OFF || */
  /*     button_state >= 100) { */
    handleAccel();
  /* } */

  /* handleRender(); */
  writeFrame(led_r, led_g, led_b);
}

void handleRender() {
  if (button_state == S_PLAY_OFF) {
  }
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  if (limiter > 32000) { Serial.println(accel_tick % accel_counts); }
  while (limiter < 32000) {}
  limiter = 0;

  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t i = 0; i < (100 * flashes); i++) {
    if ((i % 100) < 50) { writeFrame(r, g, b); }
    else {                writeFrame(0, 0, 0); }
  }
  since_trans += flashes * 100;
}

void readAccel() {
  if (accel_model == 0) {
    v1accel.readXYZ(&xg, &yg, &zg);
  } else {
    /* v2accel.readXYZ(&xg, &yg, &zg); */
  }
}

void translateAccel() {
  if (accel_model == 0) {
    xg = (xg < 64) ? ((xg < 32) ? xg : -64 + xg) : lxg; lxg = xg;
    yg = (yg < 64) ? ((yg < 32) ? yg : -64 + yg) : lyg; lyg = yg;
    zg = (zg < 64) ? ((zg < 32) ? zg : -64 + zg) : lzg; lzg = zg;
  } else {
    xg = (xg < 2048) ? xg : -4096 + xg; lxg = xg;
    yg = (yg < 2048) ? yg : -4096 + yg; lyg = yg;
    zg = (zg < 2048) ? zg : -4096 + zg; lzg = zg;
  }
}

void normalizeAccel() {
  if (accel_model == 0) {
    fxg = xg / 21.0;
    fyg = yg / 21.0;
    fzg = zg / 21.0;
  } else {
    fxg = xg / 1024.0;
    fyg = yg / 1024.0;
    fzg = zg / 1024.0;
  }
}

void updateBins() {
  for (uint8_t i = 0; i < THRESH_BINS; i++) {
    if (mag_g > thresh_test[0][i] || mag_g < thresh_test[1][i]) {
      thresh_last[i] = 0;
      thresh_cnts[i] = constrain(thresh_cnts[i] + 1, 0, 200);
    }
    thresh_last[i]++;
    if (thresh_last[i] > 12) {
      thresh_cnts[i] = 0;
    }
  }
}

void checkBins() {
  accel_high = 0;
  for (uint8_t i = 0; i < THRESH_BINS; i++) {
    if (thresh_cnts[i] > 6) {
      accel_high = i + 1;
    }
  }
}

void handleAccel() {
  switch (accel_tick % accel_counts) {
    case 0:
      readAccel();
      break;
    case 1:
      translateAccel();
      normalizeAccel();
      break;
    case 2:
      mag_g = sqrt((xg * xg) + (yg * yg) + (zg * zg));
      updateBins();
      checkBins();
      break;
    case 3:
      tiltx = sqrt(fyg * fyg + fzg * fzg);
      break;
    case 4:
      tiltx = (atan2(-fxg, tiltx) * 180.0) / M_PI;
      break;
    case 5:
      tilty = (atan2(-fyg, fzg) * 180.0) / M_PI;
      break;
    case 6:
      if (mode->accel_mode == AMODE_SPEED) {
        if (mode->accel_sens == ASENS_LOW) {
          if (mode->cur_variant == 0 && accel_high > 7) mode->cur_variant = 1;
          if (mode->cur_variant == 1 && accel_high < 5) mode->cur_variant = 0;
        } else if (mode->accel_sens == ASENS_MEDIUM) {
          if (mode->cur_variant == 0 && accel_high > 4) mode->cur_variant = 1;
          if (mode->cur_variant == 1 && accel_high < 2) mode->cur_variant = 0;
        } else {
          if (mode->cur_variant == 0 && accel_high > 1) mode->cur_variant = 1;
          if (mode->cur_variant == 1 && accel_high < 1) mode->cur_variant = 0;
        }
      } else {
        if (mode->cur_variant == 0 && accel_high < 2) {
          if (mode->accel_mode == AMODE_TILTX) {
            mode->accel_counter += (tiltx < -75) ? 1 : -mode->accel_counter;
          } else if (mode->accel_mode == AMODE_TILTY) {
            mode->accel_counter += (tilty < -80 && tilty > -100) ? 1 : -mode->accel_counter;
          } else {
            mode->accel_counter += (fzg < -0.9 && fzg > -1.1) ? 1 : -mode->accel_counter;
          }
        } else if (mode->cur_variant == 1 && accel_high < 2) {
          if (mode->accel_mode == AMODE_TILTX) {
            mode->accel_counter += (tiltx > 75) ? 1 : -mode->accel_counter;
          } else if (mode->accel_mode == AMODE_TILTY) {
            mode->accel_counter += (tilty > 80 && tilty < 100) ? 1 : -mode->accel_counter;
          } else {
            mode->accel_counter += (fzg > 0.9 && fzg < 1.1) ? 1 : -mode->accel_counter;
          }
        }
        if (mode->accel_counter > timing_thresh[mode->accel_sens]) {
          mode->cur_variant = !mode->cur_variant;
          mode->accel_sens = 0;
        }
      }
      break;
    case 7:
      /* if (cv == 0 && accel_high < 2) { */
        /* cvc += (tiltx < -75) ? 1 : -cvc; */
        /* cvc += (tilty < -80 && tilty > -100) ? 1 : -cvc; */
        /* cvc += (fzg < -0.9 && fzg > -1.1) ? 1 : -cvc; */
      /* } else if (cv == 1 && accel_high < 2) { */
        /* cvc += (tiltx > 75) ? 1 : -cvc; */
        /* cvc += (tilty > 80 && tilty < 100) ? 1 : -cvc; */
        /* cvc += (fzg > 0.9 && fzg < 1.1) ? 1 : -cvc; */
      /* } */

      /* if (cvc > 12) { */
      /*   cv = !cv; */
      /*   cvc = 0; */
      /* } */

      /* if (cv == 0 && accel_high > 7) { */
      if (cv == 0 && accel_high > 4) {
      /* if (cv == 0 && accel_high > 1) { */
        cv = 1;
      /* } else if (cv == 1 && accel_high < 5) { */
      } else if (cv == 1 && accel_high < 3) {
      /* } else if (cv == 1 && accel_high < 1) { */
        cv = 0;
      }

      if (cv == 0) {
        led_r = 64;
        led_g = led_b = 0;
      } else {
        led_b = 64;
        led_g = led_r = 0;
      }

      break;
    case 8:
      /* if (accel_high < 4) { */
      /*   led_r = 64 - (16 * accel_high); */
      /*   led_g = 16 * accel_high; */
      /*   led_b = 0; */
      /* } else if (accel_high < 8) { */
      /*   led_r = 0; */
      /*   led_g = 64 - (16 * (accel_high - 4)); */
      /*   led_b = 16 * (accel_high - 4); */
      /* } else { */
      /*   led_r = 0; */
      /*   led_g = 0; */
      /*   led_b = 64; */
      /* } */
      break;
    case 9:
      break;

    default:  // Can have no higher than case 15
      break;
  }

  accel_tick++;
  if (accel_tick >= accel_count_wrap) accel_tick = 0;
}


void resetMode() {
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
  /*
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
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        enterSleep();
        new_state = S_SLEEP_WAKE;
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        // enable config
        new_state = S_CONFIG_MENU_OFF;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) flash(0, 0, 128, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        // enable conjure
        new_state = S_CONFIG_MENU_OFF;
      }

    // Config menu
    case S_CONFIG_MENU_OFF:
      if (pressed && since_trans > PRESS_DELAY) new_state = S_CONFIG_MENU_PRESSED;
      break;

    case S_CONFIG_MENU_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_MENU_EDIT_WAIT;
      } else if (!pressed) {
        // increment config state
        new_state = S_CONFIG_MENU_OFF;
      }
      break;

    case S_CONFIG_MENU_EDIT_WAIT:
      if (since_trans == 0) flash(128, 128, 0, 5);
      if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_MENU_EXIT_WAIT;
      } else if (!pressed) {
        // enter different config modes
      }
      break;

    case S_CONFIG_MENU_EXIT_WAIT:
      if (since_trans == 0) flash(128, 128, 128, 5);
      if (!pressed) {
        new_state = S_PLAY_OFF;
      }
      break;

    // Color config
    // Pattern config
    // Accel mode config
    // Accel sensitivity config
    // Bundle select
    // Bundle slot edit

    // On Wake
    case S_SLEEP_WAKE:
      if (!pressed) {
        new_state = S_PLAY_OFF;
      }
      break;

    // Save states
    case S_SAVE_MODE:
      // save mode
      new_state = S_PLAY_OFF;
      break;

    case S_SAVE_BUNDLES:
      new_state = S_PLAY_OFF;
      break;


    // GUI states
    case S_GUI_MODE:
      break;

    case S_GUI_PALETTE:
      break;

    case S_GUI_BUNDLES:
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
  */
}


void enterSleep() {
  writeFrame(0, 0, 0);
  EEPROM.write(ADDR_SLEEPING, 1);
  accelStandby();
  digitalWrite(PIN_LDO, LOW);

  delay(400);
  wdt_enable(WDTO_15MS);
  delay(400);

  // Wait until button is releaed
  digitalWrite(PIN_LDO, HIGH);
  accelInit();
  delay(400);
}

void pushInterrupt() {
}
