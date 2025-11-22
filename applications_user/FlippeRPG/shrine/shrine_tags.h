#ifndef SHRINE_TAGS_H
#define SHRINE_TAGS_H

#include "../codex/codex.h"

void imprint_shrine_tag(int shrine_id, const char* tag);
void scan_shrine_tag(Codex* codex, const char* tag);

#endif