#include "save_system.h"
#include <string.h>
#include <stdbool.h>

// In-app stub storage: keeps last saved codex in RAM to avoid forbidden stdio APIs
static Codex g_cached_codex;
static bool g_has_cache = false;

void save_codex(Codex* codex, const char* filename) {
    (void)filename;
    if(!codex) return;
    memcpy(&g_cached_codex, codex, sizeof(Codex));
    g_has_cache = true;
}

void load_codex(Codex* codex, const char* filename) {
    (void)filename;
    if(!codex || !g_has_cache) return;
    memcpy(codex, &g_cached_codex, sizeof(Codex));
}