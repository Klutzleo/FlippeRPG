// constants.h
#ifndef FLIPPERPG_CONSTANTS_H
#define FLIPPERPG_CONSTANTS_H

#include <stdbool.h>

typedef enum {
    SIGNAL_SUBGHZ,
    SIGNAL_IR,
    SIGNAL_NFC,
    SIGNAL_BLUETOOTH,
    SIGNAL_RF, // Retired: merged into SIGNAL_SUBGHZ. Kept so saved indexes and the QR payload stay stable.
    SIGNAL_RFID,
    SIGNAL_UNKNOWN
} SignalType;

// Internal IDs use original code names — lore canonical names differ.
// SHRINE_POCKET_CARRIED    = "The Unread"           (RFID)
// SHRINE_ECHO_TOUCHED      = "The First Frequency"  (RF) — RETIRED, see shrine_is_active()
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

// The First Frequency went with RF when RF merged into Sub-GHz. Its enum value stays so saved
// shrine_progress[] indexes don't shift, but it is never shown, attempted, or required.
static inline bool shrine_is_active(ShrineID id) {
    return id < SHRINE_UNKNOWN && id != SHRINE_ECHO_TOUCHED;
}

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

// Band gate progression — Tier 1 (Tutorial → Zero Day):
// IR → Sub-GHz → NFC → Bluetooth. 5 scans each, sequential, hidden until unlocked.
// THE SUBSTRATE fires (CQ Morse) after all 4 Tier 1 bands complete.
// RFID is Tier 2 (post-Zero Day). RF was merged into Sub-GHz — same radio, same band. GPIO and iButton are Tier 3 (post-DEEP).
#define SCANS_PER_BAND 5
#define NUM_BANDS 4

// Pacing. A "day" is the RTC date; counters reset at midnight.
#define SECONDS_PER_DAY 86400u
// One band may complete per day, so Zero Day is at least NUM_BANDS days out.
// Set to 0 while testing to let every band open immediately.
#define BAND_DAY_GATE 1
// Bluetooth has no source ID to dedupe on, so its scans count per type:
// at most SCANS_PER_BAND a day, and at least this long apart (move somewhere new).
#define SOURCELESS_MIN_GAP_SECONDS 600u

// Signal gains. Scans: +3 first counted scan of a type today, +1 second, +0 after.
#define SCAN_GAIN_FIRST         3
#define SCAN_GAIN_SECOND        1
#define TRANSMISSION_ACK_BONUS  8
#define BAND_COMPLETE_BONUS     30
// Sized so the fastest path (one band a day, 296 signal) arrives at exactly PRESENT (351):
// "recognized at the fire" on the day the Substrate opens. Slower players land higher.
#define ZERO_DAY_ARRIVAL_BONUS  55

// Absence costs 1 signal per fully inactive day, capped so a long break isn't ruinous.
#define ABSENCE_DECAY_MAX_DAYS  90
// Stamina is presence, separate from signal strength. It recovers by itself:
// one point per this many seconds (full in ~8 hours).
#define STAMINA_MAX 100
#define STAMINA_RECOVERY_SECONDS_PER_POINT 300u

static const SignalType band_order[NUM_BANDS] = {
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