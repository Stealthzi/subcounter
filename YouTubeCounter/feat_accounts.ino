// Features that use your own accounts: Spotify now playing, and exact YouTube
// stats (YouTube Analytics). Each needs a one-time sign-in on your PC with
// connect-accounts.ps1, which gives the counter a private sign-in pass
// (a "refresh token"). It's kept only on the board, never in the program.

// POSTs a form and returns the reply
int postForm(const String& url, const String& form, String& body) {
  body = "";
  if (WiFi.status() != WL_CONNECTED) return -1;
  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(12000);
  if (!http.begin(tls, url)) return -2;
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int code = http.POST(form);
  if (code > 0) body = http.getString();
  http.end();
  return code;
}

int getWithToken(const String& url, const String& token, String& body) {
  body = "";
  if (WiFi.status() != WL_CONNECTED) return -1;
  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(12000);
  if (!http.begin(tls, url)) return -2;
  http.addHeader("Authorization", "Bearer " + token);
  int code = http.GET();
  if (code > 0) body = http.getString();
  http.end();
  return code;
}

// ---------------------------------------------------------------- Spotify

String        spotifyAccess;
unsigned long spotifyAccessUntil = 0;
std::vector<uint8_t> albumArt;
String        albumArtFor;

bool spotifyLogin() {
  if (spotifyAccess.length() && millis() < spotifyAccessUntil) return true;
  String body;
  int code = postForm("https://accounts.spotify.com/api/token",
                      "grant_type=refresh_token&refresh_token=" + urlEncode(cfg.spotifyToken) +
                      "&client_id=" + urlEncode(cfg.spotifyClient), body);
  JsonDocument doc;
  if (code != 200 || deserializeJson(doc, body)) {
    spotify.problem = "Spotify sign-in expired - run connect-accounts.ps1 again";
    return false;
  }
  spotifyAccess = doc["access_token"] | "";
  spotifyAccessUntil = millis() + ((doc["expires_in"] | 3600) - 120) * 1000UL;
  // Spotify sometimes hands out a new pass; keep it
  String newToken = doc["refresh_token"] | "";
  if (newToken.length() && newToken != cfg.spotifyToken) {
    cfg.spotifyToken = newToken;
    prefs.putString("sptoken", newToken);
  }
  return spotifyAccess.length() > 0;
}

bool downloadArt(const String& url) {
  if (url == albumArtFor) return true;
  tls.setInsecure();
  HTTPClient http;
  http.setTimeout(12000);
  if (!http.begin(tls, url)) return false;
  bool ok = false;
  if (http.GET() == 200) {
    int len = http.getSize();
    if (len > 0 && len < 90000) {
      albumArt.resize(len);
      auto* stream = http.getStreamPtr();
      int got = 0;
      unsigned long start = millis();
      while (got < len && millis() - start < 8000) {
        int n = stream->read(albumArt.data() + got, len - got);
        if (n > 0) got += n;
        else delay(5);
      }
      ok = got == len;
    }
  }
  http.end();
  albumArtFor = ok ? url : String("");
  if (!ok) albumArt.clear();
  return ok;
}

bool updateSpotify() {
  if (!spotifyLogin()) return false;
  String body;
  int code = getWithToken("https://api.spotify.com/v1/me/player/currently-playing", spotifyAccess, body);
  if (code == 204 || (code == 200 && body.length() == 0)) {
    spotify.playing = false;
    spotify.loaded = true;
    return true;
  }
  if (code == 401) {
    spotifyAccess = "";
    return false;
  }
  if (code != 200) return false;

  JsonDocument filter;
  filter["is_playing"] = true;
  filter["progress_ms"] = true;
  filter["item"]["name"] = true;
  filter["item"]["duration_ms"] = true;
  filter["item"]["artists"][0]["name"] = true;
  filter["item"]["album"]["images"][0]["url"] = true;
  filter["item"]["album"]["images"][0]["width"] = true;
  filter["item"]["show"]["name"] = true;  // podcasts
  JsonDocument doc;
  if (deserializeJson(doc, body, DeserializationOption::Filter(filter))) return false;

  spotify.playing = doc["is_playing"] | false;
  spotify.track = cleanText(doc["item"]["name"] | "");
  spotify.artist = cleanText(doc["item"]["artists"][0]["name"] | (doc["item"]["show"]["name"] | ""));
  spotify.progressMs = doc["progress_ms"] | 0L;
  spotify.durationMs = doc["item"]["duration_ms"] | 0L;
  spotify.fetchedAt = millis();
  // The 300px cover, drawn at 120px
  String art;
  for (JsonVariant img : doc["item"]["album"]["images"].as<JsonArray>()) {
    if ((img["width"] | 0) == 300) art = img["url"] | "";
  }
  if (art.length() && art != spotify.artUrl) {
    spotify.artUrl = art;
    downloadArt(art);
  }
  spotify.loaded = true;
  spotify.problem = "";
  return true;
}

