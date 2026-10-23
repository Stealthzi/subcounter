// Fun pages: magic 8-ball, stopwatch, free Epic games, screensavers (aquarium
// and fireplace), mood lamp, joke or fact of the day, and seasonal themes.

// ---------------------------------------------------------------- magic 8-ball

const char* EIGHT_BALL[] = {
  "It is certain", "Without a doubt", "Yes, definitely", "You may rely on it", "As I see it, yes",
  "Most likely", "Outlook good", "Yes", "Signs point to yes", "Reply hazy, try again",
  "Ask again later", "Better not tell you now", "Cannot predict now", "Concentrate and ask again",
  "Don't count on it", "My reply is no", "My sources say no", "Outlook not so good", "Very doubtful",
  "Absolutely!"};
int eightState = 0;        // 0 waiting, 1 shaking, 2 showing an answer
int eightShake = 0, eightAnswer = 0;

void drawEightBall(int dx, bool showAnswer) {
  int cx = W / 2 + dx, cy = 128;
  fillBg(0, 36, W, 190);
  tft.fillSmoothCircle(cx, cy, 80, RGB565(14, 14, 22));
  tft.fillSmoothCircle(cx - 30, cy - 34, 14, RGB565(44, 44, 58));  // shine
  if (!showAnswer) {
    tft.fillSmoothCircle(cx, cy, 34, C_WHITE);
    tft.setFont(F_HUGE);
    tft.setTextSize(1);
    tft.setTextDatum(D_MC);
    tft.setTextColor(RGB565(14, 14, 22));
    tft.drawString("8", cx, cy + 2);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString("Ask a question, then tap the ball", W / 2, 212);
    return;
  }
  tft.fillSmoothCircle(cx, cy, 46, RGB565(8, 14, 48));
  tft.fillTriangle(cx - 40, cy - 22, cx + 40, cy - 22, cx, cy + 44, RGB565(46, 91, 255));
  tft.setFont(F_SMALL);
  tft.setTextColor(C_WHITE);
  // Up to 3 short lines inside the triangle
  String a = EIGHT_BALL[eightAnswer];
  String lines[3];
  int n = 0;
  while (a.length() && n < 3) {
    int cut = a.length();
    int maxW = n == 0 ? 74 : n == 1 ? 56 : 36;
    while (tft.textWidth(a.substring(0, cut).c_str()) > maxW) {
      int sp = a.lastIndexOf(' ', cut - 1);
      if (sp <= 0) break;
      cut = sp;
    }
    lines[n++] = a.substring(0, cut);
    a = a.substring(cut);
    a.trim();
  }
  tft.setTextDatum(D_MC);
  for (int i = 0; i < n; i++) tft.drawString(lines[i].c_str(), cx, cy - 12 + i * 15);
}

void drawEightBallPage() {
  drawEightBall(0, eightState == 2);
}

bool eightBallTap(int x, int y) {
  if (y < 40 || y > 215) return false;
  eightState = 1;
  eightShake = 0;
  eightAnswer = esp_random() % 20;
  return true;
}

void eightBallFrame() {
  if (eightState != 1) return;
  eightShake++;
  drawEightBall((eightShake % 2 ? 1 : -1) * (14 - eightShake), false);
  if (eightShake >= 14) {
    eightState = 2;
    drawEightBall(0, true);
  }
}

// ---------------------------------------------------------------- stopwatch

bool          swRunning = false;
unsigned long swStart = 0, swBanked = 0;
unsigned long swLaps[3];
int           swLapCount = 0;

bool stopwatchRunning() {
  return swRunning;
}

unsigned long swElapsed() {
  return swBanked + (swRunning ? millis() - swStart : 0);
}

String swText(unsigned long ms) {
  char buf[20];
  unsigned long s = ms / 1000;
  if (s >= 3600) snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
  else snprintf(buf, sizeof(buf), "%02lu:%02lu.%lu", s / 60, s % 60, (ms / 100) % 10);
  return String(buf);
}

