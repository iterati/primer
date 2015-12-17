String[] pattern_names = {
  "RIBBON",
  "HYPER",
  "STROBE",
  "NANO",
  "DOPS",
  "STROBIE",
  "FAINT",
  "SIGNAL",
  "BLASTER",
  "HEAVYBLASTER",
  "AUTOBLASTER",
  "STROBE2",
  "HYPER3",
  "DOPS3",
  "BLASTER3",
  "HEAVYBLASTER3",
  "AUTOBLASTER3",
  "TRACER",
  "DASHDOPS",
  "DOPSDASH",
  "STROBETRACER",
  "HYPERTRACER",
  "RIBBONTRACER",
  "DOTTED",
  "FIREWORK",
  "BOTTLEROCKET",
  "GROW",
  "SHRINK",
  "SPRING",
  "WAVE",
  "SHAPESHIFT",
  "COMET",
  "METEOR",
  "EMBERS",
  "INFLUX",
  "SWORD",
  "SWORD5",
  "RAZOR",
  "RAZOR5",
  "BARBS",
  "BARBS5",
  "CYCLOPS",
  "FADEIN",
  "STROBEIN",
  "FADEOUT",
  "STROBEOUT",
  "PULSE",
  "PULSAR",
  "MORPH",
  "DOPMORPH",
  "STROBIEMORPH",
  "STROBEMORPH",
  "HYPERMORPH",
  "DASHMORPH",
  "FUSE",
  "DOPFUSE",
  "STROBEFUSE",
  "STROBIEFUSE",
  "HYPERFUSE",
  "DASHFUSE",
};

String[] accel_mode_names = {"Off", "Speed", "Tilt X", "Tilt Y", "Flip Z"};
String[] accel_sens_names = {"Low", "Medium", "High"};


class Preset {
  int idx;
  int accMode, accSens;
  int[] pattern = new int[2];
  int[] numColors = new int[2];
  int[][] colors = new int[2][16];

  Preset(int i) {
    idx = i;
  }
}

class PresetEditor {
  Group grp;
  Textlabel tlTitle;
  DropdownList ddlAccMode, ddlAccSens;
  DropdownList[] ddlPattern = new DropdownList[2];
  Button[] btnLess = new Button[2];
  Button[] btnMore = new Button[2];
  Button btnSelected;
  Button[][] btnColors = new Button[2][16];
  Button btnPrev, btnNext;
  Button btnReload, btnWrite, btnSave, btnLoad;
  Textfield tfPath;
  Preset[] presets = new Preset[16];
  int selected_v, selected_s = -1;

