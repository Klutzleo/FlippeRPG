#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
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
#include "shrine/shrine.h"
#include "core/utils.h"
#include "core/constants.h"
// Assets: icon animations compiled from root assets/icons
#include <assets_icons.h>
#include <gui/icon_animation.h>

// View IDs
enum {
    VIEW_MENU = 0,
    VIEW_CODEX = 1,
    VIEW_SIGNAL = 2,
    VIEW_SHRINE_LIST = 3,
    VIEW_SHRINE_DETAIL = 4,
    VIEW_TECHNIQUE = 5,
    VIEW_CAMPFIRE = 6,
    VIEW_CAMPFIRE_PROFILE = 7,
    VIEW_NAME_ENTRY = 8,
};

// Campfire player slot positions (N, S, E, W)
typedef enum {
    CAMP_NORTH = 0,
    CAMP_SOUTH = 1,
    CAMP_EAST = 2,
    CAMP_WEST = 3,
    MAX_CAMP_SLOTS = 4
} CampfireSlot;

// Campfire player struct
typedef struct {
    char name[16];
    char aura[16];
    uint32_t last_seen_tick;  // Timeout tracking
    bool active;  // Is this player currently visible?
    int handshake_xp;  // XP awarded for this handshake
} CampfirePlayer;

// Static Codex to avoid stack overflow (struct is ~5KB)
static Codex player_codex = {0};
static ViewDispatcher* view_dispatcher = NULL;
static uint32_t last_signal_xp = 0;  // Track last signal result for display
static int selected_shrine = 0;      // Currently selected shrine for detail view
static int last_duel_xp = 0;         // Track duel result

// Campfire state
static CampfirePlayer campfire_slots[MAX_CAMP_SLOTS] = {0};
static int selected_camp_slot = CAMP_NORTH;  // Currently selected campfire slot
static uint32_t last_scan_tick = 0;  // Last time we scanned for players

// Campfire visuals
static IconAnimation* campfire_fire_anim = NULL;

// Name entry
static TextInput* name_input = NULL;
static char name_buffer[16] = {0};

// Shrine name and description lookup
static const char* get_shrine_name(ShrineID id) {
    switch(id) {
        case SHRINE_CAVE_THAT_LISTENS: return "Cave That Listens";
        case SHRINE_FLAME_REACH: return "Flame Reach";
        case SHRINE_BIND_WHISPER: return "Bind Whisper";
        case SHRINE_THREAD_TOUCH: return "Thread Touch";
        case SHRINE_ECHO_TOUCHED: return "Echo Touched";
        default: return "Unknown Shrine";
    }
}

static const char* get_shrine_description(ShrineID id) {
    switch(id) {
        case SHRINE_CAVE_THAT_LISTENS: return "Listen to SubGHz waves";
        case SHRINE_FLAME_REACH: return "Absorb infrared signals";
        case SHRINE_BIND_WHISPER: return "Feel NFC vibrations";
        case SHRINE_THREAD_TOUCH: return "Touch GPIO connections";
        case SHRINE_ECHO_TOUCHED: return "Fuse signal echoes";
        default: return "A mysterious shrine";
    }
}

static const char* get_aura_name(ShrineID id) {
    switch(id) {
        case SHRINE_CAVE_THAT_LISTENS: return "Stormtouched";
        case SHRINE_FLAME_REACH: return "Flamebound";
        case SHRINE_BIND_WHISPER: return "Whispered";
        case SHRINE_ECHO_TOUCHED: return "Echoforged";
        case SHRINE_THREAD_TOUCH: return "Threaded";
        default: return "Unknown";
    }
}

