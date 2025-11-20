#pragma once
#include "../core/constants.h"
#include "../codex/codex.h"

bool can_fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b);
void fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b);