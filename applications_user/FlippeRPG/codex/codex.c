#include "codex.h"
#include <string.h>
#include <stdio.h>

void init_codex(Codex* codex, const char* player_name) {
    strncpy(codex->player_name, player_name, sizeof(codex->player_name));
    snprintf(codex->codex_id, sizeof(codex->codex_id), "CDX%04X", rand() % 65536);
    codex->xp_total = 0;
    codex->duel_xp = 0;
    memset(codex->callings_unlocked, 0, sizeof(codex->callings_unlocked));
    memset(codex->echo_log, 0, sizeof(codex->echo_log));
    memset(codex->signal_history, 0, sizeof(codex->signal_history));
    memset(codex->encounter_log, 0, sizeof(codex->encounter_log));
    memset(codex->shrine_progress, 0, sizeof(codex->shrine_progress));
    codex->save_timestamp = time(NULL);
}

void log_signal(Codex* codex, const char* signal_hash, int xp) {
    // Add to signal history
    for (int i = MAX_SIGNALS - 1; i > 0; i--) {
        codex->signal_history[i] = codex->signal_history[i - 1];
    }
    strncpy(codex->signal_history[0].hash, signal_hash, sizeof(codex->signal_history[0].hash));
    codex->signal_history[0].xp_awarded = xp;
    codex->signal_history[0].timestamp = time(NULL);

    // Update XP
    codex->xp_total += xp;
}

void update_duel_xp(Codex* codex, int xp) {
    codex->duel_xp += xp;
}

void log_encounter(Codex* codex, const char* signalborn_id, const char* aura, bool echo_transferred) {
    for (int i = MAX_ENCOUNTERS - 1; i > 0; i--) {
        codex->encounter_log[i] = codex->encounter_log[i - 1];
    }
    strncpy(codex->encounter_log[0].signalborn_id, signalborn_id, sizeof(codex->encounter_log[0].signalborn_id));
    strncpy(codex->encounter_log[0].aura, aura, sizeof(codex->encounter_log[0].aura));
    codex->encounter_log[0].timestamp = time(NULL);
    codex->encounter_log[0].echo_transferred = echo_transferred;
}