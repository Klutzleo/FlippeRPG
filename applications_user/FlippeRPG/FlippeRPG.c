#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/menu.h>
#include <gui/modules/text_input.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "codex/codex.h"
#include "duel/duel.h"
#include "signal/signal_engine.h"
#include "signal/rfid_scanner.h"
#include "techniques/techniques.h"
#include "save/save_system.h"
#include "shrine/shrine.h"
#include "core/utils.h"
#include "core/constants.h"
// Assets: icon animations compiled from root assets/icons
#include <assets_icons.h>
#include <gui/icon_animation.h>
#include <notification/notification_messages.h>

// View IDs
enum {
    VIEW_MENU = 0,
    VIEW_CODEX = 1,
    VIEW_SIGNAL = 2,
    VIEW_SHRINE_LIST = 3,
    VIEW_SHRINE_DETAIL = 4,
    VIEW_CAMPFIRE = 5,
    VIEW_CAMPFIRE_PROFILE = 6,
    VIEW_NAME_ENTRY = 7,
    VIEW_DUEL = 8,
    VIEW_SUBSTRATE = 9,
};

// Fixed menu callback IDs — stable regardless of which items are currently visible.
// Pre-substrate: only SIGNALS, STATUS, EXIT are shown.
// Post-substrate: all items appear. Items are hidden (not just locked) until substrate.
#define MENU_ID_SIGNALS   0
#define MENU_ID_SUBSTRATE 1
#define MENU_ID_SHRINES   2
#define MENU_ID_CAMPFIRE  3
#define MENU_ID_STATUS    4
#define MENU_ID_EXIT      5

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
static Menu* main_menu = NULL;          // Rebuilt dynamically when substrate unlocks
static bool last_scan_absorbed = false; // Flip true after a scan, drives signal view feedback
static bool rfid_scanning = false;      // True while LFRFIDWorker is listening for a card
static int selected_shrine = 0;        // Currently selected shrine for detail view
static int active_scan_band_index = 0; // Which unlocked band the signal view has selected
static DuelState current_duel;         // Active duel state

// CQ in Morse code: ─·─· (C) then ─ ─·─ (Q)
// "Calling all stations." The only vibration in the game. Fires once, when The Substrate opens.
// Unit = 100ms. Dot=1u on. Dash=3u on. Intra-element gap=1u off. Inter-letter gap=3u off.
static const NotificationSequence substrate_morse_cq = {
    // C: ─ · ─ ·
    &message_vibro_on,
    &message_delay_100, &message_delay_100, &message_delay_100,  // dash
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,  &message_delay_100,                       // dot
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,
    &message_delay_100, &message_delay_100, &message_delay_100,  // dash
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,  &message_delay_100,                       // dot
    &message_vibro_off,
    // Inter-letter gap: 3 units
    &message_delay_100, &message_delay_100, &message_delay_100,
    // Q: ─ ─ · ─
    &message_vibro_on,
    &message_delay_100, &message_delay_100, &message_delay_100,  // dash
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,
    &message_delay_100, &message_delay_100, &message_delay_100,  // dash
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,  &message_delay_100,                       // dot
    &message_vibro_off, &message_delay_100,
    &message_vibro_on,
    &message_delay_100, &message_delay_100, &message_delay_100,  // dash
    &message_vibro_off,
    NULL,
};

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
        case SHRINE_CAVE_THAT_LISTENS: return "The Open Channel";
        case SHRINE_FLAME_REACH: return "The Fixed Gaze";
        case SHRINE_BIND_WHISPER: return "The Exchange";
        case SHRINE_THREAD_TOUCH: return "The Unanswered Hello";
        case SHRINE_ECHO_TOUCHED: return "The First Frequency";
        default: return "Unknown Shrine";
    }
}

