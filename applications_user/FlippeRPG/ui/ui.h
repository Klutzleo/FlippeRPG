#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include "../codex/codex.h"

typedef struct {
    ViewDispatcher* view_dispatcher;
    Codex* codex;
    char player_name_input[64];
} FlippeRPGApp;

// Scene IDs
typedef enum {
    FlippeRPGSceneMenu,
    FlippeRPGSceneNameInput,
    FlippeRPGSceneCodexView,
    FlippeRPGSceneCount,
} FlippeRPGScene;

// Scene event stub
typedef struct {
    uint32_t type;
} SceneManagerEvent;

// Scene setup
void flippe_rpg_scene_menu_on_enter(void* context);
void flippe_rpg_scene_menu_on_exit(void* context);
bool flippe_rpg_scene_menu_on_event(void* context, SceneManagerEvent event);

void flippe_rpg_scene_name_input_on_enter(void* context);
void flippe_rpg_scene_name_input_on_exit(void* context);
bool flippe_rpg_scene_name_input_on_event(void* context, SceneManagerEvent event);

void flippe_rpg_scene_codex_view_on_enter(void* context);
void flippe_rpg_scene_codex_view_on_exit(void* context);
bool flippe_rpg_scene_codex_view_on_event(void* context, SceneManagerEvent event);
