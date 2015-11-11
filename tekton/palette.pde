class PalettePanel {
  Group grp, grpEdit;
  Textlabel tlTitle;
  Textfield tfPath;
  Button[][] btnPalette = new Button[32][4];
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
      .setColorBackground(color(64))
      .setCaptionLabel("")
      .setText("primer.palette")
      .setGroup(grpEdit);

    for (int c = 0; c < 32; c++) {
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
    int v = addr / 4;
    int c = addr % 4;
    color_palette[v][c] = val;

    btnPalette[v][0].setColorBackground(getColor(v));
    btnPalette[v][1].setColorBackground(getColor(v + 64));
    btnPalette[v][2].setColorBackground(getColor(v + 128));
    btnPalette[v][3].setColorBackground(getColor(v + 192));
  }

  void set(int addr, int val) {
    update(addr, val);
    sendCmd('W', 17, addr, val);
  }

  void guiSetR() {
    set((selected * 4), int(sldRed.getValue()));
  }

  void guiSetG() {
    set((selected * 4) + 1, int(sldGreen.getValue()));
  }

  void guiSetB() {
    set((selected * 4) + 2, int(sldBlue.getValue()));
  }

  void select(int val) {
    selected = val % 32;
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

  String compress() {
    char[] rtn = new char[192];
    int i;
    for (int c = 0; c < 32; c++) {
      i = c * 6;
      rtn[i + 0] = encode(color_palette[c][0] >> 6);
      rtn[i + 1] = encode(color_palette[c][0] % 64);
      rtn[i + 2] = encode(color_palette[c][1] >> 6);
      rtn[i + 3] = encode(color_palette[c][1] % 64);
      rtn[i + 4] = encode(color_palette[c][2] >> 6);
      rtn[i + 5] = encode(color_palette[c][2] % 64);
    }
    return new String(rtn);
  }

  void save() {
    String[] strs = new String[1];
    strs[0] = compress();
    saveStrings(tfPath.getText(), strs);
  }

  void decompress(String s) {
    if (s.length() == 192) {
      for (int c = 0; c < 32; c++) {
        palette_panel.set((c * 4) + 0, (decode(s.charAt((c * 6) + 0)) << 6) + decode(s.charAt((c * 6) + 1)));
        palette_panel.set((c * 4) + 1, (decode(s.charAt((c * 6) + 2)) << 6) + decode(s.charAt((c * 6) + 3)));
        palette_panel.set((c * 4) + 2, (decode(s.charAt((c * 6) + 4)) << 6) + decode(s.charAt((c * 6) + 5)));
      }
    }
  }

  void load() {
    String[] strs = loadStrings(tfPath.getText());
    decompress(strs[0]);
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