static const char* get_shrine_description(ShrineID id) {
    switch(id) {
        case SHRINE_CAVE_THAT_LISTENS: return "Everything passes through here.";
        case SHRINE_FLAME_REACH: return "It saw you before you saw it.";
        case SHRINE_BIND_WHISPER: return "It will not come to you.";
        case SHRINE_THREAD_TOUCH: return "Broadcasting. Still trying.";
        case SHRINE_ECHO_TOUCHED: return "It was here before the city.";
        default: return "A mysterious shrine";
    }
}

static const char* get_aura_name(ShrineID id) {
    switch(id) {
        case SHRINE_CAVE_THAT_LISTENS: return "Stormtouched";
        case SHRINE_FLAME_REACH: return "Clearseeing";
        case SHRINE_BIND_WHISPER: return "Touchmarked";
        case SHRINE_ECHO_TOUCHED: return "Grounded";
        case SHRINE_THREAD_TOUCH: return "Heard";
        default: return "Unknown";
    }
}

// Forward declaration — defined later in this file
static void trigger_substrate_unlock(void);

// ==================== CUSTOM EVENT HANDLER ====================
// Receives events posted from background worker threads (e.g. RFID scanner).
static bool app_custom_event_callback(void* context, uint32_t event) {
    (void)context;
    if(event == RFID_SCAN_DONE_EVENT) {
        char hash[32];
        rfid_scanner_get_hash(hash, sizeof(hash));
        rfid_scanner_stop();
        rfid_scanning = false;
        on_rfid_scan(&player_codex, hash);
        last_scan_absorbed = true;
        if(check_band_gate(&player_codex)) {
            trigger_substrate_unlock();
        }
        return true;
    }
    return false;
}

// ==================== MENU VIEW ====================
static void main_menu_callback(void* context, uint32_t index) {
    (void)context;
    switch(index) {
        case MENU_ID_SIGNALS:
            active_scan_band_index = 0;
            last_scan_absorbed = false;
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SIGNAL);
            break;
        case MENU_ID_SUBSTRATE:
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SUBSTRATE);
            break;
        case MENU_ID_SHRINES:
            selected_shrine = 0;
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_SHRINE_LIST);
            break;
        case MENU_ID_CAMPFIRE:
            // Campfire requires Zero Day and faction — gated post-substrate
            if(!player_codex.zero_day_confirmed || player_codex.faction[0] == '\0') break;
            selected_camp_slot = CAMP_NORTH;
            last_scan_tick = furi_get_tick();
            memset(campfire_slots, 0, sizeof(campfire_slots));
            for(int i = 0; i < MAX_ENCOUNTERS && i < MAX_CAMP_SLOTS; i++) {
                if(player_codex.encounter_log[i].signalborn_id[0] != 0) {
                    strncpy(campfire_slots[i].name, player_codex.encounter_log[i].signalborn_id, sizeof(campfire_slots[i].name));
                    strncpy(campfire_slots[i].aura, player_codex.encounter_log[i].aura, sizeof(campfire_slots[i].aura));
                    campfire_slots[i].last_seen_tick = furi_get_tick();
                    campfire_slots[i].active = true;
                    campfire_slots[i].handshake_xp = 2;
                }
            }
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
            break;
        case MENU_ID_STATUS:
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_CODEX);
            break;
        case MENU_ID_EXIT:
            view_dispatcher_stop(view_dispatcher);
            break;
    }
}

