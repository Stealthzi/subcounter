// Home pages: alerts (frost, Met Office warnings, MOT/tax due), air quality
// and pollen, and countdowns.

// ---------------------------------------------------------------- frost

int localHour() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return 12;
  return t.tm_hour;
}

// 2 = frost likely, 1 = possible, 0 = none. Looks at tonight in the evening,
// and this morning before 10am.
int frostLevel(float& low, bool& tonight) {
  if (!weather.loaded) return 0;
  int h = localHour();
  tonight = h >= 15;
  if (!tonight && h >= 10) return 0;
  low = tonight ? weather.lo[1] : weather.lo[0];
  float lowC = cfg.fahrenheit ? (low - 32) * 5 / 9 : low;
  if (lowC <= 0.5f) return 2;
  if (lowC <= 3.0f) return 1;
  return 0;
}

// ---------------------------------------------------------------- Met Office warnings

const char* MET_REGIONS[][2] = {
  {"UK", "Whole UK"}, {"se", "London and South East"}, {"sw", "South West England"},
  {"ee", "East of England"}, {"em", "East Midlands"}, {"wm", "West Midlands"},
  {"nw", "North West England"}, {"ne", "North East England"}, {"yh", "Yorkshire and Humber"},
  {"wl", "Wales"}, {"ni", "Northern Ireland"}, {"dg", "Dumfries, Galloway, Lothian, Borders"},
  {"st", "Strathclyde"}, {"ta", "Central, Tayside and Fife"}, {"gr", "Grampian"},
  {"he", "Highlands and Eilean Siar"}, {"os", "Orkney and Shetland"},
};
const int MET_REGION_COUNT = sizeof(MET_REGIONS) / sizeof(MET_REGIONS[0]);

String xmlUnescape(String s) {
  s.replace("&amp;", "&");
  s.replace("&lt;", "<");
  s.replace("&gt;", ">");
  s.replace("&quot;", "\"");
  s.replace("&#39;", "'");
  s.replace("&apos;", "'");
  return s;
}

// Text between <tag> and </tag>, starting the search at from
String xmlTag(const String& xml, const String& tag, int from, int* end) {
  int a = xml.indexOf("<" + tag + ">", from);
  if (a < 0) return "";
  a += tag.length() + 2;
  int b = xml.indexOf("</" + tag + ">", a);
  if (b < 0) return "";
  if (end) *end = b;
  return xmlUnescape(xml.substring(a, b));
}

bool updateWarnings() {
  String body;
  String url = "https://www.metoffice.gov.uk/public/data/PWSCache/WarningsRSS/Region/" + cfg.metRegion;
  if (httpGet(url, body) != 200) return false;
  warnings.count = 0;
  int pos = 0;
  while (warnings.count < WARN_MAX) {
    int item = body.indexOf("<item>", pos);
    if (item < 0) break;
    int end = 0;
    String title = xmlTag(body, "title", item, &end);
    String desc = xmlTag(body, "description", item, nullptr);
    pos = end > item ? end : item + 6;
    if (!title.length()) continue;
    int v = desc.indexOf("valid from");
    warnings.title[warnings.count] = cleanText(title);
    warnings.when[warnings.count] = v >= 0 ? cleanText(desc.substring(v)) : String("");
    warnings.count++;
  }
  warnings.loaded = true;
  return true;
}

uint16_t warningColour(const String& title) {
  if (title.startsWith("Red")) return C_RED;
  if (title.startsWith("Amber")) return C_AMBER;
  return C_YELLOW;
}

// ---------------------------------------------------------------- alerts page

// Car dates due within two weeks also count as alerts
int carAlerts(Countdown out[], int limit) {
  Countdown all[COUNTDOWN_MAX];
  int n = buildCountdowns(all, COUNTDOWN_MAX);
  int k = 0;
  uint32_t today = localDay();
  for (int i = 0; i < n && k < limit; i++) {
    if (all[i].car && (int)all[i].day - (int)today <= 14) out[k++] = all[i];
  }
  return k;
}

int alertCount() {
  float low;
  bool tonight;
  Countdown car[2];
  return (frostLevel(low, tonight) ? 1 : 0) + warnings.count + carAlerts(car, 2);
}

void drawAlertCard(int y, uint16_t colour, const String& title, const String& detail) {
  drawCardBox(8, y, W - 16, 52);
  tft.fillSmoothRoundRect(14, y + 8, 5, 36, 2, colour);
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(title, W - 50).c_str(), 28, y + 9);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(detail, W - 50).c_str(), 28, y + 30);
}

