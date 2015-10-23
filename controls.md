FROM OFF
* Press - Turn on.
* Hold 1.5s - Goto BUNDLE SELECT. Flashes blue.


BUNDLE SELECT
* Press - Cycle bundle.
* Hold 1.5s - Selects current bundle. Goto PLAY. Flashes blue.
* Hold 3.0s - Goto BUNDLE EDIT. Flashes yellow.


BUNDLE EDIT
* Press - Cycle bundle slot to next mode.
* Hold 1.5s - Sets current bundle slot to selected mode. Cycles to next bundle slot. Flashes magenta.
* Hold 3.0s - Saves bundle with current bundle slot as the end of the bundle. Flashes white.


PLAY (Normal Mode)
* Press - Cycle to next mode.
* Hold 1.0s - Put light to sleep. Flashes white.
* Hold 2.5s - Enables Conjure Mode. Flashes blue.
* Hold 4.0s - Goto CONFIG SELECT. Flashes yellow.


PLAY (Conjure Mode)
* Press - Turn light on/off (processor still running!)
* Hold 1.0s - Turn off light and deactivate Conjure Mode. Flashes white.
* Hold 2.5s - Disable Conjure Mode. Flashes blue.
* Hold 4.0s - Goto CONFIG SELECT. Flashes yellow.


CONFIG SELECT
* Press - Cycle between configuration options. Color indicates what configuration mode will be selected.
  * Palette A - red
  * Palette B - blue
  * Prime A - magenta
  * Prime B - cyan
  * Accelerometer mode - green
  * Accelerometer sensitivity - yellow
* Hold 1.5s - Goto CONFIGURE for current configuration mode. Flashes yellow.
* Hold 3.0s - Goto PLAY. Flashes white.


CONFIG PALETTE
* Press - Cycle forward through palette options.
* Dpress - Cycle backward through palette options.
* Hold 1.5s - Select color. Flashes white.
* Hold 1.5s more - Cycle to next shade. Flashes white.
* Release after hold - Goto CONFIRM COLOR


CONFIRM COLOR
* Press - Accept color.
  * If last (12th) color slot, goto PLAY and save. Flashes white.
  * Otherwise just go to next color slot.
* Hold 1.5s - Reject color.
  * If first color slot, goto CONFIG PALETTE. Flashes red.
  * Otherwise, goto CONFIRM COLOR for previous color slot. Flashes red.
* Hold 3.0s - Accept and save. Sets current color slot as last color. Goto PLAY. Flashes white.


CONFIG PRIME
* Press - Cycles to next prime.
* Hold 1.5s - Accept and save. Goto PLAY. Flashes white.


CONFIG ACCELEROMETER MODE
* Press - Cycle to next accelerometer mode. Color indicates what mode will be selected.
  * Off - dim white
  * Speed - red
  * Tilt X - blue
  * Tilt Y - yellow
  * Flip Z - green
* Hold 1.5s - Accept and save. Goto PLAY. Flashes white.


CONFIG ACCELEROMETER SENSITIVITY
* Press - Cycle to next accelerometer sensitivity. Color indicates what sensitivity will be selected.
  * Low - blue
  * Medium - magenta
  * High - red
* Hold 1.5s - Accept and save. Goto PLAY. Flashes white.
