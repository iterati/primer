int NUM_PATTERNS = 62;
String[] pattern_names = {
  "RIBBON",
  "HYPER",
  "STROBE",
  "NANO",
  "DOPS",
  "SLOW_STROBE",
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
  "VEXING",
  "VEXING3",
  "RIBBONTRACER",
  "DOTTED",
  "FIREWORK",
  "BOTTLEROCKET",
  "GROW",
  "SHRINK",
  "STRETCH",
  "WAVE",
  "SHIFT",
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
  "SLOW_MORPH",
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
      .setPosition(330, 20)
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
      .setPosition(210, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Reload Preset")
      .setGroup(grp);

    btnWrite = cp5.addButton("presetWrite")
      .setPosition(310, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Write Preset")
      .setGroup(grp);

    btnPrev = cp5.addButton("presetPrev")
      .setPosition(250, 30)
      .setSize(50, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("<< PREV")
      .setGroup(grp);

    btnNext = cp5.addButton("presetNext")
      .setPosition(500, 30)
      .setSize(50, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("NEXT >>")
      .setGroup(grp);

    btnSave = cp5.addButton("presetSave")
      .setPosition(560, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Save")
      .setGroup(grp);

    btnLoad = cp5.addButton("presetLoad")
      .setPosition(600, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Load")
      .setGroup(grp);

    tfPath = cp5.addTextfield("presetPath")
      .setValue("")
      .setPosition(410, 190)
      .setSize(140, 20)
      .setColorBackground(color(0))
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
    for (int j = 0; j < NUM_PATTERNS; j++) {
      dd.addItem((j + 1) + ": " + pattern_names[j], j);
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
      return setPattern(target, 1, val);
    } else if (addr == 4) {
      return setNumColors(target, 0, val);
    } else if (addr == 5) {
      return setNumColors(target, 1, val);
    } else if (addr < 38) {
      int v = (addr - 6) / 16;
      int s = (addr - 6) % 16;
      return setColor(target, v, s, val);
    }
    return 0;
  }

  int setv(int target, int addr, int val) {
    val = update(target, addr, val);
    sendCmd('W', target, addr, val);
    return val;
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

  void deselect() {
    selected_v = selected_s = -1;
    btnSelected.hide();
  }

  void guiChangeColor(int val) {
    if (selected_s >= 0) {
      setv(cur_preset_idx, (selected_v * 16) + selected_s + 6, val);
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
    refresh();
    return val;
  }

  JSONArray asJson() {
    JSONArray modes = new JSONArray();
    for (int i = 0; i < 16; i++) {
      modes.setJSONObject(i, modeAsJson(i));
    }
    return modes;
  }

  JSONObject modeAsJson(int i) {
    JSONObject mode = new JSONObject();
    mode.setInt("accel_mode", presets[i].accMode);
    mode.setInt("accel_sensitivity", presets[i].accSens);
    JSONArray variants = new JSONArray();
    for (int j = 0; j < 2; j++) {
      JSONObject variant = new JSONObject();
      variant.setInt("pattern", presets[i].pattern[j]);
      variant.setInt("num_colors", presets[i].numColors[j]);
      JSONArray colors = new JSONArray();
      for (int k = 0; k < 16; k++) {
        colors.setInt(k, presets[i].colors[j][k]);
      }
      variant.setJSONArray("colors", colors);
      variants.setJSONObject(j, variant);
    }
    mode.setJSONArray("variants", variants);
    return mode;
  }

  void modeFromJson(JSONObject json, int i) {
    setv(i, 0, json.getInt("accel_mode"));
    setv(i, 1, json.getInt("accel_sensitivity"));
    JSONArray variants = json.getJSONArray("variants");
    for (int j = 0; j < 2; j++) {
      JSONObject variant = variants.getJSONObject(j);
      setv(i, 2 + j, variant.getInt("pattern"));
      setv(i, 4 + j, variant.getInt("num_colors"));
      JSONArray colors = variant.getJSONArray("colors");
      for (int k = 0; k < 16; k++) {
        setv(i, 6 + (j * 16) + k, colors.getInt(k));
      }
    }
  }

  void fromJson(JSONArray json) {
    for (int i = 0; i < max(json.size(), 16); i++) {
      modeFromJson(json.getJSONObject(i), i);
    }
  }

  void save() {
    saveJSONObject(modeAsJson(cur_preset_idx), tfPath.getText());
  }

  void load() {
    modeFromJson(loadJSONObject(tfPath.getText()), cur_preset_idx);
  }

  void refresh() {
    tlTitle.setText("Preset " + (cur_preset_idx + 1));
    tfPath.setText("primer" + (cur_preset_idx + 1) + ".mode");
    ddlAccMode.setCaptionLabel(accel_mode_names[presets[cur_preset_idx].accMode]);
    ddlAccSens.setCaptionLabel(accel_sens_names[presets[cur_preset_idx].accSens]);
    for (int v = 0; v < 2; v++) {
      ddlPattern[v].setCaptionLabel("Pattern " + (v + 1) + ": " + pattern_names[presets[cur_preset_idx].pattern[v]]);
      for (int s = 0; s < 16; s++) {
        if (s < presets[cur_preset_idx].numColors[v]) {
          btnColors[v][s].setColorBackground(getColor(presets[cur_preset_idx].colors[v][s]));
          btnColors[v][s].setCaptionLabel((presets[cur_preset_idx].colors[v][s] % 64) +
                                          "/" +
                                          ((presets[cur_preset_idx].colors[v][s] >> 6) + 1));
        } else {
          btnColors[v][s].setColorBackground(color(0));
          btnColors[v][s].setCaptionLabel("off").setColorBackground(0);
        }
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

  void hide() {
    grp.hide();
  }

  void show() {
    grp.show();
  }
}
