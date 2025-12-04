#include "../shrine/shrine_tags.h"
#include "../codex/codex.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(void) {
    Codex test_codex = {0};
    strncpy(test_codex.player_name, "Jason", 15);

    // Imprint a shrine tag
    imprint_shrine_tag(SHRINE_FLAME_REACH, "TAG-FLAME", time(NULL), "Flamebound");

    // Scan the shrine tag into the codex
    scan_shrine_tag(&test_codex, "TAG-FLAME");

    // Verify codex state (example assertion)
    assert(test_codex.shrine_progress[SHRINE_FLAME_REACH].completed);

    printf("✅ Signal tag test passed: Shrine tag imprinted and scanned.\n");
    return 0;
}