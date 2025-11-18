#include <furi.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <time.h>

#include "codex/codex.h"
#include "signal/signal_engine.h"

int FlippeRPG_app() {
    srand(time(NULL));

    Codex player_codex;
    init_codex(&player_codex, "Jason");

    // Flipper screen output
    FuriString* output = furi_string_alloc();
    furi_string_printf(output, "Codex: %s\nXP: %d", player_codex.codex_id, player_codex.xp_total);
    popup_message(output);

    start_signal_loop(&player_codex);

    furi_string_free(output);
    return 0;
}