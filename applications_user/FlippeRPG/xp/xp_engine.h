#ifndef FLIPPERPG_XP_ENGINE_H
#define FLIPPERPG_XP_ENGINE_H

#include "../codex/codex.h"

typedef enum {
    XP_SOURCE_SIGNAL,
    XP_SOURCE_DUEL,
    XP_SOURCE_SHRINE
} XPSource;

void award_xp(Codex* codex, int amount, XPSource source);
bool check_xp_threshold(Codex* codex, int threshold);

#endif