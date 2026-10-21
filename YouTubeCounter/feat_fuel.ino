// Cheapest fuel near you, with a price trend so you know whether to fill up
// now or wait. Prices come from checkfuelprices.co.uk (free, no account),
// which only gets your town's rough position (rounded to about 1 km).

void loadFuelHistory() {
  memset(fuelHistory, 0, sizeof(fuelHistory));
  if (prefs.getBytesLength("fuelh") == sizeof(fuelHistory)) prefs.getBytes("fuelh", fuelHistory, sizeof(fuelHistory));
}

// "TESCO EXTRA" -> "Tesco Extra"
String titleCase(String s) {
  s.toLowerCase();
  bool start = true;
  for (size_t i = 0; i < s.length(); i++) {
    if (start && isalpha(s[i])) s.setCharAt(i, toupper(s[i]));
    start = s[i] == ' ' || s[i] == '-' || s[i] == '(';
  }
  return s;
}

bool updateFuel() {
  if (!ensurePlace()) return false;
  String url = "https://checkfuelprices.co.uk/api/widget/stations?lat=" + String(cfg.lat, 2) +
               "&lng=" + String(cfg.lon, 2) + "&fuel=" + cfg.fuelType + "&radius=" + String(cfg.fuelRadius);
  String body;
  if (httpGet(url, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body) || !(doc["success"] | false)) return false;

  fuel.avg = doc["area_stats"]["avg"] | 0.0f;
  fuel.min = doc["area_stats"]["min"] | 0.0f;
  fuel.areaCount = doc["area_stats"]["count"] | 0;

  // The 3 cheapest of the nearest stations
  FuelStation found[10];
  int n = 0;
  for (JsonVariant s : doc["stations"].as<JsonArray>()) {
    if (n >= 10) break;
    float price = s["price"] | 0.0f;
    if (price <= 0) continue;
    found[n].brand = titleCase(cleanText(s["brand"] | ""));
    String addr = cleanText(s["address"] | "");
    int comma = addr.indexOf(',');
    found[n].place = titleCase(comma > 0 ? addr.substring(0, comma) : addr);
    found[n].price = price;
    found[n].miles = s["distance"] | 0.0f;
    n++;
  }
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (found[j].price < found[i].price) {
        FuelStation t = found[i];
        found[i] = found[j];
        found[j] = t;
      }
    }
  }
  fuel.count = min(n, FUEL_SHOWN);
  for (int i = 0; i < fuel.count; i++) fuel.st[i] = found[i];
  fuel.loaded = fuel.count > 0;

  // Keep today's local average for the trend
  uint32_t today = localDay();
  if (today && fuel.avg > 0) {
    FuelDay& d = fuelHistory[today % FUEL_DAYS];
    d.day = today;
    d.avg = fuel.avg;
    d.min = fuel.min;
    prefs.putBytes("fuelh", fuelHistory, sizeof(fuelHistory));
  }
  return fuel.loaded;
}

// Change in the local average price (pence) over the last few days, or NAN if not enough history
float fuelTrend(int& overDays) {
  uint32_t today = localDay();
  const FuelDay& now = fuelHistory[today % FUEL_DAYS];
  if (now.day != today) return NAN;
  for (int back = 7; back >= 2; back--) {
    const FuelDay& then = fuelHistory[(today - back) % FUEL_DAYS];
    if (then.day == today - back && then.avg > 0) {
      overDays = back;
      return now.avg - then.avg;
    }
  }
  return NAN;
}

void drawTrendArrow(int cx, int cy, int dir, uint16_t colour) {
  if (dir == 0) {
    tft.fillSmoothRoundRect(cx - 12, cy - 3, 18, 6, 3, colour);
    tft.fillTriangle(cx + 6, cy - 9, cx + 6, cy + 9, cx + 15, cy, colour);
  } else if (dir < 0) {
    tft.fillSmoothRoundRect(cx - 3, cy - 14, 6, 18, 3, colour);
    tft.fillTriangle(cx - 10, cy + 2, cx + 10, cy + 2, cx, cy + 14, colour);
  } else {
    tft.fillSmoothRoundRect(cx - 3, cy - 4, 6, 18, 3, colour);
    tft.fillTriangle(cx - 10, cy - 2, cx + 10, cy - 2, cx, cy - 14, colour);
  }
}

