// Shared settings, data types and screen wiring for the YouTube counter.
#pragma once

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ======================= FIRMWARE =======================

#define FIRMWARE_VERSION "2.2.0"

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
  {"Synthwave",    0x12002A, 0x3A0A5E, 0x24104A, 0x5A2A9A, 0x3A1A6A, 0xC8A8E8, 0x7A5A9A, 0xFF4FD8, 0x36E4FF, 0xFF9E3D},
  {"Ocean",        0x021A2A, 0x044A6A, 0x0A2A40, 0x1A5A7A, 0x124058, 0x9EC8D8, 0x5A8A9E, 0x3DFFE0, 0x3D9BFF, 0x7AF0FF},
  {"Forest",       0x07160C, 0x123A1E, 0x12261A, 0x2A5A36, 0x1C3A24, 0xA8C8AE, 0x6A8A70, 0x9BE564, 0x2EC27E, 0xF2D06B},
  {"Sunset",       0x1A0A1E, 0x5A1E2E, 0x2A1424, 0x6A2E3E, 0x40202E, 0xE8B8A8, 0x9A6A6A, 0xFFB347, 0xFF5E7E, 0xFFD27A},
  {"Galaxy",       0x05031A, 0x1A0F4A, 0x120C33, 0x3A2A7A, 0x24184F, 0xBFB6F0, 0x6E64A8, 0xB18CFF, 0x5CE1FF, 0xFF7AE6},
  {"Gold rush",    0x140F05, 0x3A2A08, 0x241A08, 0x6A5014, 0x3A2C0C, 0xE8D8A8, 0x9A8A5A, 0xFFD700, 0xFF9E1B, 0xFFF1A8},
};
const int THEME_COUNT = sizeof(THEMES) / sizeof(THEMES[0]);
// Some themes are pet rewards: the pet level needed for each (0 = always there)
const uint8_t THEME_LEVEL[THEME_COUNT] = {0, 0, 0, 0, 0, 0, 0, 25, 50};

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
#define D_BR lgfx::textdatum_t::bottom_right

// Pages are drawn on a W x H grid and scaled up on bigger screens (see screen.h)
#include "screen.h"

#if BOARD_7INCH

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

// Elecrow CrowPanel Advance 7" (DIS02170A: ESP32-S3-N16R8, 800 x 480 IPS, GT911 touch).
//
// Pins and timings come from Elecrow's own driver for this board, as collected
// and checked against the schematic in github.com/robominds/Crowpanel-Advance-7.0-HMI-Display
// (docs/HARDWARE.md). It is NOT wired like the older CrowPanel 7.0.
//
// The backlight and the touch chip's reset belong to a little helper chip on
// the I2C bus (an STC8 at address 0x30), which takes one-byte commands. Those
// commands changed between board versions, so the version (printed on the
// back, e.g. "V1.3") is a setting: 13 means V1.3 or later, 12 means V1.2.
const int ADV_I2C_PORT = 0, ADV_SDA = 15, ADV_SCL = 16, ADV_TOUCH_INT = 1;
const uint8_t ADV_HELPER = 0x30;

static void advHelperSend(uint8_t command) {
  lgfx::i2c::transactionWrite(ADV_I2C_PORT, ADV_HELPER, &command, 1, 400000);
}

// The backlight, through the helper chip
struct Light_AdvanceHelper : public lgfx::ILight {
  uint8_t boardRev = 13;
  bool init(uint8_t brightness) override {
    setBrightness(brightness);
    return true;
  }
  void setBrightness(uint8_t level) override {
    if (boardRev >= 13) {
      // 0 is brightest, 244 dimmest, 245 off
      advHelperSend(level == 0 ? 245 : (uint8_t)(244 - (244U * level) / 255));
    } else if (level == 0) {
      advHelperSend(0x05);  // V1.2: off
    } else {
      advHelperSend(0x10);  // V1.2 needs "on" before a level
      advHelperSend((uint8_t)(0x06 + (10U * level) / 255));
    }
  }
};

