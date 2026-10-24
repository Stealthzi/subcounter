// Screens on top of the pages: Home (every screen as a tile), Settings, the
// list for choosing which screens are on, and the phone settings QR code.
// Also sleep: the screen goes dark until you touch it.
//
// Open Home with the button at the bottom of the left strip (7"), or by
// tapping the title bar at the top of any page. Settings has its own button
// at the bottom of the right strip (7"), and a tile on Home.

// Names for the list of screens, in PageId order
const char* PAGE_SHORT[PAGE_COUNT] = {
  "Subscribers", "Channel stats", "Latest video", "Recent videos", "Top video",
  "Newest comment", "7-day chart", "Head to head", "Live", "Clock", "Weather", "Crypto",
  "Energy", "Petrol", "Calendar", "Messages", "Pet", "Alerts",
  "Free games", "Relax", "Magic 8-ball", "Stopwatch", "Air quality", "Countdowns",
  "Mood lamp", "Joke or fact", "Spotify", "Exact stats", "Weekly report", "Overview"
};
static_assert(sizeof(PAGE_SHORT) / sizeof(PAGE_SHORT[0]) == PAGE_COUNT, "a page is missing a short name");

// Shorter still, for under the pictures on the Home tiles
const char* PAGE_TILE[PAGE_COUNT] = {
  "Subs", "Channel", "Latest", "Recent", "Top video", "Comment", "7 days", "Versus", "Live",
  "Clock", "Weather", "Crypto", "Energy", "Petrol", "Calendar", "Messages", "Pet", "Alerts",
  "Games", "Relax", "8-ball", "Stopwatch", "Air", "Countdown", "Mood lamp", "Joke", "Spotify",
  "Exact stats", "Report", "Overview"
};
static_assert(sizeof(PAGE_TILE) / sizeof(PAGE_TILE[0]) == PAGE_COUNT, "a page is missing a tile name");

const int TILE_COLS = 4, TILE_ROWS = 4, TILE_W = 74, TILE_H = 42, TILE_GAP = 4, TILE_TOP = 40;
const int TILES_PER_VIEW = TILE_COLS * TILE_ROWS;
const int LIST_ROWS = 6, LIST_TOP = 40, LIST_ROW_H = 24;

bool overlayOpen() {
  return overlay != OVERLAY_NONE;
}

void openOverlay(Overlay which) {
  if (catWandering()) endWander();
  overlay = which;
  overlayScroll = 0;
  overlayTouchedAt = millis();
  drawOverlay();
}

void openHome() { openOverlay(OVERLAY_HOME); }
void openSettings() { openOverlay(OVERLAY_SETTINGS); }

// Back to the page underneath (or the next one, if that page was just switched off)
void closeOverlay() {
  overlay = OVERLAY_NONE;
  int before = currentPage;
  if (!pageAvailable(currentPage)) stepPage(1);
  if (currentPage == before) drawCurrentPage();
}

// Shuts Home or Settings after a minute with nobody using it
void overlayTimeout() {
  if (overlayOpen() && millis() - overlayTouchedAt > 60000) closeOverlay();
}

// ---------------------------------------------------------------- sleep

// Screen off until it's touched (the touch that wakes it does nothing else)
void goToSleep() {
  overlay = OVERLAY_NONE;
  if (catWandering()) endWander();
  fadeTo(0, 8, 30);
  sleeping = true;
  drawCurrentPage();  // wakes up on the page, not on Settings
}

void wakeUp() {
  sleeping = false;
  lastInteraction = millis();
  wakeUntil = millis() + 30000;  // at night, stays lit for a little while
  applyBrightness();
}

// ---------------------------------------------------------------- shared parts

// Title bar for an overlay: a Sleep button (if wanted) and Close, Done or Back
void drawOverlayTop(const String& title, const char* button, bool sleepButton) {
  fillScreenBg();
  tft.setFont(F_MED);
  tft.setTextSize(1);
  gradientText(title, 12, 6, false, 0xFFFFFF, theme().accent);
  if (sleepButton) drawButton(W - 152, 5, 66, 24, "Sleep", C_CARD, C_WHITE);
  drawButton(W - 80, 5, 72, 24, button, C_CARD, C_WHITE);
  tft.drawFastHLine(0, 33, W, C_LINE);
}