void drawAlertsPage() {
  int y = 42;
  float low;
  bool tonight;
  int frost = frostLevel(low, tonight);
  if (frost) {
    String title = String(frost == 2 ? "Frost " : "Possible frost ") + (tonight ? "tonight" : "this morning");
    String detail = String(tonight ? "Low of " : "It got down to ") + String((int)round(low)) +
                    (cfg.fahrenheit ? "F" : "C") + (tonight ? " - cover the windscreen" : " - allow time to de-ice");
    drawAlertCard(y, C_BLUE, title, detail);
    y += 60;
  }
  for (int i = 0; i < warnings.count && y < 200; i++) {
    drawAlertCard(y, warningColour(warnings.title[i]), warnings.title[i], warnings.when[i]);
    y += 60;
  }
  Countdown car[2];
  int n = carAlerts(car, 2);
  for (int i = 0; i < n && y < 200; i++) {
    int days = (int)car[i].day - (int)localDay();
    String when = days < 0 ? "Overdue by " + String(-days) + (days == -1 ? " day" : " days")
                : days == 0 ? String("Due today") : "Due in " + String(days) + (days == 1 ? " day" : " days");
    drawAlertCard(y, days <= 3 ? C_RED : C_AMBER, car[i].name + " due", when);
    y += 60;
  }
}

// ---------------------------------------------------------------- air quality and pollen

bool ensurePlace() {
  return !isnan(cfg.lat) || findPlace();
}

bool updateAir() {
  if (!ensurePlace()) return false;
  String url = "https://air-quality-api.open-meteo.com/v1/air-quality?latitude=" + String(cfg.lat, 3) +
               "&longitude=" + String(cfg.lon, 3) +
               "&current=european_aqi,pm2_5,uv_index,grass_pollen,birch_pollen,alder_pollen,"
               "mugwort_pollen,ragweed_pollen,olive_pollen";
  String body;
  if (httpGet(url, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  JsonVariant c = doc["current"];
  air.aqi     = c["european_aqi"] | 0;
  air.pm25    = c["pm2_5"] | 0.0f;
  air.uv      = c["uv_index"] | 0.0f;
  air.grass   = c["grass_pollen"].isNull() ? -1 : (float)(c["grass_pollen"] | 0.0f);
  air.birch   = c["birch_pollen"].isNull() ? -1 : (float)(c["birch_pollen"] | 0.0f);
  air.alder   = c["alder_pollen"].isNull() ? -1 : (float)(c["alder_pollen"] | 0.0f);
  air.mugwort = c["mugwort_pollen"].isNull() ? -1 : (float)(c["mugwort_pollen"] | 0.0f);
  air.ragweed = c["ragweed_pollen"].isNull() ? -1 : (float)(c["ragweed_pollen"] | 0.0f);
  air.olive   = c["olive_pollen"].isNull() ? -1 : (float)(c["olive_pollen"] | 0.0f);
  air.loaded = true;
  return true;
}

const char* aqiText(int aqi) {
  if (aqi <= 20) return "Good";
  if (aqi <= 40) return "Fair";
  if (aqi <= 60) return "Moderate";
  if (aqi <= 80) return "Poor";
  if (aqi <= 100) return "Very poor";
  return "Extremely poor";
}

uint16_t aqiColour(int aqi) {
  if (aqi <= 20) return C_GREEN;
  if (aqi <= 40) return C_ACCENT2;
  if (aqi <= 60) return C_YELLOW;
  if (aqi <= 80) return C_AMBER;
  return C_RED;
}

// 0 none, 1 low, 2 moderate, 3 high, 4 very high
int pollenLevel(float grains) {
  if (grains < 0.5f) return 0;
  if (grains <= 20) return 1;
  if (grains <= 50) return 2;
  if (grains <= 200) return 3;
  return 4;
}

void drawPollenRow(int y, const char* name, float grains) {
  const char* words[] = {"None", "Low", "Moderate", "High", "Very high"};
  const uint16_t colours[] = {C_DIM, C_GREEN, C_YELLOW, C_AMBER, C_RED};
  int lvl = grains < 0 ? 0 : pollenLevel(grains);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString(name, 168, y);
  for (int i = 0; i < 4; i++) {
    tft.fillSmoothRoundRect(226 + i * 14, y + 3, 11, 10, 3, i < lvl ? colours[lvl] : C_CARD);
  }
  tft.setTextDatum(D_TR);
  tft.setTextColor(lvl ? colours[lvl] : C_DIM);
  tft.drawString(grains < 0 ? "-" : words[lvl], W - 10, y);
}

void drawAirPage() {
  // Air quality gauge on the left
  int cx = 82, cy = 128, r = 58;
  float fraction = min(1.0f, air.aqi / 100.0f);
  tft.fillArc(cx, cy, r, r - 12, 135, 405, C_CARD);
  uint16_t col = aqiColour(air.aqi);
  if (fraction > 0) tft.fillArc(cx, cy, r, r - 12, 135, 135 + 270 * fraction, col);
  drawBigNumber(String(air.aqi), cx, cy - 6, 70, 1.1f, C_WHITE);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_TC);
  tft.setTextColor(col);
  tft.drawString(aqiText(air.aqi), cx, cy + 18);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString("Air quality", cx, cy + 40);

  // Pollen on the right
  drawLabel(168, 44, "POLLEN TODAY", C_ACCENT, "");
  float trees = max(air.birch, air.alder);
  float weeds = max(air.mugwort, air.ragweed);
  drawPollenRow(70, "Grass", air.grass);
  drawPollenRow(96, "Trees", trees);
  drawPollenRow(122, "Weeds", weeds);

  tft.drawFastHLine(168, 150, W - 178, C_LINE);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString("UV index", 168, 160);
  tft.drawString("PM2.5", 168, 184);
  tft.setTextDatum(D_TR);
  tft.setTextColor(air.uv >= 6 ? C_AMBER : C_WHITE);
  tft.drawString(String(air.uv, 1).c_str(), W - 10, 160);
  tft.setTextColor(C_WHITE);
  tft.drawString((String(air.pm25, 1) + " ug/m3").c_str(), W - 10, 184);
}

// ---------------------------------------------------------------- countdowns

// "2026-12-20" -> that day; "03-02" -> the next 2nd of March (yearly)
uint32_t parseCountdownDate(String s) {
  s.trim();
  int y, m, d;
  struct tm t;
  if (!getLocalTime(&t, 0)) return 0;
  uint32_t today = localDay();
  if (sscanf(s.c_str(), "%d-%d-%d", &y, &m, &d) == 3 && y > 1900) {
    return (uint32_t)daysFromCivil(y, m, d);
  }
  if (sscanf(s.c_str(), "%d-%d", &m, &d) == 2 && m >= 1 && m <= 12) {
    uint32_t day = (uint32_t)daysFromCivil(t.tm_year + 1900, m, d);
    if (day < today) day = (uint32_t)daysFromCivil(t.tm_year + 1901, m, d);
    return day;
  }
  return 0;
}

// Upcoming countdowns, soonest first (car dates are kept for a week after they pass)
int buildCountdowns(Countdown out[], int limit) {
  int n = 0;
  uint32_t today = localDay();
  if (today == 0) return 0;
  auto add = [&](const String& name, const String& date, bool car) {
    if (n >= limit || !date.length()) return;
    uint32_t day = parseCountdownDate(date);
    if (!day) return;
    if (day < today && !(car && today - day <= 7)) return;
    out[n].name = name;
    out[n].day = day;
    out[n].car = car;
    n++;
  };
  add("MOT", cfg.motDate, true);
  add("Car tax", cfg.taxDate, true);
  String list = cfg.countdowns;
  while (list.length() && n < limit) {
    int semi = list.indexOf(';');
    String item = semi < 0 ? list : list.substring(0, semi);
    list = semi < 0 ? "" : list.substring(semi + 1);
    int eq = item.indexOf('=');
    if (eq > 0) add(cleanText(item.substring(0, eq)), item.substring(eq + 1), false);
  }
  // Sort by date
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (out[j].day < out[i].day) {
        Countdown tmp = out[i];
        out[i] = out[j];
        out[j] = tmp;
      }
    }
  }
  return n;
}

