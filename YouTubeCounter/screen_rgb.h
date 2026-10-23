// The 7" screen's connection, fed through small fast "bounce" buffers.
//
// The picture lives in the board's big but slow PSRAM. LovyanGFX's own RGB bus
// streams it to the screen straight from there, so while the program is busy
// drawing (animations especially) the stream can fall behind for a moment and
// lines slip sideways - the screen "shakes". Here Espressif's driver sends the
// screen 20 lines at a time from two small buffers in fast internal memory
// instead, refilled from the picture as each one is sent. Measured on the
// board: the processor alone copies too slowly (about 700 us for 10 lines the
// screen uses up in 585 us at 14 MHz), and so does the DMA engine alone, but
// with each copying half at the same time, 20-line slices at 12 MHz take at
// most about 1100 of the 1366 us available, even on the busiest page.
// LovyanGFX still does all the drawing, into the same picture; only the way
// it reaches the screen changes. Type "s" over USB to see these timings.
#pragma once

#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_ops.h>
#include <esp_cpu.h>
#include <esp_async_memcpy.h>
#include <esp_timer.h>

// Copies memory with the chip's DMA engine, using Espressif's own async
// memcpy driver. That has to be started from a task, so the screen's
// interrupt queues each job and wakes a top-priority task on the other
// processor core, which hands it to the DMA engine straight away.
class DmaCopier {
public:
  bool begin() {
    // The task installs the driver itself, so the DMA's interrupt runs on its core too
    if (xTaskCreatePinnedToCore(taskLoop, "dmacopy", 3072, this, configMAX_PRIORITIES - 1, &_task, 0) != pdPASS) return false;
    for (int i = 0; i < 200 && !_installed && !_failed; i++) delay(1);
    _ready = _installed;
    return _ready;
  }

  bool ready() const { return _ready; }
  // Can this job go by DMA? (buffers lined up as the engine needs)
  bool fits(const void* dst, const void* src, size_t n) const {
    return _ready && !((uintptr_t)dst % 4) && !((uintptr_t)src % 64) && !(n % 64);
  }
  void copy(void* dst, const void* src, size_t n);  // from an interrupt is fine (defined below)

  // Since last asked: jobs done, jobs dropped because the queue was full, jobs
  // the driver refused (copied by the processor instead), and the longest a
  // job took from asking to finished
  volatile uint32_t jobs = 0, waited = 0, stuck = 0, worstUs = 0;

private:
  struct Job { void* dst; const void* src; size_t n; int64_t askedAt; };
  static const int QUEUE = 8;
  Job _queue[QUEUE];
  volatile int _head = 0, _tail = 0;
  int64_t _flight[QUEUE];            // when each job now with the driver was asked for, oldest first
  volatile int _fHead = 0, _fTail = 0;
  portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
  TaskHandle_t _task = nullptr;
  async_memcpy_handle_t _mcp = nullptr;
  volatile bool _installed = false, _failed = false;
  bool _ready = false;

  static void taskLoop(void* ctx);
  static bool done(async_memcpy_handle_t, async_memcpy_event_t*, void* ctx);
};

// Copies memory with the processor, in a top-priority task rather than in the
// screen's interrupt. A long copy inside the interrupt held up the screen's
// "new frame" interrupt past the short gap between frames, so frames started
// at the wrong line and the whole picture jumped up and down.
class CpuCopier {
public:
  bool begin(int core) {
    if (xTaskCreatePinnedToCore(taskLoop, "cpucopy", 2048, this, configMAX_PRIORITIES - 1, &_task, core) != pdPASS) return false;
    _ready = true;
    return true;
  }
  bool ready() const { return _ready; }
  void copy(void* dst, const void* src, size_t n);  // from an interrupt is fine (defined below)

  // Since last asked: jobs done, jobs dropped (queue full), and the longest a
  // job took from asking to finished
  volatile uint32_t jobs = 0, dropped = 0, worstUs = 0;

private:
  struct Job { void* dst; const void* src; size_t n; int64_t askedAt; };
  static const int QUEUE = 8;
  Job _queue[QUEUE];
  volatile int _head = 0, _tail = 0;
  portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
  TaskHandle_t _task = nullptr;
  bool _ready = false;

