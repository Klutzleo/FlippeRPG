#pragma once
#include <gui/view_dispatcher.h>
#include <stddef.h>
#include <stdbool.h>

// Custom event ID posted to ViewDispatcher when a scan window closes.
// Call rf_scanner_get_hash() to retrieve the result.
#define RF_SCAN_DONE_EVENT 101

// Start a 1.5-second RF scan on 433.92 MHz.
// When the window closes, RF_SCAN_DONE_EVENT is sent to dispatcher.
// A hash is generated from the peak RSSI — no physical tag required.
void rf_scanner_start(ViewDispatcher* dispatcher);

// Stop early and free resources. Safe to call if not active.
// Will block for up to 100ms waiting for the scan thread to exit.
void rf_scanner_stop(void);

// Returns true if a scan window is currently open.
bool rf_scanner_is_active(void);

// Copy the last scan's hash into out_hash.
// Format: "RF:<RSSI_HEX><TICK_HEX>"
// Call only after receiving RF_SCAN_DONE_EVENT.
void rf_scanner_get_hash(char* out_hash, size_t size);
