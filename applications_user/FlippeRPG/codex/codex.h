#pragma once
#include <stdbool.h>
#include <time.h>
#include "../core/constants.h"

#define MAX_SIGNALS     200
#define MAX_ENCOUNTERS  10
#define MAX_ECHO_LOG    20
#define MAX_SHRINES     10
#define MAX_TECHNIQUES  10

// -------------------- STRUCTS --------------------

// Stores a scanned signal's hash and gain info
typedef struct {
    char       hash[32];        // Unique signal hash
    int        gain;            // Signal score awarded for this scan
    time_t     timestamp;       // When it was scanned
    SignalType signal_type;     // Which hardware source produced this signal
} SignalEntry;

// Stores a multiplayer encounter with another Flipper
typedef struct {
    char   signalborn_id[16]; // Unique ID of the other player
    char   aura[16];          // Their aura (signal flavor)
    time_t timestamp;         // When the encounter occurred
    bool   echo_transferred;  // Whether an Echo was shared
} EncounterEntry;

// Stores a memory Echo fragment
typedef struct {
    char echo_id[16];       // Unique Echo identifier
    char description[64];   // Narrative or flavor text
    bool fused;             // Whether it has been fused
    bool corrupted;         // Whether it is unstable or damaged
} EchoEntry;

// Tracks shrine progress
typedef struct {
    int cooldown_seconds;      // Cooldown between activations
    char shrine_id[16];        // Unique shrine name
    bool completed;            // Whether the shrine ritual was completed
    bool resettable;           // Whether corruption unlocked replay
    bool resonance_triggered;
    time_t last_completed_time;
    time_t last_visited;       // For cooldowns or resets
} ShrineProgress;

// Tracks technique unlocks, usage, and mastery
typedef struct {
    char name[16];          // Technique name (e.g. "Pulse Open")
    bool unlocked;          // Whether the player has unlocked it
    int  uses;              // How many times it's been used
    bool mastered;          // Whether it's been mastered (e.g. 10+ uses)
} TechniqueProgress;

// The full Codex — player state, progress, and logs
typedef struct {
    char player_name[16];   // Player-entered name
    char codex_id[16];      // Unique Codex ID (e.g. CDX1234)
    char aura_trait[16];    // e.g. "Flamebound", "Whispered", "Echo-Touched"

    int signal_score;            // Internal accumulation — never displayed to player
    int signal_strength_level;  // Current tier (0=STATIC … 9=RESONANT), displayed as label
    int duels_won;               // Number of duels won
    int duels_lost;              // Number of duels lost
    int stamina;                 // Presence — drains on duel loss, recovers over time (max 100)
    
    AppearanceType appearance;  // Player sprite choice (male/female/variant)

    EchoEntry       echo_log[MAX_ECHO_LOG];          // Memory Echoes collected
    SignalEntry     signal_history[MAX_SIGNALS];    // Rolling signal log
    EncounterEntry  encounter_log[MAX_ENCOUNTERS];  // Multiplayer encounters
    ShrineProgress  shrine_progress[MAX_SHRINES];   // Shrine state
    TechniqueProgress techniques[MAX_TECHNIQUES];   // Techniques and mastery

    bool   storm_active;
    time_t storm_start_time;
    bool   converged;       // Whether the Codex has undergone convergence
    time_t save_timestamp;
    bool   legacy_mode;     // Whether the Codex has entered legacy state
    char   legacy_title[16]; // Optional: “Signalborn”, “Stormtouched”, etc.

    // Zero Day — set automatically when all five signal bands are complete.
    // Immutable once set. Gates campfire, aura reveal, and PWA QR unlock.
    // Aura is determined by scan behavior and confirmed via PWA, not assigned here.
    bool   zero_day_confirmed;
    time_t zero_day_date;
    char   faction[16];     // Set via PWA after Zero Day: “Operators”, “Nodes”, “Sovereigns”
} Codex;

// -------------------- FUNCTION DECLARATIONS --------------------

// Initialization
void codex_init(Codex* codex);
void init_codex(Codex* codex, const char* player_name);

// Signal logging
void log_signal(Codex* codex, const char* signal_hash, int gain, SignalType signal_type);

// Duel results
void record_duel_result(Codex* codex, bool won);

// Signal strength
void apply_signal_gain(Codex* codex, int amount);

// Encounters
void log_encounter(Codex* codex, const char* signalborn_id,
                   const char* aura, bool echo_transferred);

// Techniques
void codex_unlock_technique(Codex* codex, const char* name);
bool codex_has_technique(Codex* codex, const char* name);
void codex_use_technique(Codex* codex, const char* name);

// Shrines
void attempt_shrine(Codex* codex, ShrineID shrine_id, bool ritual_success);
void reset_shrine(Codex* codex, ShrineID shrine_id);
void complete_shrine(Codex* codex, ShrineID shrine_id);

// Echo mechanics
void process_echo(Codex* codex, bool fusion_success, bool corruption_detected);
void mark_echo_corrupted(Codex* codex, const char* echo_id);
bool ready_for_convergence(Codex* codex);
void assign_aura(Codex* codex, ShrineID shrine_id);