  static void taskLoop(void* ctx);
};

class Bus_RGBBounce : public lgfx::IBus {
public:
  struct config_t {
    int width = 800, height = 480;
    uint32_t freq_write = 16000000;
    int8_t pin_pclk = -1, pin_vsync = -1, pin_hsync = -1, pin_henable = -1;
    int8_t pin_data[16] = {};  // blue 0-4, green 0-5, red 0-4
    uint8_t hsync_pulse_width = 4, hsync_back_porch = 8, hsync_front_porch = 8;
    uint8_t vsync_pulse_width = 4, vsync_back_porch = 8, vsync_front_porch = 8;
    uint8_t bounce_lines = 10;  // lines per bounce buffer (0 = stream straight from PSRAM)
  };

  config_t config() const { return _cfg; }
  void config(const config_t& cfg) { _cfg = cfg; }

  // Which way it ended up running, for the log
  bool bouncing() const { return _bouncing; }
  // If the fast buffers couldn't start: which step failed, and the error code
  const char* failedStep() const { return _failStep; }
  int failedCode() const { return _failCode; }

  // Health check since last asked: refills of the fast buffers, frames,
  // refills that came too late (each shows as a glitch), refills for an
  // unexpected part of the picture (the picture rolls), the worst extra
  // delay and where in the frame it was, and the longest copy
  struct Stats { uint32_t fills, frames, late, jumps, worstUs, worstPos, cpuUs, dmaUs, dmaJobs, dmaWaited, dmaStuck, vsyncWobbleUs; };
  Stats takeStats() {
    uint32_t mhz = getCpuFrequencyMhz();
    uint32_t cpuUs = _worstCopy / mhz > _cpu.worstUs ? _worstCopy / mhz : _cpu.worstUs;
    uint32_t wobble = _vsyncMax > _vsyncMin ? (_vsyncMax - _vsyncMin) / mhz : 0;
    Stats st = {_fills, _frames, _late, _jumps, _worstExtra / mhz, _worstPos, cpuUs, _dma.worstUs,
                _dma.jobs, _dma.waited + _cpu.dropped, _dma.stuck, wobble};
    _vsyncMax = 0;
    _vsyncMin = UINT32_MAX;
    _cpu.jobs = _cpu.dropped = _cpu.worstUs = 0;
    _fills = _frames = _late = _jumps = _worstExtra = _worstPos = _worstCopy = 0;
    _dma.jobs = _dma.waited = _dma.stuck = _dma.worstUs = 0;
    return st;
  }

  // How the fast buffers are refilled: by the processor, by the DMA engine,
  // or half each at the same time
  enum Refill : uint8_t { REFILL_CPU, REFILL_DMA, REFILL_SPLIT, REFILL_SPLIT_IN_INTERRUPT };
  Refill refillMode() const { return _mode; }
  const char* refillName() const {
    return _mode == REFILL_CPU ? "cpu" : _mode == REFILL_DMA ? "dma" : _mode == REFILL_SPLIT ? "split" : "split-in-interrupt";
  }
  bool setRefill(Refill mode) {
    if (mode != REFILL_CPU && !_dma.ready()) return false;
    _mode = mode;
    return true;
  }
  volatile const void* lastBuffer = nullptr;  // for testing: where the last refill went
  volatile uint8_t dmaShare = 50;             // split refills: percent copied by the DMA engine
  uint32_t bufferUs() const { return _bufCycles / (uint32_t)getCpuFrequencyMhz(); }

  // Testing: start again with bigger or smaller fast buffers (lines must divide the height)
  bool setBufferLines(int lines) {
    if (!_bouncing || lines <= 0 || _cfg.height % lines) return false;
    int old = _cfg.bounce_lines;
    Refill mode = _mode;
    _mode = REFILL_CPU;  // let any DMA copies into the old buffers finish first
    delay(10);
    esp_lcd_panel_del(_panel);
    _panel = nullptr;
    _cfg.bounce_lines = lines;
    bool ok = createPanel(lines);
    if (!ok) {
      _cfg.bounce_lines = old;
      createPanel(old);
    }
    _mode = mode;
    return ok;
  }

