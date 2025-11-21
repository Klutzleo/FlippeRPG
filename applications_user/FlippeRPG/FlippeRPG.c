#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#include "codex/codex.h"
#include "signal/signal_engine.h"
#include "techniques/techniques.h"
#include "save/save_system.h"

int32_t flippe_rpg_app(void* p) {
    (void)p;
    srand((unsigned)time(NULL));

    // Load or initialize Codex
    Codex player_codex;
    if(!load_codex_state(&player_codex)) {
        init_codex(&player_codex, "Jason");
    }

    // Show Codex summary
    FuriString* output = furi_string_alloc();
    furi_string_printf(output, "Codex: %s\nXP: %d", player_codex.codex_id, player_codex.xp_total);
    popup_message(output);
    furi_string_free(output);

    // Run signal loop (stubbed)
    start_signal_loop(&player_codex);

    // Test Pulse Open (stubbed trigger)
    bool user_pressed_pulse_open_button = true; // Replace with input check later
    if(user_pressed_pulse_open_button) {
        pulse_open(&player_codex);
    }

    // 🌀 Convergence check
    if (ready_for_convergence(&player_codex)) {
        popup_message("🌀 The Codex hums. All signals align.");
        player_codex.aura_state = AURA_ECHOFORGED;
        player_codex.converged = true;
        // Optional: unlock new shrine, reset Echo log, or begin legacy mode
    }


    // Save progress
    save_codex_state(&player_codex);

    return 0;
}