int countdownCount() {
  Countdown all[COUNTDOWN_MAX];
  return buildCountdowns(all, COUNTDOWN_MAX);
}

String shortDate(uint32_t day) {
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  time_t t = (time_t)day * 86400 + 12 * 3600;
  struct tm tmv;
  gmtime_r(&t, &tmv);
  return String(tmv.tm_mday) + " " + months[tmv.tm_mon] + " " + String(tmv.tm_year + 1900);
}

void drawCountdownPage() {
  Countdown all[COUNTDOWN_MAX];
  int n = buildCountdowns(all, COUNTDOWN_MAX);
  uint32_t today = localDay();
  for (int i = 0; i < n && i < 4; i++) {
    int y = 42 + i * 46;
    int days = (int)all[i].day - (int)today;
    drawCardBox(8, y, W - 16, 40);
    String num = days < 0 ? String(-days) : String(days);
    uint16_t col = all[i].car && days <= 14 ? (days <= 3 ? C_RED : C_AMBER) : C_WHITE;
    tft.setFont(F_MED);
    tft.setTextSize(1);
    if (col == C_WHITE) {
      gradientText(num, 20, y + 9, false, theme().accent, theme().accent2);
    } else {
      tft.setTextDatum(D_TL);
      tft.setTextColor(col);
      tft.drawString(num.c_str(), 20, y + 9);
    }
    int numW = tft.textWidth(num.c_str());
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_GREY);
    tft.drawString(days < 0 ? "days ago" : days == 1 ? "day" : "days", 24 + numW, y + 15);

    tft.setFont(F_BOLD);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(all[i].name, 150).c_str(), W - 18, y + 5);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString(days == 0 ? "Today!" : shortDate(all[i].day).c_str(), W - 18, y + 23);
  }
}
