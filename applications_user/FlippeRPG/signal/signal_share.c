// signal_share.c
#include "signal_share.h"
#include "signal_engine.h"
#include <stdio.h>
#include <string.h>

void share_signal(Codex* codex, SignalType type, const char* signal_hash) {
    (void)codex; // Unused parameter
    printf("[Signal] Sharing signal (%s) via %d...\n", signal_hash, type);
    // IR/NFC/SubGHz broadcast logic goes here
}

void receive_signal(Codex* codex, SignalType type, const char* signal_hash) {
    printf("[Signal] Received signal (%s) via %d\n", signal_hash, type);
    int gain = calculate_signal_gain(codex, signal_hash);
    log_signal(codex, signal_hash, gain, type);

    // Ambient aura feedback
    printf("[Aura] You sense a presence... Their aura is %s.\n", codex->aura_trait);
}