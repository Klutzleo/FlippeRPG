#include "codex.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <furi.h>
#include "../narrative/echo_flavor.h"
#include "../core/constants.h"
#include "../echo/echo_fusion.h"
#include "../core/utils.h"

// -------------------- INTERNAL SIGNAL GAIN --------------------

// Adds amount to signal_score and re-derives the visible tier.
// Called by all gain sources. Amount may be negative (inactive day drain).
void apply_signal_gain(Codex* codex, int amount) {
    codex->signal_score += amount;
    if(codex->signal_score < 0) codex->signal_score = 0;

    // Walk thresholds high-to-low and set the highest tier reached
    for(int t = (int)SIGNAL_STRENGTH_RESONANT; t >= (int)SIGNAL_STRENGTH_STATIC; t--) {
        if(codex->signal_score >= signal_strength_thresholds[t]) {
            codex->signal_strength_level = t;
            return;
        }
    }
    codex->signal_strength_level = SIGNAL_STRENGTH_STATIC;
}

// -------------------- INITIALIZATION --------------------

void init_codex(Codex* codex, const char* player_name) {
    // Seed RNG once at init for random flavor text
    srand(furi_get_tick());

    // Set player name and generate a unique Codex ID
    strncpy(codex->player_name, player_name, sizeof(codex->player_name));
    snprintf(codex->codex_id, sizeof(codex->codex_id), "CDX%04X", rand() % 65536);

    // Signal strength — starts at STATIC
    codex->signal_score = 0;
    codex->signal_strength_level = SIGNAL_STRENGTH_STATIC;

    // Duel record
    codex->duels_won = 0;
    codex->duels_lost = 0;

    // Full presence at start
    codex->stamina = 100;

    // Default appearance (can be changed in settings later)
    codex->appearance = APPEARANCE_MAGE_MALE;

    // Clear all Codex subsystems
    memset(codex->echo_log, 0, sizeof(codex->echo_log));
    memset(codex->signal_history, 0, sizeof(codex->signal_history));
    memset(codex->encounter_log, 0, sizeof(codex->encounter_log));
    memset(codex->shrine_progress, 0, sizeof(codex->shrine_progress));
    memset(codex->techniques, 0, sizeof(codex->techniques));

    // Timestamp the save
    codex->save_timestamp = furi_get_tick();
}

// -------------------- SIGNAL LOGGING --------------------

// Logs a scanned signal and applies the signal strength gain.
// gain is +3 (first of type today), +1 (second), or 0 (third+).
void log_signal(Codex* codex, const char* signal_hash, int gain, SignalType signal_type) {
    // Shift signal history to make room for new entry
    for(int i = MAX_SIGNALS - 1; i > 0; i--) {
        codex->signal_history[i] = codex->signal_history[i - 1];
    }

    // Store new signal at the top (index 0 = newest)
    strncpy(codex->signal_history[0].hash, signal_hash, sizeof(codex->signal_history[0].hash));
    codex->signal_history[0].gain = gain;
    codex->signal_history[0].timestamp = furi_get_tick();
    codex->signal_history[0].signal_type = signal_type;

    // Apply signal strength gain
    if(gain > 0) apply_signal_gain(codex, gain);

    // Echo mechanics hook
    bool fusion_success = (gain >= 3);
    bool corruption_detected = (gain == 0);
    process_echo(codex, fusion_success, corruption_detected);
}

// -------------------- DUEL RESULTS --------------------

// Records duel result. Wins give signal strength gain (+5).
// Losses drain stamina only — signal strength is unaffected by duels.
void record_duel_result(Codex* codex, bool won) {
    if(won) {
        codex->duels_won++;
        apply_signal_gain(codex, 5); // Winning duels — small signal gain
    } else {
        codex->duels_lost++;
        // Stamina drain is handled by duel.c before this call
    }

    // Check for lineage convergence after duel activity
    if(ready_for_convergence(codex)) {
        process_echo(codex, false, false);
    }
}

// -------------------- ENCOUNTERS --------------------

