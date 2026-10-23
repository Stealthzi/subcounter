// 7" screen health check (for the direct screen connection).
//
// The screen's DMA reads the whole picture from PSRAM once per frame and
// signals when it reaches the end. If PSRAM is too busy (big drawings), the
// DMA falls behind - that's when lines slip sideways on the screen - and the
// signal comes late, or not at all before the next frame restarts it. So
// counting frames whose end signal is off the beat counts glitches. (A long
// pause with interrupts off would also show up here, which is just as bad.)
#pragma once

#include <soc/gdma_struct.h>
#include <soc/gdma_channel.h>
#include <soc/interrupts.h>
#include <esp_intr_alloc.h>
#include <esp_cpu.h>

class ScreenWatch {
public:
  // pclkHz and the total pixel clocks per frame (including the blank parts)
  bool begin(uint32_t pclkHz, uint32_t clocksPerFrame) {
    for (int i = 0; i < SOC_GDMA_PAIRS_PER_GROUP; i++) {
      if (GDMA.channel[i].out.peri_sel.sel == SOC_GDMA_TRIG_PERIPH_LCD0) {
        _ch = i;
        break;
      }
    }
    if (_ch < 0) return false;
    _nominal = (uint32_t)((uint64_t)clocksPerFrame * getCpuFrequencyMhz() * 1000000ULL / pclkHz);
    GDMA.channel[_ch].out.int_clr.out_eof = 1;
    if (esp_intr_alloc(ETS_DMA_OUT_CH0_INTR_SOURCE + _ch, 0, isr, this, &_handle) != ESP_OK) return false;
    GDMA.channel[_ch].out.int_ena.out_eof = 1;
    return true;
  }

  // Since last asked: frames finished, frames off the beat (by over 50 us),
  // the worst amount off, and how many frames were expected in that time
  struct Stats { uint32_t frames, offbeat, worstUs, expected; };
  Stats take() {
    uint32_t mhz = getCpuFrequencyMhz();
    uint32_t now = esp_cpu_get_cycle_count();
    uint32_t expected = _nominal ? (now - _since) / _nominal : 0;
    Stats st = {_frames, _offbeat, _worst / mhz, expected};
    _frames = _offbeat = _worst = 0;
    _since = now;
    return st;
  }

private:
  int _ch = -1;
  intr_handle_t _handle = nullptr;
  uint32_t _nominal = 0, _last = 0, _since = 0;
  volatile uint32_t _frames = 0, _offbeat = 0, _worst = 0;

  static void isr(void* ctx);  // defined below, so it can live in fast memory
};

IRAM_ATTR void ScreenWatch::isr(void* ctx) {
  auto me = (ScreenWatch*)ctx;
  if (!GDMA.channel[me->_ch].out.int_st.out_eof) return;
  GDMA.channel[me->_ch].out.int_clr.out_eof = 1;
  uint32_t now = esp_cpu_get_cycle_count();
  uint32_t gap = now - me->_last;
  me->_last = now;
  me->_frames++;
  uint32_t off = gap > me->_nominal ? gap - me->_nominal : me->_nominal - gap;
  if (gap < 4 * me->_nominal) {  // ignore the first one and any after a long pause
    if (off > 50 * 240) me->_offbeat++;
    if (off > me->_worst) me->_worst = off;
  }
}