  bool setPixelClock(uint32_t hz) {
    if (!_panel || esp_lcd_rgb_panel_set_pclk(_panel, hz) != ESP_OK) return false;
    _cfg.freq_write = hz;
    setTiming(_cfg.bounce_lines);
    return true;
  }

  lgfx::bus_type_t busType() const override { return lgfx::bus_type_t::bus_unknown; }

  bool init() override {
    if (_panel) return true;
    if (_cfg.bounce_lines && start(_cfg.bounce_lines)) return true;
    release();
    return start(0);  // not enough fast memory: fall back to the plain way
  }

  void release() override {
    if (_panel) esp_lcd_panel_del(_panel);
    _panel = nullptr;
    if (_ownFb) heap_caps_free(_fb);
    _fb = nullptr;
    _ownFb = false;
  }

  uint8_t* getDMABuffer(uint32_t) override { return _fb; }

  // The screen runs by itself, so none of the usual drawing commands are needed
  void beginTransaction() override {}
  void endTransaction() override {}
  void wait() override {}
  bool busy() const override { return false; }
  void flush() override {}
  bool writeCommand(uint32_t, uint_fast8_t) override { return true; }
  void writeData(uint32_t, uint_fast8_t) override {}
  void writeDataRepeat(uint32_t, uint_fast8_t, uint32_t) override {}
  void writePixels(lgfx::pixelcopy_t*, uint32_t) override {}
  void writeBytes(const uint8_t*, uint32_t, bool, bool) override {}
  void initDMA() override {}
  void addDMAQueue(const uint8_t*, uint32_t) override {}
  void execDMAQueue() override {}
  void beginRead() override {}
  void endRead() override {}
  uint32_t readData(uint_fast8_t) override { return 0; }
  bool readBytes(uint8_t*, uint32_t, bool) override { return false; }
  void readPixels(void*, lgfx::pixelcopy_t*, uint32_t) override {}

private:
  config_t _cfg;
  esp_lcd_panel_handle_t _panel = nullptr;
  uint8_t* _fb = nullptr;
  bool _ownFb = false;
  const char* _failStep = "";
  int _failCode = 0;
  // Gives up on the panel (but keeps the picture, which may still be in use)
  bool fail(const char* step, int code) {
    if (!_failStep[0]) { _failStep = step; _failCode = code; }
    if (_panel) esp_lcd_panel_del(_panel);
    _panel = nullptr;
    return false;
  }
  bool _bouncing = false;
  // refill timing, in CPU cycles
  volatile uint32_t _fills = 0, _frames = 0, _late = 0, _jumps = 0, _worstExtra = 0, _worstPos = 0, _worstCopy = 0;
  uint32_t _lastFill = 0, _bufCycles = 0, _blankCycles = 0, _postBlankPx = 0, _nextPx = 0, _framePx = 0;

  void setTiming(int lines) {
    uint64_t lineClocks = _cfg.width + _cfg.hsync_pulse_width + _cfg.hsync_back_porch + _cfg.hsync_front_porch;
    uint64_t perLine = lineClocks * getCpuFrequencyMhz() * 1000000ULL / _cfg.freq_write;
    _bufCycles = (uint32_t)(lines * perLine);
    _blankCycles = (uint32_t)((_cfg.vsync_pulse_width + _cfg.vsync_back_porch + _cfg.vsync_front_porch) * perLine);
    // The refill straight after the gap between frames is for the third buffer of the frame
    _postBlankPx = 2 * lines * _cfg.width;
    _framePx = _cfg.width * _cfg.height;
  }

  // Runs in an interrupt each time one fast buffer has been sent (defined below)
  static bool refill(esp_lcd_panel_handle_t, void* buf, int pos_px, int len_bytes, void* ctx);

