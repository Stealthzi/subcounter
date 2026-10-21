/*
  ============================================================
   YouTube Counter  -  version 2
   for the ESP32 "Cheap Yellow Display" (ESP32-2432S028R)
  ============================================================

  Shows your YouTube channel's stats, plus a clock, weather and crypto prices,
  on pages that change every few seconds (tap the screen to skip ahead).

  Everything is set up from a phone - there's nothing to edit in here:
    - First start (or when WiFi can't be found): the screen shows how to join
      the "SubCounter-Setup" WiFi network, where a settings page opens.
    - While running: open the address shown on the info screen (hold a finger
      on the screen for 2 seconds) to change settings from any phone or PC
      on the same WiFi.
    - Hold a finger on the screen for 8 seconds to go back into setup mode.

  Remote updates: once a day the counter checks UPDATE_REPO on GitHub (see
  app.h) and installs newer versions by itself. README.md explains how.

  Libraries needed (Arduino IDE > Tools > Manage Libraries...):
    - LovyanGFX    (by lovyan03)
    - ArduinoJson  (by Benoit Blanchon)
  Board: "ESP32 Dev Module", Partition scheme: "Minimal SPIFFS (1.9MB APP with OTA)"
*/

#include "app.h"
#include "portal_html.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <vector>
#include <mbedtls/gcm.h>
#include <mbedtls/sha256.h>
#include <mbedtls/base64.h>
#include <time.h>

CheapYellowDisplay tft;
Preferences        prefs;
WiFiClientSecure   tls;
WebServer          server(80);
DNSServer          dnsServer;

Settings     cfg;
ChannelStats channel;
UploadsInfo  uploads;
VideoInfo    topVideo;
bool         topLoaded = false;
CommentInfo  comment;
CompareInfo  rival;
WeatherInfo  weather;
CryptoInfo   crypto;
DaySnap      history[HISTORY_DAYS];
ViewSample   samples[SAMPLE_COUNT];
int          sampleCount = 0;
String       sampledVideo;

EnergyInfo   energy;
FuelInfo     fuel;
FuelDay      fuelHistory[FUEL_DAYS];
CalendarInfo calendar;
Message      inbox[INBOX_MAX];   // newest first
int          inboxCount = 0;
int          unreadCount = 0;
GamesInfo    games;
PetState     pet;
PetMenu      petMenu = MENU_MAIN;
int          petFrameNo = 0;
bool         wanderDirty = false;   // the page wanted redrawing while the cat was out strolling
AirInfo      air;
WarningInfo  warnings;
JokeInfo     joke;
SpotifyInfo  spotify;
AnalyticsInfo analytics;

// Friendship lamp: when your brother sends a glow, the screen glows until this time
unsigned long lampGlowUntil = 0;
String        lampFrom;

// A memory canvas for smooth animation (aquarium, pet). Made only when needed.
LGFX_Sprite  canvas(&tft);

unsigned long lastInteraction = 0;  // last tap on a page's buttons
bool          keyboardOpen = false;
bool          touchCalibrated = false;
uint8_t       shownBrightness = 200;   // what the backlight is set to right now

String lastError;                 // last YouTube problem, empty when fine
String updateStatus = "Not checked yet";
bool   updateRequested = false;   // "Check for updates now" on the settings page
time_t youtubeBlockedUntil = 0;   // set when the daily YouTube limit runs out
String hostName;
bool   firmwareMarkedGood = false;

// Setup mode
enum SetupReason { SETUP_FIRST_TIME, SETUP_ASKED, SETUP_WIFI_FAILED };
bool          setupMode = false;
SetupReason   setupReason = SETUP_FIRST_TIME;
unsigned long setupStartedAt = 0;
unsigned long restartAt = 0;
String        scannedNetworks;

// What's on the screen
int           currentPage = -1;
int           lastNormalPage = -1;
unsigned long pageShownAt = 0;
bool          infoShowing = false;
unsigned long infoShownAt = 0;
bool          highlighted = false;
unsigned long highlightUntil = 0;
int           lastMinute = -1;
bool          nightDimmed = false;
unsigned long wakeUntil = 0;

