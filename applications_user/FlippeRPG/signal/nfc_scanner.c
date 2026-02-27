#include "nfc_scanner.h"
#include <lib/nfc/nfc.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_sync.h>
#include <furi.h>
#include <stdio.h>
#include <string.h>

// 5-second window — hold the card still against the back of the Flipper
#define NFC_SCAN_TIMEOUT_MS  5000
#define NFC_POLL_INTERVAL_MS 100

static volatile bool nfc_active    = false;
static volatile bool nfc_had_signal = false;
static ViewDispatcher* nfc_vd      = NULL;
static char nfc_last_hash[32]      = {0};
static FuriThread* nfc_thread      = NULL;

// ---- Scan thread ----

static int32_t nfc_scan_thread(void* context) {
    (void)context;

    Nfc* nfc = nfc_alloc();
    Iso14443_3aData* card = iso14443_3a_alloc();

    uint32_t start = furi_get_tick();
    Iso14443_3aError err = Iso14443_3aErrorTimeout;

    // Poll until card found, user cancels, or 5-second window expires.
    // iso14443_3a_poller_sync_read returns immediately when no card is in field.
    while(nfc_active && (furi_get_tick() - start) < NFC_SCAN_TIMEOUT_MS) {
        iso14443_3a_reset(card);
        err = iso14443_3a_poller_sync_read(nfc, card);
        if(err == Iso14443_3aErrorNone) break;
        furi_delay_ms(NFC_POLL_INTERVAL_MS);
    }

    if(err == Iso14443_3aErrorNone && nfc_active) {
        // cuid is a compact 32-bit hash of the full UID — deterministic per card
        uint32_t cuid = iso14443_3a_get_cuid(card);
        snprintf(nfc_last_hash, sizeof(nfc_last_hash), "NFC:%08lX", (unsigned long)cuid);
        nfc_had_signal = true;
    }

    iso14443_3a_free(card);
    nfc_free(nfc);

    if(nfc_active && nfc_vd) {
        view_dispatcher_send_custom_event(nfc_vd, NFC_SCAN_DONE_EVENT);
    }

    return 0;
}

// ---- Public API ----

void nfc_scanner_start(ViewDispatcher* dispatcher) {
    if(nfc_active) return;

    nfc_vd           = dispatcher;
    nfc_had_signal   = false;
    nfc_last_hash[0] = '\0';
    nfc_active       = true;

    nfc_thread = furi_thread_alloc_ex("NFCScanThread", 2048, nfc_scan_thread, NULL);
    furi_thread_start(nfc_thread);
}

void nfc_scanner_stop(void) {
    if(!nfc_active) return;
    nfc_active = false;

    if(nfc_thread) {
        furi_thread_join(nfc_thread);
        furi_thread_free(nfc_thread);
        nfc_thread = NULL;
    }
    nfc_vd = NULL;
}

bool nfc_scanner_is_active(void) {
    return nfc_active;
}

bool nfc_scanner_has_signal(void) {
    return nfc_had_signal;
}

void nfc_scanner_get_hash(char* out_hash, size_t size) {
    strncpy(out_hash, nfc_last_hash, size);
    if(size > 0) out_hash[size - 1] = '\0';
}
