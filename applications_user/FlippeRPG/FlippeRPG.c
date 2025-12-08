#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/menu.h>
#include <gui/modules/text_input.h>
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
#include "ui/ui.h"

// Static Codex to avoid stack overflow (struct is ~5KB)
static Codex player_codex = {0};

// App instance for view callbacks
static FlippeRPGApp app_instance = {0};

// Menu callback
static void flippe_rpg_menu_callback(void* context, uint32_t index) {
    FlippeRPGApp* app = (FlippeRPGApp*)context;
    (void)index;
    (void)app;
    // For now, just exit on any menu selection
    view_dispatcher_stop(app->view_dispatcher);
}

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

    // Initialize app context
    app_instance.codex = &player_codex;
    memset(app_instance.player_name_input, 0, sizeof(app_instance.player_name_input));

    // Create GUI and view dispatcher
    Gui* gui = furi_record_open(RECORD_GUI);
    app_instance.view_dispatcher = view_dispatcher_alloc();
    
    view_dispatcher_attach_to_gui(app_instance.view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // Create menu view
    Menu* menu = menu_alloc();
    menu_add_item(menu, "View Codex", NULL, 0, flippe_rpg_menu_callback, &app_instance);
    menu_add_item(menu, "Change Name", NULL, 1, flippe_rpg_menu_callback, &app_instance);
    menu_add_item(menu, "Exit", NULL, 2, flippe_rpg_menu_callback, &app_instance);
    
    view_dispatcher_add_view(app_instance.view_dispatcher, 0, menu_get_view(menu));
    view_dispatcher_switch_to_view(app_instance.view_dispatcher, 0);

    // Run event loop
    view_dispatcher_run(app_instance.view_dispatcher);

    // Cleanup
    view_dispatcher_free(app_instance.view_dispatcher);
    furi_record_close(RECORD_GUI);

    // Save progress (stubbed to RAM cache)
    save_codex(&player_codex, save_path);

    return 0;
}