// Which top-bar button was tapped: 1 = Close/Done/Back, 2 = Sleep, 0 = neither.
// The button lights up for a moment so you can see the tap landed.
int topBarButton(int x, int y) {
  if (y >= 34) return 0;
  if (x >= W - 84) {
    const char* label = overlay == OVERLAY_PAGES ? "Done" : overlay == OVERLAY_PHONE ? "Back" : "Close";
    drawButton(W - 80, 5, 72, 24, label, C_ACCENT, C_ON_ACCENT);
    delay(PRESS_MS);
    return 1;
  }
  if (x >= W - 156 && (overlay == OVERLAY_HOME || overlay == OVERLAY_SETTINGS)) {
    drawButton(W - 152, 5, 66, 24, "Sleep", C_ACCENT, C_ON_ACCENT);
    delay(PRESS_MS);
    return 2;
  }
  return 0;
}

void drawOverlay() {
  switch (overlay) {
    case OVERLAY_HOME:     drawHome(); break;
    case OVERLAY_SETTINGS: drawSettingsScreen(); break;
    case OVERLAY_PAGES:    drawPagePicker(); break;
    case OVERLAY_PHONE:    drawPhoneScreen(); break;
    default: break;
  }
}

// ---------------------------------------------------------------- Home

// The tiles on Home: every screen that's on and has something to show, then Settings
int homeTiles(int* out) {
  int n = 0;
  for (int i = 0; i < PAGE_COUNT; i++) {
    int p = PAGE_ORDER[i];
    if (pageAvailable(p)) out[n++] = p;
  }
  out[n++] = -1;  // Settings
  return n;
}

// With more tiles than fit, the last slot becomes "More" and flips to the next set
int homeViews(int n) {
  return n <= TILES_PER_VIEW ? 1 : (n + TILES_PER_VIEW - 2) / (TILES_PER_VIEW - 1);
}

// A Home tile: its picture with the name underneath. `icon` is a page, or
// ICON_SETTINGS / ICON_MORE. The page you're on has an accent outline, and
// a pressed tile lights up.
void drawTile(int slot, int icon, const String& name, bool current, bool pressed) {
  int x = 4 + (slot % TILE_COLS) * (TILE_W + TILE_GAP);
  int y = TILE_TOP + (slot / TILE_COLS) * (TILE_H + TILE_GAP);
  uint16_t bg = pressed ? C_ACCENT : C_CARD;
  if (pressed) {
    tft.fillSmoothRoundRect(x, y, TILE_W, TILE_H, 10, C_ACCENT);
  } else if (current) {
    tft.fillSmoothRoundRect(x, y, TILE_W, TILE_H, 10, C_ACCENT);
    tft.fillSmoothRoundRect(x + 2, y + 2, TILE_W - 4, TILE_H - 4, 8, C_CARD);
  } else {
    drawCardBox(x, y, TILE_W, TILE_H);
  }
  uint16_t colour = pressed ? C_ON_ACCENT : icon >= 0 ? pageColour(icon) : C_GREY;
  drawPageIcon(icon, x + TILE_W / 2, y + 15, colour, bg);
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  tft.setTextColor(pressed ? C_ON_ACCENT : C_WHITE);
  tft.setTextDatum(D_MC);
  tft.drawString(fitText(name, TILE_W - 6).c_str(), x + TILE_W / 2, y + TILE_H - 9);
}

// What's on a tile: the screen it opens, Settings, or More
void drawHomeTile(int slot, int tile, int views, bool pressed) {
  if (tile == ICON_MORE) drawTile(slot, ICON_MORE, "More " + String(overlayScroll + 1) + "/" + String(views), false, pressed);
  else if (tile < 0) drawTile(slot, ICON_SETTINGS, "Settings", false, pressed);
  else drawTile(slot, tile, PAGE_TILE[tile], tile == currentPage, pressed);
}

