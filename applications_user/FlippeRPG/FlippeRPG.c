#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "codex/codex.h"
#include "signal/signal_engine.h"
#include "techniques/techniques.h"
#include "save/save_system.h"
#include "core/utils.h"

int32_t flippe_rpg_app(void* p) {
    (void)p;
    srand((unsigned)furi_get_tick());

    // Load or initialize Codex
    Codex player_codex = {0};
    const char* save_path = "codex.sav";
    load_codex(&player_codex, save_path);
    if(strlen(player_codex.codex_id) == 0) {
        init_codex(&player_codex, "Jason");
    }

    // Show Codex summary
    char output[64];
    snprintf(output, sizeof(output), "Codex: %s\nXP: %d", player_codex.codex_id, player_codex.xp_total);
    popup_message(output);

    // Run signal loop (stubbed)
    start_signal_loop(&player_codex);

    // Test Pulse Open (stubbed trigger)
    bool user_pressed_pulse_open_button = true; // Replace with input check later
    if(user_pressed_pulse_open_button) {
        pulse_open(&player_codex);
    }

    // 🌀 Convergence check
    if (ready_for_convergence(&player_codex)) {
        popup_message("The Codex hums. All signals align.");
        strncpy(player_codex.aura_trait, "Echoforged", sizeof(player_codex.aura_trait));
        player_codex.converged = true;
        // Optional: unlock new shrine, reset Echo log, or begin legacy mode
    }


    // Save progress
    save_codex(&player_codex, save_path);

    return 0;
}