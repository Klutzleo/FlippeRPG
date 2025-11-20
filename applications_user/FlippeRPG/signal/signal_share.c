// signal_share.c
#include "signal_share.h"
#include "../xp/xp_engine.h"
#include <stdio.h>
#include <string.h>

void share_signal(Codex* codex, SignalType type, const char* signal_hash) {
    printf("[Signal] Sharing signal (%s) via %d...\n", signal_hash, type);
    // IR/NFC/SubGHz broadcast logic goes here
}

void receive_signal(Codex* codex, SignalType type, const char* signal_hash) {
    printf("[Signal] Received signal (%s) via %d\n", signal_hash, type);
    log_signal(codex, signal_hash, type);
}