#pragma once
#include <gui/view_dispatcher.h>
#include <stddef.h>
#include <stdbool.h>

// Custom event posted when a decodable Sub-GHz protocol is captured,
// or when the scan window times out with no signal.
// Check subghz_scanner_has_signal() to distinguish the two cases.
#define SUBGHZ_SCAN_DONE_EVENT 103

// Start a 4-second Sub-GHz receive window on 433.92 MHz.
// Decodes OOK protocols (Princeton, CAME, KeeLoq, etc.).
// SUBGHZ_SCAN_DONE_EVENT is posted on signal capture or timeout.
void subghz_scanner_start(ViewDispatcher* dispatcher);

// Stop early and free all resources. Safe to call if not active.
void subghz_scanner_stop(void);

// Returns true if a scan window is currently open.
bool subghz_scanner_is_active(void);

// Returns true if the last scan captured a decodable protocol frame.
bool subghz_scanner_has_signal(void);

// Copy the last decoded signal hash into out_hash.
// Format: "<PROTO>:<HASH_BYTE>"  e.g. "Princeton:A3"
// Call only after SUBGHZ_SCAN_DONE_EVENT when subghz_scanner_has_signal() is true.
void subghz_scanner_get_hash(char* out_hash, size_t size);
