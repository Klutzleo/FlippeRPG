#include "signal_tags.h"
#include "../shrine/shrine.h"
#include <string.h>
#include <stdio.h>

// Simple mapping stub — replace with real lookup later
ShrineID resolve_tag_to_shrine(const char* tag_id) {
    for (int i = 0; i < shrine_tag_count; i++) {
        if (strcmp(shrine_tag_map[i].tag_id, tag_id) == 0) {
            return shrine_tag_map[i].shrine_id;
        }
    }
    return SHRINE_UNKNOWN;
}

void imprint_shrine_tag(ShrineID shrine_id, const char* tag_id) {
    if (shrine_tag_count >= MAX_TAGS) {
        popup_message("Tag map full. Cannot imprint more shrines.");
        return;
    }

    // Check if tag already exists — overwrite if found
    for (int i = 0; i < shrine_tag_count; i++) {
        if (strcmp(shrine_tag_map[i].tag_id, tag_id) == 0) {
            shrine_tag_map[i].shrine_id = shrine_id;
            printf("[Tag] Shrine %d re-imprinted to tag %s\n", shrine_id, tag_id);
            return;
        }
    }

    // Add new entry
    strncpy(shrine_tag_map[shrine_tag_count].tag_id, tag_id, sizeof(shrine_tag_map[shrine_tag_count].tag_id) - 1);
    shrine_tag_map[shrine_tag_count].shrine_id = shrine_id;
    shrine_tag_count++;

    printf("[Tag] Shrine %d imprinted to tag %s\n", shrine_id, tag_id);
}

void scan_shrine_tag(Codex* codex, const char* tag_id) {
    ShrineID shrine_id = resolve_tag_to_shrine(tag_id);
    if (shrine_id == SHRINE_UNKNOWN) {
        popup_message("This tag holds no memory.");
        return;
    }

    trigger_shrine(codex, shrine_id, SIGNAL_NFC);
}

#define MAX_TAGS 32

typedef struct {
    char tag_id[32];
    ShrineID shrine_id;
} ShrineTagMapEntry;

static ShrineTagMapEntry shrine_tag_map[MAX_TAGS];
static int shrine_tag_count = 0;