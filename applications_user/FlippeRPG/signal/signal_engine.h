#ifndef FLIPPERPG_SIGNAL_ENGINE_H
#define FLIPPERPG_SIGNAL_ENGINE_H

#pragma once
#include "../core/constants.h"
#include "../codex/codex.h"

char* hash_signal(const char* raw_data);
int calculate_signal_gain(Codex* codex, SignalType type);
SignalType get_signal_type(Codex* codex, const char* signal_hash);
int enter_manual_signal(Codex* codex, const char* signal_hash);
void on_rfid_scan(Codex* codex, const char* tag_id);
void on_rf_scan(Codex* codex, const char* signal_hash);
void on_ir_scan(Codex* codex, const char* signal_hash);
void on_subghz_scan(Codex* codex, const char* signal_hash);
void on_nfc_scan(Codex* codex, const char* tag_id);
void on_bt_scan(Codex* codex, const char* signal_hash);

// Scan a specific band type — used by the band-select signal view.
// Generates a hash, calculates gain, logs the signal, and returns the gain awarded.
int scan_band(Codex* codex, SignalType band_type);

#endif