void drawStopwatchTime() {
  if (!ensureCanvas(W, 80)) return;
  for (int y = 0; y < 80; y++) canvas.drawFastHLine(0, y, W, bgAt(48 + y));
  canvas.setFont(F_HUGE);
  canvas.setTextSize(1.5f);
  canvas.setTextDatum(D_MC);
  canvas.setTextColor(swRunning ? C_ACCENT : C_WHITE);
  canvas.drawString(swText(swElapsed()).c_str(), W / 2, 42);
  canvas.setTextSize(1);
  canvas.pushSprite(0, 48);
}

void drawStopwatchButtons() {
  drawButton(4, 182, 152, 40, swRunning ? "Stop" : (swElapsed() ? "Resume" : "Start"),
             swRunning ? C_RED : C_GREEN, swRunning ? C_WHITE : RGB565(5, 30, 20));
  drawButton(164, 182, 152, 40, swRunning ? "Lap" : "Reset", C_CARD, C_WHITE);
}

void drawStopwatchLaps() {
  fillBg(0, 132, W, 46);
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  for (int i = 0; i < swLapCount && i < 3; i++) {
    int x = 16 + i * 102;
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_GREY);
    tft.drawString(("Lap " + String(swLapCount - i)).c_str(), x, 138);
    tft.setTextColor(C_WHITE);
    tft.drawString(swText(swLaps[i]).c_str(), x, 156);
  }
}

void drawStopwatchPage() {
  canvas.deleteSprite();
  drawStopwatchTime();
  drawStopwatchLaps();
  drawStopwatchButtons();
}

void stopwatchFrame() {
  if (swRunning) drawStopwatchTime();
}

bool stopwatchTap(int x, int y) {
  if (y < 176) return false;
  if (x < W / 2) {
    if (swRunning) {
      swBanked += millis() - swStart;
      swRunning = false;
    } else {
      swStart = millis();
      swRunning = true;
    }
  } else if (swRunning) {
    for (int i = 2; i > 0; i--) swLaps[i] = swLaps[i - 1];
    swLaps[0] = swElapsed();
    swLapCount++;
    drawStopwatchLaps();
  } else {
    swBanked = 0;
    swLapCount = 0;
    drawStopwatchLaps();
  }
  drawStopwatchTime();
  drawStopwatchButtons();
  return true;
}

// ---------------------------------------------------------------- free Epic games

bool updateGames() {
  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.useHTTP10(true);  // no chunked replies, so it can be read as a stream
  http.setUserAgent("SubCounter/" FIRMWARE_VERSION);
  if (!http.begin(tls, "https://store-site-backend-static-ipv4.ak.epicgames.com/freeGamesPromotions?locale=en-GB&country=GB&allowCountries=GB")) return false;
  if (http.GET() != 200) {
    http.end();
    return false;
  }
  // The reply is big, so only the titles and free-offer dates are kept
  JsonDocument filter;
  JsonObject el = filter["data"]["Catalog"]["searchStore"]["elements"].add<JsonObject>();
  el["title"] = true;
  const char* kinds[] = {"promotionalOffers", "upcomingPromotionalOffers"};
  for (int k = 0; k < 2; k++) {
    JsonObject o = el["promotions"][kinds[k]].add<JsonObject>()["promotionalOffers"].add<JsonObject>();
    o["startDate"] = true;
    o["endDate"] = true;
    o["discountSetting"]["discountPercentage"] = true;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();
  if (err) return false;

  games.nowCount = games.nextCount = 0;
  time_t now = time(nullptr);
  for (JsonVariant e : doc["data"]["Catalog"]["searchStore"]["elements"].as<JsonArray>()) {
    for (int k = 0; k < 2; k++) {
      for (JsonVariant group : e["promotions"][kinds[k]].as<JsonArray>()) {
        for (JsonVariant offer : group["promotionalOffers"].as<JsonArray>()) {
          if ((offer["discountSetting"]["discountPercentage"] | 100) != 0) continue;
          time_t from = parseIsoTime(offer["startDate"] | "");
          time_t until = parseIsoTime(offer["endDate"] | "");
          bool current = from <= now && until > now;
          EpicGame* list = current ? games.now : games.next;
          int& count = current ? games.nowCount : games.nextCount;
          if (count >= EPIC_MAX || until <= now) continue;
          list[count].title = cleanText(e["title"] | "");
          list[count].from = from;
          list[count].until = until;
          count++;
        }
      }
    }
  }
  games.loaded = true;
  return true;
}

String dayAndDate(time_t t) {
  struct tm d;
  localtime_r(&t, &d);
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return String(days[d.tm_wday]) + " " + String(d.tm_mday) + " " + months[d.tm_mon];
}

void drawGamesPage() {
  int y = 42;
  if (games.nowCount) {
    drawLabel(12, y, "FREE NOW ON EPIC", C_ACCENT, "");
    y += 20;
  }
  for (int i = 0; i < games.nowCount && y < 170; i++) {
    drawCardBox(8, y, W - 16, 44);
    tft.setTextSize(1);
    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TL);
    gradientText(fitText(games.now[i].title, W - 40), 18, y + 6, false, 0xFFFFFF, theme().accent);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString(("Free until " + dayAndDate(games.now[i].until)).c_str(), 18, y + 26);
    y += 50;
  }
  if (games.nextCount && y < 190) {
    drawLabel(12, y + 2, "COMING NEXT", C_PURPLE, "");
    y += 22;
    for (int i = 0; i < games.nextCount && y < 214; i++) {
      tft.setFont(F_SMALL);
      tft.setTextDatum(D_TL);
      tft.setTextColor(C_WHITE);
      tft.drawString(fitText(games.next[i].title, 200).c_str(), 14, y);
      tft.setTextDatum(D_TR);
      tft.setTextColor(C_GREY);
      tft.drawString(dayAndDate(games.next[i].from).c_str(), W - 14, y);
      y += 18;
    }
  }
}

