// constants.h
#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_BLUETOOTH,
    SIGNAL_RF,
    SIGNAL_RFID,
    SIGNAL_UNKNOWN
} SignalType;

// Internal IDs use original code names — lore canonical names differ.
// SHRINE_POCKET_CARRIED    = "The Unread"           (RFID)
// SHRINE_ECHO_TOUCHED      = "The First Frequency"  (RF)
// SHRINE_FLAME_REACH       = "The Fixed Gaze"       (IR)
// SHRINE_CAVE_THAT_LISTENS = "The Open Channel"     (Sub-GHz)
// SHRINE_BIND_WHISPER      = "The Exchange"          (NFC)
// SHRINE_THREAD_TOUCH      = "The Unanswered Hello"  (Bluetooth)
typedef enum {
    SHRINE_POCKET_CARRIED,
    SHRINE_ECHO_TOUCHED,
    SHRINE_FLAME_REACH,
    SHRINE_CAVE_THAT_LISTENS,
    SHRINE_BIND_WHISPER,
    SHRINE_THREAD_TOUCH,
    SHRINE_UNKNOWN,
    NUM_SHRINES
} ShrineID;

// Signal strength — the only visible measure of progression.
// Players see labels only. Math runs underneath. Never expose raw score.
typedef enum {
    SIGNAL_STRENGTH_STATIC = 0,
    SIGNAL_STRENGTH_FAINT,
    SIGNAL_STRENGTH_TRACE,
    SIGNAL_STRENGTH_RISING,
    SIGNAL_STRENGTH_PRESENT,
    SIGNAL_STRENGTH_CLEAR,
    SIGNAL_STRENGTH_STRONG,
    SIGNAL_STRENGTH_DEEP,
    SIGNAL_STRENGTH_RESONANT_EDGE,
    SIGNAL_STRENGTH_RESONANT,
    SIGNAL_STRENGTH_COUNT
} SignalStrengthLevel;

// Steepening curve — early tiers are about learning, late tiers are about commitment.
// Zero Day arrives at PRESENT by design.
static const int signal_strength_thresholds[SIGNAL_STRENGTH_COUNT] = {
    0,    // STATIC
    51,   // FAINT
    121,  // TRACE
    221,  // RISING
    351,  // PRESENT
    501,  // CLEAR
    651,  // STRONG
    821,  // DEEP
    1051, // RESONANT EDGE
    1251  // RESONANT (community-gated — score alone cannot cross this)
};

static const char* const signal_strength_labels[SIGNAL_STRENGTH_COUNT] = {
    "STATIC", "FAINT", "TRACE", "RISING", "PRESENT",
    "CLEAR", "STRONG", "DEEP", "RESONANT EDGE", "RESONANT"
};

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

// Band gate progression — sequential unlock: RFID → RF → IR → SubGHz → NFC → Bluetooth
// 5 scans of a band unlocks the next. Bands are hidden until unlocked.
// THE SUBSTRATE appears (with long vibration) after Bluetooth band completes.
#define SCANS_PER_BAND 5
#define NUM_BANDS 6
static const SignalType band_order[NUM_BANDS] = {
    SIGNAL_RFID,
    SIGNAL_RF,
    SIGNAL_IR,
    SIGNAL_SUBGHZ,
    SIGNAL_NFC,
    SIGNAL_BLUETOOTH,
};

// Player appearance/sprite variants
typedef enum {
    APPEARANCE_MAGE_MALE,
    APPEARANCE_MAGE_FEMALE,
    APPEARANCE_WIZARD_MALE,
    APPEARANCE_WIZARD_FEMALE,
    NUM_APPEARANCES
} AppearanceType;

#endif