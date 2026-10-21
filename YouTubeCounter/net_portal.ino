// The settings web page. In setup mode the counter makes its own WiFi network
// and the page pops up on the phone that joins it. While running normally the
// same page is at the counter's address on your home WiFi.

unsigned long lastPortalActivity = 0;

void startSetupMode(SetupReason reason) {
  setupMode = true;
  setupReason = reason;
  setupStartedAt = millis();
  if (reason != SETUP_WIFI_FAILED) markFirmwareGood();
  showMessage("Starting setup mode...", "Looking for WiFi networks", "");

  // List nearby networks so they can be picked on the setup page
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  scannedNetworks = "";
  for (int i = 0; i < n && i < 25; i++) {
    String option = "<option value=\"" + htmlEscape(WiFi.SSID(i)) + "\">";
    if (WiFi.SSID(i).length() && scannedNetworks.indexOf(option) < 0) scannedNetworks += option;
  }
  WiFi.scanDelete();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(SETUP_WIFI_NAME);
  delay(200);
  dnsServer.start(53, "*", WiFi.softAPIP());  // every web address leads to the setup page
  registerRoutes();
  server.begin();
  drawSetupScreen();
}

void setupModeLoop() {
  dnsServer.processNextRequest();
  server.handleClient();
  if (restartAt && due(restartAt)) ESP.restart();

  // If WiFi failed or setup was opened by mistake, go back to normal after a while
  unsigned long limit = setupReason == SETUP_WIFI_FAILED ? 5UL * 60 * 1000
                      : setupReason == SETUP_ASKED      ? 10UL * 60 * 1000 : 0;
  bool idle = lastPortalActivity == 0 || millis() - lastPortalActivity > 120000;
  if (limit && millis() - setupStartedAt > limit && idle) ESP.restart();
  delay(5);
}

void startWebServer() {
  registerRoutes();
  server.begin();
}

void registerRoutes() {
  server.on("/", HTTP_GET, []() { sendSettingsPage(""); });
  server.on("/save", HTTP_POST, handleSave);
  server.on("/update", HTTP_POST, []() {
    lastPortalActivity = millis();
    updateRequested = true;
    sendNotice("Checking for updates", "Watch the counter's screen. If there's a new version it installs it and restarts.");
  });
  server.on("/restart", HTTP_POST, []() {
    sendNotice("Restarting", "The counter will be back in a few seconds.");
    restartAt = millis() + 1500;
  });
  server.onNotFound([]() {
    if (setupMode) {
      server.sendHeader("Location", "http://192.168.4.1/", true);
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });
}

String htmlEscape(const String& s) {
  String out;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '&') out += "&amp;";
    else if (c == '<') out += "&lt;";
    else if (c == '>') out += "&gt;";
    else if (c == '"') out += "&quot;";
    else if (c == '\'') out += "&#39;";
    else out += c;
  }
  return out;
}

void sendNotice(const String& title, const String& text) {
  String page = PAGE_HEAD;
  page += "<h1><span class=logo></span>" + htmlEscape(title) + "</h1><p>" + htmlEscape(text) + "</p>";
  if (!setupMode) page += "<p><a href=\"/\">Back to settings</a></p>";
  page += PAGE_TAIL;
  server.send(200, "text/html", page);
}

// ---------------------------------------------------------------- page building

String optionTag(const String& value, const String& label, bool selected) {
  return "<option value=\"" + htmlEscape(value) + "\"" + (selected ? " selected" : "") + ">" + htmlEscape(label) + "</option>";
}

String checkbox(const String& name, bool on, const String& label, const String& hint) {
  String s = "<label class=check><input type=checkbox name=" + name + (on ? " checked" : "") + "><span>" + label;
  if (hint.length()) s += "<small>" + hint + "</small>";
  return s + "</span></label>";
}

String textField(const String& label, const String& name, const String& value, const String& placeholder,
                 const String& type, const String& hint) {
  String s = "<label>" + label + "<input name=" + name + " type=" + type + " value=\"" + htmlEscape(value) +
             "\" placeholder=\"" + htmlEscape(placeholder) + "\" autocomplete=off autocapitalize=off>";
  if (hint.length()) s += "<span class=hint>" + hint + "</span>";
  return s + "</label>";
}

