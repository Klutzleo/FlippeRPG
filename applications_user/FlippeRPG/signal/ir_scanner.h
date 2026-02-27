#pragma once
#include <gui/view_dispatcher.h>
#include <stddef.h>
#include <stdbool.h>

// Custom event posted when the scan window closes (signal found OR timeout).
// Check ir_scanner_has_signal() to distinguish the two cases.
#define IR_SCAN_DONE_EVENT 102

// Start a 3-second IR receive window.
// Captures the first decoded IR message (protocol + address + command).
// IR_SCAN_DONE_EVENT is posted when a signal is found or the window times out.
void ir_scanner_start(ViewDispatcher* dispatcher);

// Stop early and free resources. Safe to call if not active.
void ir_scanner_stop(void);

// Returns true if a scan window is currently open.
bool ir_scanner_is_active(void);

// Returns true if the last scan captured an actual IR signal (not a timeout).
bool ir_scanner_has_signal(void);

// Copy the last decoded signal hash into out_hash.
// Format: "<PROTO>:<ADDR>:<CMD>"  e.g. "NEC:0000:001F"
// Same remote button → same hash every time.
// Call only after receiving IR_SCAN_DONE_EVENT when ir_scanner_has_signal() is true.
void ir_scanner_get_hash(char* out_hash, size_t size);
