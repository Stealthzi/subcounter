// Shared settings, data types and screen wiring for the YouTube counter.
#pragma once

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ======================= FIRMWARE =======================

#define FIRMWARE_VERSION "2.1.0"

// GitHub repository that holds the update files, as "github-name/repo-name".
// While it says "your-github-name", remote updates are switched off.
#define UPDATE_REPO "Stealthzi/subcounter"

// The WiFi network the counter creates while it's in setup mode
#define SETUP_WIFI_NAME "SubCounter-Setup"

// ======================= SCREEN =======================

const int W = 320, H = 240;

#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

// Colour themes, picked in Settings. "Stealth glow" (the default) is deep navy
// fading to blue, with teal-to-blue gradient text and purple glows.
struct Theme {
  const char* name;
  uint32_t bgTop, bgBottom;   // background gradient (RGB888)
  uint32_t card, edge, line;  // cards, card outlines, dividers
  uint32_t grey, dim;         // secondary and faint text
  uint32_t accent, accent2;   // gradient accent (labels, big numbers, bars)
  uint32_t glow;              // highlights and glows
};
const Theme THEMES[] = {
  {"Stealth glow", 0x060A1C, 0x0B2A5C, 0x111A3A, 0x22407A, 0x1C2C55, 0x9AA4C7, 0x5C678F, 0x2EE6A8, 0x3DA5FF, 0x9B5CFF},
  {"Classic dark", 0x0A0A0C, 0x0A0A0C, 0x202024, 0x34343A, 0x34343A, 0xAAAAAA, 0x69696E, 0xFF5252, 0xFF8A3D, 0xFF5252},
  {"Purple haze",  0x0C0618, 0x2A0E4A, 0x1E1238, 0x4A2C7A, 0x2E1B50, 0xB9A7D6, 0x6E5C8F, 0xC77DFF, 0x7B8CFF, 0xFF6AD5},
};
const int THEME_COUNT = sizeof(THEMES) / sizeof(THEMES[0]);

// Current colours (RGB565) - set from the theme at start-up by applyTheme()
uint16_t C_BG, C_BG_TOP, C_BG_BOTTOM, C_CARD, C_EDGE, C_LINE, C_GREY, C_DIM, C_ACCENT, C_ACCENT2, C_GLOW;
const uint16_t C_RED    = RGB565(255, 0, 51);
const uint16_t C_WHITE  = RGB565(255, 255, 255);
const uint16_t C_GREEN  = RGB565(46, 230, 140);
const uint16_t C_AMBER  = RGB565(255, 176, 32);
const uint16_t C_BLUE   = RGB565(90, 170, 255);
const uint16_t C_YELLOW = RGB565(255, 214, 64);
const uint16_t C_CLOUD  = RGB565(215, 222, 235);
const uint16_t C_PURPLE = RGB565(155, 92, 255);

#define F_SMALL (&lgfx::fonts::Font2)
#define F_BODY  (&lgfx::fonts::FreeSans9pt7b)
#define F_BOLD  (&lgfx::fonts::FreeSansBold9pt7b)
#define F_MED   (&lgfx::fonts::FreeSansBold12pt7b)
#define F_LARGE (&lgfx::fonts::FreeSansBold18pt7b)
#define F_HUGE  (&lgfx::fonts::FreeSansBold24pt7b)

#define D_TL lgfx::textdatum_t::top_left
#define D_TC lgfx::textdatum_t::top_center
#define D_TR lgfx::textdatum_t::top_right
#define D_ML lgfx::textdatum_t::middle_left
#define D_MC lgfx::textdatum_t::middle_center
#define D_MR lgfx::textdatum_t::middle_right
#define D_BC lgfx::textdatum_t::baseline_center
#define D_BL lgfx::textdatum_t::baseline_left

// The Cheap Yellow Display's built-in screen and touch panel. Both common
// screen chips are set up, and the one picked in Settings is used.
class CheapYellowDisplay : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9341   _ili;
  lgfx::Panel_ST7789    _st;
  lgfx::Bus_SPI         _bus;
  lgfx::Light_PWM       _light;
  lgfx::Touch_XPT2046   _touch;

