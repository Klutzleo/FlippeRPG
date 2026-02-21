#include "signal_engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <furi.h>
#include "../shrine/shrine.h"
#include "../xp/xp_engine.h"
#include "../codex/codex.h"
#include "../core/utils.h"

// Generates a simple hash from raw signal data
char* hash_signal(const char* raw_data) {
    (void)raw_data; // unused in stubbed hash
    // Stubbed hash function — replace with real hash later
    static char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", rand() % 65536);
    return hash;
}

// Calculates XP based on signal repetition and cooldown
int calculate_signal_xp(Codex* codex, const char* hash) {
    int repeat_count = 0;
    uint32_t now = furi_get_tick();

    // Check how many times this signal has been scanned in the last 24 hours
    for (int i = 0; i < MAX_SIGNALS; i++) {
        if (strcmp(codex->signal_history[i].hash, hash) == 0) {
            uint32_t prev = codex->signal_history[i].timestamp;
            float hours = (float)(now - prev) / 3600000.0f; // ticks->hours assuming ms ticks
            if (hours < 24.0f) repeat_count++;
        }
    }

    // XP logic: encourage exploration, discourage farming
    if (repeat_count == 0) return 5;  // First time: full XP
    if (repeat_count == 1) return 1;  // Second time: reduced XP
    return 0;                         // Third+ time: no XP
}

// Starts the signal listening loop (stubbed for now)
void start_signal_loop(Codex* codex) {
    // Simulate scanning a SubGHz signal
    const char* dummy_signal = "433.92MHz:DEADBEEF";
    char* hash = hash_signal(dummy_signal);
    int xp = calculate_signal_xp(codex, hash);
    log_signal(codex, hash, xp, SIGNAL_SUBGHZ);

    printf("Scanned signal: %s → XP: %d\n", hash, xp);

    // Trigger shrine logic for Cave That Listens (SubGHz ritual)
    trigger_shrine(codex, SHRINE_CAVE_THAT_LISTENS, SIGNAL_SUBGHZ);

    // Feedback if shrine was awakened
    if (is_ritual_complete(codex, SHRINE_CAVE_THAT_LISTENS)) {
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
    // Check if signal already exists
    int xp = calculate_signal_xp(codex, signal_hash);

    // Manual entries yield reduced XP
    if (xp == 5) xp = 2;
    else if (xp == 1) xp = 0;

    log_signal(codex, signal_hash, xp, SIGNAL_UNKNOWN);
    printf("[Manual] Entered signal: %s → XP: %d\n", signal_hash, xp);
    return xp;
}