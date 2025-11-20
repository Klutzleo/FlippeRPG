#pragma once
#include "../core/constants.h"

typedef struct {
    ShrineID id;
    const char* name;
    SignalType required_signal;
    const char* flavor_text;
    int cooldown_seconds;
} ShrineDefinition;

#define SHRINE_COUNT 4

static const ShrineDefinition shrine_definitions[SHRINE_COUNT] = {
    {
        .id = SHRINE_CAVE_THAT_LISTENS,
        .name = "The Cave That Listens",
        .required_signal = SIGNAL_SUBGHZ,
        .flavor_text = "A whisper echoes in the dark. The gate awaits your call.",
        .cooldown_seconds = 0
    },
    {
        .id = SHRINE_FLAME_REACH,
        .name = "Flame Reach",
        .required_signal = SIGNAL_IR,
        .flavor_text = "A brazier flickers. Flame answers flame.",
        .cooldown_seconds = 0
    },
    {
        .id = SHRINE_BIND_WHISPER,
        .name = "Bind Whisper",
        .required_signal = SIGNAL_NFC,
        .flavor_text = "A memory stirs. The tag remembers.",
        .cooldown_seconds = 0
    },
    {
        .id = SHRINE_THREAD_TOUCH,
        .name = "Thread Touch",
        .required_signal = SIGNAL_GPIO,
        .flavor_text = "A current hums. The thread awaits your touch.",
        .cooldown_seconds = 0
    }
};