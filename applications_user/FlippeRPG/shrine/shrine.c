#include "shrine.h"
#include "../core/constants.h"
#include <stdio.h>
#include <string.h>
#include "../core/utils.h"
#include "shrine_definitions.h"

// Triggers a shrine ritual based on signal type and shrine ID
void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type) {
    // Check if the ritual has already been completed
    if (is_ritual_complete(codex, shrine_id)) {
        printf("[Shrine] Ritual already complete for shrine %d\n", shrine_id);
        return;
    }

    // Validate signal type against shrine requirement
    bool valid = false;
    switch (shrine_id) {
        case SHRINE_CAVE_THAT_LISTENS:
            valid = (signal_type == SIGNAL_SUBGHZ);
            break;
        case SHRINE_FLAME_REACH:
            valid = (signal_type == SIGNAL_IR);
            break;
        case SHRINE_BIND_WHISPER:
            valid = (signal_type == SIGNAL_NFC);
            break;
        case SHRINE_THREAD_TOUCH:
            valid = (signal_type == SIGNAL_GPIO);
            break;
    }

    // If signal type doesn't match, ritual fails
    if (!valid) {
        printf("[Shrine] The signal does not resonate with this shrine.\n");
        return;
    }

    // Ritual succeeds — mark shrine as complete and unlock technique
    printf("[Shrine] You feel a resonance...\n");
    complete_ritual(codex, shrine_id);
}

// Checks if a shrine ritual has already been completed
bool is_ritual_complete(Codex* codex, ShrineID shrine_id) {
    return codex->shrine_progress[shrine_id].ritual_complete;
}

// Marks a shrine ritual as complete and unlocks the associated technique
void complete_ritual(Codex* codex, ShrineID shrine_id) {
    codex->shrine_progress[shrine_id].ritual_complete = true;
    codex->shrine_progress[shrine_id].resonance_triggered = true;

    // Unlock technique based on shrine ID
    switch (shrine_id) {
        case SHRINE_CAVE_THAT_LISTENS:
            codex_unlock_technique(codex, "Pulse Open");
            break;
        case SHRINE_FLAME_REACH:
            codex_unlock_technique(codex, "Flame Reach");
            break;
        case SHRINE_BIND_WHISPER:
            codex_unlock_technique(codex, "Bind Whisper");
            break;
        case SHRINE_THREAD_TOUCH:
            codex_unlock_technique(codex, "Thread Touch");
            break;
    }
    printf("[Shrine] Ritual complete! Technique unlocked.\n");
}

// Special NFC-triggered shrine logic for Bind Whisper
void trigger_bind_whisper_shrine(Codex* codex, const char* scanned_tag_id) {
    // If already unlocked, show feedback
    if (codex_has_technique(codex, "Bind Whisper")) {
        popup_message("The memory is already bound.");
        return;
    }

    // Validate scanned tag ID (can be expanded with metadata later)
    if (strcmp(scanned_tag_id, "BIND-TAG-001") == 0) {
        codex_unlock_technique(codex, "Bind Whisper");
        popup_message("You have unlocked Bind Whisper!");
    } else {
        popup_message("The tag resists your memory.");
    }
}

void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type) {
    const ShrineDefinition* shrine = &shrine_definitions[shrine_id];

    if (is_ritual_complete(codex, shrine_id)) {
        popup_message("The ritual is already complete.");
        return;
    }

    if (signal_type != shrine->required_signal) {
        popup_message("The signal does not resonate.");
        return;
    }

    popup_message(shrine->flavor_text);
    complete_ritual(codex, shrine_id);
}