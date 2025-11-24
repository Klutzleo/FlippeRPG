#include "subghz_tx.h"
#include <stdio.h>

bool subghz_tx_start(const SubGhzTxConfig* config) {
    // Stub implementation for now
    printf("[SubGHz] TX start: freq=%u Hz, protocol=%d, size=%u\n",
           config->frequency, config->protocol, config->data_size);
    return true;
}