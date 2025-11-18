#include "techniques.h"
#include <subghz_tx.h>
#include "../codex/codex.h"
#include "../core/utils.h"

void pulse_open(Codex* codex) {
    if (!codex_has_technique(codex, "Pulse Open")) {
        popup_message_str("You have not unlocked Pulse Open.");
        return;
    }

    SubGhzTxConfig config = {
        .frequency = 433920000,
        .protocol = SubGhzProtocolCustom,
        .data = NULL,
        .data_size = 0,
    };
    subghz_tx_start(&config);
    popup_message_str("Pulse Open cast. Signal sent.");
}