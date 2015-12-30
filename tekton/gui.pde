class GuiUI {
  Group grp;
  Button btnEditPresets, btnEditPalette, btnEditBundles, btnExit;
  Button btnReload, btnWrite, btnSave, btnLoad, btnFlash, btnDisconnect;
  Textfield tfPath;

  GuiUI(int x, int y) {
    grp = cp5.addGroup("guiUi")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();

    btnEditPresets = cp5.addButton("guiEditPresets")
      .setPosition(30, 10)
      .setSize(60, 20)
      .setColorLabel(color(0))
      .setColorBackground(color(240))
      .setCaptionLabel("Edit Presets")
      .setGroup(grp);

    btnEditPalette = cp5.addButton("guiEditPalette")
      .setPosition(30, 40)
      .setSize(60, 20)
      .setColorLabel(color(0))
      .setColorBackground(color(240))
      .setCaptionLabel("Edit Palette")
      .setGroup(grp);

    btnEditBundles = cp5.addButton("guiEditBundles")
      .setPosition(30, 70)
      .setSize(60, 20)
      .setColorLabel(color(0))
      .setColorBackground(color(240))
      .setCaptionLabel("Edit Bundles")
      .setGroup(grp);

    btnReload = cp5.addButton("guiReload")
      .setPosition(30, 130)
      .setSize(60, 20)
      .setColorBackground((255 << 24) + (128 << 16) + (64 << 8) + (64))
      .setCaptionLabel("Reload Light")
      .setGroup(grp);

    btnWrite = cp5.addButton("guiWrite")
      .setPosition(30, 160)
      .setSize(60, 20)
      .setColorBackground((255 << 24) + (128 << 16) + (64 << 8) + (64))
      .setCaptionLabel("Write Light")
      .setGroup(grp);

    btnFlash = cp5.addButton("guiFlash")
      .setPosition(30, 190)
      .setSize(60, 20)
      .setColorBackground((255 << 24) + (128 << 16) + (64 << 8) + (64))
      .setCaptionLabel("Upload Light")
      .setGroup(grp);

    tfPath = cp5.addTextfield("guiPath")
      .setPosition(20, 220)
      .setSize(80, 20)
      .setColorBackground(color(0))
      .setCaptionLabel("")
      .setText("primer.light")
      .setGroup(grp);

    btnSave = cp5.addButton("guiSave")
      .setPosition(30, 250)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Save")
      .setGroup(grp);

    btnLoad = cp5.addButton("guiLoad")
      .setPosition(30, 280)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Load")
      .setGroup(grp);

    btnLoad = cp5.addButton("guiDisconnect")
      .setPosition(30, 340)
      .setSize(60, 20)
      .setColorBackground(color(64))
      .setCaptionLabel("Disconnect")
      .setGroup(grp);

    btnExit = cp5.addButton("guiExit")
      .setPosition(30, 370)
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
    JSONObject json = new JSONObject();
    json.setJSONArray("modes", preset_editor.asJson());
    json.setJSONArray("palette", palette_panel.asJson());
    json.setJSONArray("bundles", bundle_editor.asJson());
    saveJSONObject(json, tfPath.getText());
  }

  void load() {
    JSONObject json = loadJSONObject(tfPath.getText());
    preset_editor.fromJson(json.getJSONArray("modes"));
    palette_panel.fromJson(json.getJSONArray("palette"));
    bundle_editor.fromJson(json.getJSONArray("bundles"));
  }
}
