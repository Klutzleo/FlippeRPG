#include "signal_engine.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

char* hash_signal(const char* raw_data) {
    // Simple hash stub (replace with real hash later)
    static char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", rand() % 65536);
    return hash;
}

int calculate_signal_xp(Codex* codex, const char* hash) {
    int repeat_count = 0;
    time_t now = time(NULL);

    for (int i = 0; i < MAX_SIGNALS; i++) {
        if (strcmp(codex->signal_history[i].hash, hash) == 0) {
            double hours = difftime(now, codex->signal_history[i].timestamp) / 3600.0;
            if (hours < 24.0) repeat_count++;
        }
    }

    if (repeat_count == 0) return 5;
    if (repeat_count == 1) return 1;
    return 0;
}

void start_signal_loop(Codex* codex) {
    // Stubbed signal loop
    const char* dummy_signal = "433.92MHz:DEADBEEF";
    char* hash = hash_signal(dummy_signal);
    int xp = calculate_signal_xp(codex, hash);
    log_signal(codex, hash, xp);

    printf("Scanned signal: %s → XP: %d\n", hash, xp);
}