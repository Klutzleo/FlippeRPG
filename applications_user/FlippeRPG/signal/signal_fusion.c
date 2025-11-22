#include "signal_fusion.h"
#include <stdio.h>
#include <string.h>
#include "../echo/echo_fusion.h"

bool can_fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    if (strcmp(echo_a, echo_b) == 0) return false;

    // Example synergy: echoes from different signal types fuse better
    SignalType type_a = get_signal_type(codex, echo_a);
    SignalType type_b = get_signal_type(codex, echo_b);

    return type_a != type_b;
}

void fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    if (!can_fuse_echoes(codex, echo_a, echo_b)) {
        popup_message("Fusion failed. Echoes resist each other.");
        return;
    }

    // Create new fused echo ID
    char fused_echo[32];
    snprintf(fused_echo, sizeof(fused_echo), "FUSED-%s-%s", echo_a, echo_b);

    // Log new echo
    log_signal(codex, fused_echo, SIGNAL_FUSION);
    popup_message("Echo fusion successful. A new memory stirs.");
}