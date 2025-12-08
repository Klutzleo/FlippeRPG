#pragma once

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include "../codex/codex.h"

typedef struct {
    ViewDispatcher* view_dispatcher;
    Codex* codex;
    char player_name_input[64];
} FlippeRPGApp;
