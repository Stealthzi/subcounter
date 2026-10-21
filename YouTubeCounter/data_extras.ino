// Weather (Open-Meteo) and crypto prices (CoinGecko). Both are free and need no key.

// Turns the town typed in setup into a map position, once
bool findPlace() {
  String body;
  String url = "https://geocoding-api.open-meteo.com/v1/search?count=1&language=en&format=json&name="
             + urlEncode(cfg.city);
  if (httpGet(url, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  JsonVariant r = doc["results"][0];
  if (r.isNull()) {
    cfg.placeName = "Place not found: " + cfg.city;
    return false;
  }
  cfg.lat = r["latitude"] | 0.0f;
  cfg.lon = r["longitude"] | 0.0f;
  cfg.placeName = cleanText(r["name"] | "");
  String region = cleanText(r["country_code"] | "");
  if (region.length()) cfg.placeName += ", " + region;
  prefs.putFloat("lat", cfg.lat);
  prefs.putFloat("lon", cfg.lon);
  prefs.putString("place", cfg.placeName);
  return true;
}

bool updateWeather() {
  if (isnan(cfg.lat) && !findPlace()) return false;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(cfg.lat, 4)
             + "&longitude=" + String(cfg.lon, 4)
             + "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,is_day"
             + "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_probability_max"
             + "&timezone=auto&forecast_days=2&wind_speed_unit=mph"
             + (cfg.fahrenheit ? "&temperature_unit=fahrenheit" : "");
  String body;
  if (httpGet(url, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  JsonVariant cur = doc["current"];
  JsonVariant day = doc["daily"];
  weather.temp     = cur["temperature_2m"] | 0.0f;
  weather.feels    = cur["apparent_temperature"] | 0.0f;
  weather.humidity = cur["relative_humidity_2m"] | 0;
  weather.code     = cur["weather_code"] | 0;
  weather.wind     = cur["wind_speed_10m"] | 0.0f;
  weather.isDay    = (cur["is_day"] | 1) == 1;
  for (int i = 0; i < 2; i++) {
    weather.dayCode[i] = day["weather_code"][i] | 0;
    weather.hi[i]      = day["temperature_2m_max"][i] | 0.0f;
    weather.lo[i]      = day["temperature_2m_min"][i] | 0.0f;
    weather.rain[i]    = day["precipitation_probability_max"][i] | 0;
  }
  weather.loaded = true;
  return true;
}

const char* weatherText(int code) {
  switch (code) {
    case 0: return "Clear";
    case 1: return "Mostly clear";
    case 2: return "Partly cloudy";
    case 3: return "Cloudy";
    case 45: case 48: return "Fog";
    case 51: case 53: case 55: return "Drizzle";
    case 56: case 57: return "Freezing drizzle";
    case 61: return "Light rain";
    case 63: return "Rain";
    case 65: return "Heavy rain";
    case 66: case 67: return "Freezing rain";
    case 71: return "Light snow";
    case 73: return "Snow";
    case 75: return "Heavy snow";
    case 77: return "Snow grains";
    case 80: case 81: return "Showers";
    case 82: return "Heavy showers";
    case 85: case 86: return "Snow showers";
    case 95: return "Thunderstorm";
    case 96: case 99: return "Storm with hail";
  }
  return "Unknown";
}

// ---------------------------------------------------------------- crypto

bool updateCrypto() {
  // Up to 3 coins, as CoinGecko IDs like "bitcoin,ethereum,solana"
  String list = cfg.coins;
  list.replace(" ", "");
  list.toLowerCase();
  String ids[MAX_COINS];
  int n = 0;
  while (list.length() && n < MAX_COINS) {
    int comma = list.indexOf(',');
    String id = comma < 0 ? list : list.substring(0, comma);
    list = comma < 0 ? "" : list.substring(comma + 1);
    if (id.length()) ids[n++] = id;
  }
  if (n == 0) return false;

  String joined;
  for (int i = 0; i < n; i++) joined += (i ? "," : "") + ids[i];
  String cur = cfg.currency;
  String url = "https://api.coingecko.com/api/v3/simple/price?include_24hr_change=true&ids=" + joined
             + "&vs_currencies=" + cur;
  String body;
  if (httpGet(url, body) != 200) return false;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;

  String changeKey = cur + "_24h_change";
  crypto.count = 0;
  for (int i = 0; i < n; i++) {
    JsonVariant c = doc[ids[i]];
    if (c.isNull()) continue;
    int k = crypto.count++;
    crypto.id[k]     = ids[i];
    crypto.price[k]  = c[cur] | 0.0;
    crypto.change[k] = c[changeKey] | 0.0;
  }
  crypto.loaded = crypto.count > 0;
  return crypto.loaded;
}

String coinName(const String& id) {
  String name = id;
  name.replace("-", " ");
  if (name.length()) name.setCharAt(0, toupper(name[0]));
  return name;
}

String coinSymbol(const String& id) {
  if (id == "bitcoin") return "BTC";
  if (id == "ethereum") return "ETH";
  if (id == "solana") return "SOL";
  if (id == "dogecoin") return "DOGE";
  if (id == "ripple") return "XRP";
  if (id == "cardano") return "ADA";
  if (id == "litecoin") return "LTC";
  if (id == "binancecoin") return "BNB";
  String s = id.substring(0, 4);
  s.toUpperCase();
  return s;
}
