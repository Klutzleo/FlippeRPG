// constants.h
#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_BLUETOOTH,
    SIGNAL_RF,
    SIGNAL_UNKNOWN
} SignalType;

// Internal IDs use original code names — lore canonical names differ.
// SHRINE_CAVE_THAT_LISTENS = "The Open Channel"    (Sub-GHz)
// SHRINE_FLAME_REACH       = "The Fixed Gaze"      (IR)
// SHRINE_BIND_WHISPER      = "The Exchange"         (NFC)
// SHRINE_THREAD_TOUCH      = "The Unanswered Hello" (Bluetooth)
// SHRINE_ECHO_TOUCHED      = "The First Frequency"  (RF)
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
    AURA_CLEARSEEING,
    AURA_TOUCHMARKED,
    AURA_GROUNDED,
    AURA_STORMTOUCHED,
    AURA_HEARD,
    MAX_AURA_TRAITS
} AuraTrait;

typedef enum {
    ECHO_FUSION = 0,
    ECHO_CORRUPTION = 1,
    ECHO_LINEAGE = 2,
    ECHO_LEGACY   = 3
    // add more states as needed
} EchoState;

// Player appearance/sprite variants
typedef enum {
    APPEARANCE_MAGE_MALE,
    APPEARANCE_MAGE_FEMALE,
    APPEARANCE_WIZARD_MALE,
    APPEARANCE_WIZARD_FEMALE,
    NUM_APPEARANCES
} AppearanceType;

#endif