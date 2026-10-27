#include "codex.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <furi.h>
#include <furi_hal_rtc.h>
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
    // Start from a clean struct so nothing from a failed load leaks into a new player
    memset(codex, 0, sizeof(Codex));

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

    // Band gate — IR visible from the start, others hidden until earned (Tier 1: IR→SubGHz→NFC→BT)
    memset(codex->band_scans, 0, sizeof(codex->band_scans));
    codex->bands_unlocked   = 1;
    codex->substrate_unlocked = false;

    // Timestamp the save and start the daily clocks
    codex->save_timestamp = (time_t)furi_hal_rtc_get_timestamp();
    codex->last_active_day = codex_today();
    codex->stamina_updated = furi_hal_rtc_get_timestamp();
}

// -------------------- SIGNAL LOGGING --------------------

// Logs a counted scan, advances the band gate, and applies the signal gain.
// Uncounted scans (same source again today, too soon, daily max) are never logged.
// gain is +3 (first counted scan of the type today), +1 (second), or 0 (third+).
void log_signal(Codex* codex, const char* signal_hash, int gain, SignalType signal_type) {
    // Shift signal history to make room for new entry
    for(int i = MAX_SIGNALS - 1; i > 0; i--) {
        codex->signal_history[i] = codex->signal_history[i - 1];
    }

    // Store new signal at the top (index 0 = newest)
    strncpy(codex->signal_history[0].hash, signal_hash, sizeof(codex->signal_history[0].hash));
    codex->signal_history[0].gain = gain;
    codex->signal_history[0].timestamp = (time_t)furi_hal_rtc_get_timestamp();
    codex->signal_history[0].signal_type = signal_type;

    // Every counted scan advances the gate until the band is full. Scans past that still
    // earn signal but add no more transmissions.
    if(signal_type != SIGNAL_UNKNOWN && codex->band_scans[(int)signal_type] < SCANS_PER_BAND) {
        codex->band_scans[(int)signal_type]++;

        if(codex->band_scans[(int)signal_type] == SCANS_PER_BAND) {
            for(int i = 0; i < NUM_BANDS; i++) {
                if(band_order[i] == signal_type) codex->band_complete_day[i] = codex_today();
            }
        }
    }

    if(gain > 0) apply_signal_gain(codex, gain);
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

    // Only the real shrines count — the rest of the MAX_SHRINES slots are spare
    for(int i = 0; i < SHRINE_UNKNOWN; i++) {
        if(shrine_is_active(i) && !codex->shrine_progress[i].completed) return false;
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

// -------------------- BAND GATE --------------------

// RTC date as a day number. Counters reset when it changes (midnight on the Flipper's clock).
uint32_t codex_today(void) {
    return furi_hal_rtc_get_timestamp() / SECONDS_PER_DAY;
}

// Sequential unlock: band i+1 opens once band i has SCANS_PER_BAND counted scans — and, with
// BAND_DAY_GATE, only on a later day than the one band i completed.
// Bands are hidden until they open, so a waiting band simply isn't there yet.
void codex_refresh_bands(Codex* codex) {
    uint32_t today = codex_today();
    int unlocked = 1; // The first band is always visible
    for(int i = 0; i < NUM_BANDS - 1; i++) {
        if(codex->band_scans[(int)band_order[i]] < SCANS_PER_BAND) break;
#if BAND_DAY_GATE
        // 0 = completed before the day was tracked; don't hold those players back
        if(codex->band_complete_day[i] != 0 && today <= codex->band_complete_day[i]) break;
#endif
        unlocked = i + 2;
    }
    codex->bands_unlocked = unlocked;
}

// Refreshes the visible bands and checks if The Substrate just opened.
// Call after every counted scan. Returns true exactly once — when substrate_unlocked flips to true.
bool check_band_gate(Codex* codex) {
    codex_refresh_bands(codex);

    // Substrate unlocks when all Tier 1 bands (band_order[0..NUM_BANDS-1]) are complete
    if(!codex->substrate_unlocked) {
        bool all_done = true;
        for(int i = 0; i < NUM_BANDS; i++) {
            if(codex->band_scans[(int)band_order[i]] < SCANS_PER_BAND) {
                all_done = false;
                break;
            }
        }
        if(all_done) {
            codex->substrate_unlocked  = true;
            codex->zero_day_confirmed  = true;
            codex->zero_day_date       = (time_t)furi_hal_rtc_get_timestamp();
            return true; // Caller fires CQ Morse — the only haptic in the game
        }
    }
    return false;
}

// -------------------- TIME PASSAGE --------------------

// Credits the stamina earned since stamina_updated. Stamina is full → the clock just restarts.
static void recover_stamina(Codex* codex, uint32_t now) {
    if(codex->stamina >= STAMINA_MAX || codex->stamina_updated == 0 || now < codex->stamina_updated) {
        if(codex->stamina > STAMINA_MAX) codex->stamina = STAMINA_MAX;
        codex->stamina_updated = now;
        return;
    }
    uint32_t points = (now - codex->stamina_updated) / STAMINA_RECOVERY_SECONDS_PER_POINT;
    if(points == 0) return;
    if(points > STAMINA_MAX) points = STAMINA_MAX;
    codex->stamina += (int)points;
    if(codex->stamina >= STAMINA_MAX) {
        codex->stamina = STAMINA_MAX;
        codex->stamina_updated = now;
    } else {
        codex->stamina_updated += points * STAMINA_RECOVERY_SECONDS_PER_POINT;
    }
}

void codex_apply_time_passage(Codex* codex) {
    uint32_t now = furi_hal_rtc_get_timestamp();
    uint32_t today = now / SECONDS_PER_DAY;

    recover_stamina(codex, now);

    // Absence: -1 signal per whole day with no launch. The Substrate gets quieter.
    if(codex->last_active_day != 0 && today > codex->last_active_day + 1) {
        uint32_t idle_days = today - codex->last_active_day - 1;
        if(idle_days > ABSENCE_DECAY_MAX_DAYS) idle_days = ABSENCE_DECAY_MAX_DAYS;
        apply_signal_gain(codex, -(int)idle_days);
    }
    codex->last_active_day = today;
}

void codex_drain_stamina(Codex* codex, int amount) {
    recover_stamina(codex, furi_hal_rtc_get_timestamp());
    codex->stamina -= amount;
    if(codex->stamina < 0) codex->stamina = 0;
}
