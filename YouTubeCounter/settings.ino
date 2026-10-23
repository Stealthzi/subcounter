// Settings live in the board's own storage, so updates never touch them.

void loadSettings() {
  cfg.wifiName        = prefs.getString("wifi", "");
  cfg.wifiPass        = prefs.getString("wpass", "");
  cfg.apiKey          = prefs.getString("key", "");
  cfg.channel         = prefs.getString("channel", "");
  cfg.compare         = prefs.getString("compare", "");
  cfg.city            = prefs.getString("city", "");
  cfg.placeName       = prefs.getString("place", "");
  cfg.lat             = prefs.getFloat("lat", NAN);
  cfg.lon             = prefs.getFloat("lon", NAN);
  cfg.fahrenheit      = prefs.getBool("fahr", false);
  cfg.coins           = prefs.getString("coins", "bitcoin,ethereum");
  cfg.currency        = prefs.getString("currency", "gbp");
  cfg.tz              = prefs.getString("tz", "GMT0BST,M3.5.0/1,M10.5.0");
  cfg.clock24         = prefs.getBool("clock24", true);
  cfg.pages           = prefs.getULong("pages", DEFAULT_PAGES);
  cfg.refreshSecs     = prefs.getUShort("refresh", 60);
  cfg.pageSecs        = prefs.getUChar("pagesecs", 0);
  cfg.brightness      = prefs.getUChar("bright", 200);
  cfg.nightMode       = prefs.getBool("night", false);
  cfg.nightFrom       = prefs.getUChar("nfrom", 23);
  cfg.nightTo         = prefs.getUChar("nto", 7);
  cfg.nightBrightness = prefs.getUChar("nbright", 15);
  cfg.screenType      = prefs.getUChar("screen", 0);
  cfg.ledFlash        = prefs.getBool("led", true);
  cfg.autoUpdate      = prefs.getBool("autoupd", true);
  cfg.earlyUpdates    = prefs.getBool("early", false);
  cfg.octoAccount     = prefs.getString("octoacct", "");
  cfg.octoKey         = prefs.getString("octokey", "");
  cfg.gasInCubicMetres = prefs.getBool("gasm3", true);
  cfg.fuelType        = prefs.getString("fueltype", "E10");
  cfg.fuelRadius      = prefs.getUChar("fuelrad", 5);
  cfg.calendarUrl     = prefs.getString("calurl", "");
  cfg.msgName         = prefs.getString("msgname", "");
  cfg.msgCode         = prefs.getString("msgcode", "");
  cfg.petName         = prefs.getString("petname", "Pixel");
  cfg.petStrict       = prefs.getBool("petstrict", false);
  cfg.screensaver     = prefs.getUChar("saver", 2);
  cfg.screensaverAtNight = prefs.getBool("savernight", false);
  cfg.metRegion       = prefs.getString("metregion", "UK");
  cfg.motDate         = prefs.getString("motdate", "");
  cfg.taxDate         = prefs.getString("taxdate", "");
  cfg.countdowns      = prefs.getString("countdowns", "");
  cfg.jokeType        = prefs.getUChar("joketype", 2);
  cfg.themes          = prefs.getBool("themes", true);
  cfg.lampColour      = prefs.getULong("lampcol", 0xFF8C28);
  cfg.spotifyClient   = prefs.getString("spclient", "");
  cfg.spotifyToken    = prefs.getString("sptoken", "");
  cfg.googleClient    = prefs.getString("gclient", "");
  cfg.googleSecret    = prefs.getString("gsecret", "");
  cfg.googleToken     = prefs.getString("gtoken", "");
  cfg.theme           = prefs.getUChar("theme", 0);
  cfg.petType         = prefs.getUChar("pettype", 0);
  cfg.boardRev        = prefs.getUChar("boardrev", 13);
  cfg.openaiKey       = prefs.getString("aikey", "");
  cfg.aiModel         = prefs.getString("aimodel", "gpt-6-luna");
  cfg.aiShare         = prefs.getUChar("aishare", AI_DEFAULT_SHARE);
  cfg.aiOnScreen      = prefs.getBool("aiscr", true);
  if (cfg.theme >= THEME_COUNT) cfg.theme = 0;

  if (cfg.refreshSecs < 15) cfg.refreshSecs = 15;
  if (cfg.pageSecs > 0 && cfg.pageSecs < 3) cfg.pageSecs = 3;  // 0 = never change page by itself
  if (cfg.brightness < 10) cfg.brightness = 10;
  if (cfg.pages == 0) cfg.pages = 1UL << PAGE_SUBS;

  // Version 2.3 stays on the page you pick, and starts on the new channel
  // overview. Done once, so a different choice made after that is kept.
  if (!prefs.getBool("v23", false)) {
    cfg.pageSecs = 0;
    cfg.pages |= 1UL << PAGE_OVERVIEW;
    prefs.putUChar("pagesecs", cfg.pageSecs);
    prefs.putULong("pages", cfg.pages);
    prefs.putUChar("homepg", PAGE_OVERVIEW);
    prefs.putBool("v23", true);
  }
}