// ==================== MENU VIEW ====================
static void main_menu_callback(void* context, uint32_t index) {
    (void)context;
    switch(index) {
        case 0: // Absorb Signals
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SIGNAL);
            break;
        case 1: // Shrines
            selected_shrine = 0;  // Reset to first shrine
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SHRINE_LIST);
            break;
        case 2: // Techniques
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_TECHNIQUE);
            break;
        case 3: // Multiplayer (Campfire)
            selected_camp_slot = CAMP_NORTH;
            last_scan_tick = furi_get_tick();
            // Populate campfire from encounter log
            memset(campfire_slots, 0, sizeof(campfire_slots));
            for(int i = 0; i < MAX_ENCOUNTERS && i < MAX_CAMP_SLOTS; i++) {
                if(player_codex.encounter_log[i].signalborn_id[0] != 0) {
                    strncpy(campfire_slots[i].name, player_codex.encounter_log[i].signalborn_id, sizeof(campfire_slots[i].name));
                    strncpy(campfire_slots[i].aura, player_codex.encounter_log[i].aura, sizeof(campfire_slots[i].aura));
                    campfire_slots[i].last_seen_tick = furi_get_tick();
                    campfire_slots[i].active = true;
                    campfire_slots[i].handshake_xp = 2;  // Auto-handshake XP
                }
            }
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
            break;
        case 4: // Status/Codex
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CODEX);
            break;
        case 5: // Exit
            view_dispatcher_stop(view_dispatcher);
            break;
    }
}

// ==================== CODEX STATUS VIEW ====================
static void codex_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Codex Status");
    
    canvas_set_font(canvas, FontSecondary);
    char line1[64], line2[64], line3[64], line4[64], line5[64], line6[64];
    snprintf(line1, sizeof(line1), "Name: %s", player_codex.player_name);
    snprintf(line2, sizeof(line2), "ID: %s", player_codex.codex_id);
    snprintf(line3, sizeof(line3), "Signal XP: %d", player_codex.xp_total);
    snprintf(line4, sizeof(line4), "Duel XP: %d", player_codex.duel_xp);
    snprintf(line5, sizeof(line5), "Duels: %dW / %dL", player_codex.duels_won, player_codex.duels_lost);
    snprintf(line6, sizeof(line6), "Aura: %s", player_codex.aura_trait[0] ? player_codex.aura_trait : "Unbound");
    
    canvas_draw_str(canvas, 2, 22, line1);
    canvas_draw_str(canvas, 2, 30, line2);
    canvas_draw_str(canvas, 2, 38, line3);
    canvas_draw_str(canvas, 2, 46, line4);
    canvas_draw_str(canvas, 2, 54, line5);
    canvas_draw_str(canvas, 2, 62, line6);
}

// ==================== SIGNAL ABSORPTION VIEW ====================
static void signal_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Signal Absorption");
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 28, "Scanning for signals...");
    
    char xp_line[64];
    snprintf(xp_line, sizeof(xp_line), "Last Gain: +%lu XP", last_signal_xp);
    canvas_draw_str(canvas, 2, 52, xp_line);
    
    canvas_draw_str(canvas, 2, 63, "Press OK to scan");
}

static bool signal_view_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            // Trigger signal scan
            start_signal_loop(&player_codex);
            last_signal_xp = 5;  // Simulate XP gain
            return true;
        }
        else if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
            return true;
        }
    }
    return false;
}

// ==================== SHRINE VIEW ====================
static void shrine_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Shrines");
    
    canvas_set_font(canvas, FontSecondary);
    
    // Display all shrines with their status
    for(int i = 0; i < SHRINE_UNKNOWN; i++) {
        ShrineProgress* shrine = &player_codex.shrine_progress[i];
        const char* shrine_name = get_shrine_name(i);
        const char* status = shrine->completed ? "[X]" : "[ ]";

        char shrine_line[64];
        snprintf(shrine_line, sizeof(shrine_line), "%s %s", status, shrine_name);

        // Highlight selected shrine
        if(i == selected_shrine) {
            canvas_draw_str(canvas, 0, 24 + (i * 8), ">");
            canvas_draw_str(canvas, 8, 24 + (i * 8), shrine_line);
        } else {
            canvas_draw_str(canvas, 2, 24 + (i * 8), shrine_line);
        }
    }

    canvas_draw_str(canvas, 2, 63, "OK: View | Back: Menu");
}

static bool shrine_view_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            if(selected_shrine > 0) selected_shrine--;
            return true;
        }
        else if(event->key == InputKeyDown) {
            if(selected_shrine < NUM_SHRINES - 1) selected_shrine++;
            return true;
        }
        else if(event->key == InputKeyOk) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SHRINE_DETAIL);
            return true;
        }
        else if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
            return true;
        }
    }
    return false;
}

