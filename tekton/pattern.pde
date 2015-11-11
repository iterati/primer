class PatternEditor {
  Group grp;

  PatternEditor(int x, int y) {
    grp = cp5.addGroup("patternEditor")
      .setPosition(x, y)
      .hideBar()
      .hideArrow();
  }

  void hide() {
    grp.hide();
  }

  void show() {
    grp.show();
  }

  void refresh() {
  }
}