void drawHome() {
  drawOverlayTop("Home", "Close", true);
  int tiles[PAGE_COUNT + 1];
  int n = homeTiles(tiles);
  int views = homeViews(n);
  int per = views > 1 ? TILES_PER_VIEW - 1 : TILES_PER_VIEW;
  if (overlayScroll >= views) overlayScroll = 0;
  int first = overlayScroll * per;
  for (int slot = 0; slot < per && first + slot < n; slot++) drawHomeTile(slot, tiles[first + slot], views, false);
  if (views > 1) drawHomeTile(TILES_PER_VIEW - 1, ICON_MORE, views, false);
}

void homeTap(int x, int y) {
  int button = topBarButton(x, y);
  if (button == 1) return closeOverlay();
  if (button == 2) return goToSleep();
  if (y < TILE_TOP) return;
  int col = (x - 4) / (TILE_W + TILE_GAP), row = (y - TILE_TOP) / (TILE_H + TILE_GAP);
  if (col < 0 || col >= TILE_COLS || row < 0 || row >= TILE_ROWS) return;
  int slot = row * TILE_COLS + col;
  int tiles[PAGE_COUNT + 1];
  int n = homeTiles(tiles);
  int views = homeViews(n);
  int per = views > 1 ? TILES_PER_VIEW - 1 : TILES_PER_VIEW;
  if (views > 1 && slot == TILES_PER_VIEW - 1) {
    drawHomeTile(slot, ICON_MORE, views, true);
    delay(PRESS_MS);
    overlayScroll = (overlayScroll + 1) % views;
    drawOverlay();
    return;
  }
  int i = overlayScroll * per + slot;
  if (i >= n) return;
  drawHomeTile(slot, tiles[i], views, true);
  delay(PRESS_MS);
  if (tiles[i] < 0) return openSettings();
  overlay = OVERLAY_NONE;
  showPage(tiles[i]);
  pagePicked();
}

// ---------------------------------------------------------------- Settings

const uint8_t PAGE_SECS_CHOICES[] = {0, 10, 30};
const char* const PAGE_SECS_NAMES[] = {"Never", "Every 10 s", "Every 30 s"};
const uint8_t BRIGHT_CHOICES[] = {60, 150, 255};
const char* const BRIGHT_NAMES[] = {"Low", "Medium", "High"};
const char* const NIGHT_NAMES[] = {"Stay on", "Dim", "Screen off"};

const int SET_TIMER_Y = 52, SET_BRIGHT_Y = 98, SET_NIGHT_Y = 144, SET_LINKS_Y = 184, SET_BUTTON_H = 28;

// A row of three choices, the chosen one lit up
void drawChoices(int y, const char* const* names, int chosen) {
  for (int i = 0; i < 3; i++) {
    bool on = i == chosen;
    drawButton(8 + i * 104, y, 96, SET_BUTTON_H, names[i], on ? C_ACCENT : C_CARD, on ? C_ON_ACCENT : C_WHITE);
  }
}

int chosenPageSecs() {
  if (cfg.pageSecs == 0) return 0;
  return cfg.pageSecs >= 20 ? 2 : 1;
}

int chosenBrightness() {
  int best = 0;
  for (int i = 1; i < 3; i++) {
    if (abs((int)cfg.brightness - BRIGHT_CHOICES[i]) < abs((int)cfg.brightness - BRIGHT_CHOICES[best])) best = i;
  }
  return best;
}

int chosenNight() {
  if (!cfg.nightMode) return 0;
  return cfg.nightBrightness == 0 ? 2 : 1;
}

String hourText(int h) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:00", h);
  return buf;
}

