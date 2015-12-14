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

import processing.serial.*;
import controlP5.*;

Serial port;
ControlP5 cp5;

// 0, mode
// 1, bundle
// 2, palette
// 3, patterns
int gui_state = 0;
Boolean gui_initialized = false;
Boolean initialized = false;
Boolean reading = true;

int color_palette[][] = new int[32][3];
int user_patterns[][] = new int[16][8];
int cur_preset_idx = 0;

Textlabel tlLoading;
GuiUI guiui;
PresetEditor preset_editor;
PalettePanel palette_panel;
BundleEditor bundle_editor;
/* PatternEditor pattern_editor; */


void setup() {
  size(800, 600);
  cp5 = new ControlP5(this);

  guiui = new GuiUI(0, 240);
  palette_panel = new PalettePanel(0, 0);
  preset_editor = new PresetEditor(0, 0);
  bundle_editor = new BundleEditor(0, 0);
  /* pattern_editor = new PatternEditor(0, 0); */

  tlLoading = cp5.addTextlabel("loading")
    .setText("Connecting")
    .setPosition(330, 300)
    .setColorValue(0xff8888ff)
    .setFont(createFont("Arial", 32))
    .hide();

  gui_initialized = true;
}

void connectLight() {
  for (String p: Serial.list()) {
    try {
      port = new Serial(this, p, 57600);
    } catch (Exception e) {
    }
  }
}

void draw() {
  background(8);
  if (!initialized) {
    connectLight();
  }

  while (port.available() >= 3) {
    readResp();
  }

  if (!initialized || reading) {
    tlLoading.show();
    guiui.hide();
    preset_editor.hide();
    palette_panel.hide();
    bundle_editor.hide();
    /* pattern_editor.hide(); */
  } else {
    tlLoading.hide();
    guiui.show();
    if (gui_state == 0) {
      preset_editor.show();
      palette_panel.show();
      bundle_editor.hide();
      /* pattern_editor.hide(); */
    } else if (gui_state == 1) {
      preset_editor.hide();
      palette_panel.show();
      bundle_editor.hide();
      /* pattern_editor.hide(); */
    } else if (gui_state == 2) {
      preset_editor.hide();
      palette_panel.hide();
      bundle_editor.show();
      /* pattern_editor.hide(); */
    } else if (gui_state == 3) {
      preset_editor.hide();
      palette_panel.hide();
      bundle_editor.hide();
      /* pattern_editor.show(); */
    }
  }
}

void sendCmd(char cmd, int target, int addr, int val) {
  port.write(cmd);
  port.write(target);
  port.write(addr);
  port.write(val);
}

void readResp() {
  int target = port.read();
  int addr = port.read();
  int val = port.read();

  if (target == 100) { // Ack from light
    cur_preset_idx = addr;
    tlLoading.setText("Loading").setPosition(340, 300);
    initialized = true;
    sendCmd('X', 10, 0, 0); // Change to gui play mode
    sendCmd('D', 99, 0, 0); // Get dump of everything
  } else if (target == 250) { // Mode changed
    /* cur_preset_idx = addr; */
    println(cur_preset_idx + ": 250 " + addr + " " + val);
  } else if (target == 200) {
    cur_preset_idx = val;
    reading = true;
    println(cur_preset_idx + ": 200 " + addr + " " + val);
  } else if (target == 201) {
    preset_editor.refresh();
    bundle_editor.refresh();
    palette_panel.refresh();
    reading = false;
    println(cur_preset_idx + ": 201 " + addr + " " + val);
  } else {
    readData(target, addr, val);
  }
}

void readData(int target, int addr, int val) {
  if (target < 16) {
    preset_editor.update(target, addr, val);
  } else if (target == 16) {
    bundle_editor.update(addr, val);
  } else if (target == 17) {
    palette_panel.update(addr, val);
    color_palette[addr / 4][addr % 4] = val;
  } else if (target == 18) {
    /* user_patterns[addr / 8][addr % 8] = val; */
  }
}

int getColor(int v) {
  int s = v >> 6;
  int c = v % 32;
  int alpha = 127 + (128 >> s);
  if (c != 0) {
    float r = ((color_palette[c][0] / 255.0) * 192) + 63;
    float g = ((color_palette[c][1] / 255.0) * 192) + 63;
    float b = ((color_palette[c][2] / 255.0) * 192) + 63;
    return (alpha << 24) + (int(r) << 16) + (int(g) << 8) + int(b);
  } else {
    return 0;
  }
}

