#include "duel.h"
#include <furi.h>
#include <stdlib.h>
#include <string.h>

// -------------------- DESCRIPTOR TABLES --------------------

// Neutral two-word reads — what anyone sees
static const char* descriptors_neutral[SIGNAL_UNKNOWN] = {
    [SIGNAL_SUBGHZ]    = "fragmented / passing",
    [SIGNAL_IR]        = "close / watching",
    [SIGNAL_NFC]       = "still / layered",
    [SIGNAL_BLUETOOTH] = "scattered / reaching",
    [SIGNAL_RF]        = "ancient / deep",
};

// Aura-edge reads — sharpened perception on your native signal
static const char* descriptors_edge[SIGNAL_UNKNOWN] = {
    [SIGNAL_SUBGHZ]    = "pulsing / passing",
    [SIGNAL_IR]        = "fixed / watching",
    [SIGNAL_NFC]       = "bonded / layered",
    [SIGNAL_BLUETOOTH] = "heard / reaching",
    [SIGNAL_RF]        = "resonant / deep",
};

static const char* signal_names[SIGNAL_UNKNOWN] = {
    [SIGNAL_SUBGHZ]    = "SUB-GHz",
    [SIGNAL_IR]        = "IR",
    [SIGNAL_NFC]       = "NFC",
    [SIGNAL_BLUETOOTH] = "BT",
    [SIGNAL_RF]        = "RF",
};

// -------------------- AURA → SIGNAL MAPPING --------------------

// Returns which signal type the player's aura sharpens.
// Matched against codex->aura_trait string.
static SignalType aura_native_signal(const char* aura_trait) {
    if(strncmp(aura_trait, "Stormtouched", 16) == 0) return SIGNAL_SUBGHZ;
    if(strncmp(aura_trait, "Clearseeing",  16) == 0) return SIGNAL_IR;
    if(strncmp(aura_trait, "Touchmarked",  16) == 0) return SIGNAL_NFC;
    if(strncmp(aura_trait, "Heard",        16) == 0) return SIGNAL_BLUETOOTH;
    if(strncmp(aura_trait, "Grounded",     16) == 0) return SIGNAL_RF;
    return SIGNAL_UNKNOWN;
}

// -------------------- GROUND TRUTH --------------------

// Counts recent signal_history entries to find the dominant signal type.
// Falls back to random if no history exists.
static SignalType determine_ground_truth(const Codex* codex) {
    int counts[SIGNAL_UNKNOWN] = {0};

    for(int i = 0; i < DUEL_HISTORY_SCAN && i < MAX_SIGNALS; i++) {
        const SignalEntry* entry = &codex->signal_history[i];
        if(entry->hash[0] == '\0') continue;
        SignalType t = entry->signal_type;
        if(t < SIGNAL_UNKNOWN) counts[t]++;
    }

    SignalType dominant = SIGNAL_UNKNOWN;
    int max_count = 0;
    for(int t = 0; t < (int)SIGNAL_UNKNOWN; t++) {
        if(counts[t] > max_count) {
            max_count = counts[t];
            dominant = (SignalType)t;
        }
    }

    if(dominant == SIGNAL_UNKNOWN) {
        // No scan history — random ground truth
        dominant = (SignalType)(furi_get_tick() % SIGNAL_UNKNOWN);
    }

    return dominant;
}

// -------------------- OPTION BUILDING --------------------

// Fills duel->options[3] with ground_truth + 2 other signal types, shuffled.
// Sets aura_edge[i] if option matches player's native signal.
static void build_options(DuelState* duel, const Codex* codex) {
    SignalType native = aura_native_signal(codex->aura_trait);

    // Collect all signal types except ground_truth
    SignalType pool[SIGNAL_UNKNOWN - 1];
    int pool_size = 0;
    for(int t = 0; t < (int)SIGNAL_UNKNOWN; t++) {
        if((SignalType)t != duel->ground_truth) {
            pool[pool_size++] = (SignalType)t;
        }
    }

    // Pick 2 from pool using tick-seeded pseudo-random
    uint32_t seed = furi_get_tick();
    int pick1 = (int)(seed % (uint32_t)pool_size);
    int pick2 = (int)((seed / 7 + 3) % (uint32_t)pool_size);
    if(pick2 == pick1) pick2 = (pick2 + 1) % pool_size;

    SignalType raw[DUEL_OPTIONS] = {
        duel->ground_truth,
        pool[pick1],
        pool[pick2],
    };

    // Fisher-Yates shuffle (3 elements, tick-seeded)
    for(int i = DUEL_OPTIONS - 1; i > 0; i--) {
        int j = (int)((seed >> (i * 3)) % (uint32_t)(i + 1));
        SignalType tmp = raw[i];
        raw[i] = raw[j];
        raw[j] = tmp;
    }

    for(int i = 0; i < DUEL_OPTIONS; i++) {
        duel->options[i]    = raw[i];
        duel->aura_edge[i]  = (raw[i] == native && native != SIGNAL_UNKNOWN);
    }
}

// -------------------- PUBLIC API --------------------

void duel_init(DuelState* duel, const Codex* codex) {
    duel->ground_truth    = determine_ground_truth(codex);
    duel->selected_index  = 0;
    duel->start_tick      = furi_get_tick();
    duel->phase           = DUEL_PHASE_CHOOSING;
    duel->result          = DUEL_RESULT_LOSS; // safe default

    build_options(duel, codex);
}

int duel_seconds_remaining(const DuelState* duel) {
    uint32_t elapsed_ms = furi_get_tick() - duel->start_tick;
    if(elapsed_ms >= (uint32_t)DUEL_TIMER_MS) return 0;
    int remaining = (int)((DUEL_TIMER_MS - elapsed_ms + 999) / 1000);
    return remaining;
}

void duel_move_selection(DuelState* duel, int delta) {
    duel->selected_index += delta;
    if(duel->selected_index < 0) duel->selected_index = DUEL_OPTIONS - 1;
    if(duel->selected_index >= DUEL_OPTIONS) duel->selected_index = 0;
}

DuelResult duel_resolve(DuelState* duel, Codex* codex) {
    if(duel->phase != DUEL_PHASE_CHOOSING) return duel->result;

    bool correct = (duel->options[duel->selected_index] == duel->ground_truth);

    if(correct) {
        duel->result = DUEL_RESULT_WIN;
        record_duel_result(codex, true);
    } else {
        duel->result = DUEL_RESULT_LOSS;
        codex->stamina -= DUEL_STAMINA_LOSS;
        if(codex->stamina < 0) codex->stamina = 0;
        record_duel_result(codex, false);
    }

    duel->phase = DUEL_PHASE_RESULT;
    return duel->result;
}

void duel_timeout(DuelState* duel, Codex* codex) {
    if(duel->phase != DUEL_PHASE_CHOOSING) return;

    duel->result = DUEL_RESULT_TIMEOUT;
    codex->stamina -= DUEL_STAMINA_LOSS;
    if(codex->stamina < 0) codex->stamina = 0;
    record_duel_result(codex, false);

    duel->phase = DUEL_PHASE_RESULT;
}

const char* duel_signal_name(SignalType type) {
    if(type >= SIGNAL_UNKNOWN) return "???";
    return signal_names[type];
}

const char* duel_descriptor(SignalType type, bool aura_edge) {
    if(type >= SIGNAL_UNKNOWN) return "unknown / signal";
    return aura_edge ? descriptors_edge[type] : descriptors_neutral[type];
}