public:
  // screenType: 0 = standard, 1 = standard with inverted colours,
  //             2 = ST7789, 3 = ST7789 with inverted colours
  void configure(uint8_t screenType) {
    auto bus = _bus.config();
    bus.spi_host   = SPI2_HOST;
    bus.spi_mode   = 0;
    bus.freq_write = 40000000;
    bus.freq_read  = 16000000;
    bus.pin_sclk   = 14;
    bus.pin_mosi   = 13;
    bus.pin_miso   = 12;
    bus.pin_dc     = 2;
    _bus.config(bus);

    auto light = _light.config();
    light.pin_bl      = 21;
    light.invert      = false;
    light.freq        = 12000;
    light.pwm_channel = 7;
    _light.config(light);

    auto touch = _touch.config();
    touch.x_min           = 300;
    touch.x_max           = 3900;
    touch.y_min           = 200;
    touch.y_max           = 3700;
    touch.pin_int         = 36;
    touch.bus_shared      = false;
    touch.offset_rotation = 0;
    touch.spi_host        = SPI3_HOST;
    touch.freq            = 1000000;
    touch.pin_sclk        = 25;
    touch.pin_mosi        = 32;
    touch.pin_miso        = 39;
    touch.pin_cs          = 33;
    _touch.config(touch);

    lgfx::Panel_Device* panel = (screenType >= 2) ? (lgfx::Panel_Device*)&_st : (lgfx::Panel_Device*)&_ili;
    panel->setBus(&_bus);
    auto cfg = panel->config();
    cfg.pin_cs       = 15;
    cfg.pin_rst      = -1;
    cfg.pin_busy     = -1;
    cfg.panel_width  = 240;
    cfg.panel_height = 320;
    cfg.readable     = true;
    cfg.invert       = (screenType == 1 || screenType == 3);
    cfg.rgb_order    = false;
    cfg.bus_shared   = false;
    panel->config(cfg);
    panel->setLight(&_light);
    panel->setTouch(&_touch);
    setPanel(panel);
  }
};

// The RGB LED on the back of the board (LOW = on)
const int LED_PIN_RED = 4, LED_PIN_GREEN = 16, LED_PIN_BLUE = 17;
// The BOOT button on the back (LOW = pressed)
const int BOOT_BUTTON_PIN = 0;

// ======================= SETTINGS =======================

// New pages go on the end so saved on/off choices keep their meaning
enum PageId : uint8_t {
  PAGE_SUBS, PAGE_CHANNEL, PAGE_LATEST, PAGE_RECENT, PAGE_TOP, PAGE_COMMENT,
  PAGE_GROWTH, PAGE_COMPARE, PAGE_LIVE, PAGE_CLOCK, PAGE_WEATHER, PAGE_CRYPTO,
  PAGE_ENERGY, PAGE_FUEL, PAGE_CALENDAR, PAGE_MESSAGES, PAGE_PET, PAGE_ALERTS,
  PAGE_GAMES, PAGE_SCREENSAVER, PAGE_EIGHTBALL, PAGE_STOPWATCH, PAGE_AIR,
  PAGE_COUNTDOWN, PAGE_MOODLAMP, PAGE_JOKE, PAGE_SPOTIFY, PAGE_ANALYTICS,
  PAGE_COUNT
};

// Pages that only appear once they're set up (energy, fuel...) are on by default
const uint32_t DEFAULT_PAGES = ((1UL << PAGE_COUNT) - 1) & ~(1UL << PAGE_CRYPTO);

struct Settings {
  String   wifiName, wifiPass;
  String   apiKey;
  String   channel;          // "@handle" or a channel ID starting with "UC"
  String   compare;          // optional second channel to race against
  String   city;             // for the weather page, as typed
  String   placeName;        // the place the weather service matched
  float    lat = NAN, lon = NAN;
  bool     fahrenheit = false;
  String   coins = "bitcoin,ethereum";
  String   currency = "gbp";
  String   tz = "GMT0BST,M3.5.0/1,M10.5.0";
  bool     clock24 = true;
  uint32_t pages = DEFAULT_PAGES;
  uint16_t refreshSecs = 60;
  uint8_t  pageSecs = 10;
  uint8_t  brightness = 200;
  bool     nightMode = false;
  uint8_t  nightFrom = 23, nightTo = 7, nightBrightness = 15;
  uint8_t  screenType = 0;
  bool     ledFlash = true;
  bool     autoUpdate = true;
  bool     earlyUpdates = false;  // also install test versions (GitHub pre-releases)

