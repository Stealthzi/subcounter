// Text formatting and the drawing pieces every page uses

// 1234567 -> "1,234,567"
String withCommas(uint64_t n) {
  char buf[32];
  int i = sizeof(buf) - 1;
  buf[i] = '\0';
  int digits = 0;
  do {
    if (digits > 0 && digits % 3 == 0) buf[--i] = ',';
    buf[--i] = '0' + (n % 10);
    n /= 10;
    digits++;
  } while (n > 0);
  return String(&buf[i]);
}

// 1234567 -> "1.23M" (numbers under 10,000 stay as they are)
String compact(uint64_t n) {
  if (n < 10000) return withCommas(n);
  const char* units[] = {"K", "M", "B"};
  double v = n / 1000.0;
  int u = 0;
  while (v >= 999.5 && u < 2) {
    v /= 1000;
    u++;
  }
  char buf[16];
  if (v < 10)       snprintf(buf, sizeof(buf), "%.2f%s", v, units[u]);
  else if (v < 100) snprintf(buf, sizeof(buf), "%.1f%s", v, units[u]);
  else              snprintf(buf, sizeof(buf), "%.0f%s", v, units[u]);
  return String(buf);
}

String signedCompact(int64_t n) {
  if (n > 0) return "+" + compact(n);
  if (n < 0) return "-" + compact(-n);
  return "0";
}

// The screen fonts only have plain English characters, so tidy text up:
// curly quotes/dashes become plain ones, emoji and other symbols are dropped.
String cleanText(const String& in) {
  String out;
  for (size_t i = 0; i < in.length(); i++) {
    uint8_t c = in[i];
    if (c < 0x80) {
      if (c == '\n' || c == '\r' || c == '\t') c = ' ';
      if (c >= 32 && !(c == ' ' && out.endsWith(" "))) out += (char)c;
      continue;
    }
    if (c == 0xE2 && i + 2 < in.length() && (uint8_t)in[i + 1] == 0x80) {
      uint8_t d = in[i + 2];
      if (d == 0x98 || d == 0x99)      out += '\'';
      else if (d == 0x9C || d == 0x9D) out += '"';
      else if (d == 0x93 || d == 0x94) out += '-';
      else if (d == 0xA6)              out += "...";
      i += 2;
    }
  }
  out.trim();
  return out;
}

// Cuts text down (adding "...") so it fits in maxWidth pixels in the current font
String fitText(const String& text, int maxWidth) {
  if (tft.textWidth(text.c_str()) <= maxWidth) return text;
  String t = text;
  while (t.length() > 0 && tft.textWidth((t + "...").c_str()) > maxWidth) {
    t.remove(t.length() - 1);
  }
  t.trim();
  return t + "...";
}

// The same, measured with another drawing surface's font (e.g. the canvas)
String fitTextIn(Gfx& g, const String& text, int maxWidth) {
  if (g.textWidth(text.c_str()) <= maxWidth) return text;
  String t = text;
  while (t.length() > 0 && g.textWidth((t + "...").c_str()) > maxWidth) t.remove(t.length() - 1);
  t.trim();
  return t + "...";
}

// Word-wraps text into up to maxLines lines; returns how many lines it used
int drawWrapped(const String& text, int x, int y, int maxWidth, int maxLines, int lineHeight) {
  tft.setTextDatum(D_TL);
  String rest = text;
  rest.trim();
  int lines = 0;
  while (rest.length() && lines < maxLines) {
    int lineY = y + lines * lineHeight;
    if (lines == maxLines - 1) {
      tft.drawString(fitText(rest, maxWidth).c_str(), x, lineY);
      return lines + 1;
    }
    int cut = -1, pos = 0;
    while (true) {
      int space = rest.indexOf(' ', pos);
      int end = space < 0 ? rest.length() : space;
      if (tft.textWidth(rest.substring(0, end).c_str()) > maxWidth) break;
      cut = end;
      if (space < 0) break;
      pos = space + 1;
    }
    if (cut <= 0) {  // one very long word: split it
      cut = rest.length();
      while (cut > 1 && tft.textWidth(rest.substring(0, cut).c_str()) > maxWidth) cut--;
    }
    tft.drawString(rest.substring(0, cut).c_str(), x, lineY);
    rest = rest.substring(cut);
    rest.trim();
    lines++;
  }
  return lines;
}