// When each piece of data is next fetched (millis)
unsigned long nextStats = 0, nextUploads = 0, nextComment = 0, nextTop = 0, nextCompare = 0;
unsigned long nextWeather = 0, nextCrypto = 0, nextUpdateCheck = 0;
unsigned long nextEnergy = 0, nextFuel = 0, nextCalendar = 0, nextMessages = 0, nextGames = 0;
unsigned long nextAir = 0, nextWarnings = 0, nextJoke = 0, nextSpotify = 0, nextAnalytics = 0;

// ---------------------------------------------------------------- setup

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_BLUE, OUTPUT);
  digitalWrite(LED_PIN_RED, HIGH);
  digitalWrite(LED_PIN_GREEN, HIGH);
  digitalWrite(LED_PIN_BLUE, HIGH);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  prefs.begin("subcounter", false);
  loadSettings();
  applyTheme();
  loadHistory();

  uint64_t mac = ESP.getEfuseMac();
  char name[24];
  snprintf(name, sizeof(name), "subcounter-%02x%02x", (uint8_t)(mac >> 32), (uint8_t)(mac >> 40));
  hostName = name;

  tft.configure(cfg.screenType);
  tft.init();
  tft.setRotation(1);
  tft.setBrightness(cfg.brightness);
  shownBrightness = cfg.brightness;
  fillScreenBg();
  loadTouchCalibration();

  bool wantSetup = prefs.getBool("setupNext", false);
  if (wantSetup) prefs.putBool("setupNext", false);
  if (!wantSetup && bootHoldForSetup()) wantSetup = true;

  if (!settingsComplete()) {
    startSetupMode(SETUP_FIRST_TIME);
    return;
  }
  if (wantSetup) {
    startSetupMode(SETUP_ASKED);
    return;
  }
  if (!connectWiFi()) {
    startSetupMode(SETUP_WIFI_FAILED);
    return;
  }

  ensurePairCode();
  configTzTime(cfg.tz.c_str(), "pool.ntp.org", "time.google.com");
  struct tm t;
  getLocalTime(&t, 5000);

  MDNS.begin(hostName.c_str());
  startWebServer();
  loadFuelHistory();
  loadInbox();
  loadPet();

  // The first time it runs, line the touch screen up so buttons work
  if (!touchCalibrated || prefs.getBool("calNext", false)) {
    prefs.putBool("calNext", false);
    runTouchCalibration();
  }

  nextUpdateCheck = millis() + 3UL * 60 * 1000;
  showMessage("Loading your channel...", cfg.channel, "");
}

// Hold a finger on the screen while plugging in to open setup mode.
// If the touch panel reads "pressed" two boots running with nobody using the
// setup page, it's ignored, so a faulty touch panel can't trap the counter.
bool bootHoldForSetup() {
  if (!touchDown()) {
    prefs.putUChar("bootHolds", 0);
    return false;
  }
  if (prefs.getUChar("bootHolds", 0) >= 2) return false;

  showMessage("Keep holding to open setup", "Let go to start normally", "");
  unsigned long start = millis();
  while (millis() - start < 3000) {
    if (!touchDown()) return false;
    delay(20);
  }
  prefs.putUChar("bootHolds", prefs.getUChar("bootHolds", 0) + 1);
  return true;
}

bool settingsComplete() {
  return cfg.wifiName.length() > 0 && cfg.apiKey.length() > 0 && cfg.channel.length() > 0;
}

bool connectWiFi() {
  showMessage("Connecting to WiFi", cfg.wifiName, "");
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostName.c_str());
  WiFi.begin(cfg.wifiName.c_str(), cfg.wifiPass.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 45000) return false;
    delay(250);
  }
  return true;
}

// ---------------------------------------------------------------- loop

void loop() {
  if (setupMode) {
    setupModeLoop();
    return;
  }

  server.handleClient();
  handleTouchAndButton();
  if (restartAt && due(restartAt)) ESP.restart();
  if (updateRequested) {
    updateRequested = false;
    checkForUpdate();
  }

  runSchedule();
  rotatePages();
  everyMinute();
  animatePage();

  if (highlighted && due(highlightUntil)) {
    highlighted = false;
    if (currentPage == PAGE_SUBS && !infoShowing) drawCurrentPage();
  }
  if (infoShowing && millis() - infoShownAt > 15000 && !touchDown()) {
    infoShowing = false;
    drawCurrentPage();
  }
  if (lampGlowUntil && due(lampGlowUntil)) endLampGlow();
  delay(isAnimatedPage(currentPage) ? 2 : 10);
}

