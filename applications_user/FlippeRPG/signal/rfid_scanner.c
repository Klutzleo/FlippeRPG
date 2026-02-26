#include "rfid_scanner.h"
#include <lib/lfrfid/lfrfid_worker.h>
#include <lib/lfrfid/protocols/lfrfid_protocols.h>
#include <lib/toolbox/protocols/protocol_dict.h>
#include <furi.h>
#include <stdio.h>
#include <string.h>

static LFRFIDWorker* rfid_worker   = NULL;
static ProtocolDict* rfid_dict     = NULL;
static ViewDispatcher* rfid_vd     = NULL;
static char rfid_last_hash[32]     = {0};
static bool rfid_active            = false;

// Fires in the LFRFIDWorker thread — keep it minimal.
// Formats the UID, stores the hash, then wakes the main thread via custom event.
static void rfid_worker_callback(
    LFRFIDWorkerReadResult result,
    ProtocolId protocol,
    void* context) {
    (void)context;

    if(result != LFRFIDWorkerReadDone) return;

    // Get raw UID bytes
    size_t data_size = protocol_dict_get_data_size(rfid_dict, (size_t)protocol);
    if(data_size > 16) data_size = 16;

    uint8_t uid[16] = {0};
    protocol_dict_get_data(rfid_dict, (size_t)protocol, uid, data_size);

    // Build hex UID string
    char uid_hex[33] = {0};
    for(size_t i = 0; i < data_size; i++) {
        snprintf(uid_hex + i * 2, 3, "%02X", uid[i]);
    }

    // Protocol name (e.g. "EM4100", "HID Generic")
    const char* proto_name = protocol_dict_get_name(rfid_dict, (size_t)protocol);
    snprintf(
        rfid_last_hash,
        sizeof(rfid_last_hash),
        "%.10s:%.16s",
        proto_name ? proto_name : "RFID",
        uid_hex);

    // Wake the main thread
    view_dispatcher_send_custom_event(rfid_vd, RFID_SCAN_DONE_EVENT);
}

void rfid_scanner_start(ViewDispatcher* dispatcher) {
    if(rfid_active) return;

    rfid_vd            = dispatcher;
    rfid_last_hash[0]  = '\0';

    rfid_dict   = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    rfid_worker = lfrfid_worker_alloc(rfid_dict);
    lfrfid_worker_start_thread(rfid_worker);
    lfrfid_worker_read_start(
        rfid_worker,
        LFRFIDWorkerReadTypeAuto,
        rfid_worker_callback,
        NULL);

    rfid_active = true;
}

void rfid_scanner_stop(void) {
    if(!rfid_active) return;
    rfid_active = false;

    if(rfid_worker) {
        lfrfid_worker_stop(rfid_worker);
        lfrfid_worker_stop_thread(rfid_worker);
        lfrfid_worker_free(rfid_worker);
        rfid_worker = NULL;
    }
    if(rfid_dict) {
        protocol_dict_free(rfid_dict);
        rfid_dict = NULL;
    }
    rfid_vd = NULL;
}

bool rfid_scanner_is_active(void) {
    return rfid_active;
}

void rfid_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, rfid_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
