#include "save_system.h"
#include <storage/storage.h>
#include <furi.h>
#include <string.h>
#include <stddef.h>

#define TAG          "FlippeRPG"
#define SAVE_PATH    APP_DATA_PATH("codex.bin")
#define SAVE_MAGIC   0x464C5250  // "FLRP"
// v2: Codex grew (band_complete_day, last_active_day, stamina_updated).
// Fields are only ever appended, so an older save is just a shorter prefix of the current struct.
#define SAVE_VERSION 2

// Size a v1 save has: the struct as it stood before the v2 fields, padded to its alignment.
static size_t codex_v1_size(void) {
    size_t end = offsetof(Codex, faction) + sizeof(((Codex*)NULL)->faction);
    size_t align = _Alignof(Codex);
    return (end + align - 1) / align * align;
}

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

        if(header.version < 1 || header.version > SAVE_VERSION) {
            FURI_LOG_W(TAG, "Load: unsupported version (%lu vs %d)", header.version, SAVE_VERSION);
            break;
        }

        // A v1 save is a known shorter size: load it and leave the v2 fields zeroed.
        // Any other size means a different struct layout and can't be read safely.
        size_t expected = (header.version == 1) ? codex_v1_size() : sizeof(Codex);
        if(header.data_size != expected) {
            FURI_LOG_W(TAG, "Load: size mismatch (%lu vs %u)", header.data_size, sizeof(Codex));
            break;
        }

        memset(codex, 0, sizeof(Codex));
        if(storage_file_read(file, codex, header.data_size) != header.data_size) {
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
