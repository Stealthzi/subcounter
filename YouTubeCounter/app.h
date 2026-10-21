// Shared settings, data types and screen wiring for the YouTube counter.
#pragma once

#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ======================= FIRMWARE =======================

#define FIRMWARE_VERSION "2.0.0"

// GitHub repository that holds the update files, as "github-name/repo-name".
// While it says "your-github-name", remote updates are switched off.
#define UPDATE_REPO "Stealthzi/subcounter"

// The WiFi network the counter creates while it's in setup mode
#define SETUP_WIFI_NAME "SubCounter-Setup"

// ======================= SCREEN =======================

const int W = 320, H = 240;

#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
const uint16_t C_BG     = RGB565(10, 10, 12);
const uint16_t C_CARD   = RGB565(32, 32, 36);
const uint16_t C_LINE   = RGB565(52, 52, 58);
const uint16_t C_RED    = RGB565(255, 0, 0);
const uint16_t C_ACCENT = RGB565(255, 82, 82);
const uint16_t C_WHITE  = RGB565(255, 255, 255);
const uint16_t C_GREY   = RGB565(170, 170, 170);
const uint16_t C_DIM    = RGB565(105, 105, 110);
const uint16_t C_GREEN  = RGB565(46, 204, 113);
const uint16_t C_AMBER  = RGB565(255, 176, 32);
const uint16_t C_BLUE   = RGB565(90, 170, 255);
const uint16_t C_YELLOW = RGB565(255, 214, 64);
const uint16_t C_CLOUD  = RGB565(215, 220, 228);

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

enum PageId : uint8_t {
  PAGE_SUBS, PAGE_CHANNEL, PAGE_LATEST, PAGE_RECENT, PAGE_TOP, PAGE_COMMENT,
  PAGE_GROWTH, PAGE_COMPARE, PAGE_LIVE, PAGE_CLOCK, PAGE_WEATHER, PAGE_CRYPTO,
  PAGE_COUNT
};

const uint32_t DEFAULT_PAGES =
  (1UL << PAGE_SUBS) | (1UL << PAGE_CHANNEL) | (1UL << PAGE_LATEST) | (1UL << PAGE_RECENT) |
  (1UL << PAGE_TOP) | (1UL << PAGE_COMMENT) | (1UL << PAGE_GROWTH) | (1UL << PAGE_COMPARE) |
  (1UL << PAGE_LIVE) | (1UL << PAGE_CLOCK) | (1UL << PAGE_WEATHER);

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
