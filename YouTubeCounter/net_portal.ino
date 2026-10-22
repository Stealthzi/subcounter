// The settings web page. In setup mode the counter makes its own WiFi network
// and the page pops up on the phone that joins it. While running normally the
// same page is at the counter's address on your home WiFi.


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
  server.on("/message", HTTP_POST, handleMessage);
  server.on("/ask", HTTP_GET, []() { sendAskPage(""); });
  server.on("/ask", HTTP_POST, handleAsk);
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

// Name and short description for every page, in PageId order
const char* PAGE_NAMES[PAGE_COUNT] = {
  "Subscribers", "Channel stats", "Latest upload", "Recent uploads", "Most viewed video",
  "Newest comment", "7-day chart", "Head to head", "Live and premieres", "Clock", "Weather", "Crypto prices",
  "Energy (Octopus)", "Cheapest petrol", "Google Calendar", "Messages and lamp", "Desk pet", "Alerts",
  "Free Epic games", "Screensaver", "Magic 8-ball", "Stopwatch", "Air quality and pollen", "Countdowns",
  "Mood lamp", "Joke or fact of the day", "Spotify now playing", "Exact YouTube stats", "Weekly report card"
};
const char* PAGE_HINTS[PAGE_COUNT] = {
  "Big count, gained today and your next milestone",
  "Total views, views today, videos and this week",
  "Views, likes, comments and views per hour",
  "Your last 4 videos and their views",
  "Your top video of all time (uses 101 units a day)",
  "The latest comment on your newest video",
  "Views per day for the last week - fills in over a few days",
  "Race another channel - set it in the YouTube section",
  "Live viewers while you stream, a countdown before",
  "Time and date, with the weather",
  "Needs your town below",
  "Needs your coins below",
  "Yesterday's cost and the last week - needs your Octopus details",
  "Cheapest fuel near your town, with a price trend",
  "Your next events - needs the calendar link",
  "Messages with the other counter, and the friendship lamp",
  "An old-school virtual pet to look after",
  "Frost, Met Office warnings and MOT/tax due - only when something's on",
  "This week's free games",
  "Aquarium or fireplace",
  "Ask it anything",
  "With laps",
  "Air quality index, pollen and UV - needs your town",
  "MOT, car tax and your own dates",
  "Colour-changing light, tap to change",
  "A new one every day",
  "Shows when music is playing - needs the Spotify sign-in",
  "Exact gains, losses, watch time, revenue - needs the YouTube sign-in",
  "Last week's views and subs with a grade - fills in after a week"
};
static_assert(sizeof(PAGE_NAMES) / sizeof(PAGE_NAMES[0]) == PAGE_COUNT, "a page is missing a name");
const int PAGE_GROUPS[][12] = {
  {PAGE_SUBS, PAGE_CHANNEL, PAGE_LATEST, PAGE_RECENT, PAGE_TOP, PAGE_COMMENT, PAGE_GROWTH, PAGE_REPORT, PAGE_COMPARE, PAGE_LIVE, PAGE_ANALYTICS, -1},
  {PAGE_CLOCK, PAGE_WEATHER, PAGE_ALERTS, PAGE_AIR, PAGE_ENERGY, PAGE_FUEL, PAGE_CALENDAR, PAGE_COUNTDOWN, -1},
  {PAGE_MESSAGES, PAGE_PET, -1},
  {PAGE_SPOTIFY, PAGE_GAMES, PAGE_JOKE, PAGE_EIGHTBALL, PAGE_STOPWATCH, PAGE_SCREENSAVER, PAGE_MOODLAMP, PAGE_CRYPTO, -1},
};
const char* PAGE_GROUP_NAMES[] = {"YouTube", "Home", "You and your brother", "Fun"};

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

// A secret field: never shows what's saved, blank keeps it
String secretField(const String& label, const String& name, const String& saved, const String& placeholder,
                   const String& hint) {
  String ph = saved.length() ? "Saved - leave blank to keep it" : placeholder;
  return textField(label, name, "", ph, "password", hint);
}

