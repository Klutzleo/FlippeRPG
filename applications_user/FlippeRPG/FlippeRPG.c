#include <furi.h>
#include <gui/gui.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "codex/codex.h"
#include "signal/signal_engine.h"
#include "techniques/techniques.h"
#include "save/save_system.h"
#include "core/utils.h"

// Static Codex to avoid stack overflow (struct is ~5KB)
static Codex player_codex = {0};

int32_t flippe_rpg_app(void* p) {
    (void)p;
    srand((unsigned)furi_get_tick());

    // Load or initialize Codex (already static, so no stack pressure)
    const char* save_path = "codex.sav";
    load_codex(&player_codex, save_path);
    if(strlen(player_codex.codex_id) == 0) {
        init_codex(&player_codex, "Jason");
    }

    // Quick visible cue: blink green LED once
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notification, &sequence_single_vibro); // short haptic
    notification_message_block(notification, &sequence_blink_green_100);
    furi_record_close(RECORD_NOTIFICATION);

    // Save progress (stubbed to RAM cache)
    save_codex(&player_codex, save_path);

    return 0;
}