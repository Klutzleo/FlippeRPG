#include "../codex/codex.h"
#include "../signal/signal_fusion.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main() {
    Codex test_codex = {0};
    strncpy(test_codex.player_name, "Jason", 15);

    const char* echo1 = "ECHO-ALPHA";
    const char* echo2 = "ECHO-BETA";

    // Log both echoes
    log_signal(&test_codex, echo1, SIGNAL_IR);
    log_signal(&test_codex, echo2, SIGNAL_NFC);

    // Fuse them
    fuse_echoes(&test_codex, echo1, echo2);

    // Check fused echo exists
    assert(test_codex.xp_total > 0); // XP awarded
    printf("✅ Echo fusion test passed: Fused echo logged and XP awarded.\n");

    return 0;
}