// ==================== SHRINE DETAIL VIEW ====================
static void shrine_detail_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    const char* shrine_name = get_shrine_name(selected_shrine);
    canvas_draw_str(canvas, 2, 12, shrine_name);
    
    canvas_set_font(canvas, FontSecondary);
    
    ShrineProgress* shrine = &player_codex.shrine_progress[selected_shrine];
    
    // Shrine description
    const char* desc = get_shrine_description(selected_shrine);
    canvas_draw_str(canvas, 2, 28, "Ritual:");
    canvas_draw_str(canvas, 2, 38, desc);
    
    // Aura trait
    const char* aura = get_aura_name(selected_shrine);
    char aura_line[64];
    snprintf(aura_line, sizeof(aura_line), "Aura: %s", aura);
    canvas_draw_str(canvas, 2, 50, aura_line);
    
    // Status
    char status_line[64];
    if(shrine->completed) {
        snprintf(status_line, sizeof(status_line), "Status: COMPLETED");
    } else {
        snprintf(status_line, sizeof(status_line), "Status: LOCKED");
    }
    canvas_draw_str(canvas, 2, 62, status_line);
}

static bool shrine_detail_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            // Attempt the ritual
            if(!player_codex.shrine_progress[selected_shrine].completed) {
                // Simulate ritual success (70% chance)
                bool success = (rand() % 100) < 70;
                attempt_shrine(&player_codex, selected_shrine, success);
                
                // Return to list to see updated status
                view_dispatcher_switch_to_view(view_dispatcher, VIEW_SHRINE_LIST);
            }
            return true;
        }
        else if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SHRINE_LIST);
            return true;
        }
    }
    return false;
}

// ==================== CAMPFIRE VIEW ====================
static void campfire_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    
    // Title
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Campfire");
    
    canvas_set_font(canvas, FontSecondary);
    
    // Draw the campfire scene (text-based for now, will upgrade to sprites)
    //
    //        [N] (North)
    //        name
    //          |
    //  [W]---(FIRE)---[E]
    //  name    :)     name
    //          |
    //        (You)
    //        [S]
    //
    
    // North player (top)
    if(campfire_slots[CAMP_NORTH].active) {
        const char* marker = (selected_camp_slot == CAMP_NORTH) ? ">" : " ";
        canvas_draw_str(canvas, 2, 25, marker);
        canvas_draw_str(canvas, 8, 25, campfire_slots[CAMP_NORTH].name);
    }
    
    // Fire in center: try animated icon, fallback to ASCII
    if(campfire_fire_anim) {
        canvas_draw_icon_animation(canvas, 55, 32, campfire_fire_anim);
    } else {
        canvas_draw_str(canvas, 58, 35, "[*]");
    }
    
    // West player (left)
    if(campfire_slots[CAMP_WEST].active) {
        const char* marker = (selected_camp_slot == CAMP_WEST) ? ">" : " ";
        canvas_draw_str(canvas, 2, 35, marker);
        canvas_draw_str(canvas, 8, 35, campfire_slots[CAMP_WEST].name);
    }
    
    // East player (right)
    if(campfire_slots[CAMP_EAST].active) {
        const char* marker = (selected_camp_slot == CAMP_EAST) ? ">" : " ";
        canvas_draw_str(canvas, 80, 35, marker);
        canvas_draw_str(canvas, 86, 35, campfire_slots[CAMP_EAST].name);
    }
    
    // You (South, bottom)
    canvas_draw_str(canvas, 53, 50, "(You)");
    const char* you_marker = (selected_camp_slot == CAMP_SOUTH) ? ">" : " ";
    canvas_draw_str(canvas, 48, 50, you_marker);
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 62, "Arrow: Select | OK: Profile");
}

static bool campfire_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            selected_camp_slot = CAMP_NORTH;
            return true;
        }
        else if(event->key == InputKeyDown) {
            selected_camp_slot = CAMP_SOUTH;
            return true;
        }
        else if(event->key == InputKeyLeft) {
            selected_camp_slot = CAMP_WEST;
            return true;
        }
        else if(event->key == InputKeyRight) {
            selected_camp_slot = CAMP_EAST;
            return true;
        }
        else if(event->key == InputKeyOk) {
            if(selected_camp_slot != CAMP_SOUTH && campfire_slots[selected_camp_slot].active) {
                view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE_PROFILE);
            }
            return true;
        }
        else if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
            return true;
        }
    }
    return false;
}

