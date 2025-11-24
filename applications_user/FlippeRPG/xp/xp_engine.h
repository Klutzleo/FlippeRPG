#ifndef FLIPPERPG_XP_ENGINE_H
#define FLIPPERPG_XP_ENGINE_H

#include <stdbool.h>
#include "../core/constants.h"
#include "../codex/codex.h"

// Forward declare Codex so we can use pointers
struct Codex;

// Awards XP to the Codex from various sources
void award_xp(struct Codex* codex, int amount, XPSource source);
bool check_xp_threshold(struct Codex* codex, int threshold);

#endif