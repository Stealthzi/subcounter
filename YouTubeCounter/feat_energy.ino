// Octopus Energy: your electricity (and gas) use and cost, from your smart
// meter readings. Needs your account number and API key from the Octopus
// website (Account > Personal details > API access). Both stay on the board.

const char* OCTO_API = "https://api.octopus.energy/v1/";

// Octopus uses your API key as the username, with no password
int octoGet(const String& url, String& body) {
  body = "";
  if (WiFi.status() != WL_CONNECTED) return -1;
  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.setUserAgent("SubCounter/" FIRMWARE_VERSION);
  if (!http.begin(tls, url)) return -2;
  http.setAuthorization(cfg.octoKey.c_str(), "");
  int code = http.GET();
  if (code > 0) body = http.getString();
  http.end();
  return code;
}

// The agreement in force now, from a meter point's list of agreements
String currentTariff(JsonVariant agreements) {
  time_t now = time(nullptr);
  String best;
  for (JsonVariant a : agreements.as<JsonArray>()) {
    time_t from = parseIsoTime(a["valid_from"] | "");
    const char* toText = a["valid_to"] | (const char*)nullptr;
    time_t to = toText ? parseIsoTime(toText) : 0;
    if (from <= now && (to == 0 || to > now)) best = a["tariff_code"] | "";
  }
  return best;
}

// "E-1R-VAR-22-11-01-D" -> product "VAR-22-11-01"
String productFromTariff(const String& tariff) {
  if (tariff.length() < 8) return "";
  return tariff.substring(5, tariff.length() - 2);
}

// The rate (pence, including VAT) in force now from a rates list
float currentRate(const String& url) {
  String body;
  if (octoGet(url, body) != 200) return -1;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return -1;
  time_t now = time(nullptr);
  for (JsonVariant r : doc["results"].as<JsonArray>()) {
    time_t from = parseIsoTime(r["valid_from"] | "");
    const char* toText = r["valid_to"] | (const char*)nullptr;
    time_t to = toText ? parseIsoTime(toText) : 0;
    if (from <= now && (to == 0 || to > now)) return r["value_inc_vat"] | 0.0f;
  }
  return -1;
}

void readRates(const String& tariff, bool gas, float& rate, float& standing) {
  String product = productFromTariff(tariff);
  String base = String(OCTO_API) + "products/" + product + (gas ? "/gas-tariffs/" : "/electricity-tariffs/") + tariff;
  rate = currentRate(base + "/standard-unit-rates/?page_size=5");
  if (rate < 0 && !gas) rate = currentRate(base + "/day-unit-rates/?page_size=5");  // Economy 7
  standing = currentRate(base + "/standing-charges/?page_size=5");
  if (rate < 0) rate = 0;
  if (standing < 0) standing = 0;
}

// Daily readings for the last week. Tries each meter on the point until one has data.
int readConsumption(bool gas, const String& point, JsonVariant meters, uint32_t days[], float kwh[]) {
  struct tm t;
  if (!getLocalTime(&t, 0)) return 0;
  time_t since = time(nullptr) - 9 * 86400;
  struct tm s;
  gmtime_r(&since, &s);
  char from[24];
  strftime(from, sizeof(from), "%Y-%m-%dT00:00Z", &s);

  for (JsonVariant m : meters.as<JsonArray>()) {
    String serial = m["serial_number"] | "";
    if (!serial.length()) continue;
    String url = String(OCTO_API) + (gas ? "gas-meter-points/" : "electricity-meter-points/") + point +
                 "/meters/" + urlEncode(serial) + "/consumption/?group_by=day&order_by=period&page_size=10&period_from=" + from;
    String body;
    if (octoGet(url, body) != 200) continue;
    JsonDocument doc;
    if (deserializeJson(doc, body)) continue;
    int n = 0;
    for (JsonVariant r : doc["results"].as<JsonArray>()) {
      int y, mo, d;
      if (sscanf(r["interval_start"] | "", "%d-%d-%d", &y, &mo, &d) != 3 || n >= ENERGY_DAYS) continue;
      days[n] = (uint32_t)daysFromCivil(y, mo, d);
      float value = r["consumption"] | 0.0f;
      // Most smart gas meters report cubic metres; convert to kWh
      if (gas && cfg.gasInCubicMetres) value *= 1.02264f * 39.5f / 3.6f;
      kwh[n++] = value;
    }
    if (n > 0) return n;
  }
  return 0;
}