// Builds (or rebuilds) the main menu based on current substrate state.
// Pre-substrate: only Receive, Status, Exit.
// Post-substrate: The Substrate, Shrines, Campfire also appear.
static void build_menu(void) {
    if(main_menu) {
        view_dispatcher_remove_view(view_dispatcher, VIEW_MENU);
        menu_free(main_menu);
    }
    main_menu = menu_alloc();
    menu_add_item(main_menu, "Receive", NULL, MENU_ID_SIGNALS, main_menu_callback, NULL);
    if(player_codex.substrate_unlocked) {
        menu_add_item(main_menu, "The Substrate",  NULL, MENU_ID_SUBSTRATE, main_menu_callback, NULL);
        menu_add_item(main_menu, "Shrines",        NULL, MENU_ID_SHRINES,   main_menu_callback, NULL);
        menu_add_item(main_menu, "Campfire",       NULL, MENU_ID_CAMPFIRE,  main_menu_callback, NULL);
    }
    menu_add_item(main_menu, "Status", NULL, MENU_ID_STATUS, main_menu_callback, NULL);
    menu_add_item(main_menu, "Exit",   NULL, MENU_ID_EXIT,   main_menu_callback, NULL);
    view_dispatcher_add_view(view_dispatcher, VIEW_MENU, menu_get_view(main_menu));
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
    snprintf(line3, sizeof(line3), "Signal: %s",
        signal_strength_labels[player_codex.signal_strength_level]);
    snprintf(line4, sizeof(line4), "Duels: %dW / %dL",
        player_codex.duels_won, player_codex.duels_lost);
    snprintf(line5, sizeof(line5), "Faction: %s",
        player_codex.faction[0] ? player_codex.faction : "Unaligned");
    // Aura is revealed only after Zero Day — determined by scan behavior via PWA
    snprintf(line6, sizeof(line6), "Aura: %s",
        player_codex.zero_day_confirmed && player_codex.aura_trait[0]
            ? player_codex.aura_trait
            : "Not yet revealed");
    
    canvas_draw_str(canvas, 2, 22, line1);
    canvas_draw_str(canvas, 2, 30, line2);
    canvas_draw_str(canvas, 2, 38, line3);
    canvas_draw_str(canvas, 2, 46, line4);
    canvas_draw_str(canvas, 2, 54, line5);
    canvas_draw_str(canvas, 2, 62, line6);
}

// ==================== RECEIVE VIEW ====================

static const char* get_band_name(SignalType type) {
    switch(type) {
        case SIGNAL_RFID:      return "RFID";
        case SIGNAL_RF:        return "RF";
        case SIGNAL_IR:        return "IR";
        case SIGNAL_SUBGHZ:    return "Sub-GHz";
        case SIGNAL_NFC:       return "NFC";
        case SIGNAL_BLUETOOTH: return "Bluetooth";
        default:               return "---";
    }
}

// Fires CQ in Morse, rebuilds the menu with all items visible, and saves.
// Called exactly once — the moment check_band_gate returns true.
static void trigger_substrate_unlock(void) {
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    notification_message(notifications, &substrate_morse_cq);
    furi_record_close(RECORD_NOTIFICATION);
    build_menu(); // The Substrate is now in the menu — no announcement, just there
    save_codex(&player_codex, NULL);
}

static void signal_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Receive");

    canvas_set_font(canvas, FontSecondary);

    int bands = player_codex.bands_unlocked;
    if(bands < 1) bands = 1;
    if(active_scan_band_index >= bands) active_scan_band_index = bands - 1;

    // Vertical band list — new bands appear at the bottom as they unlock
    // Stride 7px: fits 6 bands (18,25,32,39,46,53) with footer clear at 62
    for(int i = 0; i < bands; i++) {
        int y = 18 + (i * 7);
        const char* name = get_band_name(band_order[i]);
        if(i == active_scan_band_index) {
            canvas_draw_str(canvas, 0, y, ">");
            canvas_draw_str(canvas, 8, y, name);
        } else {
            canvas_draw_str(canvas, 8, y, name);
        }
    }

    // Footer: scanning state, last result, or navigation hint
    if(rfid_scanning) {
        canvas_draw_str(canvas, 2, 62, "Scanning... (Back: cancel)");
    } else if(last_scan_absorbed) {
        char footer[32];
        snprintf(footer, sizeof(footer), "Signal: %s",
            signal_strength_labels[player_codex.signal_strength_level]);
        canvas_draw_str(canvas, 2, 62, footer);
    } else {
        canvas_draw_str(canvas, 2, 62, "OK: receive  Back: menu");
    }
}

