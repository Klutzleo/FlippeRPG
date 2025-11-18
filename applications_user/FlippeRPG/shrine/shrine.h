#ifndef FLIPPERPG_SHRINE_H
#define FLIPPERPG_SHRINE_H

#include "../codex/codex.h"

typedef enum {
    SHRINE_CAVE_THAT_LISTENS,
    SHRINE_FLAME_REACH,
    SHRINE_BIND_WHISPER,
    SHRINE_THREAD_TOUCH
} ShrineID;

void trigger_shrine(Codex* codex, ShrineID shrine_id);
bool is_ritual_complete(Codex* codex, ShrineID shrine_id);
void complete_ritual(Codex* codex, ShrineID shrine_id);

#endif