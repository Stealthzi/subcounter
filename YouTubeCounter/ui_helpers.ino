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

uint16_t statusColour() {
  if (WiFi.status() != WL_CONNECTED || lastError.length()) return C_RED;
  if (youtubeBlocked()) return C_AMBER;
  return C_GREEN;
}

void drawHeader() {
  tft.fillRect(0, 0, W, 34, C_BG);
  drawLogo(8, 8, 28, 20);

  struct tm t;
  String clock = getLocalTime(&t, 0) ? clockText(t) : "";
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  int clockW = tft.textWidth(clock.c_str());
  tft.setTextDatum(D_MR);
  tft.setTextColor(C_GREY);
  tft.drawString(clock.c_str(), W - 20, 18);

  tft.setFont(F_BOLD);
  tft.setTextDatum(D_ML);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(channel.title, W - 44 - clockW - 30).c_str(), 44, 18);

  tft.fillSmoothCircle(W - 9, 18, 4, statusColour());
}

// Small dots at the bottom showing which page you're on
void drawPageDots() {
  int pages[PAGE_COUNT], n = 0, current = -1;
  for (int p = 0; p < PAGE_COUNT; p++) {
    if (!pageAvailable(p)) continue;
    if (p == currentPage) current = n;
    pages[n++] = p;
  }
  tft.fillRect(0, 227, W, H - 227, C_BG);
  if (n < 2) return;
  int gap = 11;
  int x = W / 2 - (n - 1) * gap / 2;
  for (int i = 0; i < n; i++) {
    bool on = i == current;
    tft.fillSmoothCircle(x + i * gap, 233, on ? 3 : 2, on ? C_WHITE : C_DIM);
  }
}

void clearContent() {
  tft.fillRect(0, 34, W, H - 34, C_BG);
}

// Small caps label, optionally followed by grey extra text
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
  tft.fillSmoothRoundRect(x, y, w, h, 10, C_CARD);
  tft.setTextSize(1);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(label, w - 10).c_str(), x + w / 2, y + 10);
  tft.setFont(F_LARGE);
  if (tft.textWidth(value.c_str()) > w - 12) tft.setFont(F_MED);
  if (tft.textWidth(value.c_str()) > w - 12) tft.setFont(F_BOLD);
  tft.setTextColor(valueColour);
  tft.drawString(fitText(value, w - 12).c_str(), x + w / 2, y + (h > 70 ? 34 : 30));
}

void drawProgressBar(int x, int y, int w, int h, float fraction, uint16_t colour) {
  if (fraction < 0) fraction = 0;
  if (fraction > 1) fraction = 1;
  tft.fillSmoothRoundRect(x, y, w, h, h / 2, C_CARD);
  int fw = (int)(w * fraction);
  if (fw >= h) tft.fillSmoothRoundRect(x, y, fw, h, h / 2, colour);
}

// Draws a big number, scaled to fit maxWidth, visually centred on centreY
void drawBigNumber(const String& text, int cx, int centreY, int maxWidth, float maxScale, uint16_t colour) {
  tft.setFont(F_HUGE);
  tft.setTextSize(1);
  float scale = (float)maxWidth / tft.textWidth(text.c_str());
  if (scale > maxScale) scale = maxScale;
  tft.setTextSize(scale);
  tft.setTextDatum(D_BC);
  tft.setTextColor(colour);
  tft.drawString(text.c_str(), cx, centreY + (int)(17 * scale));
  tft.setTextSize(1);
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
