#include "shrine.h"
#include "../core/constants.h"
#include <stdio.h>
#include <string.h>

void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type) {
    if (is_ritual_complete(codex, shrine_id)) {
        printf("[Shrine] Ritual already complete for shrine %d\n", shrine_id);
        return;
    }

    // Check signal type requirement
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

    if (!valid) {
        printf("[Shrine] The signal does not resonate with this shrine.\n");
        return;
    }

    // Ritual succeeds
    printf("[Shrine] You feel a resonance...\n");
    complete_ritual(codex, shrine_id);
}

bool is_ritual_complete(Codex* codex, ShrineID shrine_id) {
    return codex->shrine_progress[shrine_id].ritual_complete;
}

void complete_ritual(Codex* codex, ShrineID shrine_id) {
    codex->shrine_progress[shrine_id].ritual_complete = true;
    codex->shrine_progress[shrine_id].resonance_triggered = true;

    // Unlock technique based on shrine
    switch (shrine_id) {
        case SHRINE_CAVE_THAT_LISTENS:
            strncpy(codex->callings_unlocked[0], "Pulse Open", 16);
            break;
        case SHRINE_FLAME_REACH:
            strncpy(codex->callings_unlocked[1], "Flame Reach", 16);
            break;
        case SHRINE_BIND_WHISPER:
            strncpy(codex->callings_unlocked[2], "Bind Whisper", 16);
            break;
        case SHRINE_THREAD_TOUCH:
            strncpy(codex->callings_unlocked[3], "Thread Touch", 16);
            break;
    }

    printf("[Shrine] Ritual complete! Technique unlocked.\n");
}