// ---------------------------------------------------------------- joke or fact of the day

bool updateJoke() {
  uint32_t today = localDay();
  if (joke.loaded && joke.day == today) return true;
  bool fact = cfg.jokeType == 1 || (cfg.jokeType == 2 && today % 2);
  String body;
  int code;
  if (fact) {
    code = httpGet("https://uselessfacts.jsph.pl/api/v2/facts/today?language=en", body);
  } else {
    tls.setInsecure();
    HTTPClient http;
    http.setTimeout(10000);
    http.setUserAgent("SubCounter desk display");
    if (!http.begin(tls, "https://icanhazdadjoke.com/")) return false;
    http.addHeader("Accept", "application/json");
    code = http.GET();
    if (code == 200) body = http.getString();
    http.end();
  }
  if (code != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  joke.text = cleanText(fact ? (doc["text"] | "") : (doc["joke"] | ""));
  joke.isFact = fact;
  joke.day = today;
  joke.loaded = joke.text.length() > 0;
  return joke.loaded;
}

void drawJokePage() {
  drawCardBox(8, 42, W - 16, 176);
  tft.setTextSize(1);
  tft.setFont(F_LARGE);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GLOW);
  tft.drawString(joke.isFact ? "?" : "\"", 20, 48);
  tft.setFont(joke.text.length() > 110 ? F_BODY : F_BOLD);
  tft.setTextColor(C_WHITE);
  drawWrapped(joke.text, 24, 84, W - 48, 6, 22);
}

// ---------------------------------------------------------------- screensavers

struct Fish {
  float x, y, speed;
  int dir, size;
  uint16_t colour;
};
Fish fish[6];
float bubbles[8][2];
bool fishReady = false;
uint8_t* fireHeat = nullptr;  // 80 x 60 heat map for the fireplace
const int FIRE_W = 80, FIRE_H = 60;
bool showFireplace = false;

void setupAquarium() {
  const uint16_t colours[] = {C_AMBER, C_ACCENT, C_PURPLE, RGB565(255, 110, 160), C_YELLOW, C_BLUE};
  for (int i = 0; i < 6; i++) {
    fish[i].x = esp_random() % W;
    fish[i].y = 50 + esp_random() % 140;
    fish[i].dir = (esp_random() & 1) ? 1 : -1;
    fish[i].speed = 0.6f + (esp_random() % 100) / 70.0f;
    fish[i].size = 9 + esp_random() % 8;
    fish[i].colour = colours[i];
  }
  for (int i = 0; i < 8; i++) {
    bubbles[i][0] = esp_random() % W;
    bubbles[i][1] = esp_random() % H;
  }
  fishReady = true;
}

