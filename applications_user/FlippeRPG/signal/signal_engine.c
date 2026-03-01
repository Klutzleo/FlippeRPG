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

// Returns the signal gain for a specific hash.
// Counts how many times this exact hash has been logged in the last 24 hours.
// Different physical devices produce different hashes — each gets its own gate:
//   First scan of this device today  → +3 (advances band gate)
//   Second scan of this device today → +1
//   Third+ scan of this device today → +0
int calculate_signal_gain(Codex* codex, const char* hash) {
    int hash_count = 0;
    uint32_t now = furi_hal_rtc_get_timestamp(); // Survives reboots — real wall-clock time
    uint32_t seconds_per_day = 24u * 60u * 60u;

    for(int i = 0; i < MAX_SIGNALS; i++) {
        if(codex->signal_history[i].hash[0] == '\0') continue;
        if(strcmp(codex->signal_history[i].hash, hash) != 0) continue;
        uint32_t age = now - (uint32_t)codex->signal_history[i].timestamp;
        if(age < seconds_per_day) hash_count++;
    }

    if(hash_count == 0) return 3;
    if(hash_count == 1) return 1;
    return 0;
}

// Handles RFID scans — routes through the same gain gate as all other bands.
// tag_id is used directly as the hash (125kHz RFID UIDs are persistent and unique).
void on_rfid_scan(Codex* codex, const char* tag_id) {
    int gain = calculate_signal_gain(codex, tag_id);
    log_signal(codex, tag_id, gain, SIGNAL_RFID);
    dolphin_deed(DolphinDeedRfidRead);
}

// Handles raw RF scans — hash is RSSI-derived (ephemeral by design; RF has no UID).
void on_rf_scan(Codex* codex, const char* signal_hash) {
    int gain = calculate_signal_gain(codex, signal_hash);
    log_signal(codex, signal_hash, gain, SIGNAL_RF);
    dolphin_deed(DolphinDeedSubGhzFrequencyAnalyzer);
}

// Handles IR scans — hash is "PROTO:ADDR:CMD" or "RAW:count:checksum" for unknown protocols.
void on_ir_scan(Codex* codex, const char* signal_hash) {
    int gain = calculate_signal_gain(codex, signal_hash);
    log_signal(codex, signal_hash, gain, SIGNAL_IR);
    dolphin_deed(DolphinDeedIrLearnSuccess);
}

// Handles Sub-GHz scans — hash is "PROTO:HASH_BYTE" from the decoded protocol frame.
void on_subghz_scan(Codex* codex, const char* signal_hash) {
    int gain = calculate_signal_gain(codex, signal_hash);
    log_signal(codex, signal_hash, gain, SIGNAL_SUBGHZ);
    dolphin_deed(DolphinDeedSubGhzReceiverInfo);
}

// Handles NFC scans — tag_id is the UID (persistent and unique per card).
void on_nfc_scan(Codex* codex, const char* tag_id) {
    int gain = calculate_signal_gain(codex, tag_id);
    log_signal(codex, tag_id, gain, SIGNAL_NFC);
    dolphin_deed(DolphinDeedNfcRead);
}

// Handles BT scans — hash is RSSI-derived + tick (ephemeral; passive listening has no UID).
// The Unanswered Hello: something was broadcasting. We heard it.
void on_bt_scan(Codex* codex, const char* signal_hash) {
    int gain = calculate_signal_gain(codex, signal_hash);
    log_signal(codex, signal_hash, gain, SIGNAL_BLUETOOTH);
    dolphin_deed(DolphinDeedPluginGameWin);
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
// Generates a simulated hash, calculates gain, and logs the signal.
int scan_band(Codex* codex, SignalType band_type) {
    char hash[16];
    snprintf(hash, sizeof(hash), "SIG%04X", (unsigned)(rand() % 65536));
    int gain = calculate_signal_gain(codex, hash);
    log_signal(codex, hash, gain, band_type);
    return gain;
}

int enter_manual_signal(Codex* codex, const char* signal_hash) {
    // Manual entries: same type-per-day gate, but capped lower
    // Auto: 3/1/0 → Manual: 1/0/0
    int gain = calculate_signal_gain(codex, signal_hash);
    if(gain >= 3) gain = 1;
    else gain = 0;

    log_signal(codex, signal_hash, gain, SIGNAL_UNKNOWN);
    printf("[Manual] Entered signal: %s → gain: %d\n", signal_hash, gain);
    return gain;
}
