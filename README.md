# Primer

Primer is a fully customizable firmware for the Open Source Microlight OSMxyz.


## Features

* 16 selectable strobe animation patterns
* 16 color slots per animation pattern
* 31 color + blank palette with 4 shading levels per color
* 16 fully customizable mode slots with 2 patterns per slot
* 4 accelerometer actions with 3 sensitivity levels for switching between patterns
* 4 bundles for customizing mode play order
* Tap-to-set BPM trigger for auto cycling of modes
* Conjuring mode (single-press on/off) configurable for every mode slot
* Easy to use customization interface


##  Installation

### Packaged release on Mac OS X (unavailable at the moment)

*  Download the latest .dmg from the releases tab.
*  Open the .dmg and double-click on "Upload .hex"

### Packaged release on Windows

* Download the latest .hex from the releases tab.
* Using XLoader, follow this tutorial on how to install.
  * Make sure to use 115200 for the speed.
  * Make sure to select "Uno(ATMega328)" from the Device dropdown.
  * Be sure to select the correct .hex file.


## Controls

**Off**
* Press - Turn on. Go to **Play**.
* Hold 1.0s - Go to **Bundle Select**. Flashes blue.
* Hold 4.0s - **Master Reset** (restore factory settings) and go to **Play**. Flashes red.

**Bundle Select**
* Press - Cycle bundle.
* Hold 0.5s - Selects current bundle. Go to **Play**. Flashes blue.
* Hold 1.5s - Go to **Bundle Edit**. Flashes yellow.
* Hold 2.5s - Go to **BPM Set**. Flahses green.

**Bundle Edit**
* Press - Cycle bundle slot to next mode.
* Hold 1.0s - Sets current bundle slot to selected mode. Cycles to next bundle slot. Flashes magenta.
* Hold 2.0s - Saves bundle with current bundle slot as the end of the bundle. Go to **Play**. Flashes white.

**BPM Set**
* Press 5x to set BPM timer. After the 5th press, the BPM is set and the light will go to **Play**.
* To deactivate BPM switching, select a bundle (can be the same bundle).

**Play (Normal Mode)**
* Press - Cycle to next mode.
* Hold 0.5s - Put light to sleep. Flashes white.
* Hold 1.5s - Go to **Config Select**. Flashes yellow.
* Hold 2.5s - Enables **Conjure Mode**. Flashes blue.
* Every second you hold after this will cycle through sleep, config, and conjure options.

**Play (Conjure Mode)**
* Press - Toggle light on/off (processor still running).
* Hold 0.5s - Turn off light and deactivate Conjure Mode. Flashes white.
* Hold 1.5s - Go to **Config Select**. Flashes yellow.
* Hold 2.5s - Disable **Conjure Mode**. Flashes blue.
* Every second you hold after this will cycle through sleep, config, and conjure options.

**Config Select**
* Press - Cycle between configuration options. Color indicates what configuration mode will be selected.
  * Colors A - red
  * Colors B - blue
  * Pattern A - magenta
  * Pattern B - cyan
  * Accelerometer mode - green
  * Accelerometer sensitivity - yellow
* Hold 1.0s - Go to **Configure** for current configuration mode. Flashes yellow.
* Hold 2.0s - Go to **Play**. Flashes white.

**Color Select**
* Press - Cycle forward through palette options.
* Hold 0.5s - Select color. Flashes white.
* Every second you hold after the shade will cycle. On release, go to **Confirm Color**.

**Confirm Color**
* Press - Accept color.
  * If last color slot, save and go to **Play**. Flashes white.
  * Otherwise, go to **Color Select** for the next color.
* Hold 1.0s - Reject color.
  * If first color slot, go to **Color Select** for the first color. Flashes red.
  * Otherwise, go to **Confirm Color** for previous color. Flashes red.
* Hold 2.0s - Accept, save, set current color slot as last color, and go to **Play**. Flashes white.

**Pattern Select**
* Press - Cycles to next prime.
* Hold 1.0s - Accept, save, and go to **Play**. Flashes white.

**Accelerometer Mode Select**
* Press - Cycle to next accelerometer mode. Color indicates what mode will be selected.
  * Off - dim white
  * Speed - red
  * Tilt X - blue
  * Tilt Y - yellow
  * Flip Z - green
* Hold 1.0s - Accept, save, and go to **Play**. Flashes white.

**Accelerometer Sensitivity Select**
* Press - Cycle to next accelerometer sensitivity. Color indicates what sensitivity will be selected.
  * Low - blue
  * Medium - magenta
  * High - red
* Hold 1.0s - Accept, save, and go to **Play**. Flashes white.


## Palette

**NOTE**: Colors displayed on the monitor differ from colors displayed on the light. These colors are approximations. Anything grey tends more to white than grey when shown on the LED.

![Palette](pngs/palette.png)

## Animations

These images represent the default mode set of animations. 1 pixel represents 0.5s of animation.

**NOTE**: Colors displayed on the monitor differ from colors displayed on the light. These colors are approximations. Anything grey tends more to white than grey when shown on the LED.

### Strobe
![Strobe](pngs/anim01.png)

### Hyper
![Hyper](pngs/anim02.png)

### Dops
![Hyper](pngs/anim03.png)

### Strobie
![Strobie](pngs/anim04.png)

### Pulse
![Pulse](pngs/anim05.png)

### Seizure
![Seizure](pngs/anim06.png)

### Tracer
![Tracer](pngs/anim07.png)

### Dash Dops
![Dash Dops](pngs/anim08.png)

### Blink-E
![Blink-E](pngs/anim09.png)

### Edge
![Edge](pngs/anim10.png)

### Lego
![Lego](pngs/anim11.png)

### Chase
![Chase](pngs/anim12.png)

### Morph
![Morph](pngs/anim13.png)

### Ribbon
![Ribbon](pngs/anim14.png)

### Comet
![Comet](pngs/anim15.png)

### Candy
![Candy](pngs/anim16.png)


## Default Modes

**NOTE**: Colors displayed on the monitor differ from colors displayed on the light. These colors are approximations. Anything grey tends more to white than grey when shown on the LED.

### Mode 1
![Mode 1](pngs/mode01.png)

### Mode 2
![Mode 2](pngs/mode02.png)

### Mode 3
![Mode 3](pngs/mode03.png)

### Mode 4
![Mode 4](pngs/mode04.png)

### Mode 5
![Mode 5](pngs/mode05.png)

### Mode 6
![Mode 6](pngs/mode06.png)

### Mode 7
![Mode 7](pngs/mode07.png)

### Mode 8
![Mode 8](pngs/mode08.png)

### Mode 9
![Mode 9](pngs/mode09.png)

### Mode 10
![Mode 10](pngs/mode10.png)

### Mode 11
![Mode 11](pngs/mode11.png)

### Mode 12
![Mode 12](pngs/mode12.png)

### Mode 13
![Mode 13](pngs/mode13.png)

### Mode 14
![Mode 14](pngs/mode14.png)

### Mode 15
![Mode 15](pngs/mode15.png)

### Mode 16
![Mode 16](pngs/mode16.png)
