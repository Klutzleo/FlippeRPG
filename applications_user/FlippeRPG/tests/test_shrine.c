#include "../codex/codex.h"
#include "../shrine/shrine.h"
#include <assert.h>
#include <stdio.h>
#include "../core/constants.h"

int main() {
    Codex test_codex = {0};  // Zero-initialize Codex

    // Simulate shrine trigger with correct signal
    trigger_shrine(&test_codex, SHRINE_FLAME_REACH, SIGNAL_IR);

    // Assert technique was unlocked
    assert(codex_has_technique(&test_codex, "Flame Reach"));
    printf("✅ Shrine test passed: Flame Reach unlocked.\n");

    return 0;
}