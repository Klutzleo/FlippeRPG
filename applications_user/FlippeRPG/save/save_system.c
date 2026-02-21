#include "save_system.h"
#include <storage/storage.h>
#include <furi.h>
#include <string.h>

#define TAG          "FlippeRPG"
#define SAVE_PATH    APP_DATA_PATH("codex.bin")
#define SAVE_MAGIC   0x464C5250  // "FLRP"
#define SAVE_VERSION 1

// Header written before the Codex struct to detect corrupt/stale files
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t data_size;
} SaveHeader;

void save_codex(Codex* codex, const char* filename) {
    (void)filename; // path is fixed via APP_DATA_PATH

    if(!codex) return;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, SAVE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Save: failed to open file");
            break;
        }

        SaveHeader header = {
            .magic     = SAVE_MAGIC,
            .version   = SAVE_VERSION,
            .data_size = sizeof(Codex),
        };

        if(storage_file_write(file, &header, sizeof(header)) != sizeof(header)) {
            FURI_LOG_E(TAG, "Save: failed to write header");
            break;
        }

        if(storage_file_write(file, codex, sizeof(Codex)) != sizeof(Codex)) {
            FURI_LOG_E(TAG, "Save: failed to write codex");
            break;
        }

        FURI_LOG_I(TAG, "Save: codex saved (%u bytes)", sizeof(Codex));

    } while(false);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool load_codex(Codex* codex, const char* filename) {
    (void)filename;

    if(!codex) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool success = false;

    do {
        if(!storage_file_open(file, SAVE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_I(TAG, "Load: no save file found, starting fresh");
            break;
        }

        SaveHeader header = {0};
        if(storage_file_read(file, &header, sizeof(header)) != sizeof(header)) {
            FURI_LOG_E(TAG, "Load: failed to read header");
            break;
        }

        if(header.magic != SAVE_MAGIC) {
            FURI_LOG_E(TAG, "Load: bad magic (0x%08lX)", header.magic);
            break;
        }

        if(header.version != SAVE_VERSION) {
            FURI_LOG_W(TAG, "Load: version mismatch (%lu vs %d)", header.version, SAVE_VERSION);
            break;
        }

        if(header.data_size != sizeof(Codex)) {
            FURI_LOG_W(TAG, "Load: size mismatch (%lu vs %u)", header.data_size, sizeof(Codex));
            break;
        }

        if(storage_file_read(file, codex, sizeof(Codex)) != sizeof(Codex)) {
            FURI_LOG_E(TAG, "Load: failed to read codex");
            break;
        }

        FURI_LOG_I(TAG, "Load: codex loaded for '%s'", codex->player_name);
        success = true;

    } while(false);

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return success;
}
