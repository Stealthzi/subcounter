// 7" screen only. After each drawing, LovyanGFX pushes the changed part of
// the picture out of the processor's cache into PSRAM, so the screen's DMA
// sees it. It does that with one call over the whole changed area, which
// keeps interrupts off and PSRAM busy for a long stretch, and the screen
// can fall behind. LovyanGFX lets a sketch supply this function itself, so
// here it can be done in smaller pieces with short gaps in between.
#include <sdkconfig.h>
#if CONFIG_IDF_TARGET_ESP32S3
#include <stdint.h>
#include <stddef.h>
#include <esp_cache.h>
#include <esp_rom_sys.h>

// Set over USB for testing: piece size in KB (0 = all at once, as
// LovyanGFX does), and the gap between pieces in microseconds
volatile uint16_t writebackPieceKB = 0;
volatile uint16_t writebackGapUs = 0;
volatile uint32_t writebackCalls = 0;  // shows this version is the one in use

int Cache_WriteBack_Addr(uint32_t addr, uint32_t size) {
  uintptr_t start = addr & ~127u;
  uintptr_t end = (addr + size + 127u) & ~127u;
  if (start >= end) return 0;
  writebackCalls++;
  size_t piece = writebackPieceKB ? (size_t)writebackPieceKB * 1024 : end - start;
  for (uintptr_t p = start; p < end; p += piece) {
    size_t n = end - p < piece ? end - p : piece;
    esp_cache_msync((void*)p, n, ESP_CACHE_MSYNC_FLAG_DIR_C2M | ESP_CACHE_MSYNC_FLAG_TYPE_DATA);
    if (writebackGapUs && p + n < end) esp_rom_delay_us(writebackGapUs);
  }
  return 0;
}
#endif