// "5m ago", "3h ago", "2d ago"...
String timeAgo(time_t when) {
  if (when <= 0) return "";
  long d = (long)(time(nullptr) - when);
  if (d < 60) return "just now";
  if (d < 3600) return String(d / 60) + "m ago";
  if (d < 86400) return String(d / 3600) + "h ago";
  if (d < 7 * 86400L) return String(d / 86400) + "d ago";
  if (d < 30 * 86400L) return String(d / (7 * 86400L)) + "w ago";
  if (d < 365 * 86400L) return String(d / (30 * 86400L)) + "mo ago";
  return String(d / (365 * 86400L)) + "y ago";
}

// 4800 -> "1h 20m"
String durationText(long secs) {
  if (secs < 0) secs = 0;
  long d = secs / 86400, h = (secs % 86400) / 3600, m = (secs % 3600) / 60;
  if (d > 0) return String(d) + "d " + String(h) + "h";
  if (h > 0) return String(h) + "h " + String(m) + "m";
  return String(m) + "m";
}

String clockText(const struct tm& t) {
  char buf[12];
  if (cfg.clock24) {
    strftime(buf, sizeof(buf), "%H:%M", &t);
  } else {
    int h = t.tm_hour % 12;
    snprintf(buf, sizeof(buf), "%d:%02d", h == 0 ? 12 : h, t.tm_min);
  }
  return String(buf);
}

// ---------------------------------------------------------------- drawing

void drawLogo(int x, int y, int w, int h) {
  tft.fillSmoothRoundRect(x, y, w, h, h / 4, C_RED);
  int tw = w * 3 / 10, th = h * 5 / 11;
  int cx = x + w / 2 + 1, cy = y + h / 2;
  tft.fillTriangle(cx - tw / 2, cy - th / 2, cx - tw / 2, cy + th / 2, cx + tw / 2, cy, C_WHITE);
}

// ---------------------------------------------------------------- theme and style