void drawSettingsScreen() {
  drawOverlayTop("Settings", "Close", true);
  drawLabel(10, SET_TIMER_Y - 14, "CHANGE SCREENS BY ITSELF", C_GREY, "");
  drawChoices(SET_TIMER_Y, PAGE_SECS_NAMES, chosenPageSecs());
  drawLabel(10, SET_BRIGHT_Y - 14, "BRIGHTNESS", C_GREY, "");
  drawChoices(SET_BRIGHT_Y, BRIGHT_NAMES, chosenBrightness());
  drawLabel(10, SET_NIGHT_Y - 14, "AT NIGHT", C_GREY, hourText(cfg.nightFrom) + " to " + hourText(cfg.nightTo));
  drawChoices(SET_NIGHT_Y, NIGHT_NAMES, chosenNight());
  drawButton(8, SET_LINKS_Y, 148, 32, "Choose screens", C_CARD, C_WHITE);
  drawButton(164, SET_LINKS_Y, 148, 32, "Phone settings", C_CARD, C_WHITE);
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_DIM);
  tft.drawString("Night hours and everything else are in phone settings", W / 2, 222);
}

void settingsTap(int x, int y) {
  int button = topBarButton(x, y);
  if (button == 1) return closeOverlay();
  if (button == 2) return goToSleep();
  int choice = constrain((x - 8) / 104, 0, 2);
  if (y >= SET_TIMER_Y - 4 && y < SET_TIMER_Y + SET_BUTTON_H + 4) {
    cfg.pageSecs = PAGE_SECS_CHOICES[choice];
    prefs.putUChar("pagesecs", cfg.pageSecs);
    pageShownAt = millis();
    if (!autoPages()) pagePickedAt = millis();  // remember this page to stay on
    drawOverlay();
  } else if (y >= SET_BRIGHT_Y - 4 && y < SET_BRIGHT_Y + SET_BUTTON_H + 4) {
    cfg.brightness = BRIGHT_CHOICES[choice];
    prefs.putUChar("bright", cfg.brightness);
    applyBrightness();
    drawOverlay();
  } else if (y >= SET_NIGHT_Y - 4 && y < SET_NIGHT_Y + SET_BUTTON_H + 4) {
    cfg.nightMode = choice > 0;
    if (choice == 1 && cfg.nightBrightness == 0) cfg.nightBrightness = 15;
    if (choice == 2) cfg.nightBrightness = 0;
    prefs.putBool("night", cfg.nightMode);
    prefs.putUChar("nbright", cfg.nightBrightness);
    applyBrightness();
    drawOverlay();
  } else if (y >= SET_LINKS_Y - 4 && y < SET_LINKS_Y + 36) {
    bool left = x < W / 2;
    drawButton(left ? 8 : 164, SET_LINKS_Y, 148, 32, left ? "Choose screens" : "Phone settings", C_ACCENT, C_ON_ACCENT);
    delay(PRESS_MS);
    openOverlay(left ? OVERLAY_PAGES : OVERLAY_PHONE);
  }
}

// ---------------------------------------------------------------- choose screens

const int PICKER_PER_VIEW = LIST_ROWS * 2;

int pickerViews() {
  return (PAGE_COUNT + PICKER_PER_VIEW - 1) / PICKER_PER_VIEW;
}

void drawPagePicker() {
  drawOverlayTop("Choose screens", "Done", false);
  int first = overlayScroll * PICKER_PER_VIEW;
  for (int i = 0; i < PICKER_PER_VIEW && first + i < PAGE_COUNT; i++) {
    int p = PAGE_ORDER[first + i];
    int x = i < LIST_ROWS ? 8 : 164, y = LIST_TOP + (i % LIST_ROWS) * LIST_ROW_H;
    bool on = pageEnabled(p);
    tft.fillSmoothRoundRect(x, y + 2, 18, 18, 4, on ? C_ACCENT : C_CARD);
    if (on) {
      tft.drawWideLine(x + 4, y + 11, x + 8, y + 15, 1.5f, C_ON_ACCENT);
      tft.drawWideLine(x + 8, y + 15, x + 14, y + 6, 1.5f, C_ON_ACCENT);
    }
    tft.setFont(F_SMALL);
    tft.setTextSize(1);
    tft.setTextDatum(D_ML);
    tft.setTextColor(pageHasData(p) ? C_WHITE : C_GREY);
    tft.drawString(fitText(PAGE_SHORT[p], 124).c_str(), x + 26, y + 11);
  }
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_DIM);
  tft.drawString("Grey ones need setting up on your phone first", W / 2, LIST_TOP + LIST_ROWS * LIST_ROW_H + 2);
  drawButton(8, 208, 64, 28, "<", C_CARD, C_WHITE);
  tft.setTextDatum(D_MC);
  tft.setTextColor(C_GREY);
  tft.drawString((String(overlayScroll + 1) + " of " + String(pickerViews())).c_str(), W / 2, 222);
  drawButton(W - 72, 208, 64, 28, ">", C_CARD, C_WHITE);
}

