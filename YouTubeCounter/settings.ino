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
  cfg.pageSecs        = prefs.getUChar("pagesecs", 10);
  cfg.brightness      = prefs.getUChar("bright", 200);
  cfg.nightMode       = prefs.getBool("night", false);
  cfg.nightFrom       = prefs.getUChar("nfrom", 23);
  cfg.nightTo         = prefs.getUChar("nto", 7);
  cfg.nightBrightness = prefs.getUChar("nbright", 15);
  cfg.screenType      = prefs.getUChar("screen", 0);
  cfg.ledFlash        = prefs.getBool("led", true);
  cfg.autoUpdate      = prefs.getBool("autoupd", true);
  cfg.earlyUpdates    = prefs.getBool("early", false);

  if (cfg.refreshSecs < 15) cfg.refreshSecs = 15;
  if (cfg.pageSecs < 3) cfg.pageSecs = 3;
  if (cfg.brightness < 10) cfg.brightness = 10;
  if (cfg.pages == 0) cfg.pages = 1UL << PAGE_SUBS;
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