void aquariumFrame() {
  if (!fishReady) setupAquarium();
  float t = millis() / 1000.0f;
  for (int y = 0; y < H; y++) canvas.drawFastHLine(0, y, W, mix(0x05254A, 0x01081A, (float)y / H));
  // light rays
  for (int i = 0; i < 4; i++) {
    int x = 40 + i * 80 + (int)(sin(t * 0.3f + i) * 20);
    canvas.fillTriangle(x, 0, x + 30, 0, x + 70, H, RGB565(10, 50, 90));
  }
  // sand and seaweed
  for (int x = 0; x < W; x++) {
    int h = 14 + (int)(sin(x * 0.05f) * 4);
    canvas.drawFastVLine(x, H - h, h, RGB565(194, 160, 100));
  }
  for (int k = 0; k < 5; k++) {
    int baseX = 20 + k * 68;
    for (int j = 0; j < 12; j++) {
      int x = baseX + (int)(sin(t * 1.5f + j * 0.5f + k) * (j * 0.8f));
      canvas.fillCircle(x, H - 14 - j * 7, 4, RGB565(30, 150 + k * 10, 90));
    }
  }
  // bubbles
  for (int i = 0; i < 8; i++) {
    bubbles[i][1] -= 1.2f + (i % 3) * 0.4f;
    if (bubbles[i][1] < -6) {
      bubbles[i][1] = H;
      bubbles[i][0] = esp_random() % W;
    }
    canvas.drawCircle((int)(bubbles[i][0] + sin(t * 3 + i) * 3), (int)bubbles[i][1], 2 + i % 3, RGB565(150, 200, 255));
  }
  // fish
  for (int i = 0; i < 6; i++) {
    Fish& f = fish[i];
    f.x += f.speed * f.dir;
    if (f.x > W + 30) f.dir = -1;
    if (f.x < -30) f.dir = 1;
    int y = (int)(f.y + sin(t * 1.3f + i) * 6);
    int x = (int)f.x, s = f.size;
    canvas.fillEllipse(x, y, s, s * 6 / 10, f.colour);
    canvas.fillTriangle(x - f.dir * s, y, x - f.dir * (s + s * 7 / 10), y - s / 2, x - f.dir * (s + s * 7 / 10), y + s / 2, f.colour);
    canvas.fillCircle(x + f.dir * s / 2, y - s / 6, 2, C_WHITE);
    canvas.drawPixel(x + f.dir * s / 2, y - s / 6, RGB565(0, 0, 0));
  }
}

uint16_t fireColour(uint8_t h) {
  if (h < 64) return mix(0x000000, 0x8A0A00, h / 64.0f);
  if (h < 128) return mix(0x8A0A00, 0xFF5A00, (h - 64) / 64.0f);
  if (h < 192) return mix(0xFF5A00, 0xFFC830, (h - 128) / 64.0f);
  return mix(0xFFC830, 0xFFF6D0, (h - 192) / 64.0f);
}

void fireplaceFrame() {
  if (!fireHeat) {
    fireHeat = (uint8_t*)calloc(FIRE_W * FIRE_H, 1);
    if (!fireHeat) return;
  }
  uint8_t* f = fireHeat;
  // fresh embers along the bottom
  for (int x = 0; x < FIRE_W; x++) f[(FIRE_H - 1) * FIRE_W + x] = 160 + (esp_random() % 96);
  // heat rises and cools
  for (int y = 0; y < FIRE_H - 1; y++) {
    for (int x = 0; x < FIRE_W; x++) {
      int below = (y + 1) * FIRE_W;
      int sum = f[below + (x + FIRE_W - 1) % FIRE_W] + f[below + x] + f[below + (x + 1) % FIRE_W] +
                f[min(FIRE_H - 1, y + 2) * FIRE_W + x];
      int v = sum * 16 / 66 - (esp_random() % 3);
      f[y * FIRE_W + x] = v < 0 ? 0 : v;
    }
  }
  canvas.fillScreen(RGB565(10, 6, 6));
  // the fire, inside a brick fireplace
  for (int y = 0; y < FIRE_H; y++) {
    for (int x = 0; x < FIRE_W; x++) {
      uint8_t h = f[y * FIRE_W + x];
      if (h > 8) canvas.fillRect(40 + x * 3, 50 + y * 3, 3, 3, fireColour(h));
    }
  }
  uint16_t brick = RGB565(110, 46, 32), mortar = RGB565(60, 26, 20);
  canvas.fillRect(0, 0, W, 40, mortar);
  canvas.fillRect(0, 40, 36, H - 40, mortar);
  canvas.fillRect(W - 36, 40, 36, H - 40, mortar);
  for (int row = 0; row < 12; row++) {
    int y = row * 20;
    for (int col = -1; col < 9; col++) {
      int x = col * 40 + (row % 2) * 20;
      if (y < 40 || x < 36 || x > W - 76) canvas.fillRect(x + 2, y + 2, 36, 16, brick);
    }
  }
  canvas.fillRect(30, 228, W - 60, 12, RGB565(40, 40, 46));  // hearth
}

