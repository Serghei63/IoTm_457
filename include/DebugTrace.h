#pragma once
//
#include "Global.h"

#define RESTART_DEBUG_INFO
#if defined(RESTART_DEBUG_INFO) && defined(ESP32) && !defined(esp32c3m_4mb)
#define CONFIG_RESTART_DEBUG_STACK_DEPTH 15
typedef struct {
  size_t heap_total;
  size_t heap_free;
  size_t heap_free_min;
  time_t heap_min_time;
  uint32_t backtrace[CONFIG_RESTART_DEBUG_STACK_DEPTH];
} re_restart_debug_t;

#endif // RESTART_DEBUG_INFO
extern "C" void __real_esp_panic_handler(void*);
void printDebugTrace();
void sendDebugTraceAndFreeMemory(bool);
void IRAM_ATTR debugUpdate();

extern "C" bool verifyRollbackLater();
void verifyFirmware();