String hourSelect(const String& name, int selected) {
  String s = "<select name=" + name + ">";
  for (int h = 0; h < 24; h++) {
    char label[8];
    snprintf(label, sizeof(label), "%02d:00", h);
    s += optionTag(String(h), label, h == selected);
  }
  return s + "</select>";
}

const char* PAGE_NAMES[PAGE_COUNT] = {
  "Subscribers", "Channel stats", "Latest upload", "Recent uploads", "Most viewed video",
  "Newest comment", "7-day chart", "Head to head", "Live and premieres", "Clock", "Weather", "Crypto prices"
};
const char* PAGE_HINTS[PAGE_COUNT] = {
  "Big count, gained today and progress to your next milestone",
  "Total views, views today, videos and this week",
  "Views, likes, comments and views per hour",
  "Your last 4 videos and their views",
  "Your top video of all time (uses 101 units a day)",
  "The latest comment on your newest video",
  "Views per day for the last week - fills in over a few days",
  "Race another channel - set it in the YouTube section",
  "Live viewers while you stream, a countdown before",
  "Time and date",
  "Needs your town below",
  "Needs your coins below"
};

const char* TIMEZONES[][2] = {
  {"UK and Ireland", "GMT0BST,M3.5.0/1,M10.5.0"},
  {"Central Europe (Paris, Berlin)", "CET-1CEST,M3.5.0,M10.5.0/3"},
  {"Eastern Europe (Athens)", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
  {"US Eastern (New York)", "EST5EDT,M3.2.0,M11.1.0"},
  {"US Central (Chicago)", "CST6CDT,M3.2.0,M11.1.0"},
  {"US Mountain (Denver)", "MST7MDT,M3.2.0,M11.1.0"},
  {"US Arizona", "MST7"},
  {"US Pacific (Los Angeles)", "PST8PDT,M3.2.0,M11.1.0"},
  {"Canada Atlantic", "AST4ADT,M3.2.0,M11.1.0"},
  {"Brazil (Sao Paulo)", "<-03>3"},
  {"South Africa", "SAST-2"},
  {"UAE (Dubai)", "<+04>-4"},
  {"India", "IST-5:30"},
  {"Singapore, Hong Kong, Philippines", "<+08>-8"},
  {"Japan", "JST-9"},
  {"Australia East (Sydney)", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
  {"Australia Queensland", "AEST-10"},
  {"Australia West (Perth)", "AWST-8"},
  {"New Zealand", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
};
const int TIMEZONE_COUNT = sizeof(TIMEZONES) / sizeof(TIMEZONES[0]);

void sendSettingsPage(const String& problem) {
  lastPortalActivity = millis();
  prefs.putUChar("bootHolds", 0);

  String p;
  p.reserve(16000);
  p += PAGE_HEAD;
  p += "<h1><span class=logo></span>YouTube Counter</h1>";
  p += "<p class=sub>Version " FIRMWARE_VERSION " &middot; " + htmlEscape(hostName) + "</p>";
  if (problem.length()) p += "<div class=msg>" + htmlEscape(problem) + "</div>";
  p += "<form method=post action=/save>";

  // WiFi
  p += "<fieldset><legend>WiFi</legend>";
  p += "<label>Network name<input name=wifi list=nets value=\"" + htmlEscape(cfg.wifiName) +
       "\" placeholder=\"Pick or type your WiFi\" autocomplete=off autocapitalize=off>"
       "<span class=hint>2.4GHz networks only (most home WiFi has one)</span></label>";
  p += "<datalist id=nets>" + scannedNetworks + "</datalist>";
  p += textField("Password", "wpass", "", cfg.wifiPass.length() ? "Saved - leave blank to keep it" : "",
                 "password", "");
  p += "</fieldset>";

  // YouTube
  String keyHint = cfg.apiKey.length() > 4 ? "Saved key ends in ..." + cfg.apiKey.substring(cfg.apiKey.length() - 4) +
                   " - leave blank to keep it" : "Starts with AIza... - the README explains how to get one free";
  p += "<fieldset><legend>YouTube</legend>";
  p += textField("Channel", "channel", cfg.channel, "@yourhandle", "text", "The @handle shown on the channel page");
  p += textField("API key", "key", "", cfg.apiKey.length() ? "Saved" : "AIza...", "password", keyHint);
  p += textField("Compare with (optional)", "compare", cfg.compare, "@anotherchannel", "text",
                 "Shows a head-to-head page against this channel");
  p += "<label>Check for new numbers<select name=refresh id=refresh>";
  const int refreshOptions[] = {15, 30, 60, 120, 300};
  const char* refreshLabels[] = {"Every 15 seconds", "Every 30 seconds", "Every minute", "Every 2 minutes", "Every 5 minutes"};
  for (int i = 0; i < 5; i++) p += optionTag(String(refreshOptions[i]), refreshLabels[i], cfg.refreshSecs == refreshOptions[i]);
  p += "</select><span class=hint id=quota></span></label>";
  p += "</fieldset>";

  // Pages
  p += "<fieldset><legend>Pages to show</legend>";
  for (int i = 0; i < PAGE_COUNT; i++) {
    p += checkbox("p" + String(i), pageEnabled(i), PAGE_NAMES[i], PAGE_HINTS[i]);
  }
  p += "<label>Each page shows for<select name=pagesecs>";
  const int secsOptions[] = {5, 8, 10, 15, 20, 30};
  for (int i = 0; i < 6; i++) p += optionTag(String(secsOptions[i]), String(secsOptions[i]) + " seconds", cfg.pageSecs == secsOptions[i]);
  p += "</select><span class=hint>Tap the screen to skip to the next page</span></label>";
  p += "</fieldset>";

  // Weather, crypto, time
  p += "<fieldset><legend>Weather, crypto and time</legend>";
  String placeHint = cfg.placeName.length() ? "Found: " + cfg.placeName : "Your town or city";
  p += "<div class=row>" + textField("Town for weather", "city", cfg.city, "e.g. Manchester", "text", placeHint);
  p += "<label>Units<select name=fahr>" + optionTag("0", "Celsius", !cfg.fahrenheit) +
       optionTag("1", "Fahrenheit", cfg.fahrenheit) + "</select></label></div>";
  p += "<div class=row>" + textField("Coins", "coins", cfg.coins, "bitcoin,ethereum", "text",
                                     "Up to 3, as named on coingecko.com");
  p += "<label>Currency<select name=currency>" + optionTag("gbp", "GBP", cfg.currency == "gbp") +
       optionTag("usd", "USD", cfg.currency == "usd") + optionTag("eur", "EUR", cfg.currency == "eur") +
       "</select></label></div>";
  p += "<label>Time zone<select name=tz>";
  bool tzFound = false;
  for (int i = 0; i < TIMEZONE_COUNT; i++) {
    bool sel = cfg.tz == TIMEZONES[i][1];
    tzFound |= sel;
    p += optionTag(TIMEZONES[i][1], TIMEZONES[i][0], sel);
  }
  if (!tzFound) p += optionTag(cfg.tz, "Custom: " + cfg.tz, true);
  p += "</select></label>";
  p += checkbox("clock24", cfg.clock24, "24-hour clock", "");
  p += "</fieldset>";

  // Screen
  p += "<fieldset><legend>Screen</legend>";
  p += "<label>Brightness <span id=bv></span><input type=range name=bright min=10 max=255 value=" +
       String(cfg.brightness) + "></label>";
  p += checkbox("night", cfg.nightMode, "Night mode", "Dims the screen overnight. Tap it to light up for 30 seconds.");
  p += "<div class=row><label>From" + hourSelect("nfrom", cfg.nightFrom) + "</label><label>Until" +
       hourSelect("nto", cfg.nightTo) + "</label><label>Night level<select name=nbright>" +
       optionTag("0", "Off", cfg.nightBrightness == 0) + optionTag("5", "Very dim", cfg.nightBrightness == 5) +
       optionTag("15", "Dim", cfg.nightBrightness == 15) + optionTag("40", "Low", cfg.nightBrightness == 40) +
       "</select></label></div>";
  p += "<label>Screen type<select name=screen>" +
       optionTag("0", "Standard (most boards)", cfg.screenType == 0) +
       optionTag("1", "Standard, colours inverted", cfg.screenType == 1) +
       optionTag("2", "ST7789 (some USB-C boards)", cfg.screenType == 2) +
       optionTag("3", "ST7789, colours inverted", cfg.screenType == 3) +
       "</select><span class=hint>Change this if colours look like a photo negative or the screen stays blank</span></label>";
  p += checkbox("led", cfg.ledFlash, "Flash the light on the back for new subscribers", "");
  p += "</fieldset>";

  // Updates
  p += "<fieldset><legend>Updates</legend>";
  p += checkbox("autoupd", cfg.autoUpdate, "Install updates automatically", "Checks once a day");
  p += checkbox("early", cfg.earlyUpdates, "Get test versions first",
                "Tick this on your own counter only - it installs new versions before they go to everyone else");
  p += "<p class=note>Last check: " + htmlEscape(updateStatus) + "</p>";
  p += "</fieldset>";

  p += "<button type=submit>Save and restart</button></form>";
  if (!setupMode) {
    p += "<form method=post action=/update><button class=alt>Check for updates now</button></form>";
    p += "<form method=post action=/restart><button class=alt>Restart the counter</button></form>";
  }
  p += PAGE_SCRIPT;
  p += PAGE_TAIL;
  server.send(200, "text/html", p);
}

// ---------------------------------------------------------------- saving

int argInt(const char* name, int fallback, int lo, int hi) {
  if (!server.hasArg(name)) return fallback;
  return constrain(server.arg(name).toInt(), lo, hi);
}

void handleSave() {
  lastPortalActivity = millis();
  String wifi = server.arg("wifi");
  wifi.trim();
  String pass = server.arg("wpass");
  String key = server.arg("key");
  key.trim();
  String chan = server.arg("channel");
  chan.trim();

  // A new network with the password left blank means it has no password
  if (wifi.length() && wifi != cfg.wifiName) cfg.wifiPass = pass;
  else if (pass.length()) cfg.wifiPass = pass;
  if (wifi.length()) cfg.wifiName = wifi;
  if (key.length()) cfg.apiKey = key;
  cfg.channel = chan;

  String compare = server.arg("compare");
  compare.trim();
  cfg.compare = compare;

  String city = server.arg("city");
  city.trim();
  if (!city.equalsIgnoreCase(cfg.city)) {
    cfg.city = city;
    cfg.placeName = "";
    cfg.lat = NAN;
    cfg.lon = NAN;
  }
  cfg.fahrenheit = server.arg("fahr") == "1";
  String coins = server.arg("coins");
  coins.trim();
  cfg.coins = coins;
  String currency = server.arg("currency");
  if (currency == "gbp" || currency == "usd" || currency == "eur") cfg.currency = currency;
  if (server.arg("tz").length()) cfg.tz = server.arg("tz");
  cfg.clock24 = server.hasArg("clock24");

  uint32_t pages = 0;
  for (int i = 0; i < PAGE_COUNT; i++) {
    if (server.hasArg("p" + String(i))) pages |= 1UL << i;
  }
  cfg.pages = pages ? pages : (1UL << PAGE_SUBS);
  cfg.refreshSecs     = argInt("refresh", cfg.refreshSecs, 15, 3600);
  cfg.pageSecs        = argInt("pagesecs", cfg.pageSecs, 3, 120);
  cfg.brightness      = argInt("bright", cfg.brightness, 10, 255);
  cfg.nightMode       = server.hasArg("night");
  cfg.nightFrom       = argInt("nfrom", cfg.nightFrom, 0, 23);
  cfg.nightTo         = argInt("nto", cfg.nightTo, 0, 23);
  cfg.nightBrightness = argInt("nbright", cfg.nightBrightness, 0, 255);
  cfg.screenType      = argInt("screen", cfg.screenType, 0, 3);
  cfg.ledFlash        = server.hasArg("led");
  cfg.autoUpdate      = server.hasArg("autoupd");
  cfg.earlyUpdates    = server.hasArg("early");

  if (cfg.wifiName.length() == 0 || cfg.apiKey.length() == 0 || cfg.channel.length() == 0) {
    sendSettingsPage("Please fill in the WiFi network, your channel and the API key.");
    return;
  }

  saveSettings();
  markFirmwareGood();
  prefs.putUChar("bootHolds", 0);
  sendNotice("Saved!", "The counter is restarting and will connect to \"" + cfg.wifiName +
             "\" in a few seconds. You can close this page.");
  restartAt = millis() + 2000;
}
