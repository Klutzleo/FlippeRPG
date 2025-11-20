// signal_share.h
#pragma once
#include "../core/constants.h"
#include "../codex/codex.h"

void share_signal(Codex* codex, SignalType type, const char* signal_hash);
void receive_signal(Codex* codex, SignalType type, const char* signal_hash);