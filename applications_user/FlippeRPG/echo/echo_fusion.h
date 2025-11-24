#pragma once
#include <stdbool.h>
#include "../core/constants.h"  // for EchoEventType
#include "../codex/codex.h"

// Forward declarations to avoid circular includes
struct Codex;
struct EchoEntry;

// Finds an Echo by ID
EchoEntry* find_echo(Codex* codex, const char* echo_id);

// Fuses all eligible Echoes in the Codex
void fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b);

// Handles echo events (fusion, corruption, lineage)
void echo_event(EchoEventType type);