  String   octoAccount, octoKey;  // Octopus Energy account number and API key
  bool     gasInCubicMetres = true;  // most smart gas meters report m3, older ones kWh
  String   fuelType = "E10";      // E10, E5, B7 (diesel) or SDV (super diesel)
  uint8_t  fuelRadius = 5;        // miles
  String   calendarUrl;           // private Google Apps Script link
  String   msgName;               // how your messages are signed
  String   msgCode;               // pairing code shared by both counters
  String   petName = "Pixel";
  bool     petStrict = false;     // old-school rules: the pet can die
  uint8_t  screensaver = 2;       // 0 aquarium, 1 fireplace, 2 take turns
  bool     screensaverAtNight = false;  // show only the screensaver overnight
  String   metRegion = "UK";      // Met Office warnings region code
  String   motDate, taxDate;      // "2026-11-30" - typed in, never looked up
  String   countdowns;            // "Holiday=2026-12-20;Birthday=2027-03-02"
  uint8_t  jokeType = 2;          // 0 jokes, 1 facts, 2 both
  bool     themes = true;         // seasonal decorations
  uint32_t lampColour = 0xFF8C28; // friendship lamp glow (RGB)
  String   spotifyClient, spotifyToken;     // Spotify app ID and sign-in pass
  String   googleClient, googleSecret, googleToken;  // for exact YouTube stats
  uint8_t  theme = 0;             // index into THEMES
  uint8_t  petType = 0;           // 0 classic pet, 1 cat
};

// ======================= DATA =======================

struct ChannelStats {
  bool     loaded = false;
  String   id, title, uploads;
  bool     hidden = false;
  uint64_t subs = 0, views = 0, videos = 0;
};

enum LiveState : uint8_t { LIVE_NONE, LIVE_UPCOMING, LIVE_NOW };

struct VideoInfo {
  String    id, title;
  time_t    published = 0;
  uint64_t  views = 0, likes = 0, comments = 0;
  bool      hasLikes = false, hasComments = false;
  LiveState live = LIVE_NONE;
  uint64_t  liveViewers = 0;
  time_t    liveStart = 0;  // scheduled start, or actual start once live
};

const int RECENT_COUNT = 5;
struct UploadsInfo {
  bool      loaded = false;
  int       count = 0;
  VideoInfo v[RECENT_COUNT];
};

struct CommentInfo {
  bool   loaded = false;
  String author, text, videoTitle;
  time_t published = 0;
};

struct CompareInfo {
  bool     loaded = false;
  String   title;
  bool     hidden = false;
  uint64_t subs = 0, views = 0;
};

struct WeatherInfo {
  bool  loaded = false;
  float temp = 0, feels = 0, wind = 0;
  int   code = 0, humidity = 0;
  bool  isDay = true;
  float hi[2] = {0, 0}, lo[2] = {0, 0};
  int   dayCode[2] = {0, 0}, rain[2] = {0, 0};
};

const int MAX_COINS = 3;
struct CryptoInfo {
  bool   loaded = false;
  int    count = 0;
  String id[MAX_COINS];
  double price[MAX_COINS] = {0, 0, 0};
  double change[MAX_COINS] = {0, 0, 0};
};

// One snapshot per day, taken the first time the stats load that day
struct DaySnap {
  uint64_t views;
  uint32_t day;   // local date as days since 1 Jan 1970
  uint32_t taken; // UTC time the snapshot was taken
  uint32_t subs;
  uint32_t spare;
};
const int HISTORY_DAYS = 35;

struct ViewSample {
  time_t   t;
  uint64_t views;
};
const int SAMPLE_COUNT = 16;

// ---- Energy (Octopus) ----
const int ENERGY_DAYS = 8;
struct EnergyInfo {
  bool     loaded = false;
  String   problem;                 // shown on the page when something's missing
  float    elecRate = 0, elecStanding = 0;  // pence per kWh, pence per day (inc VAT)
  float    gasRate = 0, gasStanding = 0;
  bool     hasGas = false;
  int      days = 0;                // daily readings, oldest first
  uint32_t day[ENERGY_DAYS];        // local day number
  float    elecKwh[ENERGY_DAYS];
  float    gasKwh[ENERGY_DAYS];
};

// ---- Fuel ----
struct FuelStation {
  String brand, place;
  float  price = 0, miles = 0;
};
const int FUEL_SHOWN = 3;
struct FuelInfo {
  bool        loaded = false;
  float       avg = 0, min = 0;
  int         areaCount = 0;
  int         count = 0;
  FuelStation st[FUEL_SHOWN];
};
struct FuelDay {
  uint32_t day;
  float    avg, min;
};
const int FUEL_DAYS = 14;

