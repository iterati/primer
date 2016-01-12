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
import java.awt.event.KeyEvent;
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

int color_palette[][] = new int[48][3];
int user_patterns[][] = new int[16][8];
int cur_preset_idx = 0;
int dump_mode;


Textlabel tlLoading;
GuiUI guiui;
PresetEditor preset_editor;
PalettePanel palette_panel;
BundleEditor bundle_editor;

Boolean shifted = false;


void setup() {
  surface.setTitle("Tekton for Primer 0.91");
  size(1000, 800);
  cp5 = new ControlP5(this);

  guiui = new GuiUI(0, 240);
  palette_panel = new PalettePanel(60, 0);
  preset_editor = new PresetEditor(100, 0);
  bundle_editor = new BundleEditor(100, 0);

  tlLoading = cp5.addTextlabel("loading")
    .setText("Connecting")
    .setPosition(430, 400)
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
  } else {
    tlLoading.hide();
    guiui.show();
    if (gui_state == 0) {
      preset_editor.show();
      palette_panel.show();
      bundle_editor.hide();
    } else if (gui_state == 1) {
      preset_editor.hide();
      palette_panel.show();
      bundle_editor.hide();
    } else if (gui_state == 2) {
      preset_editor.hide();
      palette_panel.hide();
      bundle_editor.show();
    } else if (gui_state == 3) {
      preset_editor.hide();
      palette_panel.hide();
      bundle_editor.hide();
    }
  }
}

void sendCmd(char cmd, int target, int addr, int val) {
  port.write(cmd);
  port.write(target);
  port.write(addr);
  port.write(val);
  /* println("out<< " + cmd + " " + target + " " + addr + " " + val); */
}

void readResp() {
  int target = port.read();
  int addr = port.read();
  int val = port.read();

  /* println("in << " + target + " " + addr + " " + val); */

  if (target == 100) { // Ack from light
    cur_preset_idx = addr;
    tlLoading.setText("Loading").setPosition(440, 400);
    initialized = true;
    dump_mode = 1;
    sendCmd('X', 10, 0, 0); // Change to gui play mode
    sendCmd('D', 99, 0, 0);
  } else if (target == 200) {
    /* cur_preset_idx = val; */
    if (addr == cur_preset_idx || addr == 17) {
      reading = true;
    }
  } else if (target == 201) {
    reading = false;
    preset_editor.refresh();
    bundle_editor.refresh();
    palette_panel.refresh();
    if (dump_mode < 16) {
      sendCmd('D', dump_mode, 0, 0);
      dump_mode += 1;
    }
  } else {
    readData(target, addr, val);
  }
}

void changeMode(int mode) {
  gui_state = mode;
  preset_editor.deselect();
  palette_panel.deselect();
  palette_panel.sldRed.setValue(0);
  palette_panel.sldGreen.setValue(0);
  palette_panel.sldBlue.setValue(0);
  bundle_editor.deselect();
}

void readData(int target, int addr, int val) {
  if (target < 16) {
    preset_editor.update(target, addr, val);
  } else if (target == 16) {
    bundle_editor.update(addr, val);
  } else if (target == 17) {
    palette_panel.update(addr, val);
  }
}

