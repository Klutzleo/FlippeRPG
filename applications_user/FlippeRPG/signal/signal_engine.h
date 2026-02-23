#ifndef FLIPPERPG_SIGNAL_ENGINE_H
#define FLIPPERPG_SIGNAL_ENGINE_H

#pragma once
#include "../core/constants.h"
#include "../codex/codex.h"

void start_signal_loop(Codex* codex);
char* hash_signal(const char* raw_data);
int calculate_signal_gain(Codex* codex, SignalType type);
SignalType get_signal_type(Codex* codex, const char* signal_hash);
int enter_manual_signal(Codex* codex, const char* signal_hash);

#endif