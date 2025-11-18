#include "xp_engine.h"
#include <stdio.h>

void award_xp(Codex* codex, int amount, XPSource source) {
    if (source == XP_SOURCE_DUEL) {
        codex->duel_xp += amount;
    } else {
        codex->xp_total += amount;
    }

    // Debug output
    const char* label = (source == XP_SOURCE_SIGNAL) ? "Signal" :
                        (source == XP_SOURCE_DUEL)   ? "Duel" :
                                                       "Shrine";
    printf("[XP] +%d XP from %s\n", amount, label);
}

bool check_xp_threshold(Codex* codex, int threshold) {
    return codex->xp_total >= threshold;
}