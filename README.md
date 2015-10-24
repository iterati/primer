# Primer

Primer is the first OSM firmware fully customizable on-chip.


## Features

* **12** Fully Customizable
* **2** Variants Per Mode
* **4** Accelerometer Triggers to Switch Variants
* Each With **3** Sensitivity Levels
* **16** LED Animations For Your Variations
* **Up to 12** Colors Per Variation
* **62** Color + Blank Palette with **4** Shading Levels.
* **4** Customizable Bundles For Custom Mode Playlists
* Enable Conjuring Mode (Toggle On/Off With 1 Press) From Any Mode
* Easy To Use Customization Interface


## Controls

**Off**
* Press - Turn on. Go to **Play**.
* Hold 1.5s - Go to **Bundle Select**. Flashes blue.

**Bundle Select**
* Press - Cycle bundle.
* Hold 1.5s - Selects current bundle. Go to **Play**. Flashes blue.
* Hold 3.0s - Go to **Bundle Edit**. Flashes yellow.

**Bundle Edit**
* Press - Cycle bundle slot to next mode.
* Hold 1.5s - Sets current bundle slot to selected mode. Cycles to next bundle slot. Flashes magenta.
* Hold 3.0s - Saves bundle with current bundle slot as the end of the bundle. Go to **Play**. Flashes white.

**Play (Normal Mode)**
* Press - Cycle to next mode.
* Hold 1.0s - Put light to sleep. Flashes white.
* Hold 2.5s - Enables **Conjure Mode**. Flashes blue.
* Hold 4.0s - Go to **Config Select**. Flashes yellow.

**Play (Conjure Mode)**
* Press - Toggle light on/off (processor still running).
* Hold 1.0s - Turn off light and deactivate Conjure Mode. Flashes white.
* Hold 2.5s - Disable **Conjure Mode**. Flashes blue.
* Hold 4.0s - Go to **Config Select**. Flashes yellow.

**Config Select**
* Press - Cycle between configuration options. Color indicates what configuration mode will be selected.
  * Palette A - red
  * Palette B - blue
  * Prime A - magenta
  * Prime B - cyan
  * Accelerometer mode - green
  * Accelerometer sensitivity - yellow
* Hold 1.5s - Go to **Configure** for current configuration mode. Flashes yellow.
* Hold 3.0s - Go to **Play**. Flashes white.

**Config Palette**
* Press - Cycle forward through palette options.
* Dpress - Cycle backward through palette options.
* Hold 1.5s - Select color. Flashes white.
* Hold 1.5s more - Cycle to next shade. Flashes white.
* Release after hold - Go to **Confirm Color**.

**Confirm Color**
* Press - Accept color.
  * If last (12th) color slot, go to **Play** and save. Flashes white.
  * Otherwise just go to next color slot.
* Hold 1.5s - Reject color.
  * If first color slot, go to **Config Palette**. Flashes red.
  * Otherwise, go to **Confirm Color** for previous color slot. Flashes red.
* Hold 3.0s - Accept and save. Sets current color slot as last color. Go to **Play**. Flashes white.

**Config Prime**
* Press - Cycles to next prime.
* Hold 1.5s - Accept and save. Go to **Play**. Flashes white.

**Config Accelerometer Mode**
* Press - Cycle to next accelerometer mode. Color indicates what mode will be selected.
  * Off - dim white
  * Speed - red
  * Tilt X - blue
  * Tilt Y - yellow
  * Flip Z - green
* Hold 1.5s - Accept and save. Go to **Play**. Flashes white.

**Config Accelerometer Sensitivity**
* Press - Cycle to next accelerometer sensitivity. Color indicates what sensitivity will be selected.
  * Low - blue
  * Medium - magenta
  * High - red
* Hold 1.5s - Accept and save. Go to **Play**. Flashes white.


## Animations

* Strobe - 5ms/8ms Strobe
* Hyper - 17ms/17ms Strobe
* Dops - 1ms/10ms Strobe
* Strobie - 3ms/23ms Strobe
* Pulse - 50ms Fade In and Out/25ms Off
* Seizure - 5ms Fade In/95ms Off
* Tracer - 3ms Color/23ms Color 1
* Dash Dops
  * 7ms Per Color for Color 2+
  * 7 1ms/10ms Strobes of Color 1
* Blink-E - 5ms Per Color/50ms Off
* Edge 
  * 2ms Per Color for Last Color to Color 2
  * 5ms Color 1
  * 2ms Per Color for Color 2 to Last Color
* Lego - 2, 8, or 16ms/8ms Random Strobe
* Chase 
  * 50ms Color A/10ms Off
  * 10ms Color B/10ms Off/30ms Color A/10ms Off
  * 20ms Color B/10ms Off/20ms Color A/10ms Off
  * 30ms Color B/10ms Off/10ms Color A/10ms Off
  * Repeat with B and next color...
* Morph - 17ms/17ms Strobe Where A Morphs to B Over 4 Strobes
* Ribbon - 11ms Per Color
* Ravin - 5ms/8ms ABCBA Strobe (Color 1 to Last to 1 to Last...)
* Candy - 5ms/8ms Strobe of First 3 Colors For 3 Cycles, Then Drop First Color And Add Next
