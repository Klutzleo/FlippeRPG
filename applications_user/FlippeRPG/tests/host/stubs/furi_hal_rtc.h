// Host-side RTC. Tests move fake_now to simulate days passing.
#pragma once
#include <stdint.h>
extern uint32_t fake_now;
static inline uint32_t furi_hal_rtc_get_timestamp(void) { return fake_now; }
