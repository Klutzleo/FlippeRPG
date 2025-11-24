#ifndef FLIPPERPG_XP_ENGINE_H
#define FLIPPERPG_XP_ENGINE_H

#include <stdbool.h>
#include "../core/constants.h"

// Forward declare Codex so we can use pointers
typedef struct Codex Codex;

// Awards XP to the Codex from various sources
void award_xp(Codex* codex, int amount, XPSource source);
bool check_xp_threshold(Codex* codex, int threshold);

#endif