static bool signal_view_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type != InputTypeShort) return false;

    int bands = player_codex.bands_unlocked;
    if(bands < 1) bands = 1;

    if(event->key == InputKeyUp) {
        if(active_scan_band_index > 0) active_scan_band_index--;
        return true;
    }
    if(event->key == InputKeyDown) {
        if(active_scan_band_index < bands - 1) active_scan_band_index++;
        return true;
    }
    if(event->key == InputKeyOk) {
        SignalType scan_type = band_order[active_scan_band_index];
        if(scan_type == SIGNAL_RFID) {
            // Real hardware — start async scan, result arrives via RFID_SCAN_DONE_EVENT
            if(!rfid_scanning) {
                rfid_scanner_start(view_dispatcher);
                rfid_scanning = true;
            }
        } else {
            // Simulated scan for bands not yet wired to hardware
            scan_band(&player_codex, scan_type);
            last_scan_absorbed = true;
            if(check_band_gate(&player_codex)) {
                trigger_substrate_unlock();
            }
        }
        return true;
    }
    if(event->key == InputKeyBack) {
        if(rfid_scanning) {
            rfid_scanner_stop();
            rfid_scanning = false;
        }
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_MENU);
        return true;
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
            // Launch Signal Reading duel
            duel_init(&current_duel, &player_codex);
            view_dispatcher_switch_to_view(view_dispatcher, VIEW_DUEL);
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

// ==================== DUEL VIEW ====================
static void duel_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);

    if(current_duel.phase == DUEL_PHASE_RESULT) {
        // Result screen
        canvas_set_font(canvas, FontPrimary);
        if(current_duel.result == DUEL_RESULT_WIN) {
            canvas_draw_str(canvas, 2, 20, "CORRECT");
            canvas_draw_str(canvas, 2, 36, "Signal identified.");
            char sig_line[32];
            snprintf(sig_line, sizeof(sig_line), "Signal: %s",
                signal_strength_labels[player_codex.signal_strength_level]);
            canvas_draw_str(canvas, 2, 50, sig_line);
        } else {
            canvas_draw_str(canvas, 2, 20, "MISREAD");
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_str(canvas, 2, 36, "Signal lost. Presence drains.");
            const char* presence;
            if(player_codex.stamina >= 80)      presence = "Presence: strong";
            else if(player_codex.stamina >= 60) presence = "Presence: steady";
            else if(player_codex.stamina >= 40) presence = "Presence: waning";
            else if(player_codex.stamina >= 20) presence = "Presence: thin";
            else                                presence = "Presence: fading";
            canvas_draw_str(canvas, 2, 50, presence);
        }
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 2, 62, "Press any key");
        return;
    }

    // Choosing screen
    int secs = duel_seconds_remaining(&current_duel);

    canvas_set_font(canvas, FontSecondary);
    char header[32];
    snprintf(header, sizeof(header), "SIGNAL READING  [%d]", secs);
    canvas_draw_str(canvas, 2, 8, header);
    canvas_draw_line(canvas, 0, 11, 127, 11);

    for(int i = 0; i < DUEL_OPTIONS; i++) {
        int y = 22 + i * 13;
        bool selected = (i == current_duel.selected_index);
        const char* name = duel_signal_name(current_duel.options[i]);
        const char* desc = duel_descriptor(current_duel.options[i], current_duel.aura_edge[i]);

        if(selected) {
            canvas_draw_box(canvas, 0, y - 8, 128, 11);
            canvas_set_color(canvas, ColorWhite);
        }

        char opt_line[48];
        if(current_duel.aura_edge[i]) {
            snprintf(opt_line, sizeof(opt_line), "> %-7s %s [!]", name, desc);
        } else {
            snprintf(opt_line, sizeof(opt_line), "  %-7s %s", name, desc);
        }
        canvas_draw_str(canvas, 2, y, opt_line);

        if(selected) {
            canvas_set_color(canvas, ColorBlack);
        }
    }

    canvas_draw_str(canvas, 2, 62, "Up/Dn: select  OK: choose");
}