bool due(unsigned long when) {
  return (long)(millis() - when) >= 0;
}

bool youtubeBlocked() {
  return youtubeBlockedUntil != 0 && time(nullptr) < youtubeBlockedUntil;
}

// Fetches at most one thing per pass, so taps on the screen stay responsive
void runSchedule() {
  unsigned long now = millis();
  if (WiFi.status() != WL_CONNECTED) return;

  if (!youtubeBlocked()) {
    if (due(nextStats)) {
      uint64_t before = channel.subs;
      bool wasLoaded = channel.loaded;
      bool ok = updateChannel();
      nextStats = now + (ok ? cfg.refreshSecs : 30) * 1000UL;
      if (ok) {
        markFirmwareGood();
        recordHistory();
        if (wasLoaded && channel.subs > before) celebrate();
        if (!wasLoaded) {
          nextUploads = nextCompare = now;
          nextTop = now + 20000;
        }
      }
      onDataChanged(ok, PAGE_SUBS);
      return;
    }
    if (channel.loaded && due(nextUploads)) {
      bool ok = updateUploads();
      bool liveNow = liveIndex() >= 0;
      nextUploads = now + (ok ? (liveNow ? 60 : 300) : 60) * 1000UL;
      if (ok && !comment.loaded) nextComment = now;
      onDataChanged(ok, PAGE_LATEST);
      return;
    }
    if (channel.loaded && cfg.compare.length() && pageEnabled(PAGE_COMPARE) && due(nextCompare)) {
      bool ok = updateCompare();
      nextCompare = now + (ok ? 300 : 120) * 1000UL;
      onDataChanged(ok, PAGE_COMPARE);
      return;
    }
    if (uploads.loaded && pageEnabled(PAGE_COMMENT) && due(nextComment)) {
      bool ok = updateComment();
      nextComment = now + (ok ? 600 : 300) * 1000UL;
      onDataChanged(ok, PAGE_COMMENT);
      return;
    }
    if (channel.loaded && pageEnabled(PAGE_TOP) && due(nextTop)) {
      bool ok = updateTopVideo();
      nextTop = now + (ok ? 24UL * 3600 : 1800) * 1000UL;
      onDataChanged(ok, PAGE_TOP);
      return;
    }
  }

  // Messages come first so a glow or message from your brother shows quickly
  if (pageEnabled(PAGE_MESSAGES) && due(nextMessages)) {
    pollMessages();
    nextMessages = now + 15000;
    return;
  }
  if (cfg.googleToken.length() && pageEnabled(PAGE_ANALYTICS) && due(nextAnalytics)) {
    bool ok = updateAnalytics();
    nextAnalytics = now + (ok ? 3600 : 900) * 1000UL;
    onDataChanged(ok, PAGE_ANALYTICS);
    return;
  }
  if (cfg.spotifyToken.length() && pageEnabled(PAGE_SPOTIFY) && due(nextSpotify)) {
    bool wasPlaying = spotify.playing;
    bool ok = updateSpotify();
    nextSpotify = now + (spotify.playing ? 15 : 60) * 1000UL;
    if (ok && spotify.playing && !wasPlaying && currentPage != PAGE_SPOTIFY && !userBusy()) {
      showPage(PAGE_SPOTIFY);  // something just started playing
    } else {
      onDataChanged(ok, PAGE_SPOTIFY);
    }
    return;
  }
  if (cfg.city.length() && (pageEnabled(PAGE_WEATHER) || pageEnabled(PAGE_ALERTS) || pageEnabled(PAGE_PET)) &&
      due(nextWeather)) {
    bool ok = updateWeather();
    nextWeather = now + (ok ? 900 : 300) * 1000UL;
    onDataChanged(ok, PAGE_WEATHER);
    return;
  }
  if (cfg.city.length() && pageEnabled(PAGE_AIR) && due(nextAir)) {
    bool ok = updateAir();
    nextAir = now + (ok ? 1800 : 600) * 1000UL;
    onDataChanged(ok, PAGE_AIR);
    return;
  }
  if (pageEnabled(PAGE_ALERTS) && due(nextWarnings)) {
    bool ok = updateWarnings();
    nextWarnings = now + (ok ? 1800 : 600) * 1000UL;
    onDataChanged(ok, PAGE_ALERTS);
    return;
  }
  if (cfg.octoKey.length() && cfg.octoAccount.length() && pageEnabled(PAGE_ENERGY) && due(nextEnergy)) {
    bool ok = updateEnergy();
    nextEnergy = now + (ok ? 3 * 3600 : 900) * 1000UL;
    onDataChanged(ok, PAGE_ENERGY);
    return;
  }
  if (cfg.city.length() && pageEnabled(PAGE_FUEL) && due(nextFuel)) {
    bool ok = updateFuel();
    nextFuel = now + (ok ? 3600 : 900) * 1000UL;
    onDataChanged(ok, PAGE_FUEL);
    return;
  }
  if (cfg.calendarUrl.length() && pageEnabled(PAGE_CALENDAR) && due(nextCalendar)) {
    bool ok = updateCalendar();
    nextCalendar = now + (ok ? 900 : 300) * 1000UL;
    onDataChanged(ok, PAGE_CALENDAR);
    return;
  }
  if (pageEnabled(PAGE_GAMES) && due(nextGames)) {
    bool ok = updateGames();
    nextGames = now + (ok ? 6 * 3600 : 1800) * 1000UL;
    onDataChanged(ok, PAGE_GAMES);
    return;
  }
  if (pageEnabled(PAGE_JOKE) && due(nextJoke)) {
    bool ok = updateJoke();
    nextJoke = now + (ok ? 3600 : 900) * 1000UL;
    onDataChanged(ok, PAGE_JOKE);
    return;
  }
  if (cfg.coins.length() && pageEnabled(PAGE_CRYPTO) && due(nextCrypto)) {
    bool ok = updateCrypto();
    nextCrypto = now + (ok ? 300 : 180) * 1000UL;
    onDataChanged(ok, PAGE_CRYPTO);
    return;
  }
  if (cfg.autoUpdate && due(nextUpdateCheck)) {
    nextUpdateCheck = now + (24UL * 3600 + random(0, 3600)) * 1000UL;
    checkForUpdate();
    return;
  }
}