String minsSecs(long ms) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%ld:%02ld", ms / 60000, (ms / 1000) % 60);
  return String(buf);
}

void drawSpotifyProgress() {
  long pos = spotify.progressMs + (spotify.playing ? (long)(millis() - spotify.fetchedAt) : 0);
  if (spotify.durationMs > 0 && pos > spotify.durationMs) pos = spotify.durationMs;
  fillBg(146, 150, W - 152, 40);
  float f = spotify.durationMs ? (float)pos / spotify.durationMs : 0;
  drawProgressBar(146, 156, W - 158, 8, f, C_ACCENT);
  tft.setFont(F_SMALL);
  tft.setTextSize(1);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_GREY);
  tft.drawString(minsSecs(pos).c_str(), 146, 170);
  tft.setTextDatum(D_TR);
  tft.drawString(minsSecs(spotify.durationMs).c_str(), W - 12, 170);
}

void drawSpotifyPage() {
  if (albumArt.size()) {
    tft.drawJpg(albumArt.data(), albumArt.size(), 12, 50, 120, 120, 0, 0, 0.4f, 0.4f);
  } else {
    drawCardBox(12, 50, 120, 120);
  }
  drawGlow(72, 110, 62, 6, theme().glow);
  tft.setTextSize(1);
  tft.setFont(F_BOLD);
  tft.setTextColor(C_WHITE);
  int lines = drawWrapped(spotify.track, 146, 54, W - 158, 3, 21);
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TL);
  tft.setTextColor(C_ACCENT);
  tft.drawString(fitText(spotify.artist, W - 158).c_str(), 146, 58 + lines * 21);
  drawSpotifyProgress();
  tft.setTextColor(C_DIM);
  tft.setTextDatum(D_TL);
  tft.drawString(spotify.playing ? "Playing on Spotify" : "Paused", 12, 194);
}

void spotifyFrame() {
  if (spotify.playing) drawSpotifyProgress();
}

// ---------------------------------------------------------------- exact YouTube stats

String        googleAccess;
unsigned long googleAccessUntil = 0;

bool googleLogin() {
  if (googleAccess.length() && millis() < googleAccessUntil) return true;
  String body;
  int code = postForm("https://oauth2.googleapis.com/token",
                      "grant_type=refresh_token&refresh_token=" + urlEncode(cfg.googleToken) +
                      "&client_id=" + urlEncode(cfg.googleClient) +
                      "&client_secret=" + urlEncode(cfg.googleSecret), body);
  JsonDocument doc;
  if (code != 200 || deserializeJson(doc, body)) {
    analytics.problem = "YouTube sign-in expired - run connect-accounts.ps1 again";
    return false;
  }
  googleAccess = doc["access_token"] | "";
  googleAccessUntil = millis() + ((doc["expires_in"] | 3600) - 120) * 1000UL;
  return googleAccess.length() > 0;
}

String isoDate(uint32_t day) {
  time_t t = (time_t)day * 86400 + 43200;
  struct tm d;
  gmtime_r(&t, &d);
  char buf[12];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d", d.tm_year + 1900, d.tm_mon + 1, d.tm_mday);
  return String(buf);
}