static bool duel_input_callback(InputEvent* event, void* context) {
    (void)context;
    if(event->type != InputTypeShort) return false;

    // Auto-timeout check — runs before any key handling
    if(current_duel.phase == DUEL_PHASE_CHOOSING && duel_seconds_remaining(&current_duel) == 0) {
        duel_timeout(&current_duel, &player_codex);
        return true;
    }

    if(current_duel.phase == DUEL_PHASE_CHOOSING) {
        if(event->key == InputKeyUp) {
            duel_move_selection(&current_duel, -1);
            return true;
        } else if(event->key == InputKeyDown) {
            duel_move_selection(&current_duel, 1);
            return true;
        } else if(event->key == InputKeyOk || event->key == InputKeyRight) {
            duel_resolve(&current_duel, &player_codex);
            return true;
        } else if(event->key == InputKeyBack) {
            // Bail — counts as a loss (no free exits)
            duel_timeout(&current_duel, &player_codex);
            return true;
        }
    } else {
        // DUEL_PHASE_RESULT — any key returns to campfire
        view_dispatcher_switch_to_view(view_dispatcher, VIEW_CAMPFIRE);
        return true;
    }

    return false;
}

// ==================== THE SUBSTRATE VIEW ====================
static void substrate_view_draw_callback(Canvas* canvas, void* model) {
    (void)model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "THE SUBSTRATE");

    // QR code placeholder — actual encoding via PWA (codex ID + state)
    canvas_draw_frame(canvas, 40, 14, 48, 40);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 50, 32, "[ QR ]");
    canvas_draw_str(canvas, 48, 42, "COMING");

    // Codex ID shown beneath — scanned by PWA to identify this Signalborn
    char id_line[24];
    snprintf(id_line, sizeof(id_line), "ID: %s", player_codex.codex_id);
    canvas_draw_str(canvas, 2, 54, id_line);

    canvas_draw_str(canvas, 2, 62, "Back: Menu");
}

static bool substrate_view_input_callback(InputEvent* event, void* context) {
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

    // Create GUI and view dispatcher
    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_custom_event_callback(view_dispatcher, app_custom_event_callback);

    // ==================== VIEW 0: MAIN MENU ====================
    // Built dynamically — shows only Receive/Status/Exit until substrate unlocks.
    build_menu();

    // ==================== VIEW 1: CODEX STATUS ====================
    View* codex_view = view_alloc();
    view_set_draw_callback(codex_view, codex_view_draw_callback);
    view_set_input_callback(codex_view, generic_back_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_CODEX, codex_view);

    // ==================== VIEW 2: RECEIVE ====================
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

    // ==================== VIEW 8: DUEL ====================
    View* duel_view = view_alloc();
    view_set_draw_callback(duel_view, duel_draw_callback);
    view_set_input_callback(duel_view, duel_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_DUEL, duel_view);

    // ==================== VIEW 9: THE SUBSTRATE ====================
    View* substrate_view = view_alloc();
    view_set_draw_callback(substrate_view, substrate_view_draw_callback);
    view_set_input_callback(substrate_view, substrate_view_input_callback);
    view_dispatcher_add_view(view_dispatcher, VIEW_SUBSTRATE, substrate_view);

    // ==================== VIEW 7: NAME ENTRY ====================
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
    rfid_scanner_stop(); // No-op if not active; guards against exit mid-scan
    view_dispatcher_remove_view(view_dispatcher, VIEW_MENU);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CODEX);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SIGNAL);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SHRINE_LIST);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SHRINE_DETAIL);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CAMPFIRE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_CAMPFIRE_PROFILE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_SUBSTRATE);
    view_dispatcher_remove_view(view_dispatcher, VIEW_NAME_ENTRY);
    view_dispatcher_remove_view(view_dispatcher, VIEW_DUEL);

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
    view_free(substrate_view);
    view_free(duel_view);
    text_input_free(name_input);
    name_input = NULL;
    menu_free(main_menu);
    main_menu = NULL;
    view_dispatcher_free(view_dispatcher);
    furi_record_close(RECORD_GUI);

    // Save progress
    save_codex(&player_codex, NULL);

    return 0;
}