void controlEvent(ControlEvent theEvent) {
  int val = int(theEvent.getValue());
  String evt = theEvent.getName();

  if (gui_initialized) {
    if (evt.startsWith("presetAccMode")) {
      preset_editor.set(cur_preset_idx, 0, val);
    } else if (evt.startsWith("presetAccSens")) {
      preset_editor.set(cur_preset_idx, 1, val);
    } else if (evt.startsWith("presetPattern1")) {
      preset_editor.set(cur_preset_idx, 2, val);
    } else if (evt.startsWith("presetPattern2")) {
      preset_editor.set(cur_preset_idx, 20, val);
    } else if (evt.startsWith("presetColor")) {
      preset_editor.select(val);
    } else if (evt.startsWith("presetLess1")) {
      preset_editor.set(cur_preset_idx, 3, 99);
    } else if (evt.startsWith("presetLess2")) {
      preset_editor.set(cur_preset_idx, 21, 99);
    } else if (evt.startsWith("presetMore1")) {
      int v = preset_editor.set(cur_preset_idx, 3, 100);
      sendCmd('R', cur_preset_idx, 3 + v, 0);
    } else if (evt.startsWith("presetMore2")) {
      int v = preset_editor.set(cur_preset_idx, 21, 100);
      sendCmd('R', cur_preset_idx, 21 + v, 0);
    } else if (evt.startsWith("presetReload")) {
      sendCmd('L', cur_preset_idx, 0, 0);
    } else if (evt.startsWith("presetWrite")) {
      sendCmd('S', cur_preset_idx, 0, 0);
    } else if (evt.startsWith("presetSave")) {
      preset_editor.save();
    } else if (evt.startsWith("presetLoad")) {
      preset_editor.load();
    } else if (evt.startsWith("presetPrev")) {
      cur_preset_idx = (cur_preset_idx + 15) % 16;
      sendCmd('X', 0, 0, 0);
      sendCmd('D', cur_preset_idx, 0, 0);
    } else if (evt.startsWith("presetNext")) {
      cur_preset_idx = (cur_preset_idx + 1) % 16;
      sendCmd('X', 1, 0, 0);
      sendCmd('D', cur_preset_idx, 0, 0);
    } else if (evt.startsWith("paletteColor")) {
      if (gui_state == 0) {
        preset_editor.guiChangeColor(val);
      } else if (gui_state == 1) {
        palette_panel.select(val);
      } else if (gui_state == 2) {
      }
    } else if (evt.startsWith("paletteRedSlider")) {
      palette_panel.guiSetR();
    } else if (evt.startsWith("paletteGreenSlider")) {
      palette_panel.guiSetG();
    } else if (evt.startsWith("paletteBlueSlider")) {
      palette_panel.guiSetB();
    } else if (evt.startsWith("paletteReload")) {
      sendCmd('L', 17, 0, 0);
      sendCmd('D', 17, 0, 0);
    } else if (evt.startsWith("paletteWrite")) {
      sendCmd('S', 17, 0, 0);
    } else if (evt.startsWith("paletteSave")) {
      palette_panel.save();
    } else if (evt.startsWith("paletteLoad")) {
      palette_panel.load();
    } else if (evt.startsWith("bundleSlot")) {
      bundle_editor.select(val);
    } else if (evt.startsWith("bundlePreset")) {
      bundle_editor.guiChangeSlot(val);
      sendCmd('X', 2, val, 0);
    } else if (evt.startsWith("bundleReload")) {
      bundle_editor.btnSelected.hide();
      bundle_editor.selected_s = -1;
      sendCmd('L', 16, 0, 0);
      sendCmd('D', 16, 0, 0);
    } else if (evt.startsWith("bundleLess")) {
      bundle_editor.set(20 * val, 99);
    } else if (evt.startsWith("bundleMore")) {
      bundle_editor.set(20 * val, 100);
    } else if (evt.startsWith("bundleWrite")) {
      sendCmd('S', 16, 0, 0);
    } else if (evt.startsWith("bundleSave")) {
      bundle_editor.save();
    } else if (evt.startsWith("bundleLoad")) {
      bundle_editor.load();
    } else if (evt.startsWith("guiEditPresets")) {
      gui_state = 0;
      sendCmd('X', 2, cur_preset_idx, 0);
      sendCmd('X', 10, 0, 0);
    } else if (evt.startsWith("guiEditPalette")) {
      gui_state = 1;
      sendCmd('X', 10, 1, 0);
    } else if (evt.startsWith("guiEditBundles")) {
      gui_state = 2;
      sendCmd('X', 10, 2, 0);
    } else if (evt.startsWith("guiEditPatterns")) {
      /* gui_state = 3; */
      /* sendCmd('X', 10, 3, 0); */
    } else if (evt.startsWith("guiReload")) {
      sendCmd('L', 99, 0, 0);
      sendCmd('D', 99, 0, 0);
    } else if (evt.startsWith("guiWrite")) {
      sendCmd('S', 99, 0, 0);
    } else if (evt.startsWith("guiSave")) {
      guiui.save();
    } else if (evt.startsWith("guiLoad")) {
      guiui.load();
    } else if (evt.startsWith("guiExit")) {
      sendCmd('X', 10, 99, 0);
      exit();
    }
  } else if (theEvent.isGroup()) {
  } else if (theEvent.isController()) {
  }
}

private static final String codes = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+=";

char encode(int i) {
  return codes.charAt(i);
}

int decode(char c) {
  return codes.indexOf(c);
}