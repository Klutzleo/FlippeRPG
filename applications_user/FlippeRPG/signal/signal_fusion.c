#include "signal_fusion.h"
#include <stdio.h>
#include <string.h>

bool can_fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    // Basic check: both echoes must exist and be different
    if (strcmp(echo_a, echo_b) == 0) return false;

    // TODO: Add deeper logic later (e.g. synergy, shrine affinity)
    return true;
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