class CrowPanelAdvance7 : public lgfx::LGFX_Device {
  lgfx::Bus_RGB       _bus;
  lgfx::Panel_RGB     _panel;
  Light_AdvanceHelper _light;
  lgfx::Touch_GT911   _touch;

  // Ask the helper chip to reset the touch chip, holding its interrupt line
  // low so it answers on address 0x5D. The touch driver also tries 0x14.
  void wakeTouch(uint8_t boardRev) {
    lgfx::i2c::init(ADV_I2C_PORT, ADV_SDA, ADV_SCL);
    for (int attempt = 0; attempt < 3; attempt++) {
      pinMode(ADV_TOUCH_INT, OUTPUT);
      digitalWrite(ADV_TOUCH_INT, LOW);
      advHelperSend(boardRev >= 13 ? 250 : 0x19);
      delay(120);
      pinMode(ADV_TOUCH_INT, INPUT);
      delay(100);
      uint8_t probe = 0x81;  // GT911 register 0x8140 (product ID), high byte
      if (lgfx::i2c::transactionWrite(ADV_I2C_PORT, 0x5D, &probe, 1, 400000).has_value() ||
          lgfx::i2c::transactionWrite(ADV_I2C_PORT, 0x14, &probe, 1, 400000).has_value()) break;
    }
  }

public:
  void configure(uint8_t boardRev) {
    _light.boardRev = boardRev;
    wakeTouch(boardRev);

    auto panel = _panel.config();
    panel.memory_width = panel.panel_width = 800;
    panel.memory_height = panel.panel_height = 480;
    panel.offset_x = panel.offset_y = 0;
    _panel.config(panel);

    auto detail = _panel.config_detail();
    detail.use_psram = 1;
    _panel.config_detail(detail);

    auto bus = _bus.config();
    bus.panel = &_panel;
    // Blue, green, red - Elecrow's 16-bit wiring
    bus.pin_d0 = GPIO_NUM_21;  bus.pin_d1 = GPIO_NUM_47;  bus.pin_d2 = GPIO_NUM_48;
    bus.pin_d3 = GPIO_NUM_45;  bus.pin_d4 = GPIO_NUM_38;
    bus.pin_d5 = GPIO_NUM_9;   bus.pin_d6 = GPIO_NUM_10;  bus.pin_d7 = GPIO_NUM_11;
    bus.pin_d8 = GPIO_NUM_12;  bus.pin_d9 = GPIO_NUM_13;  bus.pin_d10 = GPIO_NUM_14;
    bus.pin_d11 = GPIO_NUM_7;  bus.pin_d12 = GPIO_NUM_17; bus.pin_d13 = GPIO_NUM_18;
    bus.pin_d14 = GPIO_NUM_3;  bus.pin_d15 = GPIO_NUM_46;
    bus.pin_henable = GPIO_NUM_42;
    bus.pin_vsync = GPIO_NUM_41;
    bus.pin_hsync = GPIO_NUM_40;
    bus.pin_pclk = GPIO_NUM_39;
    bus.freq_write = boardRev >= 13 ? 16000000 : 21000000;
    bus.hsync_polarity = 0; bus.hsync_front_porch = 8; bus.hsync_pulse_width = 4; bus.hsync_back_porch = 8;
    bus.vsync_polarity = 0; bus.vsync_front_porch = 8; bus.vsync_pulse_width = 4; bus.vsync_back_porch = 8;
    bus.pclk_idle_high = 1;
    _bus.config(bus);
    _panel.setBus(&_bus);
    _panel.setLight(&_light);

    auto touch = _touch.config();
    touch.x_min = 0;  touch.x_max = 799;
    touch.y_min = 0;  touch.y_max = 479;
    touch.pin_int = -1;
    touch.pin_rst = -1;
    touch.bus_shared = false;
    touch.offset_rotation = 0;
    touch.i2c_port = ADV_I2C_PORT;
    touch.pin_sda = ADV_SDA;
    touch.pin_scl = ADV_SCL;
    touch.freq = 400000;
    touch.i2c_addr = 0x5D;
    _touch.config(touch);
    _panel.setTouch(&_touch);

    setPanel(&_panel);
  }
};
typedef CrowPanelAdvance7 BoardDisplay;
// Grid point size and where the 640 x 480 page area sits on the 800 x 480 screen
const int SCREEN_SCALE = 2, SCREEN_OFFSET_X = 80;
// Reading touch too often upsets the screen on this board, so it's checked every 20 ms
const unsigned long TOUCH_POLL_MS = 20;
#define BOARD_NAME "CrowPanel Advance 7-inch"

