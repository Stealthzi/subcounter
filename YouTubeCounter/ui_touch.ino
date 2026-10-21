// Touch screen line-up, on-screen buttons and the keyboard for typing messages

void loadTouchCalibration() {
  uint16_t cal[8];
  if (prefs.getBytesLength("tcal") == sizeof(cal)) {
    prefs.getBytes("tcal", cal, sizeof(cal));
    tft.setTouchCalibrate(cal);
    touchCalibrated = true;
  }
}

// Tap the four corners once so buttons react where you touch. If nobody
// touches the screen it carries on after 30 seconds, so a board with a
// faulty touch panel can't get stuck here.
void runTouchCalibration() {
  showMessage("Let's line up the touch screen", "Tap the screen to start - the stylus helps",
              "Skipping in 30 seconds if nothing is touched");
  unsigned long start = millis();
  while (!touchDown()) {
    if (millis() - start > 30000) return;
    delay(20);
  }
  while (touchDown()) delay(20);

  fillScreenBg();
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_MC);
  tft.setTextColor(C_WHITE);
  tft.drawString("Tap each arrow as it appears", W / 2, H / 2 - 10);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString("Use the stylus for the best result", W / 2, H / 2 + 16);
  uint16_t cal[8];
  tft.calibrateTouch(cal, C_WHITE, C_BG, 15);
  prefs.putBytes("tcal", cal, sizeof(cal));
  touchCalibrated = true;
  showMessage("Touch screen ready", "", "");
  delay(800);
}

// ---------------------------------------------------------------- buttons

void drawButton(int x, int y, int w, int h, const String& label, uint16_t bg, uint16_t fg) {
  tft.fillSmoothRoundRect(x, y, w, h, 8, bg);
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  if (tft.textWidth(label.c_str()) > w - 6) tft.setFont(F_SMALL);
  tft.setTextDatum(D_MC);
  tft.setTextColor(fg);
  tft.drawString(label.c_str(), x + w / 2, y + h / 2 + 1);
}

// Was (px, py) inside the box? Touch is a little imprecise, so allow a margin.
bool hitBox(int px, int py, int x, int y, int w, int h) {
  const int m = 5;
  return px >= x - m && px < x + w + m && py >= y - m && py < y + h + m;
}

// ---------------------------------------------------------------- keyboard

String kbText;
bool   kbNumbers = false;
bool   kbShift = true;   // capital letter next

const int KB_TOP = 50, KB_ROW_H = 42, KB_GAP = 4;
const char* KB_LETTERS[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
const char* KB_NUMBERS[3] = {"1234567890", "!?,.'-:()", "@#&/+=*"};

void openKeyboard() {
  keyboardOpen = true;
  kbText = "";
  kbNumbers = false;
  kbShift = true;
  drawKeyboard();
}

void closeKeyboard() {
  keyboardOpen = false;
  lastInteraction = millis();
  drawCurrentPage();
}

int kbRowY(int row) {
  return KB_TOP + row * (KB_ROW_H + KB_GAP);
}

// x position of a character key; rows 2 and 3 are indented like a real keyboard
int kbKeyX(int row, int i) {
  int indent = row == 1 ? 16 : row == 2 ? 48 : 0;
  return 1 + indent + i * 32;
}

void drawKeyboardText() {
  tft.fillRect(0, 0, W, KB_TOP - 4, C_CARD);
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_ML);
  tft.setTextColor(kbText.length() ? C_WHITE : C_DIM);
  String shown = kbText.length() ? kbText + "_" : String("Type your message...");
  // Show the end of the text if it's too long to fit
  while (shown.length() > 1 && tft.textWidth(shown.c_str()) > W - 60) shown.remove(0, 1);
  tft.drawString(shown.c_str(), 8, 22);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_MR);
  tft.setTextColor(C_DIM);
  tft.drawString(String(120 - (int)kbText.length()).c_str(), W - 8, 22);
}

void drawKeyboard() {
  fillScreenBg();
  drawKeyboardText();
  const char** rows = kbNumbers ? KB_NUMBERS : KB_LETTERS;
  for (int r = 0; r < 3; r++) {
    for (int i = 0; rows[r][i]; i++) {
      char c = rows[r][i];
      if (!kbNumbers && kbShift) c = toupper(c);
      drawButton(kbKeyX(r, i), kbRowY(r), 30, KB_ROW_H, String(c), C_CARD, C_WHITE);
    }
  }
  int y2 = kbRowY(2), y3 = kbRowY(3);
  if (!kbNumbers) drawButton(1, y2, 44, KB_ROW_H, "Aa", kbShift ? C_LINE : C_CARD, C_WHITE);
  drawButton(272, y2, 46, KB_ROW_H, "Del", C_LINE, C_WHITE);
  drawButton(1, y3, 56, KB_ROW_H, kbNumbers ? "abc" : "123", C_LINE, C_WHITE);
  drawButton(59, y3, 122, KB_ROW_H, "space", C_CARD, C_GREY);
  drawButton(183, y3, 64, KB_ROW_H, "Cancel", C_LINE, C_WHITE);
  drawButton(249, y3, 70, KB_ROW_H, "Send", C_RED, C_WHITE);
}

void keyboardTap(int x, int y) {
  int y2 = kbRowY(2), y3 = kbRowY(3);
  if (hitBox(x, y, 1, y3, 56, KB_ROW_H)) {
    kbNumbers = !kbNumbers;
    drawKeyboard();
    return;
  }
  if (hitBox(x, y, 59, y3, 122, KB_ROW_H)) {
    if (kbText.length() < 120 && kbText.length() && !kbText.endsWith(" ")) kbText += ' ';
    drawKeyboardText();
    return;
  }
  if (hitBox(x, y, 183, y3, 64, KB_ROW_H)) {
    closeKeyboard();
    return;
  }
  if (hitBox(x, y, 249, y3, 70, KB_ROW_H)) {
    String text = kbText;
    text.trim();
    keyboardOpen = false;
    if (text.length()) sendMessage("msg", text);
    closeKeyboard();
    return;
  }
  if (hitBox(x, y, 272, y2, 46, KB_ROW_H)) {
    if (kbText.length()) kbText.remove(kbText.length() - 1);
    drawKeyboardText();
    return;
  }
  if (!kbNumbers && hitBox(x, y, 1, y2, 44, KB_ROW_H)) {
    kbShift = !kbShift;
    drawKeyboard();
    return;
  }

  // Character keys: pick the row by height, then the nearest key
  const char** rows = kbNumbers ? KB_NUMBERS : KB_LETTERS;
  for (int r = 0; r < 3; r++) {
    if (y < kbRowY(r) - KB_GAP || y >= kbRowY(r) + KB_ROW_H + KB_GAP) continue;
    int n = strlen(rows[r]);
    int i = (x - kbKeyX(r, 0)) / 32;
    if (i < 0 || i >= n || kbText.length() >= 120) return;
    char c = rows[r][i];
    bool wasShift = kbShift;
    if (!kbNumbers && kbShift) c = toupper(c);
    kbText += c;
    // Capital letter again after the end of a sentence
    kbShift = !kbNumbers && (c == '.' || c == '!' || c == '?');
    if (kbNumbers && (c == '.' || c == '!' || c == '?')) kbShift = true;
    if (wasShift != kbShift && !kbNumbers) drawKeyboard();
    else drawKeyboardText();
    return;
  }
}
