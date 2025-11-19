#include "signal_engine.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "../shrine/shrine.h"
#include "../xp/xp_engine.h"
#include "../codex/codex.h"

// Generates a simple hash from raw signal data
char* hash_signal(const char* raw_data) {
    // Stubbed hash function — replace with real hash later
    static char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", rand() % 65536);
    return hash;
}

// Calculates XP based on signal repetition and cooldown
int calculate_signal_xp(Codex* codex, const char* hash) {
    int repeat_count = 0;
    time_t now = time(NULL);

    // Check how many times this signal has been scanned in the last 24 hours
    for (int i = 0; i < MAX_SIGNALS; i++) {
        if (strcmp(codex->signal_history[i].hash, hash) == 0) {
            double hours = difftime(now, codex->signal_history[i].timestamp) / 3600.0;
            if (hours < 24.0) repeat_count++;
        }
    }

    // XP logic: encourage exploration, discourage farming
    if (repeat_count == 0) return 5;  // First time: full XP
    if (repeat_count == 1) return 1;  // Second time: reduced XP
    return 0;                         // Third+ time: no XP
}

// Starts the signal listening loop (stubbed for now)
void start_signal_loop(Codex* codex) {
    // Simulate scanning a signal
    const char* dummy_signal = "433.92MHz:DEADBEEF";
    char* hash = hash_signal(dummy_signal);
    int xp = calculate_signal_xp(codex, hash);
    log_signal(codex, hash, xp);

    printf("Scanned signal: %s → XP: %d\n", hash, xp);

    // Trigger shrine logic for Cave That Listens (SubGHz ritual)
    trigger_shrine(codex, SHRINE_CAVE_THAT_LISTENS, SIGNAL_SUBGHZ);

    // Feedback if shrine was awakened
    if (is_ritual_complete(codex, SHRINE_CAVE_THAT_LISTENS)) {
        popup_message_str("Shrine awakened. Pulse Open unlocked.");
    }
}

// Handles NFC scans and shrine rituals
void on_nfc_scan(const char* tag_id) {
    // Check if player is near the Bind Whisper shrine
    if (is_near_bind_whisper_shrine()) {
        trigger_bind_whisper_shrine(&player_codex, tag_id);
    }
}