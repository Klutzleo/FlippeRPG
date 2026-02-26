#pragma once
#include <gui/view_dispatcher.h>
#include <stddef.h>
#include <stdbool.h>

// Custom event ID posted to ViewDispatcher when a tag is successfully read.
// Handle this in your ViewDispatcherCustomEventCallback, then call
// rfid_scanner_get_hash() to retrieve the result.
#define RFID_SCAN_DONE_EVENT 100

// Start scanning for LF RFID tags.
// When a tag is read, RFID_SCAN_DONE_EVENT is sent to dispatcher.
// Only one scan session is allowed at a time — check rfid_scanner_is_active() first.
void rfid_scanner_start(ViewDispatcher* dispatcher);

// Stop the active scan and free all hardware resources.
// Safe to call even if no scan is running.
void rfid_scanner_stop(void);

// Returns true if a scan session is currently active.
bool rfid_scanner_is_active(void);

// Copy the last detected tag hash into out_hash.
// Format: "<PROTOCOL>:<HEXUID>"  e.g. "EM4100:0A1B2C3D0E"
// Call only after receiving RFID_SCAN_DONE_EVENT.
void rfid_scanner_get_hash(char* out_hash, size_t size);