String selectBox(const String& name, const char* const values[], const char* const labels[], int n, const String& current) {
  String s = "<select name=" + name + ">";
  for (int i = 0; i < n; i++) s += optionTag(values[i], labels[i], current == values[i]);
  return s + "</select>";
}

String fieldset(const String& legend) {
  return "</fieldset><fieldset><legend>" + legend + "</legend>";
}

void sendSettingsPage(const String& problem) {
  lastPortalActivity = millis();
  prefs.putUChar("bootHolds", 0);

  String p;
  p.reserve(26000);
  p += PAGE_HEAD;
  p += "<h1><span class=logo></span>YouTube Counter</h1>";
  p += "<p class=sub>Version " FIRMWARE_VERSION " &middot; " BOARD_NAME " &middot; " + htmlEscape(hostName) + "</p>";
  if (problem.length()) p += "<div class=msg>" + htmlEscape(problem) + "</div>";
  if (!setupMode && cfg.openaiKey.length()) p += "<p><a href=\"/ask\">Ask ChatGPT about your data &rarr;</a></p>";

  // Messages can be sent from here too (not in setup mode - there's no internet then)
  if (!setupMode && cfg.msgCode.length()) {
    p += "<form method=post action=/message><fieldset><legend>Send a message</legend>"
         "<label>To the other counter<input name=text maxlength=120 placeholder=\"Type a message\"></label>"
         "<button>Send</button></fieldset></form>";
  }

  p += "<form method=post action=/save><fieldset><legend>WiFi</legend>";
  p += "<label>Network name<input name=wifi list=nets value=\"" + htmlEscape(cfg.wifiName) +
       "\" placeholder=\"Pick or type your WiFi\" autocomplete=off autocapitalize=off>"
       "<span class=hint>2.4GHz networks only (most home WiFi has one)</span></label>";
  p += "<datalist id=nets>" + scannedNetworks + "</datalist>";
  p += secretField("Password", "wpass", cfg.wifiPass, "", "");

  // YouTube
  String keyHint = cfg.apiKey.length() > 4 ? "Saved key ends in ..." + cfg.apiKey.substring(cfg.apiKey.length() - 4)
                                           : String("Starts with AIza... - the README explains how to get one free");
  p += fieldset("YouTube");
  p += textField("Channel", "channel", cfg.channel, "@yourhandle", "text", "The @handle shown on the channel page");
  p += secretField("API key", "key", cfg.apiKey, "AIza...", keyHint);
  p += textField("Compare with (optional)", "compare", cfg.compare, "@anotherchannel", "text",
                 "Shows a head-to-head page against this channel");
  p += "<label>Check for new numbers<select name=refresh id=refresh>";
  const int refreshOptions[] = {15, 30, 60, 120, 300};
  const char* refreshLabels[] = {"Every 15 seconds", "Every 30 seconds", "Every minute", "Every 2 minutes", "Every 5 minutes"};
  for (int i = 0; i < 5; i++) p += optionTag(String(refreshOptions[i]), refreshLabels[i], cfg.refreshSecs == refreshOptions[i]);
  p += "</select><span class=hint id=quota></span></label>";

  // Pages, grouped
  p += fieldset("Pages to show");
  for (int g = 0; g < 4; g++) {
    p += "<p class=note><b>" + String(PAGE_GROUP_NAMES[g]) + "</b></p>";
    for (int i = 0; PAGE_GROUPS[g][i] >= 0; i++) {
      int id = PAGE_GROUPS[g][i];
      p += checkbox("p" + String(id), pageEnabled(id), PAGE_NAMES[id], PAGE_HINTS[id]);
    }
  }
  p += "<label>Each page shows for<select name=pagesecs>";
  const int secsOptions[] = {5, 8, 10, 15, 20, 30};
  for (int i = 0; i < 6; i++) p += optionTag(String(secsOptions[i]), String(secsOptions[i]) + " seconds", cfg.pageSecs == secsOptions[i]);
  p += "</select><span class=hint>Tap the screen to skip to the next page</span></label>";

  // Where you are, weather and time
  p += fieldset("Your area and time");
  String placeHint = cfg.placeName.length() ? "Found: " + cfg.placeName
                                            : String("Used for weather, air quality and petrol prices");
  p += "<div class=row>" + textField("Town", "city", cfg.city, "e.g. Manchester", "text", placeHint);
  p += "<label>Units<select name=fahr>" + optionTag("0", "Celsius", !cfg.fahrenheit) +
       optionTag("1", "Fahrenheit", cfg.fahrenheit) + "</select></label></div>";
  p += "<label>Met Office warnings for<select name=metregion>";
  for (int i = 0; i < MET_REGION_COUNT; i++) p += optionTag(MET_REGIONS[i][0], MET_REGIONS[i][1], cfg.metRegion == MET_REGIONS[i][0]);
  p += "</select></label>";
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

  // Energy and petrol
  p += fieldset("Energy and petrol");
  p += textField("Octopus account number", "octoacct", cfg.octoAccount, "A-1234ABCD", "text", "");
  p += secretField("Octopus API key", "octokey", cfg.octoKey, "sk_live_...",
                   "On the Octopus website: Account, Personal details, API access");
  p += "<label>Gas meter reports in<select name=gasm3>" + optionTag("1", "Cubic metres (most smart meters)", cfg.gasInCubicMetres) +
       optionTag("0", "kWh (some older meters)", !cfg.gasInCubicMetres) + "</select></label>";
  const char* const fuelValues[] = {"E10", "E5", "B7", "SDV"};
  const char* const fuelLabels[] = {"Unleaded (E10)", "Super unleaded (E5)", "Diesel", "Super diesel"};
  p += "<div class=row><label>Fuel" + selectBox("fueltype", fuelValues, fuelLabels, 4, cfg.fuelType) + "</label>";
  const char* const radiusValues[] = {"2", "3", "5", "10", "15"};
  const char* const radiusLabels[] = {"2 miles", "3 miles", "5 miles", "10 miles", "15 miles"};
  p += "<label>Search within" + selectBox("fuelrad", radiusValues, radiusLabels, 5, String(cfg.fuelRadius)) + "</label></div>";

  // Calendar and countdowns
  p += fieldset("Calendar and countdowns");
  p += secretField("Google Calendar link", "calurl", cfg.calendarUrl, "https://script.google.com/macros/s/.../exec",
                   "Your private link from the calendar script - the README shows how");
  p += "<div class=row>" + textField("MOT due", "motdate", cfg.motDate, "", "date", "");
  p += textField("Car tax due", "taxdate", cfg.taxDate, "", "date", "") + "</div>";
  String lines = cfg.countdowns;
  lines.replace(";", "\n");
  p += "<label>Your countdowns<textarea name=countdowns rows=4 style=\"width:100%;box-sizing:border-box;margin-top:4px;"
       "padding:10px 12px;font-size:16px;color:#fff;background:#0b0b0d;border:1px solid #34343a;border-radius:9px\" "
       "placeholder=\"Holiday=2026-12-20&#10;Mum's birthday=03-02\">" + htmlEscape(lines) + "</textarea>"
       "<span class=hint>One per line: name=date. Write just month-day (like 03-02) for something every year.</span></label>";

  // Messages
  p += fieldset("Messages and friendship lamp");
  p += textField("Your name", "msgname", cfg.msgName, "e.g. Sam", "text", "How your messages are signed");
  p += "<label>Pairing code<input name=msgcode id=msgcode value=\"" + htmlEscape(cfg.msgCode) +
       "\" autocomplete=off autocapitalize=characters>"
       "<span class=hint>Put the same code on both counters. Messages are encrypted with it.</span></label>"
       "<button type=button class=alt onclick=\"var c='ABCDEFGHJKLMNPQRSTUVWXYZ23456789',s='',r=new Uint8Array(16);"
       "crypto.getRandomValues(r);for(var i=0;i<16;i++){if(i&&i%4==0)s+='-';s+=c[r[i]%32]}"
       "document.getElementById('msgcode').value=s\">Make a new code</button>";
  char colour[8];
  snprintf(colour, sizeof(colour), "#%06lX", (unsigned long)cfg.lampColour);
  p += "<label>Lamp colour<input type=color name=lampcol value=\"" + String(colour) + "\" style=\"height:44px\"></label>";

  // Pet and fun
  p += fieldset("Desk pet and fun");
  {
    const char* desc[SP_COUNT] = {"Classic - an old-school pixel pet", "Cat - a kitten with a surprise coat",
                                  "Dog", "Hamster", "Dragon - hatches from an egg"};
    p += "<label>Pet type<select name=pettype>";
    for (int sp = 0; sp < SP_COUNT; sp++) {
      bool open = speciesUnlocked(sp);
      p += "<option value=" + String(sp) + (cfg.petType == sp ? " selected" : "") + (open ? "" : " disabled") + ">" +
           desc[sp] + (open ? String("") : " (unlocks at pet level " + String(SPECIES_LEVEL[sp]) + ")") + "</option>";
    }
    p += "</select><span class=hint>Your pet is level " + String(rpg.level) + " - more pets unlock as it levels up</span></label>";
  }
  p += textField("Pet's name", "petname", cfg.petName, "Pixel", "text", "");
  p += "<label>Pet rules<select name=petstrict>" + optionTag("0", "Gentle - it gets sad but never dies", !cfg.petStrict) +
       optionTag("1", "Old-school - neglect it and it dies", cfg.petStrict) + "</select></label>";
  p += "<label>Screensaver<select name=saver>" + optionTag("0", "Aquarium", cfg.screensaver == 0) +
       optionTag("1", "Fireplace", cfg.screensaver == 1) + optionTag("2", "Take turns", cfg.screensaver == 2) +
       "</select></label>";
  p += checkbox("savernight", cfg.screensaverAtNight, "Only show the screensaver at night",
                "Uses the night hours from the Screen section");
  p += "<label>Daily<select name=joketype>" + optionTag("0", "Joke", cfg.jokeType == 0) +
       optionTag("1", "Fact", cfg.jokeType == 1) + optionTag("2", "Joke and fact on alternate days", cfg.jokeType == 2) +
       "</select></label>";
  p += checkbox("themes", cfg.themes, "Seasonal decorations", "Christmas, Halloween, New Year, Valentine's and Easter");
  p += "<div class=row>" + textField("Coins", "coins", cfg.coins, "bitcoin,ethereum", "text", "Up to 3, as named on coingecko.com");
  p += "<label>Currency<select name=currency>" + optionTag("gbp", "GBP", cfg.currency == "gbp") +
       optionTag("usd", "USD", cfg.currency == "usd") + optionTag("eur", "EUR", cfg.currency == "eur") +
       "</select></label></div>";

  // Accounts
  p += fieldset("Spotify and exact YouTube stats");
  p += "<p class=note>Run connect-accounts.ps1 on your PC once to get these. They're only stored on this counter.</p>";
  p += textField("Spotify app ID", "spclient", cfg.spotifyClient, "", "text", "");
  p += secretField("Spotify sign-in pass", "sptoken", cfg.spotifyToken, "", "");
  p += textField("Google client ID", "gclient", cfg.googleClient, "", "text", "");
  p += secretField("Google client secret", "gsecret", cfg.googleSecret, "", "");
  p += secretField("YouTube sign-in pass", "gtoken", cfg.googleToken, "", "");

  // Ask ChatGPT
  p += fieldset("Ask ChatGPT about your data");
  p += "<p class=note>Ask questions from your phone on the <a href=\"/ask\">Ask ChatGPT</a> page. It needs your own OpenAI "
       "API key (from platform.openai.com - pay as you go, pennies a month). Your ChatGPT Plus subscription doesn't cover it.</p>";
  p += secretField("OpenAI API key", "aikey", cfg.openaiKey, "sk-...", "Stays on the counter");
  p += textField("Model", "aimodel", cfg.aiModel, "gpt-6-luna", "text", "Leave as it is unless OpenAI changes their models");
  p += "<p class=note>What it may send with a question:</p>";
  p += checkbox("aich", cfg.aiShare & AI_CHANNEL, "YouTube channel stats", "");
  p += checkbox("aien", cfg.aiShare & AI_ENERGY, "Energy use and cost", "");
  p += checkbox("aipe", cfg.aiShare & AI_PETROL, "Petrol prices nearby", "");
  p += checkbox("aiwe", cfg.aiShare & AI_WEATHER, "Weather", "");
  p += checkbox("aica", cfg.aiShare & AI_CALENDAR, "Calendar events", "Off unless you tick it - event names can be personal");
  p += checkbox("aipt", cfg.aiShare & AI_PET, "Your desk pet", "");
  p += checkbox("aiscr", cfg.aiOnScreen, "Show answers on the counter too", "");

  // Screen
  p += fieldset("Screen");
  p += "<label>Colours<select name=theme>";
  for (int i = 0; i < THEME_COUNT; i++) {
    bool open = themeUnlocked(i);
    p += "<option value=" + String(i) + (cfg.theme == i ? " selected" : "") + (open ? "" : " disabled") + ">" +
         THEMES[i].name + (open ? String("") : " (pet reward at level " + String(THEME_LEVEL[i]) + ")") + "</option>";
  }
  p += "</select></label>";
  p += "<label>Brightness <span id=bv></span><input type=range name=bright min=10 max=255 value=" +
       String(cfg.brightness) + "></label>";
  p += checkbox("night", cfg.nightMode, "Night mode", "Dims the screen overnight. Tap it to light up for 30 seconds.");
  p += "<div class=row><label>From" + hourSelect("nfrom", cfg.nightFrom) + "</label><label>Until" +
       hourSelect("nto", cfg.nightTo) + "</label><label>Night level<select name=nbright>" +
       optionTag("0", "Off", cfg.nightBrightness == 0) + optionTag("5", "Very dim", cfg.nightBrightness == 5) +
       optionTag("15", "Dim", cfg.nightBrightness == 15) + optionTag("40", "Low", cfg.nightBrightness == 40) +
       "</select></label></div>";
#if BOARD_7INCH
  p += checkbox("recal", false, "Check the touch screen again", "Asks you to tap a dot when it restarts - try this if taps land in the wrong place");
  p += "<label>Board version<select name=boardrev>" +
       optionTag("13", "V1.3, V1.4 or V1.5 (most boards)", cfg.boardRev >= 13) +
       optionTag("12", "V1.2", cfg.boardRev == 12) +
       "</select><span class=hint>Printed on the back of the board. Change this if the screen stays dark or touch doesn't work.</span></label>";
#else
  p += "<label>Screen type<select name=screen>" +
       optionTag("0", "Standard (most boards)", cfg.screenType == 0) +
       optionTag("1", "Standard, colours inverted", cfg.screenType == 1) +
       optionTag("2", "ST7789 (some USB-C boards)", cfg.screenType == 2) +
       optionTag("3", "ST7789, colours inverted", cfg.screenType == 3) +
       "</select><span class=hint>Change this if colours look like a photo negative or the screen stays blank</span></label>";
  p += checkbox("led", cfg.ledFlash, "Flash the light on the back for news", "New subscribers, messages, the pet calling");
  p += checkbox("recal", false, "Line up the touch screen again", "Asks you to tap the corners when it restarts");
#endif

  // Updates
  p += fieldset("Updates");
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

String argText(const char* name) {
  String s = server.arg(name);
  s.trim();
  return s;
}

// Keeps the saved secret when the box was left blank
void argSecret(const char* name, String& target) {
  String s = argText(name);
  if (s.length()) target = s;
}

void handleSave() {
  lastPortalActivity = millis();
  String wifi = argText("wifi");
  String pass = server.arg("wpass");

  // A new network with the password left blank means it has no password
  if (wifi.length() && wifi != cfg.wifiName) cfg.wifiPass = pass;
  else if (pass.length()) cfg.wifiPass = pass;
  if (wifi.length()) cfg.wifiName = wifi;
  argSecret("key", cfg.apiKey);
  cfg.channel = argText("channel");
  cfg.compare = argText("compare");

  String city = argText("city");
  if (!city.equalsIgnoreCase(cfg.city)) {
    cfg.city = city;
    cfg.placeName = "";
    cfg.lat = NAN;
    cfg.lon = NAN;
  }
  cfg.fahrenheit = server.arg("fahr") == "1";
  cfg.coins = argText("coins");
  String currency = server.arg("currency");
  if (currency == "gbp" || currency == "usd" || currency == "eur") cfg.currency = currency;
  if (server.arg("tz").length()) cfg.tz = server.arg("tz");
  cfg.clock24 = server.hasArg("clock24");
  if (server.arg("metregion").length()) cfg.metRegion = server.arg("metregion");

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
  cfg.boardRev        = argInt("boardrev", cfg.boardRev, 12, 13);
  int wantTheme       = argInt("theme", cfg.theme, 0, THEME_COUNT - 1);
  if (themeUnlocked(wantTheme)) cfg.theme = wantTheme;
  cfg.ledFlash        = server.hasArg("led");
  cfg.autoUpdate      = server.hasArg("autoupd");
  cfg.earlyUpdates    = server.hasArg("early");

  cfg.octoAccount = argText("octoacct");
  argSecret("octokey", cfg.octoKey);
  cfg.gasInCubicMetres = server.arg("gasm3") != "0";
  String fuelType = server.arg("fueltype");
  if (fuelType == "E10" || fuelType == "E5" || fuelType == "B7" || fuelType == "SDV") cfg.fuelType = fuelType;
  cfg.fuelRadius = argInt("fuelrad", cfg.fuelRadius, 1, 25);
  argSecret("calurl", cfg.calendarUrl);
  cfg.motDate = argText("motdate");
  cfg.taxDate = argText("taxdate");
  String lines = server.arg("countdowns");
  lines.replace("\r", "");
  lines.replace(";", ",");
  lines.trim();
  lines.replace("\n", ";");
  cfg.countdowns = lines;
  cfg.msgName = argText("msgname");
  String code = argText("msgcode");
  if (code.length()) cfg.msgCode = code;
  String lamp = argText("lampcol");
  if (lamp.length() == 7 && lamp[0] == '#') cfg.lampColour = strtoul(lamp.c_str() + 1, nullptr, 16);
  cfg.petName = argText("petname");
  if (!cfg.petName.length()) cfg.petName = "Pixel";
  cfg.petStrict = server.arg("petstrict") == "1";
  int wantPet = argInt("pettype", cfg.petType, 0, SP_COUNT - 1);
  if (wantPet != cfg.petType && speciesUnlocked(wantPet)) switchSpecies(wantPet);
  cfg.screensaver = argInt("saver", cfg.screensaver, 0, 2);
  cfg.screensaverAtNight = server.hasArg("savernight");
  cfg.jokeType = argInt("joketype", cfg.jokeType, 0, 2);
  cfg.themes = server.hasArg("themes");
  cfg.spotifyClient = argText("spclient");
  argSecret("sptoken", cfg.spotifyToken);
  cfg.googleClient = argText("gclient");
  argSecret("gsecret", cfg.googleSecret);
  argSecret("gtoken", cfg.googleToken);
  argSecret("aikey", cfg.openaiKey);
  cfg.aiModel = argText("aimodel");
  if (!cfg.aiModel.length()) cfg.aiModel = "gpt-6-luna";
  cfg.aiShare = (server.hasArg("aich") ? AI_CHANNEL : 0) | (server.hasArg("aien") ? AI_ENERGY : 0) |
                (server.hasArg("aipe") ? AI_PETROL : 0) | (server.hasArg("aiwe") ? AI_WEATHER : 0) |
                (server.hasArg("aica") ? AI_CALENDAR : 0) | (server.hasArg("aipt") ? AI_PET : 0);
  cfg.aiOnScreen = server.hasArg("aiscr");
  if (server.hasArg("recal")) prefs.putBool("calNext", true);

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

void handleMessage() {
  lastPortalActivity = millis();
  String text = argText("text");
  if (text.length() > 120) text = text.substring(0, 120);
  bool ok = text.length() && sendMessage("msg", text);
  sendNotice(ok ? "Message sent" : "Couldn't send it", ok ? "It'll pop up on the other counter in a few seconds."
                                                          : "Check both counters have the same pairing code.");
  if (ok && currentPage == PAGE_MESSAGES) drawCurrentPage();
}
