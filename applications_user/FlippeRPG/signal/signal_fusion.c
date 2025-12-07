#include "signal_fusion.h"
#include "signal_engine.h"
#include <stdio.h>
#include <string.h>
#include "../echo/echo_fusion.h"
#include "../core/utils.h"   // for popup_message

bool can_fuse_echoes(Codex* codex, const char* echo_a, const char* echo_b) {
    if (strcmp(echo_a, echo_b) == 0) return false;

    // Example synergy: echoes from different signal types fuse better
    SignalType type_a = get_signal_type(codex, echo_a);
    SignalType type_b = get_signal_type(codex, echo_b);

    return type_a != type_b;
}
