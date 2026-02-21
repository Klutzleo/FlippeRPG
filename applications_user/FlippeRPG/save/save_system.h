#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "../codex/codex.h"
#include <stdbool.h>

// Save Codex state to SD card (/ext/apps_data/flipperpg/codex.bin)
void save_codex(Codex* codex, const char* filename);

// Load Codex state from SD card.
// Returns true on success, false if no save exists or file is corrupt.
// On failure the codex is left untouched — call init_codex() to set defaults.
bool load_codex(Codex* codex, const char* filename);

#endif