#else

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
typedef CheapYellowDisplay BoardDisplay;
const int SCREEN_SCALE = 1, SCREEN_OFFSET_X = 0;
#define BOARD_NAME "Cheap Yellow Display 2.8-inch"

// The RGB LED on the back of the board (LOW = on)
const int LED_PIN_RED = 4, LED_PIN_GREEN = 16, LED_PIN_BLUE = 17;
// The BOOT button on the back (LOW = pressed)
const int BOOT_BUTTON_PIN = 0;

#endif

// Each board has its own update file in the GitHub release
#if BOARD_7INCH
#define FIRMWARE_FILE "firmware-7inch.bin"
#else
#define FIRMWARE_FILE "firmware.bin"
#endif

// ======================= SETTINGS =======================

// New pages go on the end so saved on/off choices keep their meaning
enum PageId : uint8_t {
  PAGE_SUBS, PAGE_CHANNEL, PAGE_LATEST, PAGE_RECENT, PAGE_TOP, PAGE_COMMENT,
  PAGE_GROWTH, PAGE_COMPARE, PAGE_LIVE, PAGE_CLOCK, PAGE_WEATHER, PAGE_CRYPTO,
  PAGE_ENERGY, PAGE_FUEL, PAGE_CALENDAR, PAGE_MESSAGES, PAGE_PET, PAGE_ALERTS,
  PAGE_GAMES, PAGE_SCREENSAVER, PAGE_EIGHTBALL, PAGE_STOPWATCH, PAGE_AIR,
  PAGE_COUNTDOWN, PAGE_MOODLAMP, PAGE_JOKE, PAGE_SPOTIFY, PAGE_ANALYTICS, PAGE_REPORT,
  PAGE_COUNT
};

// Pages that only appear once they're set up (energy, fuel...) are on by default
const uint32_t DEFAULT_PAGES = ((1UL << PAGE_COUNT) - 1) & ~(1UL << PAGE_CRYPTO);

