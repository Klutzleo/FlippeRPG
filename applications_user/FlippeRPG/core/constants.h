// constants.h
#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_GPIO
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

// Echo event types (fusion, corruption, lineage convergence)
typedef enum {
    ECHO_FUSION,
    ECHO_CORRUPTION,
    ECHO_LINEAGE
} EchoEventType;

#endif