void screensaverFrame() {
  if (!ensureCanvas(W, H)) return;
  if (showFireplace) fireplaceFrame();
  else aquariumFrame();
  canvas.pushSprite(0, 0);
}

void startScreensaver() {
  static bool alternate = false;
  if (cfg.screensaver == 2) alternate = !alternate;
  showFireplace = cfg.screensaver == 1 || (cfg.screensaver == 2 && alternate);
  if (!showFireplace && fireHeat) {
    free(fireHeat);
    fireHeat = nullptr;
  }
  screensaverFrame();
}

// ---------------------------------------------------------------- mood lamp

unsigned long moodLabelUntil = 0;
int moodMode = 0;  // 0 rainbow, 1 warm, 2 cool, 3 purple glow, 4 your lamp colour
const char* MOOD_NAMES[] = {"Rainbow", "Warm", "Cool", "Purple glow", "Lamp colour"};

uint32_t hsvToRgb(float h, float s, float v) {
  float c = v * s, x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1)), m = v - c;
  float r, g, b;
  if (h < 60)       { r = c; g = x; b = 0; }
  else if (h < 120) { r = x; g = c; b = 0; }
  else if (h < 180) { r = 0; g = c; b = x; }
  else if (h < 240) { r = 0; g = x; b = c; }
  else if (h < 300) { r = x; g = 0; b = c; }
  else              { r = c; g = 0; b = x; }
  return ((uint32_t)((r + m) * 255) << 16) | ((uint32_t)((g + m) * 255) << 8) | (uint32_t)((b + m) * 255);
}