bool updateEnergy() {
  String body;
  int code = octoGet(String(OCTO_API) + "accounts/" + urlEncode(cfg.octoAccount) + "/", body);
  if (code == 401 || code == 403) {
    energy.problem = "Octopus didn't accept the API key";
    return false;
  }
  if (code == 404) {
    energy.problem = "Octopus account number not found";
    return false;
  }
  if (code != 200) {
    energy.problem = "Couldn't reach Octopus";
    return false;
  }

  // Only keep the meter details - the rest of the account isn't needed
  JsonDocument filter;
  JsonObject fp = filter["properties"][0].to<JsonObject>();
  fp["moved_out_at"] = true;
  fp["electricity_meter_points"][0]["mpan"] = true;
  fp["electricity_meter_points"][0]["is_export"] = true;
  fp["electricity_meter_points"][0]["meters"][0]["serial_number"] = true;
  fp["electricity_meter_points"][0]["agreements"][0]["tariff_code"] = true;
  fp["electricity_meter_points"][0]["agreements"][0]["valid_from"] = true;
  fp["electricity_meter_points"][0]["agreements"][0]["valid_to"] = true;
  fp["gas_meter_points"][0]["mprn"] = true;
  fp["gas_meter_points"][0]["meters"][0]["serial_number"] = true;
  fp["gas_meter_points"][0]["agreements"][0]["tariff_code"] = true;
  fp["gas_meter_points"][0]["agreements"][0]["valid_from"] = true;
  fp["gas_meter_points"][0]["agreements"][0]["valid_to"] = true;
  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) {
    energy.problem = "Couldn't read the Octopus account";
    return false;
  }

  // The home you live in now
  JsonVariant home;
  for (JsonVariant p : doc["properties"].as<JsonArray>()) {
    if (p["moved_out_at"].isNull()) home = p;
  }
  if (home.isNull()) {
    energy.problem = "No current home on this Octopus account";
    return false;
  }

  JsonVariant elec;
  for (JsonVariant mp : home["electricity_meter_points"].as<JsonArray>()) {
    if (!(mp["is_export"] | false)) elec = mp;
  }
  JsonVariant gas = home["gas_meter_points"][0];

  uint32_t eDays[ENERGY_DAYS], gDays[ENERGY_DAYS];
  float eKwh[ENERGY_DAYS], gKwh[ENERGY_DAYS];
  int en = 0, gn = 0;
  if (!elec.isNull()) {
    readRates(currentTariff(elec["agreements"]), false, energy.elecRate, energy.elecStanding);
    en = readConsumption(false, elec["mpan"] | "", elec["meters"], eDays, eKwh);
  }
  energy.hasGas = !gas.isNull();
  if (energy.hasGas) {
    readRates(currentTariff(gas["agreements"]), true, energy.gasRate, energy.gasStanding);
    gn = readConsumption(true, gas["mprn"] | "", gas["meters"], gDays, gKwh);
  }

  // Line the electricity and gas days up
  energy.days = 0;
  for (int i = 0; i < en && energy.days < ENERGY_DAYS; i++) {
    int k = energy.days++;
    energy.day[k] = eDays[i];
    energy.elecKwh[k] = eKwh[i];
    energy.gasKwh[k] = 0;
    for (int j = 0; j < gn; j++) {
      if (gDays[j] == eDays[i]) energy.gasKwh[k] = gKwh[j];
    }
  }
  energy.problem = energy.days ? "" : "No smart meter readings yet - they can take a day or two";
  energy.loaded = true;
  return true;
}