// Redraws whatever's on screen if it shows the data that just changed
void onDataChanged(bool ok, int page) {
  if (catWandering()) {
    wanderDirty = true;  // redraw once the cat has walked off
    return;
  }
  if (infoShowing || keyboardOpen || lampGlowing()) return;
  if (!channel.loaded) {
    if (!ok && page == PAGE_SUBS) showMessage("Can't load your channel", lastError, "Trying again in 30 seconds...");
    return;
  }
  if (currentPage < 0) {
    showPage(PAGE_SUBS);
    return;
  }
  drawHeader();
  bool affected = currentPage == page ||
                  (page == PAGE_SUBS && (currentPage == PAGE_CHANNEL || currentPage == PAGE_GROWTH ||
                                         currentPage == PAGE_COMPARE)) ||
                  (page == PAGE_LATEST && (currentPage == PAGE_RECENT || currentPage == PAGE_LIVE)) ||
                  (page == PAGE_WEATHER && (currentPage == PAGE_ALERTS || currentPage == PAGE_CLOCK));
  if (!affected) return;
  if (pageHasData(currentPage)) drawCurrentPage();
  else nextPage();
}

// Once the new version has loaded real data (or been set up), keep it
void markFirmwareGood() {
  if (firmwareMarkedGood) return;
  esp_ota_mark_app_valid_cancel_rollback();
  firmwareMarkedGood = true;
  prefs.putUChar("bootHolds", 0);
}

void celebrate() {
  highlighted = true;
  highlightUntil = millis() + 15000;
  petCelebrate();
  if (currentPage != PAGE_SUBS && !userBusy()) showPage(PAGE_SUBS);
  if (cfg.ledFlash) flashLed(0, 255, 0, 3);
}