void drawFuelPage() {
  const FuelStation& best = fuel.st[0];
  String fuelName = cfg.fuelType == "E10" ? "Unleaded" : cfg.fuelType == "E5" ? "Super unleaded"
                  : cfg.fuelType == "B7" ? "Diesel" : "Super diesel";
  String heading = "CHEAPEST " + fuelName;
  heading.toUpperCase();
  drawLabel(12, 42, heading, C_ACCENT, "within " + String(cfg.fuelRadius) + " miles");

  // Cheapest price, big
  tft.setFont(F_HUGE);
  tft.setTextSize(1.25f);
  String price = String(best.price, 1);
  gradientText(price, 12, 106, true, theme().accent, theme().accent2);
  int pw = tft.textWidth(price.c_str());
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextDatum(D_BL);
  tft.setTextColor(C_GREY);
  tft.drawString("p", 16 + pw, 104);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_WHITE);
  tft.drawString(fitText(best.brand, 180).c_str(), 12, 114);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(best.place + "  -  " + String(best.miles, 1) + " mi", 190).c_str(), 12, 134);

  // Trend card on the right
  drawCardBox(206, 58, 106, 92);
  int days = 0;
  float change = fuelTrend(days);
  tft.setTextDatum(D_TC);
  if (isnan(change)) {
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    tft.drawString("Learning", 259, 76);
    tft.drawString("local prices", 259, 94);
    tft.setTextColor(C_DIM);
    tft.drawString("trend in a", 259, 116);
    tft.drawString("few days", 259, 132);
  } else {
    int dir = fabs(change) < 0.5f ? 0 : change < 0 ? -1 : 1;
    uint16_t col = dir < 0 ? C_GREEN : dir > 0 ? C_AMBER : C_ACCENT2;
    drawTrendArrow(259, 82, dir, col);
    tft.setFont(F_BOLD);
    tft.setTextColor(col);
    tft.drawString(dir < 0 ? "Falling" : dir > 0 ? "Rising" : "Steady", 259, 102);
    tft.setFont(F_SMALL);
    tft.setTextColor(C_GREY);
    char buf[20];
    snprintf(buf, sizeof(buf), "%+.1fp in %dd", change, days);
    tft.drawString(buf, 259, 124);
  }

  // Advice line
  if (!isnan(change)) {
    int dir = fabs(change) < 0.5f ? 0 : change < 0 ? -1 : 1;
    String advice = dir < 0 ? "Prices are dropping - wait a day or two if you can"
                  : dir > 0 ? "Prices are going up - fill up soon"
                            : "Prices are steady - fill up when it suits you";
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TL);
    tft.setTextColor(dir > 0 ? C_AMBER : dir < 0 ? C_GREEN : C_GREY);
    tft.drawString(fitText(advice, W - 24).c_str(), 12, 158);
  }

  // Runners-up
  for (int i = 1; i < fuel.count; i++) {
    int y = 158 + i * 20;
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TL);
    tft.setTextColor(C_WHITE);
    tft.drawString(fitText(fuel.st[i].brand + ", " + fuel.st[i].place, 210).c_str(), 12, y);
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_GREY);
    tft.drawString((String(fuel.st[i].price, 1) + "p  " + String(fuel.st[i].miles, 1) + "mi").c_str(), W - 12, y);
  }
  tft.setTextDatum(D_TR);
  tft.setTextColor(C_DIM);
  tft.drawString(("Area avg " + String(fuel.avg, 1) + "p  -  Prices: checkfuelprices.co.uk").c_str(), W - 8, 214);
}