  // Runs in an interrupt at each new frame: notes how evenly spaced they are
  // (a late "new frame" interrupt is what makes the picture jump up and down)
  uint32_t _lastVsync = 0;
  volatile uint32_t _vsyncMax = 0, _vsyncMin = UINT32_MAX;
  static bool onVsync(esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void* ctx);

  DmaCopier _dma;
  CpuCopier _cpu;
  Refill _mode = REFILL_CPU;

  bool start(int lines) {
    // With the fast buffers, the picture is kept here and copied by refill(),
    // which also times each copy
    // Refill half by the processor and half by the DMA engine at the same time:
    // measured, neither alone is quick enough, together they are
    if (lines > 0 && !_cpu.ready()) _cpu.begin(1);  // same core as the screen's interrupt
    if (lines > 0 && !_dma.ready() && _dma.begin()) _mode = REFILL_SPLIT;
    if (lines > 0 && !_fb) {
      _fb = (uint8_t*)heap_caps_aligned_calloc(64, 1, _cfg.width * _cfg.height * 2, MALLOC_CAP_SPIRAM);
      if (!_fb) return fail("picture memory", 0);
      _ownFb = true;
    }
    return createPanel(lines);
  }

  bool createPanel(int lines) {
    esp_lcd_rgb_panel_config_t pc;
    memset(&pc, 0, sizeof(pc));
    pc.clk_src = LCD_CLK_SRC_PLL240M;
    pc.timings.pclk_hz = _cfg.freq_write;
    pc.timings.h_res = _cfg.width;
    pc.timings.v_res = _cfg.height;
    pc.timings.hsync_pulse_width = _cfg.hsync_pulse_width;
    pc.timings.hsync_back_porch = _cfg.hsync_back_porch;
    pc.timings.hsync_front_porch = _cfg.hsync_front_porch;
    pc.timings.vsync_pulse_width = _cfg.vsync_pulse_width;
    pc.timings.vsync_back_porch = _cfg.vsync_back_porch;
    pc.timings.vsync_front_porch = _cfg.vsync_front_porch;
    // The same signal shapes LovyanGFX's bus used on this board
    pc.timings.flags.hsync_idle_low = 1;
    pc.timings.flags.vsync_idle_low = 1;
    pc.timings.flags.de_idle_high = 0;
    pc.timings.flags.pclk_active_neg = 1;
    pc.timings.flags.pclk_idle_high = 0;
    pc.data_width = 16;
    pc.bits_per_pixel = 16;
    pc.num_fbs = lines > 0 ? 0 : 1;  // with the fast buffers the picture is our own (see below)
    pc.bounce_buffer_size_px = lines * _cfg.width;
    pc.dma_burst_size = 64;
    pc.hsync_gpio_num = _cfg.pin_hsync;
    pc.vsync_gpio_num = _cfg.pin_vsync;
    pc.de_gpio_num = _cfg.pin_henable;
    pc.pclk_gpio_num = _cfg.pin_pclk;
    pc.disp_gpio_num = -1;
    // LovyanGFX keeps each pixel's two bytes the other way round, so the
    // data lines are swapped over in pairs of bytes to match
    for (int i = 0; i < 16; i++) pc.data_gpio_nums[i < 8 ? i + 8 : i - 8] = _cfg.pin_data[i];
    pc.flags.fb_in_psram = 1;
    pc.flags.no_fb = lines > 0;

    esp_err_t err = esp_lcd_new_rgb_panel(&pc, &_panel);
    if (err != ESP_OK) {
      _panel = nullptr;
      return fail("new panel", err);
    }
    if (lines > 0) {
      esp_lcd_rgb_panel_event_callbacks_t cbs;
      memset(&cbs, 0, sizeof(cbs));
      cbs.on_bounce_empty = refill;
      cbs.on_vsync = onVsync;
      setTiming(lines);
      err = esp_lcd_rgb_panel_register_event_callbacks(_panel, &cbs, this);
      if (err != ESP_OK) return fail("callbacks", err);
    }
    err = esp_lcd_panel_reset(_panel);
    if (err != ESP_OK) return fail("reset", err);
    err = esp_lcd_panel_init(_panel);
    if (err != ESP_OK) return fail("init", err);
    if (lines == 0) {
      void* fb = nullptr;
      if (esp_lcd_rgb_panel_get_frame_buffer(_panel, 1, &fb) != ESP_OK || !fb) return fail("frame buffer", 0);
      _fb = (uint8_t*)fb;
    }
    _bouncing = lines > 0;
    return true;
  }
};

