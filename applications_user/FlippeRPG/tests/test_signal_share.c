#include "../codex/codex.h"
#include "../signal/signal_share.h"
#include <assert.h>
#include <stdio.h>

int main() {
    Codex test_codex = {0};
    strncpy(test_codex.player_name, "Jason", 15);

    const char* signal = "ECHO-1234";
    SignalType type = SIGNAL_IR;

    share_signal(&test_codex, type, signal);
    receive_signal(&test_codex, type, signal);

    assert(test_codex.xp_total > 0);
    printf("✅ Signal sharing test passed: XP awarded for received signal.\n");

    return 0;
}