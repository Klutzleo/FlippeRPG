#include "save_system.h"
#include <stdio.h>

void save_codex(Codex* codex, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        printf("[Save] Failed to open file: %s\n", filename);
        return;
    }
    fwrite(codex, sizeof(Codex), 1, f);
    fclose(f);
    printf("[Save] Codex state saved to %s\n", filename);
}

void load_codex(Codex* codex, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("[Load] No save file found: %s\n", filename);
        return;
    }
    fread(codex, sizeof(Codex), 1, f);
    fclose(f);
    printf("[Load] Codex state loaded from %s\n", filename);
}