// ==================== CAMPFIRE PLAYER PROFILE VIEW ====================
static void campfire_profile_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    
    CampfirePlayer* player = &campfire_slots[selected_camp_slot];
    
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, player->name);
    
    canvas_set_font(canvas, FontSecondary);
    
    char aura_line[64];
    snprintf(aura_line, sizeof(aura_line), "Aura: %s", player->aura);
    canvas_draw_str(canvas, 2, 28, aura_line);
    
    // Handshake info
    canvas_draw_str(canvas, 2, 40, "Handshake: +2 XP");
    
    canvas_draw_str(canvas, 2, 52, "Options:");
    canvas_draw_str(canvas, 2, 62, "L: Duel | R: Echo");
}

static bool campfire_profile_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            // Duel selected
            bool won = (rand() % 100) < 60;  // 60% win rate
            int xp = won ? 15 : 5;  // Win = 15 XP, lose = 5 XP
            record_duel_result(&player_codex, won, xp);
            last_duel_xp = xp;
            // Return to campfire
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
            return true;
        }
        else if(event->key == InputKeyRight) {
            // Echo exchange
            campfire_slots[selected_camp_slot].handshake_xp = 0;  // Already got handshake
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
            return true;
        }
        else if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
            return true;
        }
    }
    return false;
}

// ==================== TECHNIQUE VIEW ====================
static void technique_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Techniques");
    
    canvas_set_font(canvas, FontSecondary);
    
    // Display unlocked techniques
    int display_count = 0;
    for(int i = 0; i < MAX_TECHNIQUES && display_count < 3; i++) {
        TechniqueProgress* tech = &player_codex.techniques[i];
        if(tech->name[0] != 0 && tech->unlocked) {
            char tech_line[64];
            const char* mastery = tech->mastered ? " [MASTERED]" : "";
            snprintf(tech_line, sizeof(tech_line), "%s (x%d%s)", tech->name, tech->uses, mastery);
            canvas_draw_str(canvas, 2, 28 + (display_count * 12), tech_line);
            display_count++;
        }
    }
    
    if(display_count == 0) {
        canvas_draw_str(canvas, 2, 28, "No techniques unlocked");
        canvas_draw_str(canvas, 2, 40, "Complete shrines to gain power");
    }
    
    canvas_draw_str(canvas, 2, 63, "Back: Return to Menu");
}

static bool technique_view_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
        return true;
    }
    return false;
}

// ==================== GENERIC BACK BUTTON ====================
static bool generic_back_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
        return true;
    }
    return false;
}

// Called when player confirms their name in the text input screen
static void name_input_callback(void* context) {
    (void)context;
    init_codex(&player_codex, name_buffer);
    save_codex(&player_codex, NULL);
    view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
}

