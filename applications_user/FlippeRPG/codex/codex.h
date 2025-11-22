#pragma once
#include <stdbool.h>
#include <time.h>
#include "../core/constants.h"

#define MAX_SIGNALS 200
#define MAX_ENCOUNTERS 10
#define MAX_ECHO_LOG 20
#define MAX_SHRINES 10
#define MAX_TECHNIQUES 10

// -------------------- ENUMS --------------------


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

// -------------------- STRUCTS --------------------

// Stores a scanned signal's hash and XP info
typedef struct {
    char hash[32];         // Unique signal hash
    int xp_awarded;        // XP granted for this signal
    time_t timestamp;      // When it was scanned
} SignalEntry;

// Stores a multiplayer encounter with another Flipper
typedef struct {
    char signalborn_id[16]; // Unique ID of the other player
    char aura[16];          // Their aura (signal flavor)
    time_t timestamp;       // When the encounter occurred
    bool echo_transferred;  // Whether an Echo was shared
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
    char shrine_id[16];     // Unique shrine name
    bool completed;         // Whether the shrine ritual was completed
    bool resettable;        // Whether corruption unlocked replay
    bool resonance_triggered;
    time_t last_completed_time;
    time_t last_visited;    // For cooldowns or resets
} ShrineProgress;

// Tracks technique unlocks, usage, and mastery
typedef struct {
    char name[16];          // Technique name (e.g. "Pulse Open")
    bool unlocked;          // Whether the player has unlocked it
    int uses;               // How many times it's been used
    bool mastered;          // Whether it's been mastered (e.g. 10+ uses)
} TechniqueProgress;

// The full Codex — player state, progress, and logs
typedef struct {
    char player_name[16];                       // Player-entered name
    char codex_id[16];                          // Unique Codex ID (e.g. CDX1234)
    char aura_trait[16];                        // e.g. "Flamebound", "Whispered", "Echo-Touched"

    int xp_total;                               // Total XP from signals
    int duel_xp;                                // XP from duels

    EchoEntry echo_log[MAX_ECHO_LOG];           // Memory Echoes collected
    SignalEntry signal_history[MAX_SIGNALS];    // Rolling signal log
    EncounterEntry encounter_log[MAX_ENCOUNTERS]; // Multiplayer encounters
    ShrineProgress shrine_progress[MAX_SHRINES];  // Shrine state
    TechniqueProgress techniques[MAX_TECHNIQUES]; // Techniques and mastery

    bool storm_active;
    time_t storm_start_time;
    bool converged;                             // Whether the Codex has undergone convergence
    time_t save_timestamp;
    bool legacy_mode;                           // Whether the Codex has entered legacy state
    char legacy_title[16];                      // Optional: “Signalborn”, “Stormtouched”, etc.
} Codex;

// -------------------- FUNCTION DECLARATIONS --------------------

void codex_init(Codex* codex);

// Initializes a new Codex with default values
void init_codex(Codex* codex, const char* player_name);

// Logs a scanned signal and awards XP
void log_signal(Codex* codex, const char* signal_hash, int xp);

// Adds XP from duels
void update_duel_xp(Codex* codex, int xp);

// Logs a multiplayer encounter
void log_encounter(Codex* codex, const char* signalborn_id, const char* aura, bool echo_transferred);

// Unlocks a new technique
void codex_unlock_technique(Codex* codex, const char* name);

// Checks if a technique is unlocked
bool codex_has_technique(Codex* codex, const char* name);

// Tracks usage of a technique and checks for mastery
void codex_use_technique(Codex* codex, const char* name);

// Shrine mechanics
void attempt_shrine(Codex* codex, ShrineID shrine_id, bool ritual_success);
void reset_shrine(Codex* codex, ShrineID shrine_id);
void complete_shrine(Codex* codex, ShrineID shrine_id);

// Echo mechanics
void process_echo(Codex* codex, bool fusion_success, bool corruption_detected);
void mark_echo_corrupted(Codex* codex, const char* echo_id);
bool ready_for_convergence(Codex* codex);
void assign_aura(Codex* codex, ShrineID shrine_id);