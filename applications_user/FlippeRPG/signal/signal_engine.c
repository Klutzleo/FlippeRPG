#include "signal_engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <furi.h>
#include "../shrine/shrine.h"
#include "../codex/codex.h"
#include "../core/utils.h"

// Generates a simple hash from raw signal data.
// NOTE: This is a stub — always returns a random hash.
// Replace with a deterministic hash of actual signal data (frequency, ID bytes, etc.)
// so that the same physical source produces the same hash across scans.
char* hash_signal(const char* raw_data) {
    (void)raw_data;
    static char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", rand() % 65536);
    return hash;
}

// Returns the signal gain for logging a new signal of the given type.
// Counts how many times this type has been logged in the last 24 hours
// using the existing signal_history. This is the per-type-per-day anti-farming gate:
//   First scan of this type today  → +3
//   Second scan of this type today → +1
//   Third+ scan of this type today → +0
int calculate_signal_gain(Codex* codex, SignalType type) {
    int type_count = 0;
    uint32_t now = furi_get_tick();
    // Flipper ticks are milliseconds
    uint32_t ms_per_day = 24u * 60u * 60u * 1000u;

    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(codex->signal_history[i].hash[0] == '\0') continue;
        if(codex->signal_history[i].signal_type != type) continue;
        uint32_t age = now - codex->signal_history[i].timestamp;
        if(age < ms_per_day) type_count++;
    }

    if(type_count == 0) return 3;
    if(type_count == 1) return 1;
    return 0;
}

// Handles RFID scans — routes through the same gain gate as all other bands.
// tag_id is used directly as the hash (125kHz RFID UIDs are persistent and unique).
void on_rfid_scan(Codex* codex, const char* tag_id) {
    int gain = calculate_signal_gain(codex, SIGNAL_RFID);
    log_signal(codex, tag_id, gain, SIGNAL_RFID);
}

// Handles NFC scans — routes through the same gain gate as all other bands.
// tag_id is used directly as the hash (NFC UIDs are persistent and unique).
void on_nfc_scan(Codex* codex, const char* tag_id) {
    int gain = calculate_signal_gain(codex, SIGNAL_NFC);
    log_signal(codex, tag_id, gain, SIGNAL_NFC);
}

SignalType get_signal_type(Codex* codex, const char* signal_hash) {
    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(strcmp(codex->signal_history[i].hash, signal_hash) == 0) {
            return codex->signal_history[i].signal_type;
        }
    }
    return SIGNAL_UNKNOWN;
}

// Scans a specific band type — called by the band-select signal view.
// Generates a simulated hash, calculates gain, and logs the signal.
int scan_band(Codex* codex, SignalType band_type) {
    char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", (unsigned)(rand() % 65536));
    int gain = calculate_signal_gain(codex, band_type);
    log_signal(codex, hash, gain, band_type);
    return gain;
}

int enter_manual_signal(Codex* codex, const char* signal_hash) {
    // Manual entries: same type-per-day gate, but capped lower
    // Auto: 3/1/0 → Manual: 1/0/0
    int gain = calculate_signal_gain(codex, SIGNAL_UNKNOWN);
    if(gain >= 3) gain = 1;
    else gain = 0;

    log_signal(codex, signal_hash, gain, SIGNAL_UNKNOWN);
    printf("[Manual] Entered signal: %s → gain: %d\n", signal_hash, gain);
    return gain;
}