// Blinks the LED on the back. Colours are 0 or 255 (it can only be on or off).
void flashLed(int r, int g, int b, int times) {
  for (int i = 0; i < times; i++) {
    setLed(r, g, b);
    delay(150);
    setLed(0, 0, 0);
    delay(150);
  }
}

void setLed(int r, int g, int b) {
  digitalWrite(LED_PIN_RED, r > 127 ? LOW : HIGH);
  digitalWrite(LED_PIN_GREEN, g > 127 ? LOW : HIGH);
  digitalWrite(LED_PIN_BLUE, b > 127 ? LOW : HIGH);
}

// ---------------------------------------------------------------- pages

bool pageEnabled(int page) {
  return (cfg.pages >> page) & 1;
}

bool pageHasData(int page) {
  switch (page) {
    case PAGE_SUBS:        return channel.loaded;
    case PAGE_CHANNEL:     return channel.loaded;
    case PAGE_LATEST:      return latestIndex() >= 0;
    case PAGE_RECENT:      return publishedCount() >= 2;
    case PAGE_TOP:         return topLoaded;
    case PAGE_COMMENT:     return comment.loaded;
    case PAGE_GROWTH:      return growthAvailable();
    case PAGE_COMPARE:     return rival.loaded && cfg.compare.length() > 0;
    case PAGE_LIVE:        return liveIndex() >= 0 || upcomingIndex() >= 0;
    case PAGE_CLOCK:       return time(nullptr) > 1700000000;
    case PAGE_WEATHER:     return weather.loaded;
    case PAGE_CRYPTO:      return crypto.loaded;
    case PAGE_ENERGY:      return energy.loaded || (cfg.octoKey.length() && energy.problem.length());
    case PAGE_FUEL:        return fuel.loaded;
    case PAGE_CALENDAR:    return calendar.loaded;
    case PAGE_MESSAGES:    return cfg.msgCode.length() > 0;
    case PAGE_PET:         return pet.stage != PET_NONE;
    case PAGE_ALERTS:      return alertCount() > 0;
    case PAGE_GAMES:       return games.loaded && games.nowCount + games.nextCount > 0;
    case PAGE_SCREENSAVER: return true;
    case PAGE_EIGHTBALL:   return true;
    case PAGE_STOPWATCH:   return true;
    case PAGE_AIR:         return air.loaded;
    case PAGE_COUNTDOWN:   return countdownCount() > 0;
    case PAGE_MOODLAMP:    return true;
    case PAGE_JOKE:        return joke.loaded;
    case PAGE_SPOTIFY:     return spotify.loaded && spotify.playing;
    case PAGE_ANALYTICS:   return analytics.loaded || (cfg.googleToken.length() && analytics.problem.length());
  }
  return false;
}

bool pageAvailable(int page) {
  return pageEnabled(page) && pageHasData(page);
}

// Pages that animate need a quicker loop
bool isAnimatedPage(int page) {
  return lampGlowing() || catWandering() || page == PAGE_PET || page == PAGE_SCREENSAVER || page == PAGE_STOPWATCH ||
         page == PAGE_MOODLAMP || page == PAGE_EIGHTBALL || page == PAGE_SPOTIFY;
}

void showPage(int page) {
  if (catWandering()) endWander();
  // A quick fade of the backlight makes page changes look smooth
  bool fade = page != currentPage && currentPage >= 0 && !infoShowing;
  uint8_t level = shownBrightness;
  if (fade) fadeTo(level / 5, 4, 12);
  if (page != currentPage) leavePage(currentPage);
  currentPage = page;
  if (page != PAGE_LIVE) lastNormalPage = page;
  pageShownAt = millis();
  infoShowing = false;
  drawCurrentPage();
  if (fade) fadeTo(level, 5, 12);
}

// Frees anything the old page was using
void leavePage(int page) {
  if (page == PAGE_SCREENSAVER || page == PAGE_PET) canvas.deleteSprite();
  if (page == PAGE_MOODLAMP) setLed(0, 0, 0);
  if (page == PAGE_PET) petLeave();
  if (page == PAGE_MESSAGES) markMessagesRead();
}

