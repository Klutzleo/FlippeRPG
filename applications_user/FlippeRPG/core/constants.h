// constants.h
#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_GPIO,
    SIGNAL_FUSION
} SignalType;

typedef enum {
    SHRINE_CAVE_THAT_LISTENS,
    SHRINE_FLAME_REACH,
    SHRINE_BIND_WHISPER,
    SHRINE_THREAD_TOUCH,
    SHRINE_ECHO_TOUCHED,
    SHRINE_UNKNOWN,
    NUM_SHRINES
} ShrineID;

typedef enum {
    XP_SOURCE_SIGNAL,
    XP_SOURCE_DUEL,
    XP_SOURCE_SHRINE,
    XP_SOURCE_FUSION   // optional, if you want fusion XP tracked separately
} XPSource;

// Aura traits unlocked by shrines or lineage
typedef enum {
    AURA_NONE,
    AURA_FLAMEBOUND,
    AURA_WHISPERED,
    AURA_ECHOFORGED,
    AURA_STORMTOUCHED,
    MAX_AURA_TRAITS
} AuraTrait;

typedef enum {
    ECHO_FUSION = 0,
    ECHO_CORRUPTION = 1,
    ECHO_LINEAGE = 2,
    ECHO_LEGACY   = 3
    // add more states as needed
} EchoState;

static const char* ECHO_FUSION_TEXT[] = {
    "The echoes merge into harmony.",
    "Fusion resonates through the Codex.",
    "Two signals become one."
};

static const char* ECHO_CORRUPTION_TEXT[] = {
    "The echo twists into corruption.",
    "Dark resonance destabilizes the Codex.",
    "A fragment falls into shadow."
};

static const char* ECHO_LINEAGE_TEXT[] = {
    "Lineage awakens within the Codex.",
    "Ancestral resonance flows through signals.",
    "Echoes carry the memory of generations."
};

#endif