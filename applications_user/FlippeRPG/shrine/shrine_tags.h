#ifndef SHRINE_TAGS_H
#define SHRINE_TAGS_H

#include "../codex/codex.h"

void imprint_shrine_tag(ShrineID shrine_id, const char* tag_id, time_t timestamp, const char* aura);
void scan_shrine_tag(Codex* codex, const char* tag);

#endif