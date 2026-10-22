// Draws the pages at the right size for each board.
//
// Every page is laid out on a 320 x 240 grid - the 2.8" screen's size. Gfx
// passes drawing on to the real screen (or a memory canvas), multiplying every
// position and size by `s`. On the 2.8" board s = 1 and nothing changes. On the
// 7" board (800 x 480) s = 2: everything comes out twice the size, centred, and
// text switches to fonts twice as big so it stays sharp rather than blocky.
#pragma once

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define BOARD_7INCH 1
#else
#define BOARD_7INCH 0
#endif

#if BOARD_7INCH
#include "fonts_big.h"
#endif

class Gfx {
public:
  lgfx::LovyanGFX* d;   // where the drawing really goes
  int s;                // real pixels per grid point
  int ox, oy;           // where the grid starts on the real screen

  Gfx(lgfx::LovyanGFX* target, int scale = 1, int offX = 0, int offY = 0)
    : d(target), s(scale), ox(offX), oy(offY) {}

  int X(int x) const { return x * s + ox; }
  int Y(int y) const { return y * s + oy; }
  int CX(int x) const { return x * s + ox + s / 2; }  // middle of a grid point
  int CY(int y) const { return y * s + oy + s / 2; }
  int32_t FX(float x) const { return s == 1 ? (int32_t)x + ox : (int32_t)lroundf(x * s + ox + (s - 1) * 0.5f); }
  int32_t FY(float y) const { return s == 1 ? (int32_t)y + oy : (int32_t)lroundf(y * s + oy + (s - 1) * 0.5f); }
  int32_t toGrid(int32_t v) const { return s == 1 ? v : (v + s - 1) / s; }

  int32_t width() const { return (d->width() - 2 * ox) / s; }
  int32_t height() const { return (d->height() - 2 * oy) / s; }

  // ---------------------------------------------------------------- text

  void setFont(const lgfx::IFont* f) { _font = f; applyFont(); }
  void setTextSize(float size) { _size = size; applyFont(); }
  template <typename T> void setTextColor(T fg) { d->setTextColor(fg); }
  template <typename T, typename U> void setTextColor(T fg, U bg) { d->setTextColor(fg, bg); }
  void setTextDatum(lgfx::textdatum_t datum) { d->setTextDatum(datum); }
  int32_t textWidth(const char* str) { return toGrid(d->textWidth(str)); }
  int32_t textWidth(const String& str) { return textWidth(str.c_str()); }
  int32_t fontHeight() { return d->fontHeight() / s; }
  int32_t drawString(const char* str, int x, int y) { return toGrid(d->drawString(str, X(x), Y(y))); }
  int32_t drawString(const String& str, int x, int y) { return drawString(str.c_str(), x, y); }

  // ---------------------------------------------------------------- shapes

