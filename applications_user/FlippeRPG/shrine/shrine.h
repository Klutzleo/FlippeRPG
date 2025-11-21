#ifndef FLIPPERPG_SHRINE_H
#define FLIPPERPG_SHRINE_H

#include "../codex/codex.h"
#include "../core/constants.h"  // ✅ This brings in ShrineID and SignalType
#include <stdbool.h>
#include <time.h>               // Needed for time_t in struct fields

void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type);
bool is_ritual_complete(Codex* codex, ShrineID shrine_id);
void complete_ritual(Codex* codex, ShrineID shrine_id);

// 🌀 Legacy shrine logic
bool shrine_ready_for_legacy_reset(ShrineProgress* progress);

#endif