// Runs in an interrupt each time one fast buffer has been sent: copy the next
// few lines of the picture into it, and time how long since the last one.
// (Defined outside the class so it can live in fast internal memory.)
IRAM_ATTR bool Bus_RGBBounce::refill(esp_lcd_panel_handle_t, void* buf, int pos_px, int len_bytes, void* ctx) {
  auto me = (Bus_RGBBounce*)ctx;
  uint32_t start = esp_cpu_get_cycle_count();
  const uint8_t* src = me->_fb + pos_px * 2;
  me->lastBuffer = buf;
  int half = (len_bytes * (100 - me->dmaShare) / 100) & ~63;  // the processor's part
  if (me->_mode == REFILL_DMA && me->_dma.fits(buf, src, len_bytes)) {
    me->_dma.copy(buf, src, len_bytes);  // finishes in the background
  } else if (me->_mode == REFILL_SPLIT && me->_cpu.ready() &&
             me->_dma.fits((uint8_t*)buf + half, src + half, len_bytes - half)) {
    // The DMA engine takes the second half while the processor copies the
    // first - both outside this interrupt, so it finishes straight away
    me->_dma.copy((uint8_t*)buf + half, src + half, len_bytes - half);
    me->_cpu.copy(buf, src, half);
  } else if (me->_mode == REFILL_SPLIT_IN_INTERRUPT && me->_dma.fits((uint8_t*)buf + half, src + half, len_bytes - half)) {
    // The old way, for comparison: the processor's half copied right here
    me->_dma.copy((uint8_t*)buf + half, src + half, len_bytes - half);
    memcpy(buf, src, half);
    uint32_t copy = esp_cpu_get_cycle_count() - start;
    if (copy > me->_worstCopy) me->_worstCopy = copy;
  } else if (me->_cpu.ready()) {
    me->_cpu.copy(buf, src, len_bytes);
  } else {
    memcpy(buf, src, len_bytes);
    uint32_t copy = esp_cpu_get_cycle_count() - start;
    if (copy > me->_worstCopy) me->_worstCopy = copy;
  }

  uint32_t gap = start - me->_lastFill;
  me->_lastFill = start;
  me->_fills++;
  if (pos_px == 0) me->_frames++;
  if ((uint32_t)pos_px != me->_nextPx) me->_jumps++;
  me->_nextPx = pos_px + len_bytes / 2;
  if (me->_nextPx >= me->_framePx) me->_nextPx = 0;

  uint32_t expected = me->_bufCycles + ((uint32_t)pos_px == me->_postBlankPx ? me->_blankCycles : 0);
  uint32_t extra = gap > expected ? gap - expected : 0;
  if (extra > me->_bufCycles / 2) me->_late++;
  if (extra > me->_worstExtra) {
    me->_worstExtra = extra;
    me->_worstPos = pos_px / (len_bytes / 2);  // which buffer of the frame
  }
  return false;
}

IRAM_ATTR void DmaCopier::copy(void* dst, const void* src, size_t n) {
  int64_t now = esp_timer_get_time();
  portENTER_CRITICAL_SAFE(&_lock);
  int next = (_head + 1) % QUEUE;
  if (next == _tail) {
    waited++;  // queue full: drop it (that slice shows stale for one frame)
  } else {
    _queue[_head] = {dst, src, n, now};
    _head = next;
  }
  portEXIT_CRITICAL_SAFE(&_lock);
  BaseType_t woken = pdFALSE;
  vTaskNotifyGiveFromISR(_task, &woken);
}

