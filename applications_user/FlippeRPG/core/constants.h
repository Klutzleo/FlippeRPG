#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

// Signal types
typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_GPIO
} SignalType;

// Shrine IDs
typedef enum {
    SHRINE_CAVE_THAT_LISTENS,
    SHRINE_FLAME_REACH,
    SHRINE_BIND_WHISPER,
    SHRINE_THREAD_TOUCH,
    SHRINE_ECHO_TOUCHED,
    SHRINE_UNKNOWN,
    NUM_SHRINES
} ShrineID;

// XP sources
typedef enum {
    XP_SOURCE_SIGNAL,
    XP_SOURCE_DUEL,
    XP_SOURCE_SHRINE
} XPSource;

#endif