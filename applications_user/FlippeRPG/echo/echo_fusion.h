#pragma once
#include "../codex/codex.h"
#include <stdbool.h>

// Attempts to fuse two Echoes by ID
bool fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b);

// Finds an Echo by ID
EchoEntry* find_echo(Codex* codex, const char* echo_id);