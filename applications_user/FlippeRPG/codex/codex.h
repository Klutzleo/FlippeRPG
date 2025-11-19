#define MAX_TECHNIQUES 10

typedef struct {
    char name[16];
    bool unlocked;
    int uses;
    bool mastered;
} TechniqueProgress;

typedef struct {
    char codex_id[16];
    int xp_total;
    TechniqueProgress techniques[MAX_TECHNIQUES];
    ShrineProgress shrine_progress[MAX_SHRINES];
    // ... other fields like signal history, duel stats
} Codex;