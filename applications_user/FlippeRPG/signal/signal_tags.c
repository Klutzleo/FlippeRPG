#include "signal_tags.h"
#include "../shrine/shrine.h"
#include <string.h>
#include <stdio.h>

// Simple mapping stub — replace with real lookup later
ShrineID resolve_tag_to_shrine(const char* tag_id) {
    if (strcmp(tag_id, "TAG-FLAME") == 0) return SHRINE_FLAME_REACH;
    if (strcmp(tag_id, "TAG-BIND") == 0) return SHRINE_BIND_WHISPER;
    return SHRINE_UNKNOWN;
}

void imprint_shrine_tag(ShrineID shrine_id, const char* tag_id) {
    printf("[Tag] Shrine %d imprinted to tag %s\n", shrine_id, tag_id);
    // TODO: Write shrine metadata to NFC tag or SubGHz beacon
}

void scan_shrine_tag(Codex* codex, const char* tag_id) {
    ShrineID shrine_id = resolve_tag_to_shrine(tag_id);
    if (shrine_id == SHRINE_UNKNOWN) {
        popup_message("This tag holds no memory.");
        return;
    }

    trigger_shrine(codex, shrine_id, SIGNAL_NFC);
}