bool updateAnalytics() {
  if (!googleLogin()) return false;
  uint32_t today = localDay();
  if (!today) return false;
  String base = "https://youtubeanalytics.googleapis.com/v2/reports?ids=channel%3D%3DMINE&dimensions=day&sort=day"
                "&startDate=" + isoDate(today - 30) + "&endDate=" + isoDate(today - 1);
  String body;
  int code = getWithToken(base + "&metrics=views,estimatedMinutesWatched,averageViewDuration,subscribersGained,subscribersLost,likes",
                          googleAccess, body);
  if (code == 401) {
    googleAccess = "";
    return false;
  }
  if (code != 200) {
    analytics.problem = "YouTube Analytics error " + String(code);
    return false;
  }
  JsonDocument doc;
  if (deserializeJson(doc, body)) return false;
  JsonArray rows = doc["rows"].as<JsonArray>();
  int n = rows.size();
  // YouTube's daily figures arrive a day or two late; use the newest day with views
  int last = n - 1;
  while (last > 0 && (rows[last][1] | 0L) == 0) last--;
  analytics.gained28 = analytics.lost28 = analytics.views28 = analytics.minutes28 = 0;
  for (int i = max(0, last - 27); i <= last && i < n; i++) {
    analytics.views28 += rows[i][1] | 0L;
    analytics.minutes28 += rows[i][2] | 0L;
    analytics.gained28 += rows[i][4] | 0L;
    analytics.lost28 += rows[i][5] | 0L;
  }
  if (last >= 0 && last < n) {
    JsonVariant r = rows[last];
    analytics.dayLabel = String((const char*)(r[0] | ""));
    analytics.views = r[1] | 0L;
    analytics.minutes = r[2] | 0L;
    analytics.avgSeconds = r[3] | 0L;
    analytics.gained = r[4] | 0L;
    analytics.lost = r[5] | 0L;
    analytics.likes = r[6] | 0L;
  }

  // Revenue only works for monetised channels; leave it out otherwise
  code = getWithToken(base + "&metrics=estimatedRevenue", googleAccess, body);
  analytics.revenue = analytics.revenue28 = -1;
  if (code == 200 && !deserializeJson(doc, body)) {
    JsonArray rr = doc["rows"].as<JsonArray>();
    double sum = 0;
    int m = rr.size();
    for (int i = max(0, m - 28); i < m; i++) sum += rr[i][1] | 0.0;
    analytics.revenue28 = sum;
    if (m) analytics.revenue = rr[min(last, m - 1)][1] | 0.0;
  }
  analytics.loaded = true;
  analytics.problem = "";
  return true;
}

void drawAnalyticsPage() {
  if (!analytics.loaded) {
    drawLabel(12, 44, "EXACT STATS", C_ACCENT, "");
    tft.setFont(F_BOLD);
    tft.setTextColor(C_WHITE);
    drawWrapped(analytics.problem, 12, 70, W - 24, 3, 22);
    return;
  }
  // "2026-10-19" -> a friendly label
  int y, m, d;
  String day = analytics.dayLabel;
  if (sscanf(day.c_str(), "%d-%d-%d", &y, &m, &d) == 3) day = shortDate((uint32_t)daysFromCivil(y, m, d));
  drawLabel(12, 40, "EXACT STATS", C_ACCENT, day + " (latest from YouTube)");

  long net = analytics.gained - analytics.lost;
  drawCard(8, 60, 97, 70, "GAINED", "+" + withCommas(analytics.gained), C_GREEN);
  drawCard(111, 60, 97, 70, "LOST", "-" + withCommas(analytics.lost), C_RED);
  drawCard(214, 60, 97, 70, "NET", (net >= 0 ? "+" : "-") + withCommas(labs(net)), C_WHITE);

  char avg[12];
  snprintf(avg, sizeof(avg), "%ld:%02ld", analytics.avgSeconds / 60, analytics.avgSeconds % 60);
  drawCard(8, 136, 97, 70, "WATCH HOURS", compact(analytics.minutes / 60), C_WHITE);
  drawCard(111, 136, 97, 70, "AVG VIEW", avg, C_WHITE);
  if (analytics.revenue >= 0) {
    char rev[16];
    snprintf(rev, sizeof(rev), "%.2f", analytics.revenue);
    drawCard(214, 136, 97, 70, "EST. REVENUE", rev, C_GREEN);
  } else {
    drawCard(214, 136, 97, 70, "LIKES", compact(analytics.likes), C_WHITE);
  }

  String month = "Last 28 days: " + signedCompact(analytics.gained28 - analytics.lost28) + " subs, " +
                 compact(analytics.minutes28 / 60) + " watch hours";
  if (analytics.revenue28 >= 0) month += ", " + String(analytics.revenue28, 2) + " revenue";
  tft.setFont(F_SMALL);
  tft.setTextDatum(D_TC);
  tft.setTextColor(C_GREY);
  tft.drawString(fitText(month, W - 16).c_str(), W / 2, 212);
}