  void startWrite() { d->startWrite(); }
  void endWrite() { d->endWrite(); }
  template <typename T> void fillScreen(const T& c) { d->fillScreen(c); }
  template <typename T> void fillRect(int x, int y, int w, int h, const T& c) { d->fillRect(X(x), Y(y), w * s, h * s, c); }
  template <typename T> void drawPixel(int x, int y, const T& c) { d->fillRect(X(x), Y(y), s, s, c); }
  template <typename T> void drawFastHLine(int x, int y, int w, const T& c) { d->fillRect(X(x), Y(y), w * s, s, c); }
  template <typename T> void drawFastVLine(int x, int y, int h, const T& c) { d->fillRect(X(x), Y(y), s, h * s, c); }
  template <typename T> void writeFastHLine(int x, int y, int w, const T& c) { d->writeFillRect(X(x), Y(y), w * s, s, c); }
  template <typename T> void writeFastVLine(int x, int y, int h, const T& c) { d->writeFillRect(X(x), Y(y), s, h * s, c); }
  template <typename T> void fillRoundRect(int x, int y, int w, int h, int r, const T& c) { d->fillRoundRect(X(x), Y(y), w * s, h * s, r * s, c); }
  template <typename T> void fillSmoothRoundRect(int x, int y, int w, int h, int r, const T& c) { d->fillSmoothRoundRect(X(x), Y(y), w * s, h * s, r * s, c); }
  template <typename T> void fillCircle(int x, int y, int r, const T& c) { d->fillCircle(CX(x), CY(y), r * s, c); }
  template <typename T> void fillSmoothCircle(int x, int y, int r, const T& c) { d->fillSmoothCircle(CX(x), CY(y), r * s, c); }
  template <typename T> void fillEllipse(int x, int y, int rx, int ry, const T& c) { d->fillEllipse(CX(x), CY(y), rx * s, ry * s, c); }
  template <typename T> void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, const T& c) {
    d->fillTriangle(CX(x0), CY(y0), CX(x1), CY(y1), CX(x2), CY(y2), c);
  }
  template <typename T> void fillArc(int x, int y, int r0, int r1, float a0, float a1, const T& c) {
    d->fillArc(CX(x), CY(y), r0 * s, r1 * s, a0, a1, c);
  }
  // Outlines and lines stay one grid point thick
  template <typename T> void drawCircle(int x, int y, int r, const T& c) {
    if (s == 1) d->drawCircle(CX(x), CY(y), r, c);
    else d->fillArc(CX(x), CY(y), r * s, r * s - (s - 1), 0, 360, c);
  }
  template <typename T> void drawLine(int x0, int y0, int x1, int y1, const T& c) {
    if (s == 1) d->drawLine(X(x0), Y(y0), X(x1), Y(y1), c);
    else d->drawWideLine(CX(x0), CY(y0), CX(x1), CY(y1), s / 2.0f, c);
  }
  template <typename T> void drawWideLine(float x0, float y0, float x1, float y1, float r, const T& c) {
    d->drawWideLine(FX(x0), FY(y0), FX(x1), FY(y1), r * s, c);
  }

  // ---------------------------------------------------------------- other

  void setClipRect(int x, int y, int w, int h) { d->setClipRect(X(x), Y(y), w * s, h * s); }
  void clearClipRect() { d->clearClipRect(); }
  template <typename T> void readRect(int x, int y, int w, int h, T* data) { d->readRect(X(x), Y(y), w * s, h * s, data); }
  void qrcode(const char* text, int x, int y, int w, uint8_t version, bool margin) { d->qrcode(text, X(x), Y(y), w * s, version, margin); }
  bool drawJpg(const uint8_t* data, uint32_t len, int x, int y, int maxW, int maxH, int offX, int offY, float scaleX, float scaleY) {
    return d->drawJpg(data, len, X(x), Y(y), maxW * s, maxH * s, offX * s, offY * s, scaleX * s, scaleY * s);
  }

protected:
  const lgfx::IFont* _font = &lgfx::fonts::Font0;
  float _size = 1;

  // On a bigger screen, swap in the font nearest the size the page asked for
  void applyFont() {
    if (s == 1) {
      d->setFont(_font);
      d->setTextSize(_size);
      return;
    }
    float extra = 1;
    const lgfx::IFont* real = biggerFont(_font, _size * s, extra);
    d->setFont(real);
    d->setTextSize(extra);
  }

  struct Sized { const lgfx::IFont* font; float pt; };

  static const lgfx::IFont* biggerFont(const lgfx::IFont* f, float times, float& extra) {
    static const Sized bold[] = {
      {&lgfx::fonts::FreeSansBold9pt7b, 9}, {&lgfx::fonts::FreeSansBold12pt7b, 12},
      {&lgfx::fonts::FreeSansBold18pt7b, 18}, {&lgfx::fonts::FreeSansBold24pt7b, 24},
#if BOARD_7INCH
      {&BigSansBold36, 36}, {&BigSansBold48, 48}, {&BigSansBold64, 64}, {&BigSansBold84, 84},
#endif
    };
    static const Sized regular[] = {
      {&lgfx::fonts::FreeSans9pt7b, 9}, {&lgfx::fonts::FreeSans12pt7b, 12},
      {&lgfx::fonts::FreeSans18pt7b, 18}, {&lgfx::fonts::FreeSans24pt7b, 24},
    };
    const Sized* family;
    int count;
    float pt;
    if (f == &lgfx::fonts::Font2) { family = regular; count = 4; pt = 6; }  // a small plain font
    else if (f == &lgfx::fonts::FreeSans9pt7b) { family = regular; count = 4; pt = 9; }
    else {
      family = bold;
      count = sizeof(bold) / sizeof(bold[0]);
      if (f == &lgfx::fonts::FreeSansBold9pt7b) pt = 9;
      else if (f == &lgfx::fonts::FreeSansBold12pt7b) pt = 12;
      else if (f == &lgfx::fonts::FreeSansBold18pt7b) pt = 18;
      else if (f == &lgfx::fonts::FreeSansBold24pt7b) pt = 24;
      else { extra = times; return f; }  // anything else just gets stretched
    }
    // The biggest font that isn't much bigger than wanted, stretched the rest of the way
    float want = pt * times;
    int pick = 0;
    for (int i = 0; i < count; i++) {
      if (family[i].pt <= want * 1.05f) pick = i;
    }
    extra = want / family[pick].pt;
    if (extra > 0.95f && extra < 1.05f) extra = 1;
    return family[pick].font;
  }
};

