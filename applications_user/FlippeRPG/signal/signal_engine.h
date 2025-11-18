#ifndef FLIPPERPG_SIGNAL_ENGINE_H
#define FLIPPERPG_SIGNAL_ENGINE_H

#include "../codex/codex.h"

void start_signal_loop(Codex* codex);
char* hash_signal(const char* raw_data);
int calculate_signal_xp(Codex* codex, const char* hash);

#endif