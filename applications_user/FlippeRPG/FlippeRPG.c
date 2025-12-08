#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/menu.h>
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
static ViewDispatcher* view_dispatcher = NULL;

// Custom view draw callback to show Codex info
static void codex_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "FlippeRPG Codex");
    
    canvas_set_font(canvas, FontSecondary);
    char line1[64], line2[64], line3[64], line4[64];
    snprintf(line1, sizeof(line1), "Name: %s", player_codex.player_name);
    snprintf(line2, sizeof(line2), "ID: %s", player_codex.codex_id);
    snprintf(line3, sizeof(line3), "XP: %d", player_codex.xp_total);
    snprintf(line4, sizeof(line4), "Duel XP: %d", player_codex.duel_xp);
    
    canvas_draw_str(canvas, 2, 28, line1);
    canvas_draw_str(canvas, 2, 40, line2);
    canvas_draw_str(canvas, 2, 52, line3);
    canvas_draw_str(canvas, 2, 62, line4);
}

// Menu callback
static void menu_callback(void* context, uint32_t index) {
    (void)context;
    switch(index) {
        case 0: // View Codex - switch to codex view
            view_dispatcher_switch_to_view(view_dispatcher, 1);
            break;
        default: // Exit
            view_dispatcher_stop(view_dispatcher);
            break;
    }
}

// Custom view input callback to handle back button
static bool codex_view_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(view_dispatcher, 0);
        return true;
    }
    return false;
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
    notification_message_block(notification, &sequence_single_vibro);
    notification_message_block(notification, &sequence_blink_green_100);
    furi_record_close(RECORD_NOTIFICATION);

    // Create GUI and view dispatcher
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // Create menu view (view 0)
    Menu* menu = menu_alloc();
    menu_add_item(menu, "View Codex", NULL, 0, menu_callback, NULL);
    menu_add_item(menu, "Exit", NULL, 1, menu_callback, NULL);
    view_dispatcher_add_view(view_dispatcher, 0, menu_get_view(menu));

    // Create custom codex view (view 1)
    View* codex_view = view_alloc();
    view_set_draw_callback(codex_view, codex_view_draw_callback);
    view_set_input_callback(codex_view, codex_view_input_callback);
    view_dispatcher_add_view(view_dispatcher, 1, codex_view);

    // Start at menu
    view_dispatcher_switch_to_view(view_dispatcher, 0);

    // Run event loop
    view_dispatcher_run(view_dispatcher);

    // Cleanup
    view_dispatcher_remove_view(view_dispatcher, 0);
    view_dispatcher_remove_view(view_dispatcher, 1);
    view_free(codex_view);
    menu_free(menu);
    view_dispatcher_free(view_dispatcher);
    furi_record_close(RECORD_GUI);

    // Save progress (stubbed to RAM cache)
    save_codex(&player_codex, save_path);

    return 0;
}