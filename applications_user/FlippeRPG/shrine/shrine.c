#include "../codex/codex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <furi.h>

#define AURA_CLEARSEEING  "Clearseeing"
#define AURA_TOUCHMARKED  "Touchmarked"
#define AURA_GROUNDED     "Grounded"
#define AURA_STORMTOUCHED "Stormtouched"
#define AURA_HEARD        "Heard"
#define AURA_UNKNOWN      "Unknown"

// Attempt a shrine ritual; success or failure determines outcome
void attempt_shrine(Codex* codex, ShrineID shrine_id, bool ritual_success) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];

    if (ritual_success) {
        shrine->completed = true;
        shrine->last_completed_time = furi_get_tick();

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

// Maps each shrine to the signal type required to activate it
static const SignalType shrine_requirements[NUM_SHRINES] = {
    [SHRINE_CAVE_THAT_LISTENS] = SIGNAL_SUBGHZ,
    [SHRINE_FLAME_REACH]       = SIGNAL_IR,
    [SHRINE_BIND_WHISPER]      = SIGNAL_NFC,
    [SHRINE_THREAD_TOUCH]      = SIGNAL_BLUETOOTH,
    [SHRINE_ECHO_TOUCHED]      = SIGNAL_RF,
    [SHRINE_UNKNOWN]           = SIGNAL_UNKNOWN,
};

// Triggers a shrine ritual — only succeeds if the signal type matches the shrine's requirement
void trigger_shrine(Codex* codex, ShrineID shrine_id, SignalType signal_type) {
    if(shrine_id >= NUM_SHRINES) return;
    bool correct_signal = (signal_type == shrine_requirements[shrine_id]);
    attempt_shrine(codex, shrine_id, correct_signal);
}

bool is_ritual_complete(Codex* codex, ShrineID shrine_id) {
    return codex->shrine_progress[shrine_id].completed;
}

// Mark shrine as resettable (corruption unlocks replay)
void reset_shrine(Codex* codex, ShrineID shrine_id) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];
    if (shrine->completed && shrine->resettable) {
        shrine->completed = false;
        shrine->resettable = false;
        shrine->last_visited = furi_get_tick();
        printf("[Shrine] Shrine %d reset — ritual can be replayed.\n", shrine_id);
    } else {
        printf("[Shrine] Shrine %d cannot be reset.\n", shrine_id);
    }
}

// Complete shrine directly (used for scripted unlocks or guaranteed rituals)
void complete_shrine(Codex* codex, ShrineID shrine_id) {
    ShrineProgress* shrine = &codex->shrine_progress[shrine_id];
    shrine->completed = true;
    shrine->last_completed_time = furi_get_tick();

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
        case SHRINE_FLAME_REACH:       aura = AURA_CLEARSEEING;  break;
        case SHRINE_BIND_WHISPER:      aura = AURA_TOUCHMARKED;  break;
        case SHRINE_ECHO_TOUCHED:      aura = AURA_GROUNDED;     break;
        case SHRINE_CAVE_THAT_LISTENS: aura = AURA_STORMTOUCHED; break;
        case SHRINE_THREAD_TOUCH:      aura = AURA_HEARD;        break;
        default:                       aura = AURA_UNKNOWN;      break;
    }
    snprintf(codex->aura_trait, sizeof(codex->aura_trait), "%s", aura);
}