// Next page in order. While you're live, the live page shows between every page.
void nextPage() {
  if (!channel.loaded) return;
  if (liveIndex() >= 0 && pageAvailable(PAGE_LIVE) && currentPage != PAGE_LIVE) {
    showPage(PAGE_LIVE);
    return;
  }
  int from = lastNormalPage < 0 ? -1 : lastNormalPage;
  for (int i = 1; i <= PAGE_COUNT; i++) {
    int p = (from + i + PAGE_COUNT) % PAGE_COUNT;
    if (p == PAGE_LIVE && liveIndex() >= 0) continue;  // already shown in between
    if (pageAvailable(p)) {
      showPage(p);
      return;
    }
  }
  showPage(PAGE_SUBS);
}

// Someone is using the screen, so don't jump pages on them
bool userBusy() {
  return keyboardOpen || infoShowing || millis() - lastInteraction < 60000;
}

// Pages that shouldn't be flipped away from right now
bool pageHolds(int page) {
  if (page == PAGE_STOPWATCH && stopwatchRunning()) return true;
  if (page == PAGE_PET && petBusy()) return true;
  return false;
}

void rotatePages() {
  if (currentPage < 0 || highlighted || lampGlowing() || userBusy() || pageHolds(currentPage) || catWandering()) return;
  if (cfg.screensaverAtNight && isNightTime() && pageEnabled(PAGE_SCREENSAVER)) {
    if (currentPage != PAGE_SCREENSAVER) showPage(PAGE_SCREENSAVER);
    return;
  }
  unsigned long showFor = (currentPage == PAGE_SUBS ? cfg.pageSecs + 4 : cfg.pageSecs) * 1000UL;
  if (millis() - pageShownAt >= showFor) nextPage();
}

// ---------------------------------------------------------------- touch and buttons

bool touchDown() {
  int32_t x, y;
  return tft.getTouch(&x, &y) > 0;
}

// Tap: press a button on the page, or go to the next page (or wake the screen at night)
// Hold 2 s: info screen. Hold 8 s, or hold BOOT for 3 s: setup mode.
void handleTouchAndButton() {
  static bool wasDown = false, armed = false;
  static unsigned long downAt = 0;
  static unsigned long bootDownAt = 0;
  static int32_t tapX = 0, tapY = 0;

  static int32_t lastX = 0;
  static int moved = 0;          // how far the finger has slid while down
  static bool stroked = false;
  static unsigned long lastStroke = 0;

  int32_t x = 0, y = 0;
  bool down = tft.getTouch(&x, &y) > 0;
  unsigned long now = millis();
  if (down && !wasDown) {
    downAt = now;
    tapX = x;
    tapY = y;
    lastX = x;
    moved = 0;
    stroked = false;
  }
  if (down && wasDown) {
    moved += abs((int)(x - lastX));
    lastX = x;
    // Sliding across the cat strokes it
    if (armed && moved > 30 && cfg.petType == 1 && currentPage == PAGE_PET && !keyboardOpen &&
        y > PET_SCENE_Y && y < PET_BTN_Y && now - lastStroke > 300) {
      catStroke();
      stroked = true;
      lastStroke = now;
      lastInteraction = now;
    }
  }
  unsigned long held = down ? now - downAt : 0;

  if (down && armed && !keyboardOpen && moved < 20) {
    if (held >= 2000 && !infoShowing) showInfoScreen();
    if (held >= 8000) requestSetupMode();
  }
  if (!down && wasDown && armed && now - downAt < 2000 && !stroked) onTap(tapX, tapY);
  if (!down) armed = true;  // ignore a touch that was already down at start
  wasDown = down;

  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    if (bootDownAt == 0) bootDownAt = now;
    if (now - bootDownAt >= 3000) requestSetupMode();
  } else {
    bootDownAt = 0;
  }
}

void onTap(int x, int y) {
  if (catWandering()) endWander();  // the cat scarpers when you tap
  if (nightDimmed && millis() > wakeUntil) {
    wakeUntil = millis() + 30000;
    applyBrightness();
    return;
  }
  if (infoShowing) {
    infoShowing = false;
    drawCurrentPage();
    return;
  }
  if (keyboardOpen) {
    keyboardTap(x, y);
    return;
  }
  if (lampGlowing()) {
    lampTap(x, y);
    return;
  }
  highlighted = false;
  if (currentPage >= 0 && pageTap(currentPage, x, y)) {
    lastInteraction = millis();
    return;
  }
  nextPage();
}

