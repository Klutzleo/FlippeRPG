#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../core/constants.h"
#include "../codex/codex.h"

#define DUEL_OPTIONS       3
#define DUEL_TIMER_MS      3000
#define DUEL_STAMINA_LOSS  20   // Stamina drained on loss — signal strength unaffected
#define STAMINA_MAX        100
#define DUEL_HISTORY_SCAN  20

typedef enum {
    DUEL_PHASE_CHOOSING,
    DUEL_PHASE_RESULT,
} DuelPhase;

typedef enum {
    DUEL_RESULT_WIN,
    DUEL_RESULT_LOSS,
    DUEL_RESULT_TIMEOUT,
} DuelResult;

typedef struct {
    SignalType options[DUEL_OPTIONS];
    bool       aura_edge[DUEL_OPTIONS];
    SignalType ground_truth;
    int        selected_index;
    uint32_t   start_tick;
    DuelPhase  phase;
    DuelResult result;
} DuelState;

void        duel_init(DuelState* duel, const Codex* codex);
int         duel_seconds_remaining(const DuelState* duel);
void        duel_move_selection(DuelState* duel, int delta);
DuelResult  duel_resolve(DuelState* duel, Codex* codex);
void        duel_timeout(DuelState* duel, Codex* codex);
const char* duel_signal_name(SignalType type);
const char* duel_descriptor(SignalType type, bool aura_edge);