  PresetEditor(int x, int y) {
    for (int i = 0; i < 16; i++) {
      presets[i] = new Preset(i);
    }

    grp = cp5.addGroup("presetEditor")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();

    tlTitle = cp5.addTextlabel("presetTitle")
      .setText("Preset 1")
      .setPosition(320, 20)
      .setColorValue(0xff8888ff)
      .setFont(createFont("Arial", 32))
      .setGroup(grp);

    btnSelected = cp5.addButton("presetSelected")
      .setPosition(0, 0)
      .setSize(40, 40)
      .setColorBackground(color(192))
      .setCaptionLabel("")
      .setGroup(grp)
      .hide();

    btnReload = cp5.addButton("presetReload")
      .setPosition(110, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Reload Preset")
      .setGroup(grp);

    btnWrite = cp5.addButton("presetWrite")
      .setPosition(210, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Write Preset")
      .setGroup(grp);

    btnPrev = cp5.addButton("presetPrev")
      .setPosition(350, 190)
      .setSize(40, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("<<")
      .setGroup(grp);

    btnNext = cp5.addButton("presetNext")
      .setPosition(410, 190)
      .setSize(40, 20)
      .setColorBackground(color(64))
      .setCaptionLabel(">>")
      .setGroup(grp);

    btnSave = cp5.addButton("presetSave")
      .setPosition(640, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Save")
      .setGroup(grp);

    btnLoad = cp5.addButton("presetLoad")
      .setPosition(680, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Load")
      .setGroup(grp);

    tfPath = cp5.addTextfield("presetPath")
      .setValue("")
      .setPosition(490, 190)
      .setSize(140, 20)
      .setColorBackground(color(64))
      .setText("primer1.mode")
      .setCaptionLabel("")
      .setGroup(grp);

    for (int v = 0; v < 2; v++) {
      for (int s = 0; s < 16; s++) {
        btnColors[v][s] = cp5.addButton("presetColor" + v + "_" + s)
          .setValue((v << 4) + s)
          .setPosition(84 + (s * 40), 100 + (v * 40))
          .setSize(32, 32)
          .setColorBackground(color(0))
          .setCaptionLabel("")
          .setGroup(grp);
      }

      btnLess[v] = cp5.addButton("presetLess" + (v + 1))
        .setPosition(34, 104 + (v * 40))
        .setSize(40, 20)
        .setColorBackground(color(64))
        .setCaptionLabel("Less")
        .setGroup(grp);

      btnMore[v] = cp5.addButton("presetMore" + (v + 1))
        .setPosition(726, 104 + (v * 40))
        .setSize(40, 20)
        .setColorBackground(color(64))
        .setCaptionLabel("More")
        .setGroup(grp);
    }

    ddlPattern[0] = buildPatternDropdown(0)
      .setPosition(70, 70)
      .setSize(150, 500)
      .setItemHeight(15)
      .setColorBackground(color(64))
      .setGroup(grp);

    ddlAccMode = buildAccModeDropdown()
      .setPosition(240, 70)
      .setSize(150, 500)
      .setItemHeight(15)
      .setColorBackground(color(64))
      .setGroup(grp);

    ddlAccSens = buildAccSensDropdown()
      .setPosition(410, 70)
      .setSize(150, 500)
      .setItemHeight(15)
      .setColorBackground(color(64))
      .setGroup(grp);

    ddlPattern[1] = buildPatternDropdown(1)
      .setPosition(580, 70)
      .setSize(150, 500)
      .setItemHeight(15)
      .setColorBackground(color(64))
      .setGroup(grp);
  }

  DropdownList buildAccSensDropdown() {
    DropdownList dd = cp5.addDropdownList("presetAccSens");
    dd.setBackgroundColor(color(200));
    dd.setColorActive(color(255, 128));
    dd.setItemHeight(20);
    dd.setBarHeight(15);
    for (int j = 0; j < 3; j++) {
      dd.addItem(accel_sens_names[j], j);
    }
    dd.close();
    return dd;
  }

  DropdownList buildAccModeDropdown() {
    DropdownList dd = cp5.addDropdownList("presetAccMode");
    dd.setBackgroundColor(color(200));
    dd.setColorActive(color(255, 128));
    dd.setItemHeight(20);
    dd.setBarHeight(15);
    for (int j = 0; j < 5; j++) {
      dd.addItem(accel_mode_names[j], j);
    }
    dd.close();
    return dd;
  }

  DropdownList buildPatternDropdown(int var) {
    DropdownList dd = cp5.addDropdownList("presetPattern" + (var + 1));
    dd.setBackgroundColor(color(200));
    dd.setColorActive(color(255, 128));
    dd.setItemHeight(20);
    dd.setBarHeight(15);
    for (int j = 0; j < 60; j++) {
      dd.addItem("Pattern " + var + ": " + pattern_names[j], j);
    }
    dd.close();
    return dd;
  }

  int update(int target, int addr, int val) {
    if (addr == 0) {
      return setAccMode(target, val);
    } else if (addr == 1) {
      return setAccSens(target, val);
    } else if (addr == 2) {
      return setPattern(target, 0, val);
    } else if (addr == 3) {
      return setNumColors(target, 0, val);
    } else if (addr < 20) {
      return setColor(target, 0, addr - 4, val);
    } else if (addr == 20) {
      return setPattern(target, 1, val);
    } else if (addr == 21) {
      return setNumColors(target, 1, val);
    } else if (addr < 38) {
      return setColor(target, 1, addr - 22, val);
    }
    return 0;
  }

  int setv(int target, int addr, int val) {
    val = update(target, addr, val);
    sendCmd('W', target, addr, val);
    return val;
  }

  void hide() {
    grp.hide();
  }

  void show() {
    grp.show();
  }

  void select(int i) {
    int v = i / 16;
    int s = i % 16;
    if (s < presets[cur_preset_idx].numColors[v]) {
      float[] pos = btnColors[v][s].getPosition();
      btnSelected.setPosition(pos[0] - 4, pos[1] - 4);
      btnSelected.show();
    }
    selected_v = v;
    selected_s = s;
  }

  void guiChangeColor(int val) {
    if (selected_s >= 0) {
      setv(cur_preset_idx, (selected_v * 18) + selected_s + 4, val);
    }
  }

  int setAccMode(int target, int val) {
    if (target == cur_preset_idx) {
      ddlAccMode.setCaptionLabel(accel_mode_names[val]);
    }
    presets[target].accMode = val;
    return val;
  }

  int setAccSens(int target, int val) {
    if (target == cur_preset_idx) {
      ddlAccSens.setCaptionLabel(accel_sens_names[val]);
    }
    presets[target].accSens = val;
    return val;
  }

  int setPattern(int target, int var, int val) {
    if (target == cur_preset_idx) {
      ddlPattern[var].setCaptionLabel("Pattern " + (var + 1) + ": " + pattern_names[val]);
    }
    presets[target].pattern[var] = val;
    return val;
  }

  int setColor(int target, int var, int slot, int val) {
    if (target == cur_preset_idx) {
      btnColors[var][slot].setColorBackground(getColor(val));
    }
    presets[target].colors[var][slot] = val;
    setNumColors(target, var, presets[target].numColors[var]);
    return val;
  }

  int setNumColors(int target, int var, int val) {
    if (val == 100) {
      val = presets[target].numColors[var] + 1;
    } else if (val == 99) {
      val = presets[target].numColors[var] - 1;
      if (selected_v == var && selected_s == val) {
        selected_s = -1;
        btnSelected.hide();
      }
    }

    if (val < 1 || val > 16) {
      return presets[target].numColors[var];
    }
    presets[target].numColors[var] = val;
    if (target == cur_preset_idx) {
      if (val == 1) {
        btnLess[var].hide();
        btnMore[var].show();
      } else if (val == 16) {
        btnLess[var].show();
        btnMore[var].hide();
      } else {
        btnLess[var].show();
        btnMore[var].show();
      }
      for (int i = 0; i < 16; i++) {
        if (i < val) {
          btnColors[var][i].setCaptionLabel("");
        } else {
          btnColors[var][i].setCaptionLabel("off").setColorBackground(0);
        }
      }
    }
    return val;
  }

  String compressMode(int target) {
    char[] rtn = new char[69];
    rtn[0] = encode((preset_editor.presets[target].accSens * 8) + preset_editor.presets[target].accMode);
    for (int v = 0; v < 2; v++) {
      rtn[(v * 34) + 1] = encode(preset_editor.presets[target].pattern[v]);
      rtn[(v * 34) + 2] = encode(preset_editor.presets[target].numColors[v]);
      for (int s = 0; s < 16; s++) {
        rtn[(v * 34) + (s * 2) + 3] = encode((preset_editor.presets[target].colors[v][s] >> 6));
        rtn[(v * 34) + (s * 2) + 4] = encode((preset_editor.presets[target].colors[v][s] % 64));
      }
    }

    return new String(rtn);
  }

  void save() {
    String[] strs = new String[1];
    strs[0] = compressMode(cur_preset_idx);
    saveStrings(tfPath.getText(), strs);
  }

  void decompressMode(int target, String s) {
    if (s.length() == 69) {
      setv(target, 0, decode(s.charAt(0)) % 8);
      setv(target, 1, decode(s.charAt(0)) / 8);
      for (int v = 0; v < 2; v++) {
        setv(target, (v * 18) + 2, decode(s.charAt((v * 34) + 1)));
        setv(target, (v * 18) + 3, decode(s.charAt((v * 34) + 2)));
        for (int c = 0; c < 16; c++) {
          setv(target, (v * 18) + c + 4,
              (decode(s.charAt((v * 34) + (2 * c) + 3)) << 6) +
              decode(s.charAt((v * 34) + (2 * c) + 4)));
        }
      }
    }
  }

  void load() {
    String[] strs = loadStrings(tfPath.getText());
    decompressMode(cur_preset_idx, strs[0]);
  }

  void refresh() {
    tlTitle.setText("Preset " + (cur_preset_idx + 1));
    tfPath.setText("primer" + (cur_preset_idx + 1) + ".mode");
    btnSelected.hide();
    ddlAccMode.setCaptionLabel(accel_mode_names[presets[cur_preset_idx].accMode]);
    ddlAccSens.setCaptionLabel(accel_sens_names[presets[cur_preset_idx].accSens]);
    for (int v = 0; v < 2; v++) {
      ddlPattern[v].setCaptionLabel("Pattern " + (v + 1) + ": " + pattern_names[presets[cur_preset_idx].pattern[v]]);
      for (int s = 0; s < 16; s++) {
        btnColors[v][s].setColorBackground(getColor(presets[cur_preset_idx].colors[v][s]));
      }
      if (presets[cur_preset_idx].numColors[v] == 1) {
        btnLess[v].hide();
        btnMore[v].show();
      } else if (presets[cur_preset_idx].numColors[v] == 16) {
        btnLess[v].show();
        btnMore[v].hide();
      } else {
        btnLess[v].show();
        btnMore[v].show();
      }
    }
  }
}
