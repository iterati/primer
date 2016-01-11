int NUM_COLORS = 48;

class PalettePanel {
  Group grp, grpEdit;
  Textlabel tlTitle;
  Textfield tfPath;
  Button[][] btnPalette = new Button[48][4];
  Button btnSelected;
  Button btnReload, btnWrite, btnSave, btnLoad;
  Slider sldRed, sldGreen, sldBlue;
  int selected = 0;
  int selected_shade = 0;

  PalettePanel(int x, int y) {
    grpEdit = cp5.addGroup("paletteEditor")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();

    grp = cp5.addGroup("palettePanel")
      .setPosition(x, y + 240)
      .hideBar()
      .hideArrow();

    btnSelected = cp5.addButton("paletteSelected")
      .setPosition(0, 0)
      .setSize(40, 160)
      .setColorBackground(color(192))
      .setCaptionLabel("")
      .setGroup(grp)
      .hide();

    btnReload = cp5.addButton("paletteReload")
      .setPosition(110, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Reload Palette")
      .setGroup(grpEdit);

    btnWrite = cp5.addButton("paletteWrite")
      .setPosition(210, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Write Palette")
      .setGroup(grpEdit);

    btnSave = cp5.addButton("paletteSave")
      .setPosition(640, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Save")
      .setGroup(grpEdit);

    btnLoad = cp5.addButton("paletteLoad")
      .setPosition(680, 190)
      .setSize(30, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Load")
      .setGroup(grpEdit);

    tfPath = cp5.addTextfield("palettePath")
      .setValue("")
      .setPosition(490, 190)
      .setSize(140, 20)
      .setColorBackground(color(0))
      .setCaptionLabel("")
      .setText("primer.palette")
      .setGroup(grpEdit);

    for (int c = 0; c < 48; c++) {
      for (int s = 0; s < 4; s++) {
        int v = (s << 6) + c;
        int g = c / 8;
        btnPalette[c][s] = cp5.addButton("paletteColor" + v, v)
          .setPosition(((g % 2) * 340) + ((c % 8) * 40) + 114,
                       ((g / 2) * 180) + (s * 40) + 4)
          .setSize(32, 32)
          .setCaptionLabel(c + "/" + (s + 1))
          .setGroup(grp)
          .setColorBackground(getColor(v));
      }
    }

    tlTitle = cp5.addTextlabel("paletteTitle")
      .setText("Palette")
      .setPosition(330, 20)
      .setColorValue(0xff8888ff)
      .setFont(createFont("Arial", 32))
      .setGroup(grpEdit);

    sldRed = cp5.addSlider("paletteRedSlider")
       .setPosition(40, 70)
       .setSize(720, 30)
       .setRange(0, 255)
       .setValue(0)
       .setColorForeground(color(192, 0, 0))
       .setColorBackground(color(64, 0, 0))
       .setColorActive(color(255, 0, 0))
       .setCaptionLabel("")
       .setGroup(grpEdit);

    sldGreen = cp5.addSlider("paletteGreenSlider")
       .setPosition(40, 110)
       .setSize(720, 30)
       .setRange(0, 255)
       .setValue(0)
       .setColorForeground(color(0, 192, 0))
       .setColorBackground(color(0, 64, 0))
       .setColorActive(color(0, 255, 0))
       .setCaptionLabel("")
       .setGroup(grpEdit);

    sldBlue = cp5.addSlider("paletteBlueSlider")
       .setPosition(40, 150)
       .setSize(720, 30)
       .setRange(0, 255)
       .setValue(0)
       .setColorForeground(color(0, 0, 192))
       .setColorBackground(color(0, 0, 64))
       .setColorActive(color(0, 0, 255))
       .setCaptionLabel("")
       .setGroup(grpEdit);
  }

  void update(int addr, int val) {
    int v = addr / 3;
    int c = addr % 3;
    if (v >= 48) {
    } else {
      color_palette[v][c] = val;
      btnPalette[v][0].setColorBackground(getColor(v));
      btnPalette[v][1].setColorBackground(getColor(v + 64));
      btnPalette[v][2].setColorBackground(getColor(v + 128));
      btnPalette[v][3].setColorBackground(getColor(v + 192));
    }
  }

  void setv(int addr, int val) {
    update(addr, val);
    sendCmd('W', 17, addr, val);
  }

  void guiSetR() {
    if (selected >= 0) {
      setv((selected * 3), int(sldRed.getValue()));
    }
  }

  void guiSetG() {
    if (selected >= 0) {
      setv((selected * 3) + 1, int(sldGreen.getValue()));
    }
  }

  void guiSetB() {
    if (selected >= 0) {
      setv((selected * 3) + 2, int(sldBlue.getValue()));
    }
  }

  void select(int val) {
    selected = val % 64;
    selected_shade = val / 64;
    if (selected != 0) {
      float[] pos = btnPalette[selected][0].getPosition();
      btnSelected.setPosition(pos[0] - 4, pos[1] - 4);
      btnSelected.show();
      sldRed.setValue(color_palette[selected][0]);
      sldGreen.setValue(color_palette[selected][1]);
      sldBlue.setValue(color_palette[selected][2]);
      sendCmd('X', 20, selected, selected_shade);
    } else {
      btnSelected.hide();
    }
  }

  void deselect() {
    btnSelected.hide();
    selected = selected_shade = -1;
  }

  void show() {
    if (gui_state == 1) {
      grpEdit.show();
    } else {
      grpEdit.hide();
    }
    grp.show();
  }

  void hide() {
    grp.hide();
    grpEdit.hide();
  }

  void save() {
    saveJSONArray(asJson(), tfPath.getText());
  }

  void load() {
    fromJson(loadJSONArray(tfPath.getText()));
  }

  JSONArray asJson() {
    JSONArray colors = new JSONArray();
    for (int i = 0; i < NUM_COLORS; i++) {
      JSONArray c = new JSONArray();
      c.setInt(0, color_palette[i][0]);
      c.setInt(1, color_palette[i][1]);
      c.setInt(2, color_palette[i][2]);
      colors.setJSONArray(i, c);
    }
    return colors;
  }

  void fromJson(JSONArray json) {
    for (int i = 0; i < max(json.size(), NUM_COLORS); i++) {
      JSONArray c = json.getJSONArray(i);
      setv((i * 3) + 0, c.getInt(0));
      setv((i * 3) + 1, c.getInt(1));
      setv((i * 3) + 2, c.getInt(2));
    }
  }

  void refresh() {
    select(0);
    for (int c = 0; c < 32; c++) {
      for (int s = 0; s < 4; s++) {
        int v = (s << 6) + c;
        btnPalette[c][s].setColorBackground(getColor(v));
      }
    }
  }
}
