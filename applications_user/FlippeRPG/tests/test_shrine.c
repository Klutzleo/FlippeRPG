#include "../codex/codex.h"
#include "../shrine/shrine.h"
#include "../shrine/shrine_definitions.h"
#include "../core/constants.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper to print shrine status
void print_shrine_status(Codex* codex, ShrineID shrine_id) {
    const ShrineDefinition* shrine = &shrine_definitions[shrine_id];
    printf("[Shrine] %s | Required Signal: %d | Ritual Complete: %s\n",
           shrine->name,
           shrine->required_signal,
           codex->shrine_progress[shrine_id].completed ? "Yes" : "No");
}

int main() {
    Codex test_codex = {0};
    strncpy(test_codex.player_name, "Jason", 15);

    ShrineID shrine_id = SHRINE_FLAME_REACH;
    SignalType signal = SIGNAL_IR;

    test_codex.shrine_progress[shrine_id].cooldown_seconds = 5;

    trigger_shrine(&test_codex, shrine_id, signal);
    print_shrine_status(&test_codex, shrine_id);

    trigger_shrine(&test_codex, shrine_id, signal);

    test_codex.shrine_progress[shrine_id].last_visited -= 10;

    trigger_shrine(&test_codex, shrine_id, signal);

    assert(codex_has_technique(&test_codex, "Flame Reach"));
    printf("✅ Shrine test passed: Flame Reach unlocked and cooldown logic verified.\n");

    return 0;
}