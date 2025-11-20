#pragma once
#include "../codex/codex.h"
#include "../core/constants.h"

void imprint_shrine_tag(ShrineID shrine_id, const char* tag_id);
void scan_shrine_tag(Codex* codex, const char* tag_id);
ShrineID resolve_tag_to_shrine(const char* tag_id);