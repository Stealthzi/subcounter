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
  loadHistory();

  uint64_t mac = ESP.getEfuseMac();
  char name[24];
  snprintf(name, sizeof(name), "subcounter-%02x%02x", (uint8_t)(mac >> 32), (uint8_t)(mac >> 40));
  hostName = name;

  tft.configure(cfg.screenType);
  tft.init();
  tft.setRotation(1);
  tft.setBrightness(cfg.brightness);
  tft.fillScreen(C_BG);

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

  configTzTime(cfg.tz.c_str(), "pool.ntp.org", "time.google.com");
  struct tm t;
  getLocalTime(&t, 5000);

  MDNS.begin(hostName.c_str());
  startWebServer();

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
  if (restartAt && (long)(millis() - restartAt) >= 0) ESP.restart();
  if (updateRequested) {
    updateRequested = false;
    checkForUpdate();
  }

  runSchedule();
  rotatePages();
  everyMinute();

  if (highlighted && (long)(millis() - highlightUntil) >= 0) {
    highlighted = false;
    if (currentPage == PAGE_SUBS && !infoShowing) drawCurrentPage();
  }
  if (infoShowing && millis() - infoShownAt > 15000 && !touchDown()) {
    infoShowing = false;
    drawCurrentPage();
  }
  delay(10);
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
  bool wifiUp = WiFi.status() == WL_CONNECTED;
  if (!wifiUp) return;

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

  if (cfg.city.length() && pageEnabled(PAGE_WEATHER) && due(nextWeather)) {
    bool ok = updateWeather();
    nextWeather = now + (ok ? 900 : 300) * 1000UL;
    onDataChanged(ok, PAGE_WEATHER);
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
  if (infoShowing) return;
  if (!channel.loaded) {
    if (!ok) showMessage("Can't load your channel", lastError, "Trying again in 30 seconds...");
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
                  (page == PAGE_LATEST && (currentPage == PAGE_RECENT || currentPage == PAGE_LIVE));
  if (affected && pageHasData(currentPage)) drawCurrentPage();
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
  if (currentPage != PAGE_SUBS) showPage(PAGE_SUBS);
  if (!cfg.ledFlash) return;
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN_GREEN, LOW);
    delay(150);
    digitalWrite(LED_PIN_GREEN, HIGH);
    delay(150);
  }
}

// ---------------------------------------------------------------- pages

bool pageEnabled(int page) {
  return (cfg.pages >> page) & 1;
}

bool pageHasData(int page) {
  switch (page) {
    case PAGE_SUBS:    return channel.loaded;
    case PAGE_CHANNEL: return channel.loaded;
    case PAGE_LATEST:  return latestIndex() >= 0;
    case PAGE_RECENT:  return publishedCount() >= 2;
    case PAGE_TOP:     return topLoaded;
    case PAGE_COMMENT: return comment.loaded;
    case PAGE_GROWTH:  return growthAvailable();
    case PAGE_COMPARE: return rival.loaded && cfg.compare.length() > 0;
    case PAGE_LIVE:    return liveIndex() >= 0 || upcomingIndex() >= 0;
    case PAGE_CLOCK:   return time(nullptr) > 1700000000;
    case PAGE_WEATHER: return weather.loaded;
    case PAGE_CRYPTO:  return crypto.loaded;
  }
  return false;
}

bool pageAvailable(int page) {
  return pageEnabled(page) && pageHasData(page);
}

void showPage(int page) {
  currentPage = page;
  if (page != PAGE_LIVE) lastNormalPage = page;
  pageShownAt = millis();
  infoShowing = false;
  drawCurrentPage();
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

void rotatePages() {
  if (currentPage < 0 || infoShowing || highlighted) return;
  unsigned long showFor = (currentPage == PAGE_SUBS ? cfg.pageSecs + 4 : cfg.pageSecs) * 1000UL;
  if (millis() - pageShownAt >= showFor) nextPage();
}

// ---------------------------------------------------------------- touch and buttons

bool touchDown() {
  int32_t x, y;
  return tft.getTouch(&x, &y) > 0;
}

// Tap: next page (or wake the screen at night)
// Hold 2 s: info screen. Hold 8 s, or hold BOOT for 3 s: setup mode.
void handleTouchAndButton() {
  static bool wasDown = false, armed = false;
  static unsigned long downAt = 0;
  static unsigned long bootDownAt = 0;

  bool down = touchDown();
  unsigned long now = millis();
  if (down && !wasDown) downAt = now;
  unsigned long held = down ? now - downAt : 0;

  if (down && armed) {
    if (held >= 2000 && !infoShowing) showInfoScreen();
    if (held >= 8000) requestSetupMode();
  }
  if (!down && wasDown && armed && now - downAt < 2000) onTap();
  if (!down) armed = true;  // ignore a touch that was already down at start
  wasDown = down;

  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    if (bootDownAt == 0) bootDownAt = now;
    if (now - bootDownAt >= 3000) requestSetupMode();
  } else {
    bootDownAt = 0;
  }
}

void onTap() {
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
  highlighted = false;
  nextPage();
}

void requestSetupMode() {
  showMessage("Opening setup mode...", "", "");
  prefs.putBool("setupNext", true);
  delay(800);
  ESP.restart();
}

// ---------------------------------------------------------------- once a minute

void everyMinute() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return;
  if (t.tm_min == lastMinute) return;
  lastMinute = t.tm_min;

  applyBrightness();
  recordHistory();  // starts a new day's snapshot just after midnight
  if (infoShowing || currentPage < 0) return;
  drawHeader();
  if (currentPage == PAGE_CLOCK || currentPage == PAGE_LIVE || currentPage == PAGE_COMMENT ||
      currentPage == PAGE_LATEST) {
    drawCurrentPage();  // these show times that move on
  }
}

bool isNightTime() {
  if (!cfg.nightMode) return false;
  struct tm t;
  if (!getLocalTime(&t, 0)) return false;
  int h = t.tm_hour;
  if (cfg.nightFrom == cfg.nightTo) return false;
  if (cfg.nightFrom < cfg.nightTo) return h >= cfg.nightFrom && h < cfg.nightTo;
  return h >= cfg.nightFrom || h < cfg.nightTo;
}

void applyBrightness() {
  nightDimmed = isNightTime();
  bool awake = millis() < wakeUntil;
  tft.setBrightness(nightDimmed && !awake ? cfg.nightBrightness : cfg.brightness);
}
