#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "../codex/codex.h"  // so we can access Codex struct

// Save Codex state to file
void save_codex(Codex* codex, const char* filename);

// Load Codex state from file
void load_codex(Codex* codex, const char* filename);

#endif