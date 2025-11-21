#include "codex.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../narrative/echo_flavor.h"

// Initializes a new Codex for the player
void init_codex(Codex* codex, const char* player_name) {
    // Seed RNG once at init for random flavor text
    srand(time(NULL));

    // Set player name and generate a unique Codex ID
    strncpy(codex->player_name, player_name, sizeof(codex->player_name));
    snprintf(codex->codex_id, sizeof(codex->codex_id), "CDX%04X", rand() % 65536);

    // Reset XP counters
    codex->xp_total = 0;
    codex->duel_xp = 0;

    // Clear all Codex subsystems
    memset(codex->echo_log, 0, sizeof(codex->echo_log));
    memset(codex->signal_history, 0, sizeof(codex->signal_history));
    memset(codex->encounter_log, 0, sizeof(codex->encounter_log));
    memset(codex->shrine_progress, 0, sizeof(codex->shrine_progress));
    memset(codex->techniques, 0, sizeof(codex->techniques));

    // Timestamp the save
    codex->save_timestamp = time(NULL);
}

// Logs a scanned signal and awards XP
void log_signal(Codex* codex, const char* signal_hash, int xp) {
    // Shift signal history to make room for new entry
    for (int i = MAX_SIGNALS - 1; i > 0; i--) {
        codex->signal_history[i] = codex->signal_history[i - 1];
    }

    // Store new signal at the top
    strncpy(codex->signal_history[0].hash, signal_hash, sizeof(codex->signal_history[0].hash));
    codex->signal_history[0].xp_awarded = xp;
    codex->signal_history[0].timestamp = time(NULL);

    // Add XP to total
    codex->xp_total += xp;

    // 🔮 Echo mechanics hook
    bool fusion_success = (xp >= 5);        // placeholder condition for fusion
    bool corruption_detected = (xp == 0);   // placeholder condition for corruption

    process_echo(codex, fusion_success, corruption_detected);
}

// Adds XP from duels (separate from signal XP)
// Adds XP from duels (separate from signal XP)
void update_duel_xp(Codex* codex, int xp) {
    // Add duel XP
    codex->duel_xp += xp;

    // 🔮 After duel XP update, check for lineage convergence
    if (ready_for_convergence(codex)) {
        process_echo(codex, false, false); // lineage trigger
    }
}

// Logs a multiplayer encounter with another Flipper
void log_encounter(Codex* codex, const char* signalborn_id, const char* aura, bool echo_transferred) {
    // Shift encounter log to make room
    for (int i = MAX_ENCOUNTERS - 1; i > 0; i--) {
        codex->encounter_log[i] = codex->encounter_log[i - 1];
    }

    // Store new encounter at the top
    strncpy(codex->encounter_log[0].signalborn_id, signalborn_id, sizeof(codex->encounter_log[0].signalborn_id));
    strncpy(codex->encounter_log[0].aura, aura, sizeof(codex->encounter_log[0].aura));
    codex->encounter_log[0].timestamp = time(NULL);
    codex->encounter_log[0].echo_transferred = echo_transferred;

    // 🔮 Echo mechanics hook
    if (echo_transferred) {
        // Chance of corruption when echoes are exchanged
        process_echo(codex, false, true);
    }
}

// Unlocks a new technique and adds it to the Codex
void codex_unlock_technique(Codex* codex, const char* name) {
    for (int i = 0; i < MAX_TECHNIQUES; i++) {
        if (strlen(codex->techniques[i].name) == 0) {
            strncpy(codex->techniques[i].name, name, sizeof(codex->techniques[i].name));
            codex->techniques[i].unlocked = true;
            codex->techniques[i].uses = 0;
            codex->techniques[i].mastered = false;
            printf("[Codex] Technique unlocked: %s\n", name);
            return;
        }
    }
    printf("[Codex] No available slot to unlock technique: %s\n", name);
}

// Checks if a technique is unlocked
bool codex_has_technique(Codex* codex, const char* name) {
    for (int i = 0; i < MAX_TECHNIQUES; i++) {
        if (strcmp(codex->techniques[i].name, name) == 0 && codex->techniques[i].unlocked) {
            return true;
        }
    }
    return false;
}