void log_encounter(Codex* codex, const char* signalborn_id, const char* aura, bool echo_transferred) {
    // Shift encounter log to make room
    for(int i = MAX_ENCOUNTERS - 1; i > 0; i--) {
        codex->encounter_log[i] = codex->encounter_log[i - 1];
    }

    // Store new encounter at the top
    strncpy(codex->encounter_log[0].signalborn_id, signalborn_id, sizeof(codex->encounter_log[0].signalborn_id));
    strncpy(codex->encounter_log[0].aura, aura, sizeof(codex->encounter_log[0].aura));
    codex->encounter_log[0].timestamp = furi_get_tick();
    codex->encounter_log[0].echo_transferred = echo_transferred;

    if(echo_transferred) {
        // 30% chance of corruption when echoes are exchanged
        if(rand() % 100 < 30) {
            process_echo(codex, false, true);
        }
    }
}

// -------------------- TECHNIQUES --------------------

void codex_unlock_technique(Codex* codex, const char* name) {
    for(int i = 0; i < MAX_TECHNIQUES; i++) {
        if(strlen(codex->techniques[i].name) == 0) {
            strncpy(codex->techniques[i].name, name, sizeof(codex->techniques[i].name));
            codex->techniques[i].unlocked = true;
            codex->techniques[i].uses = 0;
            codex->techniques[i].mastered = false;
            return;
        }
    }
}

bool codex_has_technique(Codex* codex, const char* name) {
    for(int i = 0; i < MAX_TECHNIQUES; i++) {
        if(strcmp(codex->techniques[i].name, name) == 0 && codex->techniques[i].unlocked) {
            return true;
        }
    }
    return false;
}

void codex_use_technique(Codex* codex, const char* name) {
    for(int i = 0; i < MAX_TECHNIQUES; i++) {
        if(strcmp(codex->techniques[i].name, name) == 0 && codex->techniques[i].unlocked) {
            codex->techniques[i].uses++;
            if(codex->techniques[i].uses >= 10 && !codex->techniques[i].mastered) {
                codex->techniques[i].mastered = true;
            }
            return;
        }
    }
}

// -------------------- ECHO MECHANICS --------------------

void mark_echo_corrupted(Codex* codex, const char* echo_id) {
    for(int i = 0; i < MAX_ECHO_LOG; i++) {
        if(strcmp(codex->echo_log[i].echo_id, echo_id) == 0) {
            codex->echo_log[i].corrupted = true;
            echo_event(ECHO_CORRUPTION);

            // Corruption makes completed shrines replayable
            for(int s = 0; s < MAX_SHRINES; s++) {
                if(codex->shrine_progress[s].completed) {
                    codex->shrine_progress[s].resettable = true;
                }
            }
            return;
        }
    }
}

bool ready_for_convergence(Codex* codex) {
    // Require DEEP or higher signal strength (not just duel count)
    if(codex->signal_strength_level < SIGNAL_STRENGTH_DEEP) return false;

    for(int i = 0; i < MAX_SHRINES; i++) {
        if(!codex->shrine_progress[i].completed) return false;
    }

    for(int i = 0; i < MAX_ECHO_LOG; i++) {
        if(strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].fused) {
            return false;
        }
    }

    return true;
}

void process_echo(Codex* codex, bool fusion_success, bool corruption_detected) {
    if(fusion_success) {
        // Fusion: medium signal gain
        apply_signal_gain(codex, 8);
        echo_event(ECHO_FUSION);

        for(int i = 0; i < MAX_ECHO_LOG; i++) {
            if(strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].fused) {
                codex->echo_log[i].fused = true;
                break;
            }
        }

    } else if(corruption_detected) {
        // Corruption: small signal drain
        apply_signal_gain(codex, -2);
        echo_event(ECHO_CORRUPTION);

        for(int i = 0; i < MAX_ECHO_LOG; i++) {
            if(strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].corrupted) {
                codex->echo_log[i].corrupted = true;
                break;
            }
        }

    } else if(ready_for_convergence(codex)) {
        // Lineage convergence: large signal gain
        apply_signal_gain(codex, 50);
        echo_event(ECHO_LINEAGE);

        // Placeholder: aura assignment happens via PWA, not here
        // strncpy(codex->aura_trait, "Signalborn", sizeof(codex->aura_trait));
    }
}