void saveSettings() {
  prefs.putString("wifi", cfg.wifiName);
  prefs.putString("wpass", cfg.wifiPass);
  prefs.putString("key", cfg.apiKey);
  prefs.putString("channel", cfg.channel);
  prefs.putString("compare", cfg.compare);
  prefs.putString("city", cfg.city);
  prefs.putString("place", cfg.placeName);
  prefs.putFloat("lat", cfg.lat);
  prefs.putFloat("lon", cfg.lon);
  prefs.putBool("fahr", cfg.fahrenheit);
  prefs.putString("coins", cfg.coins);
  prefs.putString("currency", cfg.currency);
  prefs.putString("tz", cfg.tz);
  prefs.putBool("clock24", cfg.clock24);
  prefs.putULong("pages", cfg.pages);
  prefs.putUShort("refresh", cfg.refreshSecs);
  prefs.putUChar("pagesecs", cfg.pageSecs);
  prefs.putUChar("bright", cfg.brightness);
  prefs.putBool("night", cfg.nightMode);
  prefs.putUChar("nfrom", cfg.nightFrom);
  prefs.putUChar("nto", cfg.nightTo);
  prefs.putUChar("nbright", cfg.nightBrightness);
  prefs.putUChar("screen", cfg.screenType);
  prefs.putBool("led", cfg.ledFlash);
  prefs.putBool("autoupd", cfg.autoUpdate);
  prefs.putBool("early", cfg.earlyUpdates);
  prefs.putString("octoacct", cfg.octoAccount);
  prefs.putString("octokey", cfg.octoKey);
  prefs.putBool("gasm3", cfg.gasInCubicMetres);
  prefs.putString("fueltype", cfg.fuelType);
  prefs.putUChar("fuelrad", cfg.fuelRadius);
  prefs.putString("calurl", cfg.calendarUrl);
  prefs.putString("msgname", cfg.msgName);
  prefs.putString("msgcode", cfg.msgCode);
  prefs.putString("petname", cfg.petName);
  prefs.putBool("petstrict", cfg.petStrict);
  prefs.putUChar("saver", cfg.screensaver);
  prefs.putBool("savernight", cfg.screensaverAtNight);
  prefs.putString("metregion", cfg.metRegion);
  prefs.putString("motdate", cfg.motDate);
  prefs.putString("taxdate", cfg.taxDate);
  prefs.putString("countdowns", cfg.countdowns);
  prefs.putUChar("joketype", cfg.jokeType);
  prefs.putBool("themes", cfg.themes);
  prefs.putULong("lampcol", cfg.lampColour);
  prefs.putString("spclient", cfg.spotifyClient);
  prefs.putString("sptoken", cfg.spotifyToken);
  prefs.putString("gclient", cfg.googleClient);
  prefs.putString("gsecret", cfg.googleSecret);
  prefs.putString("gtoken", cfg.googleToken);
  prefs.putUChar("theme", cfg.theme);
  prefs.putUChar("pettype", cfg.petType);
  prefs.putUChar("boardrev", cfg.boardRev);
  prefs.putString("aikey", cfg.openaiKey);
  prefs.putString("aimodel", cfg.aiModel);
  prefs.putUChar("aishare", cfg.aiShare);
  prefs.putBool("aiscr", cfg.aiOnScreen);
}

// Each counter starts with its own random pairing code for messages. Called once
// WiFi is on, because the chip's random numbers are only truly random then.
void ensurePairCode() {
  if (cfg.msgCode.length()) return;
  cfg.msgCode = newPairCode();
  prefs.putString("msgcode", cfg.msgCode);
}

// e.g. "K7QM-2XPA-9RTD-4WCE" - letters and numbers that are hard to mix up
String newPairCode() {
  const char* chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
  String code;
  for (int i = 0; i < 16; i++) {
    if (i && i % 4 == 0) code += '-';
    code += chars[esp_random() % 32];
  }
  return code;
}

// Rough number of YouTube API units this counter uses a day (the free limit is 10,000)
long dailyUnits() {
  long units = 86400L / cfg.refreshSecs;            // channel stats
  units += 288 * 2;                                  // latest uploads, every 5 minutes
  if (pageEnabled(PAGE_COMMENT)) units += 144;       // newest comment, every 10 minutes
  if (pageEnabled(PAGE_TOP)) units += 101;           // most viewed video, once a day
  if (pageEnabled(PAGE_COMPARE) && cfg.compare.length()) units += 288;
  return units;
}