// What may go to ChatGPT with a question (one bit each). The calendar is off
// unless you tick it, as event names can be personal.
enum AiShare : uint8_t { AI_CHANNEL = 1, AI_ENERGY = 2, AI_PETROL = 4, AI_WEATHER = 8, AI_CALENDAR = 16, AI_PET = 32 };
const uint8_t AI_DEFAULT_SHARE = AI_CHANNEL | AI_ENERGY | AI_PETROL | AI_WEATHER | AI_PET;

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
  uint8_t  petType = 0;           // Species: 0 classic, 1 cat, 2 dog, 3 hamster, 4 dragon
  uint8_t  boardRev = 13;         // 7" Advance board version: 13 = V1.3 or later, 12 = V1.2

  // Ask ChatGPT about your data (from your phone)
  String   openaiKey;             // your own OpenAI API key - stays on the board
  String   aiModel = "gpt-6-luna";
  uint8_t  aiShare = AI_DEFAULT_SHARE;  // which data it may send with a question
  bool     aiOnScreen = true;     // show answers on the counter too
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
// What the pet page shows. The first few are button rows under the scene;
// from MENU_HUB on they're whole screens (see ui_petscreens.ino).
enum PetMenu : uint8_t {
  MENU_MAIN, MENU_FEED, MENU_PLAY, MENU_GAME, MENU_LASER, MENU_CARE, MENU_TRICKS, MENU_MINI,
  MENU_HUB, MENU_MISSIONS, MENU_MISSION_LEN, MENU_REPORT, MENU_FIGHT_PICK, MENU_BATTLE, MENU_SHOP,
  MENU_BAG, MENU_BOOK, MENU_BADGES, MENU_QUESTS, MENU_PETS, MENU_BOX, MENU_MINIGAME, MENU_LEVELUP
};
const int PET_SCENE_Y = 36, PET_SCENE_H = 142;   // the pet page's animated area
const int PET_BTN_Y = 182, PET_BTN_H = 40;        // its row of buttons
// What the cat (or dog, hamster or dragon) is up to
enum CatHabit : uint8_t {
  H_SIT, H_GROOM, H_STRETCH, H_WALK, H_ZOOMIES, H_NAP, H_BOX, H_CHASETAIL, H_KNEAD,
  H_YAWN, H_BLINK, H_YARN, H_MUG, H_LASER, H_PURR, H_EAT, H_SPECIAL, H_TOY, H_TRICK, H_HABIT_COUNT
};
// Pet types. Classic and cat are always there; the rest unlock as the pet levels up.
enum Species : uint8_t { SP_CLASSIC, SP_CAT, SP_DOG, SP_HAMSTER, SP_DRAGON, SP_COUNT };
enum PetPose : uint8_t { POSE_SIT, POSE_SIT2, POSE_SLEEP, POSE_WALK1, POSE_WALK2 };
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

// ---- Pet adventures: levels, items, quests, missions (feat_rpg.ino and friends) ----
const int MAX_LEVEL = 50;
const int ITEM_MAX = 96;        // room for the item catalogue to grow
const int ROOM_SLOTS = 6;       // wallpaper, wall, rug, bed, corner, toy
const int GEAR_SLOTS = 3;       // head, neck, back
const uint8_t NO_ITEM = 0xFF;
const int REPORT_ITEMS = 6;

enum ItemType : uint8_t {
  IT_FOOD, IT_TOY, IT_WALLPAPER, IT_WALL, IT_RUG, IT_BED, IT_CORNER, IT_HEAD, IT_NECK, IT_BACK, IT_TREASURE
};
enum Rarity : uint8_t { R_COMMON, R_RARE, R_EPIC, R_LEGENDARY };

struct ItemDef {
  const char* name;
  uint8_t  type;      // ItemType
  uint8_t  rarity;    // Rarity
  uint8_t  shape;     // how it's drawn (see feat_items.ino)
  uint32_t c1, c2;    // its colours
  uint8_t  season;    // 0 any time, otherwise only found in that season (see season())
  uint16_t price;     // in the shop, 0 = never sold
};

struct Quest {
  uint8_t kind;       // QuestKind (feat_rpg.ino)
  uint8_t goal;
  uint8_t progress;
  uint8_t done;
};

