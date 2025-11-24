#include "../codex/codex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define AURA_FLAMEBOUND   "Flamebound"
#define AURA_WHISPERED    "Whispered"
#define AURA_ECHOFORGED   "Echoforged"
#define AURA_STORMTOUCHED "Stormtouched"
#define AURA_UNKNOWN      "Unknown"

// Attempt a shrine ritual; success or failure determines outcome
void attempt_shrine(Codex* codex, ShrineID shrine_id, bool ritual_success) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];

    if (ritual_success) {
        shrine->completed = true;
        shrine->last_completed_time = time(NULL);

        assign_aura(codex, shrine_id);
        printf("[Shrine] Shrine %d completed. Aura assigned: %s\n",
               shrine_id, codex->aura_trait);

        // 🔮 After shrine completion, check for lineage convergence
        if (ready_for_convergence(codex)) {
            process_echo(codex, false, false); // lineage trigger
        }
    } else {
        printf("[Shrine] Shrine %d ritual failed.\n", shrine_id);

        // 🔮 50% chance corruption on failure
        if (rand() % 100 < 50) {
            process_echo(codex, false, true); // corruption event
            shrine->resettable = true;        // corruption unlocks replay
            printf("[Shrine] Shrine %d corrupted — reset unlocked.\n", shrine_id);
        } else {
            printf("[Shrine] Failure passed without corruption.\n");
        }
    }
}

// Mark shrine as resettable (corruption unlocks replay)
void reset_shrine(Codex* codex, ShrineID shrine_id) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];
    if (shrine->completed && shrine->resettable) {
        shrine->completed = false;
        shrine->resettable = false;
        shrine->last_visited = time(NULL);
        printf("[Shrine] Shrine %d reset — ritual can be replayed.\n", shrine_id);
    } else {
        printf("[Shrine] Shrine %d cannot be reset.\n", shrine_id);
    }
}

// Complete shrine directly (used for scripted unlocks or guaranteed rituals)
void complete_shrine(Codex* codex, ShrineID shrine_id) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];
    shrine->completed = true;
    shrine->last_completed_time = time(NULL);

    assign_aura(codex, shrine_id);
    printf("[Shrine] Shrine %d completed. Aura assigned: %s\n",
           shrine_id, codex->aura_trait);

    if (ready_for_convergence(codex)) {
        process_echo(codex, false, false);
    }
}

// Map shrine IDs to aura traits
void assign_aura(Codex* codex, ShrineID shrine_id) {
    const char* aura = NULL;
    switch (shrine_id) {
        case SHRINE_FLAME_REACH:       aura = AURA_FLAMEBOUND;   break;
        case SHRINE_BIND_WHISPER:      aura = AURA_WHISPERED;    break;
        case SHRINE_ECHO_TOUCHED:      aura = AURA_ECHOFORGED;   break;
        case SHRINE_CAVE_THAT_LISTENS: aura = AURA_STORMTOUCHED; break;
        default:                       aura = AURA_UNKNOWN;      break;
    }
    snprintf(codex->aura_trait, sizeof(codex->aura_trait), "%s", aura);
}