// ---- Calendar ----
struct CalEvent {
  String title;
  time_t start = 0, end = 0;
  bool   allDay = false;
};
const int CAL_MAX = 8;
struct CalendarInfo {
  bool     loaded = false;
  int      count = 0;
  CalEvent ev[CAL_MAX];
};

// ---- Messages ----
struct Message {
  String from, text;
  time_t t = 0;
  bool   mine = false, poke = false;
};
const int INBOX_MAX = 6;

// ---- Free Epic games ----
struct EpicGame {
  String title;
  time_t from = 0, until = 0;
};
const int EPIC_MAX = 3;
struct GamesInfo {
  bool     loaded = false;
  int      nowCount = 0, nextCount = 0;
  EpicGame now[EPIC_MAX], next[EPIC_MAX];
};

// ---- Air quality and pollen (Open-Meteo) ----
struct AirInfo {
  bool  loaded = false;
  int   aqi = 0;           // European air quality index
  float pm25 = 0, uv = 0;
  float grass = -1, birch = -1, alder = -1, mugwort = -1, ragweed = -1, olive = -1;  // grains/m3, -1 = no data
};

// ---- Met Office weather warnings ----
const int WARN_MAX = 3;
struct WarningInfo {
  bool   loaded = false;
  int    count = 0;
  String title[WARN_MAX];   // "Amber warning of wind affecting ..."
  String when[WARN_MAX];    // "valid from 0200 Fri 23 Oct to 0900 Fri 23 Oct"
};

// ---- Joke or fact of the day ----
struct JokeInfo {
  bool     loaded = false;
  String   text;
  bool     isFact = false;
  uint32_t day = 0;
};

// ---- Spotify ----
struct SpotifyInfo {
  bool          loaded = false;
  bool          playing = false;
  String        track, artist, artUrl;
  long          progressMs = 0, durationMs = 0;
  unsigned long fetchedAt = 0;
  String        problem;
};

// ---- Exact YouTube stats (YouTube Analytics) ----
struct AnalyticsInfo {
  bool   loaded = false;
  String problem;
  String dayLabel;                   // which day "yesterday" means in the data
  long   gained = 0, lost = 0, views = 0, minutes = 0, avgSeconds = 0, likes = 0;
  long   gained28 = 0, lost28 = 0, views28 = 0, minutes28 = 0;
  double revenue = -1, revenue28 = -1;  // -1 = not monetised / no access
};

// ---- Countdowns (MOT, tax and your own) ----
const int COUNTDOWN_MAX = 8;
struct Countdown {
  String   name;
  uint32_t day;   // local day number
  bool     car;   // MOT or tax
};

// ---- Desk pet ----
enum PetStage : uint8_t { PET_NONE, PET_EGG, PET_BABY, PET_CHILD, PET_TEEN, PET_ADULT, PET_GONE };
// What the pet page shows below the scene
enum PetMenu : uint8_t { MENU_MAIN, MENU_FEED, MENU_PLAY, MENU_GAME, MENU_LASER };
const int PET_SCENE_Y = 36, PET_SCENE_H = 142;   // the pet page's animated area
const int PET_BTN_Y = 182, PET_BTN_H = 40;        // its row of buttons
// What the cat is up to
enum CatHabit : uint8_t {
  H_SIT, H_GROOM, H_STRETCH, H_WALK, H_ZOOMIES, H_NAP, H_BOX, H_CHASETAIL, H_KNEAD,
  H_YAWN, H_BLINK, H_YARN, H_MUG, H_LASER, H_PURR, H_EAT, H_HABIT_COUNT
};
struct PetState {
  uint32_t born;         // UTC the egg appeared
  uint32_t lastTick;     // UTC of the last simulated minute
  uint32_t calledAt;     // UTC it started needing you (0 = it's fine)
  uint32_t hungryAt;     // UTC its food hearts hit zero
  uint32_t sadAt;        // UTC its fun hearts hit zero
  uint32_t sickAt;       // UTC it fell ill
  uint32_t nextHunger, nextHappy, nextPoop;
  uint8_t  stage;
  uint8_t  form;         // adult look: 0 star, 1 normal, 2 grumpy
  uint8_t  hunger;       // 0-4 hearts
  uint8_t  happy;        // 0-4 hearts
  uint8_t  poops;        // 0-4
  uint8_t  sick;         // doses of medicine still needed
  uint8_t  asleep;
  uint8_t  lightsOff;
  uint8_t  mistakes;     // care mistakes - decide the adult look
  uint8_t  weight;
  uint8_t  callCounted;  // this call has already counted as a mistake
  uint8_t  spare[9];
};