// Everything the pet has earned. Saved as one block, so new fields go on the end.
struct PetRpg {
  uint8_t  version;
  uint8_t  level;               // 1-50
  uint8_t  unlocked;            // bit per Species
  uint8_t  boxes;               // unopened mystery boxes
  uint32_t xp;                  // towards the next level
  uint32_t totalXp;
  uint32_t coins;
  uint16_t streak, bestStreak;
  uint32_t lastCheckIn;         // local day of the last daily check-in
  uint8_t  owned[ITEM_MAX];     // how many of each item (up to 99)
  uint8_t  found[ITEM_MAX / 8]; // ever found - for the collection book
  uint8_t  room[ROOM_SLOTS];    // item in each room slot
  uint8_t  gear[GEAR_SLOTS];    // worn on head, neck, back
  uint8_t  coat[SP_COUNT];      // each pet type's colours
  uint32_t badges;              // achievements earned, one bit each
  Quest    daily[3], weekly[2];
  uint32_t questDay, questWeek;
  // Missions
  uint8_t  missionPlace;        // NO_ITEM = at home
  uint8_t  missionLen;          // 0 one hour, 1 four hours, 2 overnight
  uint32_t missionEnd;          // UTC
  uint32_t missionSeed;
  uint8_t  placesVisited;       // bit per place
  // The last mission's story, shown when you next look in
  uint8_t  reportPending;
  uint8_t  reportPlace;
  uint8_t  reportWins, reportLosses;
  uint8_t  reportItems[REPORT_ITEMS];
  uint16_t reportCoins, reportXp;
  uint32_t reportSeed;
  // Battles
  uint8_t  energy;              // 0-100, a fight costs 20
  uint32_t energyAt;            // UTC energy last topped up
  uint16_t battlesWon, bossesBeaten, missionsDone, boxesOpened, gamesPlayed, itemsBought;
  uint16_t subsXpEvents, lowEnergyDays, cheapFills, bestGameScore;
  uint8_t  bossWins;            // bit per place: its boss has been beaten
  // Channel and real-life XP bookkeeping
  uint32_t lastUploadHash;      // latest video already rewarded
  uint32_t lastBeatHash;        // latest video that beat your average
  uint32_t lastEnergyDay;       // day already checked for low energy use
  uint32_t lastFillDay;         // day a fill-up was logged
  uint32_t careDay;             // day careXp counts
  uint16_t careXp;              // care XP today (capped)
  uint8_t  dragonHatched;
  uint32_t dragonEggAt;         // UTC the dragon egg started hatching (0 = not yet)
  uint8_t  trickDay[4];         // day of month each trick last earned XP
  uint8_t  spare[32];
};

enum QuestKind : uint8_t {
  Q_NONE, Q_FEED, Q_STROKE, Q_PLAY, Q_CLEAN, Q_MINIGAME, Q_MISSION, Q_WIN, Q_TRICK, Q_BUY,
  Q_SUBS, Q_UPLOAD, Q_LOWENERGY, Q_FILLUP, Q_CHECKIN, Q_KIND_COUNT
};
// Badges (achievements), one bit each in PetRpg::badges
enum BadgeId : uint8_t {
  B_LEVEL5, B_LEVEL10, B_LEVEL25, B_LEVEL50, B_FIRSTBOX, B_TENBOXES, B_LEGENDARY, B_COLLECT25,
  B_COLLECTALL, B_FIRSTMISSION, B_ALLPLACES, B_OVERNIGHT, B_FIRSTWIN, B_BOSS, B_FIFTYWINS, B_STREAK7,
  B_STREAK30, B_SHOPPER, B_DESIGNER, B_GAMER, B_HIGHSCORE, B_SUBS10, B_UPLOAD, B_BEATAVG,
  B_ECO, B_FILLER, B_TRICKSTER, B_DRAGON, B_EVOLVED, B_SEASONAL, B_RICH, B_DRESSED, B_COUNT
};
// What unlocks at which pet level
const uint8_t SPECIES_LEVEL[SP_COUNT] = {1, 1, 10, 20, 30};
const uint8_t TRICK_LEVEL[4] = {15, 25, 35, 45};
const uint8_t MINIGAME_LEVEL[3] = {3, 12, 22};
const uint8_t EVOLVE_LEVEL = 40;

// Weekly report and "vs your usual": views each recent video had at set ages
const int CHECKPOINT_VIDEOS = 12;
struct VideoCheckpoint {
  uint32_t idHash;
  uint32_t published;             // UTC
  uint32_t v6, v24, v48, v168;    // views at 6 hours, 1, 2 and 7 days (0 = not seen)
};