void moodLampFrame() {
  float t = millis() / 1000.0f;
  uint32_t c;
  switch (moodMode) {
    case 0: c = hsvToRgb(fmod(t * 12, 360), 0.85f, 1.0f); break;
    case 1: c = hsvToRgb(25 + sin(t * 0.4f) * 10, 0.8f, 0.9f + sin(t * 0.7f) * 0.1f); break;
    case 2: c = hsvToRgb(200 + sin(t * 0.3f) * 20, 0.7f, 0.9f); break;
    case 3: c = hsvToRgb(270 + sin(t * 0.5f) * 15, 0.75f, 0.75f + sin(t * 0.8f) * 0.25f); break;
    default: c = cfg.lampColour; break;
  }
  tft.fillScreen(to565(c));
  drawNavArrowsIn(mix(c, 0x000000, 0.55f), mix(c, 0xFFFFFF, 0.25f));
  setLed((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
  if (millis() < moodLabelUntil) {
    tft.setFont(F_BOLD);
    tft.setTextSize(1);
    tft.setTextDatum(D_MC);
    tft.setTextColor(mix(c, 0x000000, 0.6f));
    tft.drawString((String(MOOD_NAMES[moodMode]) + (BOARD_7INCH ? " - tap to change" : " - tap the middle to change")).c_str(), W / 2, H - 20);
  }
}

// Tapping the middle changes the colour; the edges move on to the next page
bool moodLampTap(int x, int y) {
#if !BOARD_7INCH
  if (x < W / 4 || x >= W * 3 / 4) return false;  // the edges move on a page (no arrows on this screen)
#endif
  moodMode = (moodMode + 1) % 5;
  moodLabelUntil = millis() + 2500;
  return true;
}

// ---------------------------------------------------------------- seasonal themes

// 0 none, 1 Christmas, 2 Halloween, 3 New Year, 4 Valentine's, 5 Easter
int season() {
  if (!cfg.themes) return 0;
  struct tm t;
  if (!getLocalTime(&t, 0)) return 0;
  int m = t.tm_mon + 1, d = t.tm_mday, y = t.tm_year + 1900;
  if (m == 12 && d <= 26) return 1;
  if (m == 10 && d >= 24) return 2;
  if ((m == 12 && d == 31) || (m == 1 && d == 1)) return 3;
  if (m == 2 && d == 14) return 4;
  // Easter Sunday (anonymous Gregorian method), Good Friday to Easter Monday
  int a = y % 19, b = y / 100, c = y % 100, dd = b / 4, e = b % 4, f = (b + 8) / 25, g = (b - f + 1) / 3;
  int h = (19 * a + b - dd - g + 15) % 30, i = c / 4, k = c % 4, l = (32 + 2 * e + 2 * i - h - k) % 7;
  int mm = (a + 11 * h + 22 * l) / 451;
  int em = (h + l - 7 * mm + 114) / 31, ed = ((h + l - 7 * mm + 114) % 31) + 1;
  long easter = daysFromCivil(y, em, ed), today = daysFromCivil(y, m, d);
  if (today >= easter - 2 && today <= easter + 1) return 5;
  return 0;
}

// A string of little lights along the header line
void drawSeasonalHeader() {
  int s = season();
  if (!s) return;
  const uint16_t sets[][3] = {
    {0, 0, 0},
    {C_RED, C_GREEN, C_YELLOW},
    {C_AMBER, C_PURPLE, C_GREEN},
    {C_YELLOW, C_WHITE, C_ACCENT},
    {RGB565(255, 90, 140), C_WHITE, C_RED},
    {RGB565(255, 214, 120), RGB565(170, 220, 255), RGB565(200, 170, 255)},
  };
  for (int x = 6, i = 0; x < W; x += 16, i++) {
    tft.fillSmoothCircle(x, 33, 2, sets[s][i % 3]);
  }
}

// Seasonal hats for the pet
void drawSeasonalPet(int px, int py, int s) {
  int sn = season();
  int cx = px + 8 * s, top = py + (pet.stage == PET_BABY ? 5 : pet.stage >= PET_TEEN ? 1 : 2) * s;
  if (sn == 1) {         // Santa hat
    canvas.fillTriangle(cx - 5 * s, top + s, cx + 5 * s, top + s, cx + 3 * s, top - 5 * s, C_RED);
    canvas.fillRect(cx - 5 * s, top, 10 * s, 2 * s, C_WHITE);
    canvas.fillCircle(cx + 3 * s, top - 5 * s, s, C_WHITE);
  } else if (sn == 2) {  // witch hat
    canvas.fillRect(cx - 7 * s, top + s, 14 * s, s, RGB565(20, 16, 30));
    canvas.fillTriangle(cx - 4 * s, top + s, cx + 4 * s, top + s, cx + s, top - 7 * s, RGB565(20, 16, 30));
    canvas.fillRect(cx - 4 * s, top - s, 8 * s, s, C_PURPLE);
  } else if (sn == 3) {  // party hat
    canvas.fillTriangle(cx - 3 * s, top + s, cx + 3 * s, top + s, cx, top - 6 * s, C_YELLOW);
    canvas.fillCircle(cx, top - 6 * s, s, C_ACCENT);
  } else if (sn == 4) {  // floating heart
    canvas.fillCircle(cx - 4, top - 18, 5, RGB565(255, 90, 140));
    canvas.fillCircle(cx + 4, top - 18, 5, RGB565(255, 90, 140));
    canvas.fillTriangle(cx - 9, top - 16, cx + 9, top - 16, cx, top - 6, RGB565(255, 90, 140));
  } else if (sn == 5) {  // bunny ears
    canvas.fillEllipse(cx - 3 * s, top - 4 * s, s + 2, 4 * s, C_WHITE);
    canvas.fillEllipse(cx + 3 * s, top - 4 * s, s + 2, 4 * s, C_WHITE);
    canvas.fillEllipse(cx - 3 * s, top - 4 * s, s / 2 + 1, 3 * s, RGB565(255, 160, 190));
    canvas.fillEllipse(cx + 3 * s, top - 4 * s, s / 2 + 1, 3 * s, RGB565(255, 160, 190));
  }
}