// Tracks usage of a technique and marks it as mastered after 10 uses
void codex_use_technique(Codex* codex, const char* name) {
    for (int i = 0; i < MAX_TECHNIQUES; i++) {
        if (strcmp(codex->techniques[i].name, name) == 0 && codex->techniques[i].unlocked) {
            codex->techniques[i].uses++;
            if (codex->techniques[i].uses >= 10 && !codex->techniques[i].mastered) {
                codex->techniques[i].mastered = true;
                printf("[Codex] Technique mastered: %s\n", name);
            }
            return;
        }
    }
    printf("[Codex] Technique not found or not unlocked: %s\n", name);
}

void assign_aura(Codex* codex, ShrineID shrine_id) {
    switch (shrine_id) {
        case SHRINE_FLAME_REACH:
            strncpy(codex->aura_trait, "Flamebound", sizeof(codex->aura_trait));
            break;
        case SHRINE_BIND_WHISPER:
            strncpy(codex->aura_trait, "Whispered", sizeof(codex->aura_trait));
            break;
        case SHRINE_CAVE_THAT_LISTENS:
            strncpy(codex->aura_trait, "Echo-Touched", sizeof(codex->aura_trait));
            break;
        default:
            strncpy(codex->aura_trait, "Wandering", sizeof(codex->aura_trait));
            break;
    }
}

// Marks a shrine as completed and assigns aura
void complete_shrine(Codex* codex, ShrineID shrine_id) {
    // Mark shrine as completed
    codex->shrine_progress[shrine_id].completed = true;

    // Assign aura trait based on shrine
    assign_aura(codex, shrine_id);

    printf("[Shrine] Shrine %d completed. Aura assigned: %s\n", shrine_id, codex->aura_trait);

    // 🔮 After shrine completion, check for lineage convergence
    if (ready_for_convergence(codex)) {
        process_echo(codex, false, false); // lineage trigger
    }
}

void mark_echo_corrupted(Codex* codex, const char* echo_id) {
    for (int i = 0; i < MAX_ECHO_LOG; i++) {
        if (strcmp(codex->echo_log[i].echo_id, echo_id) == 0) {
            codex->echo_log[i].corrupted = true;
            printf("[Echo] %s marked as corrupted.\n", echo_id);
            // Narrative feedback
            echo_event(ECHO_CORRUPTION);
            return;
        }
    }
}

bool ready_for_convergence(Codex* codex) {
    if (codex->duel_xp < 100) return false;

    for (int i = 0; i < MAX_SHRINES; i++) {
        if (!codex->shrine_progress[i].completed) return false;
    }

    for (int i = 0; i < MAX_ECHO_LOG; i++) {
        if (strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].fused) {
            return false;
        }
    }

    return true;
}

void echo_event(EchoState state) {
    switch (state) {
        case ECHO_FUSION:
            popup_message(ECHO_FUSION_TEXT[rand() % 3]);
            break;
        case ECHO_CORRUPTION:
            popup_message(ECHO_CORRUPTION_TEXT[rand() % 3]);
            break;
        case ECHO_LINEAGE:
            popup_message(ECHO_LINEAGE_TEXT[rand() % 3]);
            break;
    }
}

void process_echo(Codex* codex, bool fusion_success, bool corruption_detected) {
    if (fusion_success) {
        // Fusion: +10 XP bonus, mark fused
        codex->xp_total += 10;
        echo_event(ECHO_FUSION);

        // Example: mark latest echo as fused
        for (int i = 0; i < MAX_ECHO_LOG; i++) {
            if (strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].fused) {
                codex->echo_log[i].fused = true;
                printf("[Echo] %s fused.\n", codex->echo_log[i].echo_id);
                break;
            }
        }

    } else if (corruption_detected) {
        // Corruption: apply penalty, mark corrupted
        codex->xp_total -= 2; // penalty per event
        echo_event(ECHO_CORRUPTION);

        for (int i = 0; i < MAX_ECHO_LOG; i++) {
            if (strlen(codex->echo_log[i].echo_id) > 0 && !codex->echo_log[i].corrupted) {
                codex->echo_log[i].corrupted = true;
                printf("[Echo] %s corrupted.\n", codex->echo_log[i].echo_id);
                break;
            }
        }

    } else if (ready_for_convergence(codex)) {
        // Lineage convergence: +50 XP reward, permanent aura/title
        codex->xp_total += 50;
        echo_event(ECHO_LINEAGE);

        // Example: assign permanent aura/title
        strncpy(codex->aura_trait, "Signalborn", sizeof(codex->aura_trait));
        printf("[Codex] Lineage convergence achieved. Aura: %s\n", codex->aura_trait);
    }
}