// Buttons on interactive pages. Returns true if the tap was used.
bool pageTap(int page, int x, int y) {
  switch (page) {
    case PAGE_PET:       return petTap(x, y);
    case PAGE_MESSAGES:  return messagesTap(x, y);
    case PAGE_EIGHTBALL: return eightBallTap(x, y);
    case PAGE_STOPWATCH: return stopwatchTap(x, y);
    case PAGE_MOODLAMP:  return moodLampTap(x, y);
  }
  return false;
}

void requestSetupMode() {
  showMessage("Opening setup mode...", "", "");
  prefs.putBool("setupNext", true);
  delay(800);
  ESP.restart();
}

// ---------------------------------------------------------------- animation

void animatePage() {
  static unsigned long last = 0;
  if (infoShowing || keyboardOpen) return;
  unsigned long now = millis();
  if (catWandering()) {
    if (now - last >= 60) {
      last = now;
      wanderFrame();
    }
    return;
  }
  if (lampGlowing()) {
    if (now - last >= 40) {
      last = now;
      lampGlowFrame();
    }
    return;
  }
  if (currentPage < 0) return;
  int interval = currentPage == PAGE_SCREENSAVER ? 40
               : currentPage == PAGE_STOPWATCH   ? 100
               : currentPage == PAGE_MOODLAMP    ? 50
               : currentPage == PAGE_PET         ? 250
               : currentPage == PAGE_EIGHTBALL   ? 60
               : currentPage == PAGE_SPOTIFY     ? 1000 : 0;
  if (!interval || now - last < (unsigned long)interval) return;
  last = now;
  switch (currentPage) {
    case PAGE_SCREENSAVER: screensaverFrame(); break;
    case PAGE_STOPWATCH:   stopwatchFrame(); break;
    case PAGE_MOODLAMP:    moodLampFrame(); break;
    case PAGE_PET:         petFrame(); break;
    case PAGE_EIGHTBALL:   eightBallFrame(); break;
    case PAGE_SPOTIFY:     spotifyFrame(); break;
  }
}

// ---------------------------------------------------------------- once a minute

void everyMinute() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return;
  if (t.tm_min == lastMinute) return;
  lastMinute = t.tm_min;

  applyBrightness();
  recordHistory();  // starts a new day's snapshot just after midnight
  petMinute();

  // The pet calls for you when it needs something
  static unsigned long lastPetCall = 0;
  if (petNeedsYou() && pageEnabled(PAGE_PET) && currentPage != PAGE_PET && !userBusy() &&
      (lastPetCall == 0 || millis() - lastPetCall > 5UL * 60 * 1000)) {
    lastPetCall = millis();
    showPage(PAGE_PET);
    if (cfg.ledFlash) flashLed(255, 128, 0, 2);
    return;
  }

  if (infoShowing || keyboardOpen || currentPage < 0 || lampGlowing() || catWandering()) return;
  maybeCatVisit();
  if (catWandering()) return;
  drawHeader();
  if (currentPage == PAGE_CLOCK || currentPage == PAGE_LIVE || currentPage == PAGE_COMMENT ||
      currentPage == PAGE_LATEST || currentPage == PAGE_CALENDAR || currentPage == PAGE_COUNTDOWN ||
      currentPage == PAGE_MESSAGES) {
    drawCurrentPage();  // these show times that move on
  }
}

bool isNightTime() {
  if (!cfg.nightMode && !cfg.screensaverAtNight) return false;
  struct tm t;
  if (!getLocalTime(&t, 0)) return false;
  int h = t.tm_hour;
  if (cfg.nightFrom == cfg.nightTo) return false;
  if (cfg.nightFrom < cfg.nightTo) return h >= cfg.nightFrom && h < cfg.nightTo;
  return h >= cfg.nightFrom || h < cfg.nightTo;
}

void applyBrightness() {
  nightDimmed = cfg.nightMode && isNightTime();
  bool awake = millis() < wakeUntil;
  shownBrightness = nightDimmed && !awake ? cfg.nightBrightness : cfg.brightness;
  tft.setBrightness(shownBrightness);
}
