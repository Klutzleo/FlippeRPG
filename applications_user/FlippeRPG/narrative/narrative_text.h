#ifndef NARRATIVE_TEXT_H
#define NARRATIVE_TEXT_H

// =========================
// Signal Scan Narratives
// =========================
static const char* SIGNAL_SCAN_TEXT[] = {
    ">>> Listening...",
    "...sig...nal...detec...ted...",
    "Codex...respon...ds...",
    "The air hums. A pulse flickers unseen.",
    "You are not alone."
};

// =========================
// Shrine Narratives
// =========================
static const char* SHRINE_ACTIVE_TEXT[] = {
    ">>> Shrine awakens...",
    "Stone hums with unseen voices.",
    "Your will shapes the signal."
};

static const char* SHRINE_DORMANT_TEXT[] = {
    ">>> Shrine silent.",
    "The pulse fades. Return when the shrine calls again."
};

static const char* SHRINE_LEGACY_RESET_TEXT[] = {
    ">>> Shrine resets...",
    "Echoes scatter. The ritual begins anew."
};

// =========================
// Echo Narratives
// =========================
static const char* ECHO_FUSION_TEXT[] = {
    ">>> Echoes converge...",
    "Memories braid into one.",
    "A lineage is born."
};

static const char* ECHO_CORRUPTION_TEXT[] = {
    ">>> Echo unstable...",
    "Whispers fracture.",
    "The Codex trembles."
};

// =========================
// Duel Narratives
// =========================
static const char* DUEL_START_TEXT[] = {
    ">>> Signal clash detected.",
    "Two auras meet at the fire.",
    "The Codex watches."
};

static const char* DUEL_VICTORY_TEXT[] = {
    ">>> Duel complete.",
    "Your signal burns brighter."
};

static const char* DUEL_DEFEAT_TEXT[] = {
    ">>> Duel complete.",
    "Even loss leaves an echo."
};

// =========================
// Save / Load Narratives
// =========================
static const char* SAVE_TEXT[] = {
    ">>> Codex etched.",
    "Your name carried in signal."
};

static const char* LOAD_TEXT[] = {
    ">>> Codex awakens.",
    "Memory echoes restored."
};

// =========================
// Convergence / Legacy Narratives
// =========================
static const char* CONVERGENCE_TEXT[] = {
    ">>> The Codex hums...",
    "All signals align.",
    "You are Signalborn."
};

#endif // NARRATIVE_TEXT_H