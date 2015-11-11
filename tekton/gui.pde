class GuiUI {
  Group grp;
  Button btnEditPresets, btnEditPalette, btnEditBundles, btnEditPatterns, btnExit;
  Button btnReload, btnWrite, btnSave, btnLoad;
  Textfield tfPath;

  GuiUI(int x, int y) {
    grp = cp5.addGroup("guiUi")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();

    btnEditPresets = cp5.addButton("guiEditPresets")
      .setPosition(30, 10)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Edit Presets")
      .setGroup(grp);

    btnEditPalette = cp5.addButton("guiEditPalette")
      .setPosition(30, 40)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Edit Palette")
      .setGroup(grp);

    btnEditBundles = cp5.addButton("guiEditBundles")
      .setPosition(30, 70)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Edit Bundles")
      .setGroup(grp);

    /* btnEditPatterns = cp5.addButton("guiEditPatterns") */
    /*   .setPosition(30, 100) */
    /*   .setSize(60, 20) */
    /*   .setColorBackground(color(64)) */
    /*   .setCaptionLabel("Edit Patterns") */
    /*   .setGroup(grp); */

    btnReload = cp5.addButton("guiReload")
      .setPosition(30, 130)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Reload Light")
      .setGroup(grp);

    btnWrite = cp5.addButton("guiWrite")
      .setPosition(30, 160)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Write Light")
      .setGroup(grp);

    tfPath = cp5.addTextfield("guiPath")
      .setPosition(20, 190)
      .setSize(80, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("")
      .setText("primer.light")
      .setGroup(grp);

    btnSave = cp5.addButton("guiSave")
      .setPosition(30, 220)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Save")
      .setGroup(grp);

    btnLoad = cp5.addButton("guiLoad")
      .setPosition(30, 250)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Load")
      .setGroup(grp);


    btnExit = cp5.addButton("guiExit")
      .setPosition(30, 310)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Exit")
      .setGroup(grp);
  }

  void hide() {
    grp.hide();
  }

  void show() {
    grp.show();
  }

  void save() {
    String[] strs = new String[18];
    for (int i = 0; i < 16; i++) {
      strs[i] = preset_editor.compressMode(i);
    }
    strs[16] = bundle_editor.compress();
    strs[17] = palette_panel.compress();
    saveStrings(tfPath.getText(), strs);
  }

  void load() {
    String[] strs = loadStrings(tfPath.getText());
    palette_panel.decompress(strs[17]);
    palette_panel.refresh();
    for (int i = 0; i < 16; i++) {
      preset_editor.decompressMode(i, strs[i]);
    }
    preset_editor.refresh();
    bundle_editor.decompress(strs[16]);
    bundle_editor.refresh();
  }
}
