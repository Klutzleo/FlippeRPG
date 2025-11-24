#pragma once
#include <stdbool.h>
#include "../core/constants.h"  // for EchoEventType

// Forward declarations to avoid circular includes
struct Codex;
struct EchoEntry;

// Finds an Echo by ID
EchoEntry* find_echo(Codex* codex, const char* echo_id);

// Fuses all eligible Echoes in the Codex
void fuse_all_echoes(Codex* codex);

// Handles echo events (fusion, corruption, lineage)
void echo_event(EchoEventType type);