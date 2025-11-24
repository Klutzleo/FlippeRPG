#include "signal_tags.h"
#include "../shrine/shrine.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "../codex/codex.h"

#define MAX_TAGS 32

// Shrine tag metadata structure
typedef struct {
    char tag_id[32];            // Unique tag identifier
    ShrineID shrine_id;         // Associated shrine
    char aura_hint[16];         // Optional: aura clue (e.g. "Flamebound")
    time_t cooldown_hint;       // Optional: next activation time
    char echo_hint[32];         // Optional: Echo ID fragment or lore
} ShrineTagMapEntry;

static ShrineTagMapEntry shrine_tag_map[MAX_TAGS];
static int shrine_tag_count = 0;

// Resolves a tag ID to its associated shrine
ShrineID resolve_tag_to_shrine(const char* tag_id) {
    for (int i = 0; i < shrine_tag_count; i++) {
        if (strcmp(shrine_tag_map[i].tag_id, tag_id) == 0) {
            return shrine_tag_map[i].shrine_id;
        }
    }
    return SHRINE_UNKNOWN;
}

// Imprints a shrine and optional metadata onto a tag
void imprint_shrine_tag(ShrineID shrine_id,
                        const char* tag_id,
                        const char* aura_hint,
                        time_t cooldown_hint,
                        const char* echo_hint){
    if (shrine_tag_count >= MAX_TAGS) {
        popup_message("Tag map full. Cannot imprint more shrines.");
        return;
    }

    // Overwrite if tag already exists
    for (int i = 0; i < shrine_tag_count; i++) {
        if (strcmp(shrine_tag_map[i].tag_id, tag_id) == 0) {
            shrine_tag_map[i].shrine_id = shrine_id;
            strncpy(shrine_tag_map[i].aura_hint, aura_hint ? aura_hint : "", sizeof(shrine_tag_map[i].aura_hint));
            shrine_tag_map[i].cooldown_hint = cooldown_hint;
            strncpy(shrine_tag_map[i].echo_hint, echo_hint ? echo_hint : "", sizeof(shrine_tag_map[i].echo_hint));
            return;
        }
    }

    // Add new entry
    ShrineTagMapEntry* entry = &shrine_tag_map[shrine_tag_count++];
    strncpy(entry->tag_id, tag_id, sizeof(entry->tag_id));
    entry->shrine_id = shrine_id;
    strncpy(entry->aura_hint, aura_hint ? aura_hint : "", sizeof(entry->aura_hint));
    entry->cooldown_hint = cooldown_hint;
    strncpy(entry->echo_hint, echo_hint ? echo_hint : "", sizeof(entry->echo_hint));
}

// Scans a tag and triggers its shrine, showing metadata feedback
void scan_shrine_tag(Codex* codex, const char* tag_id) {
    for (int i = 0; i < shrine_tag_count; i++) {
        if (strcmp(shrine_tag_map[i].tag_id, tag_id) == 0) {
            ShrineTagMapEntry* entry = &shrine_tag_map[i];

            // Feedback
            printf("[Tag] Shrine: %d\n", entry->shrine_id);
            printf("[Tag] Aura Hint: %s\n", entry->aura_hint);

            char* time_str = ctime(&entry->cooldown_hint);
            if (time_str) {
                time_str[strcspn(time_str, "\n")] = '\0'; // Strip newline
                printf("[Tag] Cooldown Hint: %s\n", time_str);
            }

            printf("[Tag] Echo Hint: %s\n", entry->echo_hint);

            trigger_shrine(codex, entry->shrine_id, SIGNAL_NFC);
            return;
        }
    }

    popup_message("This tag holds no memory.");
}