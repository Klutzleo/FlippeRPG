#include "ui.h"
#include <gui/modules/menu.h>
#include <gui/modules/text_input.h>
#include <gui/view.h>
#include <string.h>
#include <stdio.h>

// Simplified UI functions for basic rendering
// Scene callbacks are stubbed here but can be expanded later

void flippe_rpg_scene_menu_on_enter(void* context) {
    (void)context;
}

void flippe_rpg_scene_menu_on_exit(void* context) {
    (void)context;
}

bool flippe_rpg_scene_menu_on_event(void* context, SceneManagerEvent event) {
    (void)context;
    (void)event;
    return false;
}

void flippe_rpg_scene_name_input_on_enter(void* context) {
    (void)context;
}

void flippe_rpg_scene_name_input_on_exit(void* context) {
    (void)context;
}

bool flippe_rpg_scene_name_input_on_event(void* context, SceneManagerEvent event) {
    (void)context;
    (void)event;
    return false;
}

void flippe_rpg_scene_codex_view_on_enter(void* context) {
    (void)context;
}

void flippe_rpg_scene_codex_view_on_exit(void* context) {
    (void)context;
}

bool flippe_rpg_scene_codex_view_on_event(void* context, SceneManagerEvent event) {
    (void)context;
    (void)event;
    return false;
}

