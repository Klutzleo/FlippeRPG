#include "bind_whisper.h"
#include "../codex/codex.h"
#include "../core/utils.h"
#include "../core/constants.h"
#include <stdio.h>

// Core logic for using the Bind Whisper technique
void bind_whisper(Codex* codex) {
    // Check if the technique is unlocked
    if (!codex_has_technique(codex, "Bind Whisper")) {
        popup_message("You have not yet unlocked Bind Whisper.");
        return;
    }

    // Track usage and check for mastery
    codex_use_technique(codex, "Bind Whisper");

    // Feedback to player
    popup_message("You bind the memory to the tag.");

    // Future: write to NFC tag, store Echo fragment, or trigger memory-based gameplay
    // e.g., write_echo_to_tag(codex->echo_log[0]);
}