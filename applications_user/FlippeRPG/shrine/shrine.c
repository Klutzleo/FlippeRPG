#include "shrine.h"
#include "../core/constants.h"
#include <stdio.h>
#include <string.h>
#include "../core/utils.h"
#include "shrine_definitions.h"
#include "shrine_flavor.h"
#include <time.h> // for time tracking

// Checks if a shrine ritual has already been completed
bool is_ritual_complete(Codex* codex, ShrineID shrine_id) {
    return codex->shrine_progress[shrine_id].ritual_complete;
}

// Marks a shrine ritual as complete and unlocks the associated technique
void complete_ritual(Codex* codex, ShrineID shrine_id) {
    ShrineProgress* progress = &codex->shrine_progress[shrine_id];

    progress->ritual_complete = true;
    progress->resonance_triggered = true;
    progress->last_completed_time = time(NULL);
    progress->legacy_reset_ready = false;

    assign_aura(codex, shrine_id); // 🌈 Set aura based on shrine

    // Unlock technique + immersive narrative
    switch (shrine_id) {
        case SHRINE_CAVE_THAT_LISTENS:
            codex_unlock_technique(codex, "Pulse Open");
            popup_message(">>> The Cave yields. Pulse Open unlocked.");
            break;
        case SHRINE_FLAME_REACH:
            codex_unlock_technique(codex, "Flame Reach");
            popup_message(">>> Flame leaps. Technique unlocked: Flame Reach.");
            break;
        case SHRINE_BIND_WHISPER:
            codex_unlock_technique(codex, "Bind Whisper");
            popup_message(">>> Whisper binds. Technique unlocked: Bind Whisper.");
            break;
        case SHRINE_THREAD_TOUCH:
            codex_unlock_technique(codex, "Thread Touch");
            popup_message(">>> Threads tighten. Technique unlocked: Thread Touch.");
            break;
    }

    // Optional debug log
    printf("[Shrine] Ritual complete for shrine %d\n", shrine_id);
}

// Special NFC-triggered shrine logic for Bind Whisper
void trigger_bind_whisper_shrine(Codex* codex, const char* scanned_tag_id) {
    // If already unlocked, show feedback
    if (codex_has_technique(codex, "Bind Whisper")) {
        popup_message(">>> The memory is already bound.");
        return;
    }

    // Validate scanned tag ID (can be expanded with metadata later)
    if (strcmp(scanned_tag_id, "BIND-TAG-001") == 0) {
        codex_unlock_technique(codex, "Bind Whisper");
        popup_message(">>> Whisper binds. Technique unlocked: Bind Whisper.");
    } else {
        popup_message(">>> The tag resists your memory.");
    }
}

void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type) {
    const ShrineDefinition* shrine = &shrine_definitions[shrine_id];
    ShrineProgress* progress = &codex->shrine_progress[shrine_id];

    time_t now = time(NULL);
    double elapsed = difftime(now, progress->last_visited);

    // Cooldown check
    if (progress->ritual_complete && shrine->cooldown_seconds > 0 && elapsed < shrine->cooldown_seconds) {
        switch (shrine_id) {
            case SHRINE_CAVE_THAT_LISTENS:
                popup_message(SHRINE_CAVE_DORMANT[rand() % 3]);
                break;
            case SHRINE_FLAME_REACH:
                popup_message(SHRINE_FLAME_DORMANT[rand() % 3]);
                break;
            case SHRINE_BIND_WHISPER:
                popup_message(SHRINE_BIND_DORMANT[rand() % 3]);
                break;
            case SHRINE_THREAD_TOUCH:
                popup_message(SHRINE_THREAD_DORMANT[rand() % 3]);
                break;
        }
        return;
    }

    // Signal mismatch
    if (signal_type != shrine->required_signal) {
        popup_message(">>> The signal falters. Resonance denied.");
        return;
    }

    // Ritual succeeds
    switch (shrine_id) {
        case SHRINE_CAVE_THAT_LISTENS:
            popup_message(SHRINE_CAVE_ACTIVE[rand() % 3]);
            break;
        case SHRINE_FLAME_REACH:
            popup_message(SHRINE_FLAME_ACTIVE[rand() % 3]);
            break;
        case SHRINE_BIND_WHISPER:
            popup_message(SHRINE_BIND_ACTIVE[rand() % 3]);
            break;
        case SHRINE_THREAD_TOUCH:
            popup_message(SHRINE_THREAD_ACTIVE[rand() % 3]);
            break;
    }

    complete_ritual(codex, shrine_id);
    progress->last_visited = now;
}

bool shrine_ready_for_legacy_reset(ShrineProgress* progress) {
    if (!progress->ritual_complete) return false;
    double elapsed = difftime(time(NULL), progress->last_completed_time);
    return elapsed > 604800; // 7 days
}