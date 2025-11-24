// signal_tags.h
#pragma once
#include <time.h>
#include "../core/constants.h"

void imprint_shrine_tag(ShrineID shrine_id,
                        const char* tag_id,
                        const char* aura_hint,
                        time_t cooldown_hint,
                        const char* echo_hint);
void scan_shrine_tag(Codex* codex, const char* tag_id);
ShrineID resolve_tag_to_shrine(const char* tag_id);