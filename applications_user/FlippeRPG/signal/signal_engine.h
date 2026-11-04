#ifndef FLIPPERPG_SIGNAL_ENGINE_H
#define FLIPPERPG_SIGNAL_ENGINE_H

#pragma once
#include "../core/constants.h"
#include "../codex/codex.h"

char* hash_signal(const char* raw_data);
// Outcome of weighing one scan against today's history.
typedef struct {
    bool counted; // Advances the band gate and is logged
    int  gain;    // Signal awarded (+3 / +1 / +0); always 0 when not counted
} ScanResult;

// Decides whether a scan counts and what it earns. Does not change the codex.
//  - IR, Sub-GHz, NFC, RFID have a source ID: each source counts once per day.
//  - Bluetooth has none: at most SCANS_PER_BAND a day, SOURCELESS_MIN_GAP_SECONDS apart.
//  - Gain is per type per day: first counted scan +3, second +1, third and later +0.
ScanResult evaluate_scan(Codex* codex, SignalType type, const char* hash);

SignalType get_signal_type(Codex* codex, const char* signal_hash);
int enter_manual_signal(Codex* codex, const char* signal_hash);

// Each on_*_scan evaluates the scan, logs it if it counted, and returns the result.
ScanResult on_rfid_scan(Codex* codex, const char* tag_id);
ScanResult on_ir_scan(Codex* codex, const char* signal_hash);
ScanResult on_subghz_scan(Codex* codex, const char* signal_hash);
ScanResult on_nfc_scan(Codex* codex, const char* tag_id);
ScanResult on_bt_scan(Codex* codex, const char* signal_hash);

// Scan a specific band type — used by the band-select signal view.
// Generates a hash, logs the signal if it counted, and returns the gain awarded.
int scan_band(Codex* codex, SignalType band_type);

#endif