uint16_t to565(uint32_t rgb) {
  return RGB565((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

uint32_t to888(uint16_t c) {
  uint32_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return ((r * 255 / 31) << 16) | ((g * 255 / 63) << 8) | (b * 255 / 31);
}

// Mix two RGB888 colours: t = 0 gives a, t = 1 gives b
uint16_t mix(uint32_t a, uint32_t b, float t) {
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  int r = ((a >> 16) & 0xFF) + (int)((((int)((b >> 16) & 0xFF)) - (int)((a >> 16) & 0xFF)) * t);
  int g = ((a >> 8) & 0xFF) + (int)((((int)((b >> 8) & 0xFF)) - (int)((a >> 8) & 0xFF)) * t);
  int bl = (a & 0xFF) + (int)((((int)(b & 0xFF)) - (int)(a & 0xFF)) * t);
  return RGB565(r, g, bl);
}

void applyTheme() {
  const Theme& th = THEMES[cfg.theme < THEME_COUNT ? cfg.theme : 0];
  C_BG_TOP    = to565(th.bgTop);
  C_BG_BOTTOM = to565(th.bgBottom);
  C_BG        = mix(th.bgTop, th.bgBottom, 0.5f);
  C_CARD      = to565(th.card);
  C_EDGE      = to565(th.edge);
  C_LINE      = to565(th.line);
  C_GREY      = to565(th.grey);
  C_DIM       = to565(th.dim);
  C_ACCENT    = to565(th.accent);
  C_ACCENT2   = to565(th.accent2);
  C_GLOW      = to565(th.glow);
}

const Theme& theme() {
  return THEMES[cfg.theme < THEME_COUNT ? cfg.theme : 0];
}

// The background is a gentle top-to-bottom gradient
uint16_t bgAt(int y) {
  return mix(theme().bgTop, theme().bgBottom, (float)y / (H - 1));
}

void fillBg(int x, int y, int w, int h) {
  tft.startWrite();
  for (int row = y; row < y + h; row++) tft.writeFastHLine(x, row, w, bgAt(row));
  tft.endWrite();
}

void fillScreenBg() {
  fillBg(0, 0, W, H);
  drawSides();
}

// On the 7" screen the page sits in the middle; the strips either side match
// the background, with the title bar's line carried across
void drawSides() {
  if (tft.ox <= 0) return;
  tft.fillSides(bgAt);
  lcd.fillRect(0, tft.Y(33), tft.ox, tft.s, C_LINE);
  lcd.fillRect(lcd.width() - tft.ox, tft.Y(33), tft.ox, tft.s, C_LINE);
  drawNavArrows();
}

// 7" only: in the strips either side of the page, a back and a forward arrow,
// with Home (left) and Settings (right) buttons below them
void drawNavArrows() {
  drawNavArrowsIn(C_GREY, C_CARD);
}

void drawNavArrowsIn(uint16_t arrow, uint16_t button) {
  if (tft.ox <= 0 || currentPage < 0 || keyboardOpen) return;
  int cy = lcd.height() / 2;
  for (int side = -1; side <= 1; side += 2) {
    int cx = side < 0 ? tft.ox / 2 : lcd.width() - tft.ox / 2;
    if (!overlayOpen()) {  // Home and Settings have their own buttons for moving about
      lcd.fillSmoothCircle(cx, cy, 28, button);
      int tip = cx + side * 7, back = cx - side * 7;
      lcd.drawWideLine(back, cy - 13, tip, cy, 3, arrow);
      lcd.drawWideLine(tip, cy, back, cy + 13, 3, arrow);
    }
    // Home and Settings, lit up while open
    bool lit = side < 0 ? overlay == OVERLAY_HOME : (overlayOpen() && overlay != OVERLAY_HOME);
    uint16_t bg = lit ? C_ACCENT : button, fg = lit ? C_ON_ACCENT : arrow;
    int by = tft.Y(SIDE_BUTTON_CY);
    lcd.fillSmoothCircle(cx, by, 26, bg);
    if (side < 0) drawHomeIcon(cx, by, fg, bg);
    else drawGearIcon(cx, by, fg, bg);
    lcd.setFont(F_BOLD);
    lcd.setTextSize(1);
    lcd.setTextDatum(D_TC);
    lcd.setTextColor(arrow);
    lcd.drawString(side < 0 ? "Home" : "Settings", cx, by + 33);
  }
}

// 7": lights up the arrow (button false) or the Home/Settings button
// (button true) on one side for a moment, so you can see the tap landed
void pressSide(int side, bool button) {
  if (tft.ox <= 0) return;
  int cx = side < 0 ? tft.ox / 2 : lcd.width() - tft.ox / 2;
  if (button) {
    int by = tft.Y(SIDE_BUTTON_CY);
    lcd.fillSmoothCircle(cx, by, 26, C_ACCENT);
    if (side < 0) drawHomeIcon(cx, by, C_ON_ACCENT, C_ACCENT);
    else drawGearIcon(cx, by, C_ON_ACCENT, C_ACCENT);
  } else {
    int cy = lcd.height() / 2;
    lcd.fillSmoothCircle(cx, cy, 28, C_ACCENT);
    int tip = cx + side * 7, back = cx - side * 7;
    lcd.drawWideLine(back, cy - 13, tip, cy, 3, C_ON_ACCENT);
    lcd.drawWideLine(tip, cy, back, cy + 13, 3, C_ON_ACCENT);
  }
  delay(PRESS_MS);
}

// A little house, drawn straight onto the 7" screen around (cx, cy)
void drawHomeIcon(int cx, int cy, uint16_t fg, uint16_t bg) {
  lcd.fillTriangle(cx - 14, cy - 1, cx, cy - 14, cx + 14, cy - 1, fg);
  lcd.fillRect(cx - 9, cy - 2, 19, 13, fg);
  lcd.fillRect(cx - 3, cy + 3, 7, 8, bg);  // the door
}

// A cog
void drawGearIcon(int cx, int cy, uint16_t fg, uint16_t bg) {
  for (int i = 0; i < 8; i++) {
    float a = i * PI / 4;
    lcd.drawWideLine(cx + cosf(a) * 8, cy + sinf(a) * 8, cx + cosf(a) * 14, cy + sinf(a) * 14, 3.2f, fg);
  }
  lcd.fillSmoothCircle(cx, cy, 10, fg);
  lcd.fillSmoothCircle(cx, cy, 4, bg);
}

// Draws text with a left-to-right colour gradient, like a styled display name.
// Uses the current font and size; (x, y) is the text's top-left, or its
// baseline-left if baseline is true.
void gradientText(const String& text, int x, int y, bool baseline, uint32_t from, uint32_t to) {
  int w = tft.textWidth(text.c_str());
  if (w <= 0) return;
  tft.setTextDatum(baseline ? D_BL : D_TL);
  const int bands = 14;
  int bandW = w / bands + 1;
  for (int i = 0; i < bands; i++) {
    tft.setClipRect(x + i * bandW, 0, bandW, H);
    tft.setTextColor(mix(from, to, (float)i / (bands - 1)));
    tft.drawString(text.c_str(), x, y);
  }
  tft.clearClipRect();
}

// Same, centred on cx
void gradientTextCentred(const String& text, int cx, int y, bool baseline) {
  gradientText(text, cx - tft.textWidth(text.c_str()) / 2, y, baseline, theme().accent, theme().accent2);
}

// A soft glowing ring, fading out into the background
void drawGlow(int cx, int cy, int r, int spread, uint32_t colour) {
  for (int i = spread; i >= 0; i--) {
    float t = (float)i / spread;
    tft.drawCircle(cx, cy, r + i, mix(colour, to888(bgAt(cy)), t));
  }
}

// A card: rounded box with a thin outline
void drawCardBox(int x, int y, int w, int h) {
  tft.fillSmoothRoundRect(x, y, w, h, 10, C_EDGE);
  tft.fillSmoothRoundRect(x + 1, y + 1, w - 2, h - 2, 9, C_CARD);
}

uint16_t statusColour() {
  if (WiFi.status() != WL_CONNECTED || lastError.length()) return C_RED;
  if (youtubeBlocked()) return C_AMBER;
  return C_GREEN;
}

// Each section of pages has its own colour and title
bool isYouTubePage(int page) {
  return page <= PAGE_LIVE || page == PAGE_ANALYTICS || page == PAGE_REPORT || page == PAGE_OVERVIEW;
}

uint16_t pageColour(int page) {
  switch (page) {
    case PAGE_ENERGY: case PAGE_FUEL: case PAGE_CALENDAR: case PAGE_ALERTS: case PAGE_AIR:
    case PAGE_COUNTDOWN: case PAGE_CLOCK: case PAGE_WEATHER:
      return C_BLUE;
    case PAGE_MESSAGES:
      return C_PURPLE;
    case PAGE_PET:
      return C_GREEN;
    case PAGE_GAMES: case PAGE_SCREENSAVER: case PAGE_EIGHTBALL: case PAGE_STOPWATCH: case PAGE_MOODLAMP:
    case PAGE_JOKE: case PAGE_SPOTIFY: case PAGE_CRYPTO:
      return C_AMBER;
  }
  return C_ACCENT;
}

const char* pageTitle(int page) {
  switch (page) {
    case PAGE_CLOCK:       return "Clock";
    case PAGE_WEATHER:     return "Weather";
    case PAGE_CRYPTO:      return "Crypto";
    case PAGE_ENERGY:      return "Energy";
    case PAGE_FUEL:        return "Petrol prices";
    case PAGE_CALENDAR:    return "Calendar";
    case PAGE_MESSAGES:    return "Messages";
    case PAGE_PET:         return cfg.petName.c_str();
    case PAGE_ALERTS:      return "Alerts";
    case PAGE_GAMES:       return "Free games";
    case PAGE_SCREENSAVER: return "Relax";
    case PAGE_EIGHTBALL:   return "Magic 8-ball";
    case PAGE_STOPWATCH:   return "Stopwatch";
    case PAGE_AIR:         return "Air and pollen";
    case PAGE_COUNTDOWN:   return "Countdowns";
    case PAGE_MOODLAMP:    return "Mood lamp";
    case PAGE_JOKE:        return joke.isFact ? "Fact of the day" : "Joke of the day";
    case PAGE_SPOTIFY:     return "Now playing";
  }
  return "";
}

void drawHeader() {
  fillBg(0, 0, W, 34);

  // Right side: unread messages and the status dot (no clock - it's on the Clock page if wanted)
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  tft.fillSmoothCircle(W - 9, 18, 4, statusColour());
  int rightEdge = W - 20;
  if (unreadCount > 0) {
    tft.fillSmoothRoundRect(rightEdge - 22, 10, 20, 16, 4, C_PURPLE);
    tft.setTextDatum(D_MC);
    tft.setTextColor(C_WHITE);
    tft.drawString(String(unreadCount).c_str(), rightEdge - 12, 18);
    rightEdge -= 28;
  }
#if !BOARD_7INCH
  // A small grid: tap the title bar for Home (the 7" has a Home button instead)
  for (int i = 0; i < 4; i++) tft.fillRoundRect(rightEdge - 14 + (i % 2) * 7, 11 + (i / 2) * 7, 5, 5, 1, C_GREY);
  rightEdge -= 20;
#endif

  // Left side: YouTube logo and channel name, or the page's colour and title
  tft.setFont(F_BOLD);
  if (currentPage < 0 || isYouTubePage(currentPage)) {
    drawLogo(8, 8, 28, 20);
    gradientText(fitText(channel.title, rightEdge - 50), 44, 11, false, 0xFFFFFF, theme().accent);
  } else {
    uint16_t c = pageColour(currentPage);
    tft.fillSmoothRoundRect(8, 9, 6, 18, 3, c);
    gradientText(fitText(pageTitle(currentPage), rightEdge - 28), 22, 11, false, 0xFFFFFF, to888(c));
  }
  drawSeasonalHeader();
  drawHeaderCat();
  tft.drawFastHLine(0, 33, W, C_LINE);
}

// Small dots at the bottom showing which page you're on
void drawPageDots() {
  int n = 0, current = -1;
  for (int i = 0; i < PAGE_COUNT; i++) {
    int p = PAGE_ORDER[i];
    if (!pageAvailable(p)) continue;
    if (p == currentPage) current = n;
    n++;
  }
  fillBg(0, 227, W, H - 227);
  if (n < 2) return;
  int gap = n > 20 ? 9 : 11;
  int x = W / 2 - (n - 1) * gap / 2;
  for (int i = 0; i < n; i++) {
    if (i == current) tft.fillSmoothRoundRect(x + i * gap - 5, 230, 10, 6, 3, C_ACCENT);
    else tft.fillSmoothCircle(x + i * gap, 233, 2, C_DIM);
  }
}

void clearContent() {
  fillBg(0, 34, W, H - 34);
}

// Small label, optionally followed by grey extra text
void drawLabel(int x, int y, const String& label, uint16_t colour, const String& extra) {
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  tft.setTextDatum(D_TL);
  tft.setTextColor(colour);
  int w = tft.drawString(label.c_str(), x, y);
  if (extra.length()) {
    tft.setTextColor(C_GREY);
    tft.drawString(("  " + extra).c_str(), x + w, y);
  }
}

// A rounded card with a small label and a big value
void drawCard(int x, int y, int w, int h, const String& label, const String& value, uint16_t valueColour) {
  drawCardBox(x, y, w, h);
  tft.setTextSize(1);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(label, w - 10).c_str(), x + w / 2, y + 10);
  tft.setFont(F_LARGE);
  if (tft.textWidth(value.c_str()) > w - 12) tft.setFont(F_MED);
  if (tft.textWidth(value.c_str()) > w - 12) tft.setFont(F_BOLD);
  String shown = fitText(value, w - 12);
  int vy = y + (h > 70 ? 34 : 30);
  if (valueColour == C_WHITE) {
    gradientTextCentred(shown, x + w / 2, vy, false);
  } else {
    tft.setTextDatum(D_TC);
    tft.setTextColor(valueColour);
    tft.drawString(shown.c_str(), x + w / 2, vy);
  }
}

// A rounded bar filled with the accent gradient
void drawProgressBar(int x, int y, int w, int h, float fraction, uint16_t colour) {
  if (fraction < 0) fraction = 0;
  if (fraction > 1) fraction = 1;
  tft.fillSmoothRoundRect(x, y, w, h, h / 2, C_CARD);
  int fw = (int)(w * fraction);
  if (fw < h) return;
  uint32_t from = colour == C_ACCENT ? theme().accent : to888(colour);
  uint32_t to = colour == C_ACCENT ? theme().accent2 : to888(colour);
  int r = h / 2;
  tft.fillSmoothCircle(x + r, y + r, r, mix(from, to, 0));
  tft.fillSmoothCircle(x + fw - r - 1, y + r, r, mix(from, to, (float)fw / w));
  tft.startWrite();
  for (int i = r; i < fw - r; i++) tft.writeFastVLine(x + i, y, h, mix(from, to, (float)i / w));
  tft.endWrite();
}

// Draws a big number, scaled to fit maxWidth, visually centred on centreY.
// White numbers get the theme's gradient.
void drawBigNumber(const String& text, int cx, int centreY, int maxWidth, float maxScale, uint16_t colour) {
  tft.setFont(F_HUGE);
  tft.setTextSize(1);
  float scale = (float)maxWidth / tft.textWidth(text.c_str());
  if (scale > maxScale) scale = maxScale;
  tft.setTextSize(scale);
  int baseline = centreY + (int)(17 * scale);
  if (colour == C_WHITE) {
    gradientTextCentred(text, cx, baseline, true);
  } else {
    tft.setTextDatum(D_BC);
    tft.setTextColor(colour);
    tft.drawString(text.c_str(), cx, baseline);
  }
  tft.setTextSize(1);
}

// Fade the backlight out and back in around a page change

void fadeTo(uint8_t target, int steps, int stepMs) {
  int from = shownBrightness;
  for (int i = 1; i <= steps; i++) {
    tft.setBrightness(from + (target - from) * i / steps);
    delay(stepMs);
  }
  shownBrightness = target;
}

// Text where '`' stands for a degree sign (the fonts don't have one)
int degreeTextWidth(const String& s) {
  int r = max(2, (int)(tft.fontHeight() / 10));
  int w = 0, start = 0;
  for (int i = 0; i <= (int)s.length(); i++) {
    if (i == (int)s.length() || s[i] == '`') {
      w += tft.textWidth(s.substring(start, i).c_str());
      if (i < (int)s.length()) w += r * 2 + 3;
      start = i + 1;
    }
  }
  return w;
}

int drawDegreeText(const String& s, int x, int y, uint16_t colour) {
  tft.setTextDatum(D_TL);
  tft.setTextColor(colour);
  int r = max(2, (int)(tft.fontHeight() / 10));
  int thick = max(1, r / 3);
  int cx = x, start = 0;
  for (int i = 0; i <= (int)s.length(); i++) {
    if (i == (int)s.length() || s[i] == '`') {
      cx += tft.drawString(s.substring(start, i).c_str(), cx, y);
      if (i < (int)s.length()) {
        int cy = y + r + tft.fontHeight() / 10;
        for (int k = 0; k < thick; k++) tft.drawCircle(cx + r + 1, cy, r - k, colour);
        cx += r * 2 + 3;
      }
      start = i + 1;
    }
  }
  return cx - x;
}

// "14`C" style temperature text
String tempText(float t) {
  return String((int)round(t)) + "`";
}