// A memory canvas that follows the same grid as the screen it's shown on
class GfxSprite : public Gfx {
public:
  LGFX_Sprite sprite;
  Gfx* parent;

  GfxSprite(Gfx* p) : Gfx(nullptr, p->s), sprite(p->d), parent(p) { d = &sprite; }

  void setColorDepth(int bits) { sprite.setColorDepth(bits); }
  void* createSprite(int w, int h) {
#if BOARD_7INCH
    sprite.setPsram(true);
#endif
    return sprite.createSprite(w * s, h * s);
  }
  void deleteSprite() { sprite.deleteSprite(); }
  void pushSprite(int x, int y) { sprite.pushSprite(parent->d, parent->X(x), parent->Y(y)); }
  void* getBuffer() { return sprite.getBuffer(); }
  uint32_t bufferLength() const { return sprite.bufferLength(); }
};

// The real screen, plus its touch panel and backlight
class Screen : public Gfx {
public:
  lgfx::LGFX_Device* dev;
  uint8_t touchFlip = 0;            // 7" only: 1 = left/right swapped, 2 = up/down swapped
  unsigned long touchPollMs = 0;    // read the touch panel at most this often (0 = every time)

  Screen(lgfx::LGFX_Device* device, int scale, int offX, int offY)
    : Gfx(device, scale, offX, offY), dev(device) {}

  void setBrightness(uint8_t level) { dev->setBrightness(level); }

  // Where the finger is, on the page grid
  int getTouch(int32_t* x, int32_t* y) {
    int32_t px = 0, py = 0;
    int n = readTouch(&px, &py);
    if (n <= 0 || s == 1) {
      *x = px;
      *y = py;
      return n;
    }
    if (touchFlip & 1) px = dev->width() - 1 - px;
    if (touchFlip & 2) py = dev->height() - 1 - py;
    *x = constrain((px - ox) / s, 0, W - 1);
    *y = constrain((py - oy) / s, 0, H - 1);
    return n;
  }

  // Raw touch position in real pixels (used to check the touch panel's direction)
  int rawTouch(int32_t* x, int32_t* y) { return readTouch(x, y); }

private:
  unsigned long _lastRead = 0;
  int _lastN = 0;
  int32_t _lastX = 0, _lastY = 0;

  int readTouch(int32_t* x, int32_t* y) {
    if (touchPollMs && millis() - _lastRead < touchPollMs) {
      *x = _lastX;
      *y = _lastY;
      return _lastN;
    }
    _lastRead = millis();
    _lastN = dev->getTouch(&_lastX, &_lastY);
    *x = _lastX;
    *y = _lastY;
    return _lastN;
  }

public:

  // The 7" screen is wider than the page grid: paint the strips either side
  void fillSides(uint16_t (*colourAt)(int)) {
    if (ox <= 0) return;
    d->startWrite();
    for (int y = 0; y < H; y++) {
      uint16_t c = colourAt(y);
      d->writeFillRect(0, Y(y), ox, s, c);
      d->writeFillRect(d->width() - ox, Y(y), ox, s, c);
    }
    d->endWrite();
  }
  void fillSides(uint16_t colour) {
    if (ox <= 0) return;
    d->fillRect(0, 0, ox, d->height(), colour);
    d->fillRect(d->width() - ox, 0, ox, d->height(), colour);
  }
};
