#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../codex/codex.h"

// Helper to print technique state
void print_technique_state(Codex* codex, const char* name) {
    for (int i = 0; i < MAX_TECHNIQUES; i++) {
        if (strcmp(codex->techniques[i].name, name) == 0) {
            printf("[Technique] %s | Uses: %d | Mastered: %s\n",
                   codex->techniques[i].name,
                   codex->techniques[i].uses,
                   codex->techniques[i].mastered ? "Yes" : "No");
            return;
        }
    }
    printf("[Technique] %s not found.\n", name);
}

int main() {
    Codex test_codex = {0};
    strncpy(test_codex.player_name, "Jason", 15);

    codex_unlock_technique(&test_codex, "Pulse Open");
    assert(codex_has_technique(&test_codex, "Pulse Open"));

    for (int i = 0; i < 10; i++) {
        codex_use_technique(&test_codex, "Pulse Open");
        print_technique_state(&test_codex, "Pulse Open");
    }

    assert(test_codex.techniques[0].mastered);
    printf("✅ Codex test passed: Pulse Open mastered after 10 uses.\n");
    return 0;
}