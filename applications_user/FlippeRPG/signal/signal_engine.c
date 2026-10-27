#include "signal_engine.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <furi.h>
#include <furi_hal_rtc.h>
#include <applications/services/dolphin/dolphin.h>
#include "../shrine/shrine.h"
#include "../codex/codex.h"
#include "../core/utils.h"

// Generates a simple hash from raw signal data.
// NOTE: This is a stub — always returns a random hash.
// Replace with a deterministic hash of actual signal data (frequency, ID bytes, etc.)
// so that the same physical source produces the same hash across scans.
char* hash_signal(const char* raw_data) {
    (void)raw_data;
    static char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", rand() % 65536);
    return hash;
}

// Bluetooth only measures energy on the band — there is no device ID to tell sources apart.
static bool is_sourceless(SignalType type) {
    return type == SIGNAL_BLUETOOTH;
}

ScanResult evaluate_scan(Codex* codex, SignalType type, const char* hash) {
    ScanResult result = {false, 0};

    uint32_t now = furi_hal_rtc_get_timestamp();
    uint32_t today = now / SECONDS_PER_DAY;

    // Only counted scans are logged, so history is today's counted scans of this type
    int counted_today = 0;
    bool source_seen = false;
    uint32_t newest_age = UINT32_MAX;
    for(int i = 0; i < MAX_SIGNALS; i++) {
        const SignalEntry* entry = &codex->signal_history[i];
        if(entry->hash[0] == '\0' || entry->signal_type != type) continue;
        uint32_t then = (uint32_t)entry->timestamp;
        if(then / SECONDS_PER_DAY != today) continue;
        counted_today++;
        if(strcmp(entry->hash, hash) == 0) source_seen = true;
        uint32_t age = (now > then) ? now - then : 0;
        if(age < newest_age) newest_age = age;
    }

    if(is_sourceless(type)) {
        if(counted_today >= SCANS_PER_BAND) return result;
        if(counted_today > 0 && newest_age < SOURCELESS_MIN_GAP_SECONDS) return result;
    } else if(source_seen) {
        return result;
    }

    result.counted = true;
    if(counted_today == 0) result.gain = SCAN_GAIN_FIRST;
    else if(counted_today == 1) result.gain = SCAN_GAIN_SECOND;
    return result;
}

// Evaluates a scan and logs it if it counted. Shared by every on_*_scan handler below.
static ScanResult record_scan(Codex* codex, SignalType type, const char* hash) {
    ScanResult result = evaluate_scan(codex, type, hash);
    if(result.counted) log_signal(codex, hash, result.gain, type);
    return result;
}

// Handles RFID scans.
// tag_id is used directly as the hash (125kHz RFID UIDs are persistent and unique).
ScanResult on_rfid_scan(Codex* codex, const char* tag_id) {
    ScanResult result = record_scan(codex, SIGNAL_RFID, tag_id);
    dolphin_deed(DolphinDeedRfidRead);
    return result;
}

// Handles IR scans — hash is "PROTO:ADDR:CMD" or "RAW:count:checksum" for unknown protocols.
ScanResult on_ir_scan(Codex* codex, const char* signal_hash) {
    ScanResult result = record_scan(codex, SIGNAL_IR, signal_hash);
    dolphin_deed(DolphinDeedIrLearnSuccess);
    return result;
}

// Handles Sub-GHz scans — hash is "PROTO:HASH_BYTE" from the decoded protocol frame.
ScanResult on_subghz_scan(Codex* codex, const char* signal_hash) {
    ScanResult result = record_scan(codex, SIGNAL_SUBGHZ, signal_hash);
    dolphin_deed(DolphinDeedSubGhzReceiverInfo);
    return result;
}

// Handles NFC scans — tag_id is the UID (persistent and unique per card).
ScanResult on_nfc_scan(Codex* codex, const char* tag_id) {
    ScanResult result = record_scan(codex, SIGNAL_NFC, tag_id);
    dolphin_deed(DolphinDeedNfcRead);
    return result;
}

// Handles BT scans — hash is RSSI-derived + tick (ephemeral; passive listening has no UID).
// The Unanswered Hello: something was broadcasting. We heard it.
ScanResult on_bt_scan(Codex* codex, const char* signal_hash) {
    ScanResult result = record_scan(codex, SIGNAL_BLUETOOTH, signal_hash);
    dolphin_deed(DolphinDeedPluginGameWin);
    return result;
}

SignalType get_signal_type(Codex* codex, const char* signal_hash) {
    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(strcmp(codex->signal_history[i].hash, signal_hash) == 0) {
            return codex->signal_history[i].signal_type;
        }
    }
    return SIGNAL_UNKNOWN;
}

// Scans a specific band type — called by the band-select signal view.
// Generates a simulated hash, logs the signal if it counted, and returns the gain awarded.
int scan_band(Codex* codex, SignalType band_type) {
    char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", (unsigned)(rand() % 65536));
    ScanResult result = record_scan(codex, band_type, hash);
    return result.gain;
}

int enter_manual_signal(Codex* codex, const char* signal_hash) {
    // Manual entries: same gate, but capped lower
    // Auto: 3/1/0 → Manual: 1/0/0
    ScanResult result = record_scan(codex, SIGNAL_UNKNOWN, signal_hash);
    int gain = (result.gain >= SCAN_GAIN_FIRST) ? 1 : 0;
    printf("[Manual] Entered signal: %s → gain: %d\n", signal_hash, gain);
    return gain;
}