int32_t flippe_rpg_app(void* p) {
    (void)p;
    srand((unsigned)furi_get_tick());

    // Load Codex — if no save exists, show name entry screen instead of using a default
    bool is_new_player = !load_codex(&player_codex, NULL);

    // Quick visible cue: blink green LED once
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notification, &sequence_single_vibro);
    notification_message_block(notification, &sequence_blink_green_100);
    furi_record_close(RECORD_NOTIFICATION);

    // Create GUI and view dispatcher
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    // ==================== VIEW 0: MAIN MENU ====================
    Menu* menu = menu_alloc();
    menu_add_item(menu, "Absorb Signals", NULL, 0, main_menu_callback, NULL);
    menu_add_item(menu, "Shrines", NULL, 1, main_menu_callback, NULL);
    menu_add_item(menu, "Techniques", NULL, 2, main_menu_callback, NULL);
    menu_add_item(menu, "Multiplayer", NULL, 3, main_menu_callback, NULL);
    menu_add_item(menu, "Status", NULL, 4, main_menu_callback, NULL);
    menu_add_item(menu, "Exit", NULL, 5, main_menu_callback, NULL);
    view_dispatcher_add_view(view_dispatcher, VIEW_MENU, menu_get_view(menu));

    // ==================== VIEW 1: CODEX STATUS ====================
    View* codex_view = view_alloc();
    view_set_draw_callback(codex_view, codex_view_draw_callback);
    view_set_input_callback(codex_view, generic_back_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_CODEX, codex_view);

    // ==================== VIEW 2: SIGNAL ABSORPTION ====================
    View* signal_view = view_alloc();
    view_set_draw_callback(signal_view, signal_view_draw_callback);
    view_set_input_callback(signal_view, signal_view_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_SIGNAL, signal_view);

    // ==================== VIEW 3: SHRINE LIST ====================
    View* shrine_view = view_alloc();
    view_set_draw_callback(shrine_view, shrine_view_draw_callback);
    view_set_input_callback(shrine_view, shrine_view_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_SHRINE_LIST, shrine_view);

    // ==================== VIEW 4: SHRINE DETAIL ====================
    View* shrine_detail_view = view_alloc();
    view_set_draw_callback(shrine_detail_view, shrine_detail_draw_callback);
    view_set_input_callback(shrine_detail_view, shrine_detail_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_SHRINE_DETAIL, shrine_detail_view);

    // ==================== VIEW 6: CAMPFIRE ====================
    View* campfire_view = view_alloc();
    view_set_draw_callback(campfire_view, campfire_draw_callback);
    view_set_input_callback(campfire_view, campfire_input_callback);
    // Allocate and tie campfire fire animation (from root assets/icons)
    // Symbol is generated from folder name: assets/icons/.../Campfire_Fire_16x16
    // When present, A_Campfire_Fire_16x16 will be declared in assets_icons.h
    #ifdef A_Campfire_Fire_16x16
    campfire_fire_anim = icon_animation_alloc(&A_Campfire_Fire_16x16);
    if(campfire_fire_anim) {
        view_tie_icon_animation(campfire_view, campfire_fire_anim);
        icon_animation_start(campfire_fire_anim);
    }
    #endif
    view_dispatcher_add_view(view_dispatcher, VIEW_CAMPFIRE, campfire_view);

    // ==================== VIEW 7: CAMPFIRE PROFILE ====================
    View* campfire_profile_view = view_alloc();
    view_set_draw_callback(campfire_profile_view, campfire_profile_draw_callback);
    view_set_input_callback(campfire_profile_view, campfire_profile_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_CAMPFIRE_PROFILE, campfire_profile_view);

    // ==================== VIEW 5: TECHNIQUES ====================
    View* technique_view = view_alloc();
    view_set_draw_callback(technique_view, technique_view_draw_callback);
    view_set_input_callback(technique_view, technique_view_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_TECHNIQUE, technique_view);

    // ==================== VIEW 8: NAME ENTRY ====================
    name_input = text_input_alloc();
    text_input_set_header_text(name_input, "What is your name?");
    memset(name_buffer, 0, sizeof(name_buffer));
    text_input_set_result_callback(
        name_input,
        name_input_callback,
        NULL,
        name_buffer,
        sizeof(name_buffer),
        false);
    text_input_set_minimum_length(name_input, 1);
    view_dispatcher_add_view(view_dispatcher, VIEW_NAME_ENTRY, text_input_get_view(name_input));

    // New players see name entry first; returning players go straight to menu
    if(is_new_player) {
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_NAME_ENTRY);
    } else {
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
    }

    // Run event loop
    view_dispatcher_run(view_dispatcher);

    // ==================== CLEANUP ====================
    view_dispatcher_remove_view(view_dispatcher, VIEW_MENU);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CODEX);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SIGNAL);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SHRINE_LIST);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SHRINE_DETAIL);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CAMPFIRE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CAMPFIRE_PROFILE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_TECHNIQUE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_NAME_ENTRY);
    
    view_free(codex_view);
    view_free(signal_view);
    view_free(shrine_view);
    view_free(shrine_detail_view);
    view_free(campfire_view);
    if(campfire_fire_anim) {
        icon_animation_stop(campfire_fire_anim);
        icon_animation_free(campfire_fire_anim);
        campfire_fire_anim = NULL;
    }
    view_free(campfire_profile_view);
    view_free(technique_view);
    text_input_free(name_input);
    name_input = NULL;
    menu_free(menu);
    view_dispatcher_free(view_dispatcher);
    furi_record_close(RECORD_GUI);

    // Save progress
    save_codex(&player_codex, NULL);

    return 0;
}