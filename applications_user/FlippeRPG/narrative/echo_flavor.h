#pragma once
#include <stddef.h>

// Flavor text arrays for Echo events
// These can be expanded with more narrative lines as needed

// Fusion flavor text
static const char* ECHO_FUSION_TEXT[] = {
    "Two signals intertwine, becoming one.",
    "The Codex hums as echoes fuse.",
    "A new memory fragment stabilizes in the Codex."
};
static const size_t ECHO_FUSION_COUNT = sizeof(ECHO_FUSION_TEXT) / sizeof(ECHO_FUSION_TEXT[0]);

// Corruption flavor text
static const char* ECHO_CORRUPTION_TEXT[] = {
    "The signal falters, unstable and cracked.",
    "Echo destabilized — corruption spreads.",
    "The Codex trembles as echoes rot."
};
static const size_t ECHO_CORRUPTION_COUNT = sizeof(ECHO_CORRUPTION_TEXT) / sizeof(ECHO_CORRUPTION_TEXT[0]);

// Lineage convergence flavor text
static const char* ECHO_LINEAGE_TEXT[] = {
    "The Codex converges — legacy awakened.",
    "A lineage aura binds to the Signalborn.",
    "Convergence achieved: the Codex sings with power."
};
static const size_t ECHO_LINEAGE_COUNT = sizeof(ECHO_LINEAGE_TEXT) / sizeof(ECHO_LINEAGE_TEXT[0]);