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

// Starts the signal listening loop (stubbed for now)
void start_signal_loop(Codex* codex) {
    // Simulate scanning a SubGHz signal
    const char* dummy_signal = "433.92MHz:DEADBEEF";
    char* hash = hash_signal(dummy_signal);
    int gain = calculate_signal_gain(codex, SIGNAL_SUBGHZ);
    log_signal(codex, hash, gain, SIGNAL_SUBGHZ);

    printf("Scanned signal: %s → gain: %d\n", hash, gain);

    // Trigger shrine logic for Cave That Listens (SubGHz ritual)
    trigger_shrine(codex, SHRINE_CAVE_THAT_LISTENS, SIGNAL_SUBGHZ);

    // Feedback if shrine was awakened
    if(is_ritual_complete(codex, SHRINE_CAVE_THAT_LISTENS)) {
        popup_message_str("Shrine awakened. Pulse Open unlocked.");
    }
}

// Handles NFC scans and shrine rituals
void on_nfc_scan(Codex* codex, const char* tag_id) {
    (void)codex;
    (void)tag_id;
    // TODO: integrate Bind Whisper shrine proximity once available
}

SignalType get_signal_type(Codex* codex, const char* signal_hash) {
    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(strcmp(codex->signal_history[i].hash, signal_hash) == 0) {
            return codex->signal_history[i].signal_type;
        }
    }
    return SIGNAL_UNKNOWN;
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
