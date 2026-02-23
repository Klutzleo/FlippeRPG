#include "../echo/echo_fusion.h"
#include <string.h>
#include <stdio.h>
#include "../core/utils.h"   // for popup_message
#include "../codex/codex.h"
#include "../core/constants.h"
#include <stdlib.h>
#include "../echo/echo_texts.h"

EchoEntry* find_echo(Codex* codex, const char* echo_id) {
    for (int i = 0; i < MAX_ECHO_LOG; i++) {
        if (strcmp(codex->echo_log[i].echo_id, echo_id) == 0) {
            return &codex->echo_log[i];
        }
    }
    return NULL;
}

void fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    EchoEntry* a = find_echo(codex, echo_a);
    EchoEntry* b = find_echo(codex, echo_b);

    if (!a || !b) {
        popup_message("Fusion failed. Echo not found.");
        return;
    }

    // Example fusion logic
    a->fused = true;
    b->fused = true;
    apply_signal_gain(codex, 10);
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
        case ECHO_LEGACY:
            popup_message("Legacy echo triggered.");
            break;
    }
}