void DmaCopier::taskLoop(void* ctx) {
  auto me = (DmaCopier*)ctx;
  async_memcpy_config_t cfg = ASYNC_MEMCPY_DEFAULT_CONFIG();
  cfg.backlog = QUEUE;
  cfg.dma_burst_size = 64;
  if (esp_async_memcpy_install(&cfg, &me->_mcp) != ESP_OK) {
    me->_failed = true;
    vTaskDelete(nullptr);
    return;
  }
  me->_installed = true;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;) {
      Job job;
      portENTER_CRITICAL(&me->_lock);
      bool any = me->_tail != me->_head;
      if (any) {
        job = me->_queue[me->_tail];
        me->_tail = (me->_tail + 1) % QUEUE;
        me->_flight[me->_fHead] = job.askedAt;
        me->_fHead = (me->_fHead + 1) % QUEUE;
      }
      portEXIT_CRITICAL(&me->_lock);
      if (!any) break;
      if (esp_async_memcpy(me->_mcp, job.dst, (void*)job.src, job.n, done, me) != ESP_OK) {
        portENTER_CRITICAL(&me->_lock);
        me->_fHead = (me->_fHead + QUEUE - 1) % QUEUE;  // never went in flight
        portEXIT_CRITICAL(&me->_lock);
        memcpy(job.dst, job.src, job.n);
        me->stuck++;
      }
    }
  }
}

// The DMA engine finished a job (runs in an interrupt)
IRAM_ATTR bool DmaCopier::done(async_memcpy_handle_t, async_memcpy_event_t*, void* ctx) {
  auto me = (DmaCopier*)ctx;
  int64_t now = esp_timer_get_time();
  portENTER_CRITICAL_SAFE(&me->_lock);
  if (me->_fTail != me->_fHead) {
    uint32_t took = (uint32_t)(now - me->_flight[me->_fTail]);
    me->_fTail = (me->_fTail + 1) % QUEUE;
    if (took > me->worstUs) me->worstUs = took;
  }
  me->jobs++;
  portEXIT_CRITICAL_SAFE(&me->_lock);
  return false;
}

IRAM_ATTR void CpuCopier::copy(void* dst, const void* src, size_t n) {
  int64_t now = esp_timer_get_time();
  portENTER_CRITICAL_SAFE(&_lock);
  int next = (_head + 1) % QUEUE;
  if (next == _tail) {
    dropped++;
  } else {
    _queue[_head] = {dst, src, n, now};
    _head = next;
  }
  portEXIT_CRITICAL_SAFE(&_lock);
  BaseType_t woken = pdFALSE;
  vTaskNotifyGiveFromISR(_task, &woken);
  if (woken) portYIELD_FROM_ISR();
}

void CpuCopier::taskLoop(void* ctx) {
  auto me = (CpuCopier*)ctx;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    for (;;) {
      Job job;
      portENTER_CRITICAL(&me->_lock);
      bool any = me->_tail != me->_head;
      if (any) {
        job = me->_queue[me->_tail];
        me->_tail = (me->_tail + 1) % QUEUE;
      }
      portEXIT_CRITICAL(&me->_lock);
      if (!any) break;
      memcpy(job.dst, job.src, job.n);
      uint32_t took = (uint32_t)(esp_timer_get_time() - job.askedAt);
      if (took > me->worstUs) me->worstUs = took;
      me->jobs++;
    }
  }
}

IRAM_ATTR bool Bus_RGBBounce::onVsync(esp_lcd_panel_handle_t, const esp_lcd_rgb_panel_event_data_t*, void* ctx) {
  auto me = (Bus_RGBBounce*)ctx;
  uint32_t now = esp_cpu_get_cycle_count();
  uint32_t gap = now - me->_lastVsync;
  me->_lastVsync = now;
  if (gap < 240000000) {  // ignore the first one, and any after a pause
    if (gap > me->_vsyncMax) me->_vsyncMax = gap;
    if (gap < me->_vsyncMin) me->_vsyncMin = gap;
  }
  return false;
}