void pagePickerTap(int x, int y) {
  if (topBarButton(x, y) == 1) return openSettings();
  int views = pickerViews();
  if (y >= 204) {
    if (x < 90) {
      drawButton(8, 208, 64, 28, "<", C_ACCENT, C_ON_ACCENT);
      overlayScroll = (overlayScroll + views - 1) % views;
    } else if (x > W - 90) {
      drawButton(W - 72, 208, 64, 28, ">", C_ACCENT, C_ON_ACCENT);
      overlayScroll = (overlayScroll + 1) % views;
    } else {
      return;
    }
    delay(PRESS_MS);
    drawOverlay();
    return;
  }
  if (y < LIST_TOP) return;
  int row = (y - LIST_TOP) / LIST_ROW_H;
  if (row >= LIST_ROWS) return;
  int i = overlayScroll * PICKER_PER_VIEW + (x < 160 ? row : LIST_ROWS + row);
  if (i >= PAGE_COUNT) return;
  uint32_t bit = 1UL << PAGE_ORDER[i];
  if ((cfg.pages & bit) && (cfg.pages & ~bit) == 0) return;  // keep at least one screen on
  cfg.pages ^= bit;
  prefs.putULong("pages", cfg.pages);
  drawOverlay();
}

// ---------------------------------------------------------------- phone settings

void drawPhoneScreen() {
  drawOverlayTop("Phone settings", "Back", false);
  String url = "http://" + WiFi.localIP().toString();
  tft.fillSmoothRoundRect(10, 44, 136, 136, 8, C_WHITE);  // QR codes need a light background
  tft.qrcode(url.c_str(), 14, 48, 128, 3, true);
  tft.setFont(F_BOLD);
  tft.setTextSize(1);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_WHITE);
  tft.drawString("Scan with your phone", 156, 50);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  drawWrapped("Or open this on a phone or computer on the same WiFi:", 156, 74, W - 166, 3, 16);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(url, W - 166).c_str(), 156, 112);
  tft.setTextColor(C_GREY);
  drawWrapped("Everything else is there: your channel, WiFi, night hours, the pet, themes and more.", 156, 136,
              W - 166, 5, 16);
}

void phoneTap(int x, int y) {
  if (topBarButton(x, y) == 1) openSettings();
}

// ---------------------------------------------------------------- taps

// side: -1 or 1 for a tap on the 7" strips, 0 on the page
void overlayTap(int x, int y, int side) {
  overlayTouchedAt = millis();
  lastInteraction = millis();
  if (side) {
    // The Home and Settings buttons open their screen, or shut it if it's already open
    if (y < SIDE_BUTTON_Y) return;
    pressSide(side, true);
    if (side < 0) {
      if (overlay == OVERLAY_HOME) closeOverlay();
      else openHome();
    } else {
      if (overlay == OVERLAY_SETTINGS) closeOverlay();
      else openSettings();
    }
    return;
  }
  switch (overlay) {
    case OVERLAY_HOME:     homeTap(x, y); break;
    case OVERLAY_SETTINGS: settingsTap(x, y); break;
    case OVERLAY_PAGES:    pagePickerTap(x, y); break;
    case OVERLAY_PHONE:    phoneTap(x, y); break;
    default: break;
  }
}