int getColor(int v) {
  int s = v >> 6;
  int c = v % 64;
  int alpha = 127 + (128 >> s);
  if (c != 0) {
    float r = (((color_palette[c][0] >> s) / 255.0) * 192) + 63;
    float g = (((color_palette[c][1] >> s) / 255.0) * 192) + 63;
    float b = (((color_palette[c][2] >> s) / 255.0) * 192) + 63;
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
      preset_editor.setv(cur_preset_idx, 0, val);
    } else if (evt.startsWith("presetAccSens")) {
      preset_editor.setv(cur_preset_idx, 1, val);
    } else if (evt.startsWith("presetPattern1")) {
      preset_editor.setv(cur_preset_idx, 2, val);
    } else if (evt.startsWith("presetPattern2")) {
      preset_editor.setv(cur_preset_idx, 3, val);
    } else if (evt.startsWith("presetColor")) {
      preset_editor.select(val);
    } else if (evt.startsWith("presetLess1")) {
      preset_editor.setv(cur_preset_idx, 4, 99);
    } else if (evt.startsWith("presetLess2")) {
      preset_editor.setv(cur_preset_idx, 5, 99);
    } else if (evt.startsWith("presetMore1")) {
      int v = preset_editor.setv(cur_preset_idx, 4, 100);
      /* sendCmd('R', cur_preset_idx, 3 + v, 0); */
    } else if (evt.startsWith("presetMore2")) {
      int v = preset_editor.setv(cur_preset_idx, 5, 100);
      /* sendCmd('R', cur_preset_idx, 21 + v, 0); */
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
      sendCmd('D', 17, 0, 0);
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
      bundle_editor.setv(20 * val, 99);
    } else if (evt.startsWith("bundleMore")) {
      bundle_editor.setv(20 * val, 100);
    } else if (evt.startsWith("bundleWrite")) {
      sendCmd('S', 16, 0, 0);
    } else if (evt.startsWith("bundleSave")) {
      bundle_editor.save();
    } else if (evt.startsWith("bundleLoad")) {
      bundle_editor.load();
      sendCmd('D', 16, 0, 0);
    } else if (evt.startsWith("guiEditPresets")) {
      changeMode(0);
      sendCmd('X', 2, cur_preset_idx, 0);
      sendCmd('X', 10, 0, 0);
    } else if (evt.startsWith("guiEditPalette")) {
      changeMode(1);
      sendCmd('X', 10, 1, 0);
    } else if (evt.startsWith("guiEditBundles")) {
      changeMode(2);
      sendCmd('D', 16, 0, 0);
      sendCmd('X', 10, 2, 0);
    } else if (evt.startsWith("guiReload")) {
      sendCmd('L', 99, 0, 0);
      dump_mode = 1;
      sendCmd('D', 17, 0, 0);
      sendCmd('D', 16, 0, 0);
      sendCmd('D', 0, 0, 0);
    } else if (evt.startsWith("guiWrite")) {
      sendCmd('S', 99, 0, 0);
    } else if (evt.startsWith("guiSave")) {
      guiui.save();
    } else if (evt.startsWith("guiLoad")) {
      guiui.load();
    } else if (evt.startsWith("guiFlash")) {
      guiui.load();
      sendCmd('S', 99, 0, 0);
    } else if (evt.startsWith("guiDisconnect")) {
      sendCmd('X', 10, 99, 0);
      initialized = false;
      tlLoading.setText("Connecting").setPosition(430, 400);
      reading = true;
    } else if (evt.startsWith("guiExit")) {
      sendCmd('X', 10, 99, 0);
      exit();
    }
  } else if (theEvent.isGroup()) {
  } else if (theEvent.isController()) {
  }
}

void keyPressed() {
  if (keyCode == KeyEvent.VK_SHIFT) {
    shifted = true;
  }

  if (initialized) {
    if (gui_state == 0) {
      if (keyCode == KeyEvent.VK_UP) {
        int v = (shifted) ? 1 : 0;
        int p = (preset_editor.presets[cur_preset_idx].pattern[v] + NUM_PATTERNS - 1) % NUM_PATTERNS;
        preset_editor.setv(cur_preset_idx, 2 + (v * 18), p);
      } else if (keyCode == DOWN) {
        int v = (shifted) ? 1 : 0;
        int p = (preset_editor.presets[cur_preset_idx].pattern[v] + 1) % NUM_PATTERNS;
        preset_editor.setv(cur_preset_idx, 2 + (v * 18), p);
      }
    } else if (gui_state == 1) {
      if (key == 'q') {
        palette_panel.sldRed.setValue(float(int(palette_panel.sldRed.getValue() - 8)));
      } else if (key == 'w') {
        palette_panel.sldRed.setValue(float(int(palette_panel.sldRed.getValue() - 1)));
      } else if (key == 'e') {
        palette_panel.sldRed.setValue(float(int(palette_panel.sldRed.getValue() + 1)));
      } else if (key == 'r') {
        palette_panel.sldRed.setValue(float(int(palette_panel.sldRed.getValue() + 8)));

      } else if (key == 'a') {
        palette_panel.sldGreen.setValue(float(int(palette_panel.sldGreen.getValue() - 8)));
      } else if (key == 's') {
        palette_panel.sldGreen.setValue(float(int(palette_panel.sldGreen.getValue() - 1)));
      } else if (key == 'd') {
        palette_panel.sldGreen.setValue(float(int(palette_panel.sldGreen.getValue() + 1)));
      } else if (key == 'f') {
        palette_panel.sldGreen.setValue(float(int(palette_panel.sldGreen.getValue() + 8)));

      } else if (key == 'z') {
        palette_panel.sldBlue.setValue(float(int(palette_panel.sldBlue.getValue() - 8)));
      } else if (key == 'x') {
        palette_panel.sldBlue.setValue(float(int(palette_panel.sldBlue.getValue() - 1)));
      } else if (key == 'c') {
        palette_panel.sldBlue.setValue(float(int(palette_panel.sldBlue.getValue() + 1)));
      } else if (key == 'v') {
        palette_panel.sldBlue.setValue(float(int(palette_panel.sldBlue.getValue() + 8)));
      }
    }
  }
}

void keyReleased() {
  if (keyCode == KeyEvent.VK_SHIFT) {
    shifted = false;
  }
}