// Cost of one day in pounds, including standing charges
float dayCost(int i) {
  float pence = energy.elecKwh[i] * energy.elecRate + energy.elecStanding;
  if (energy.hasGas) pence += energy.gasKwh[i] * energy.gasRate + energy.gasStanding;
  return pence / 100.0f;
}


void drawEnergyPage() {
  if (!energy.loaded) {
    drawLabel(12, 44, "OCTOPUS ENERGY", C_ACCENT, "");
    tft.setFont(F_BOLD);
    tft.setTextColor(C_WHITE);
    drawWrapped(energy.problem, 12, 70, W - 24, 3, 22);
    return;
  }

  int last = energy.days - 1;
  float week = 0;
  int weekDays = 0;
  for (int i = energy.days - 1; i >= 0 && weekDays < 7; i--, weekDays++) week += dayCost(i);

  // Two cards: the latest day and the last 7 days
  drawCardBox(8, 40, 148, 66);
  drawCardBox(164, 40, 148, 66);
  tft.setTextSize(1);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  String lastLabel = last >= 0 && energy.day[last] == localDay() - 1 ? String("YESTERDAY")
                   : last >= 0 ? shortDate(energy.day[last]) : String("LATEST");
  tft.drawString(lastLabel.c_str(), 82, 47);
  tft.drawString(weekDays >= 7 ? "LAST 7 DAYS" : ("LAST " + String(weekDays) + " DAYS").c_str(), 238, 47);
  tft.setFont(F_LARGE);
  char buf[16];
  snprintf(buf, sizeof(buf), "%.2f", last >= 0 ? dayCost(last) : 0.0f);
  gradientTextCentred(String(buf), 82, 64, false);
  snprintf(buf, sizeof(buf), "%.2f", week);
  gradientTextCentred(String(buf), 238, 64, false);
  tft.setFont(F_SMALL);
  tft.setTextColor(C_DIM);
  tft.drawString("pounds", 82, 90);
  tft.drawString("pounds", 238, 90);

  // Bar chart of daily cost, electricity and gas stacked
  float biggest = 0.01f;
  int first = max(0, energy.days - 7);
  for (int i = first; i < energy.days; i++) biggest = max(biggest, dayCost(i));
  int n = energy.days - first;
  int slot = n ? (W - 24) / n : 1, barW = min(30, slot - 10);
  int top = 120, bottom = 186;
  const char* dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  for (int k = 0; k < n; k++) {
    int i = first + k;
    int x = 12 + k * slot + (slot - barW) / 2;
    float elecPart = (energy.elecKwh[i] * energy.elecRate + energy.elecStanding) / 100.0f;
    int h = max(3, (int)((bottom - top) * dayCost(i) / biggest));
    int he = (int)(h * elecPart / max(0.01f, dayCost(i)));
    tft.fillSmoothRoundRect(x, bottom - h, barW, h, 4, C_GLOW);          // gas on top
    tft.fillSmoothRoundRect(x, bottom - he, barW, he, 4, C_ACCENT2);      // electricity
    time_t dt = (time_t)energy.day[i] * 86400 + 43200;
    struct tm tmv;
    gmtime_r(&dt, &tmv);
    tft.setFont(F_SMALL);
    tft.setTextDatum(D_TC);
    tft.setTextColor(C_GREY);
    tft.drawString(dayNames[tmv.tm_wday], x + barW / 2, bottom + 4);
  }

  // Rates
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_ACCENT2);
  String rates = "Elec " + String(energy.elecRate, 1) + "p/kWh + " + String(energy.elecStanding, 0) + "p/day";
  tft.drawString(fitText(rates, W / 2 + 10).c_str(), 10, 210);
  if (energy.hasGas) {
    tft.setTextDatum(D_TR);
    tft.setTextColor(C_GLOW);
    String g = "Gas " + String(energy.gasRate, 1) + "p + " + String(energy.gasStanding, 0) + "p/day";
    tft.drawString(fitText(g, W / 2 - 20).c_str(), W - 10, 210);
  }
}
