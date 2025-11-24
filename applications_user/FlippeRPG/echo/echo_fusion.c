#include "echo_fusion.h"
#include <string.h>
#include <stdio.h>
#include "../core/utils.h"   // for popup_message
#include "../xp/xp_engine.h" // for award_xp

EchoEntry* find_echo(Codex* codex, const char* echo_id) {
    for (int i = 0; i < MAX_ECHO_LOG; i++) {
        if (strcmp(codex->echo_log[i].echo_id, echo_id) == 0) {
            return &codex->echo_log[i];
        }
    }
    return NULL;
}

bool fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    EchoEntry* a = find_echo(codex, echo_a);
    EchoEntry* b = find_echo(codex, echo_b);

    if (!a || !b || a->fused || b->fused) {
        popup_message("Fusion failed. Echoes missing or already fused.");
        return false;
    }

    // Corruption check
    if (a->corrupted || b->corrupted) {
        popup_message("Fusion unstable. Echo corrupted.");
        a->corrupted = true;
        b->corrupted = true;
        return false;
    }

    // Normal fusion
    a->fused = true;
    b->fused = true;
    award_xp(codex, 10);
    popup_message("Echo fusion successful.");
    return true;
}

void echo_event(EchoEventType type) {
    // TODO: handle fusion/corruption/lineage events
}