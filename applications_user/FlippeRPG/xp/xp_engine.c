#include "xp_engine.h"
#include <stdio.h>
#include "../codex/codex.h"   // brings in typedef Codex

void award_xp(Codex* codex, int amount, XPSource source) {
    if (!codex) return;

    switch (source) {
        case XP_SOURCE_SIGNAL:
            codex->xp_total += amount;
            break;
        case XP_SOURCE_DUEL:
            codex->duel_xp += amount;
            break;
        case XP_SOURCE_SHRINE:
            codex->xp_total += amount;
            // TODO: shrine-specific tracking later
            break;
        case XP_SOURCE_FUSION:
            codex->xp_total += amount;
            // TODO: fusion-specific tracking later
            break;
    }

    const char* label =
        (source == XP_SOURCE_SIGNAL) ? "Signal" :
        (source == XP_SOURCE_DUEL)   ? "Duel"   :
        (source == XP_SOURCE_SHRINE) ? "Shrine" :
                                       "Fusion";

    printf("[XP] +%d XP from %s\n", amount, label);
}

bool check_xp_threshold(Codex* codex, int threshold) {
    if (!codex) return false;
    return codex->xp_total >= threshold;
}